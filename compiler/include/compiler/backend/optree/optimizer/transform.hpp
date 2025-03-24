#pragma once

#include <cstddef>
#include <deque>
#include <memory>
#include <string_view>

#include "compiler/optree/operation.hpp"

#include "compiler/backend/optree/optimizer/opt_builder.hpp"

namespace optree {
namespace optimizer {

struct BaseTransform {
    using Ptr = std::shared_ptr<BaseTransform>;

    BaseTransform() = default;
    BaseTransform(const BaseTransform &) = default;
    BaseTransform(BaseTransform &&) = default;
    virtual ~BaseTransform() = default;

    virtual std::string_view name() const = 0;
    virtual bool canRun(const Operation::Ptr &op) const = 0;
    virtual void run(const Operation::Ptr &op, OptBuilder &builder) const = 0;
    virtual bool recurse() const;
};

template <typename... AdaptorTypes>
struct Transform : public BaseTransform {
    Transform() = default;
    Transform(const Transform &) = default;
    Transform(Transform &&) = default;
    ~Transform() override = default;

    bool canRun(const Operation::Ptr &op) const final {
        if constexpr (sizeof...(AdaptorTypes) == 0)
            return true;
        else
            return (op->is<AdaptorTypes>() || ...);
    }
};

class CascadeTransform : public BaseTransform {
    std::deque<BaseTransform::Ptr> transforms;
    std::string_view commonName;
    size_t iterLimit;

    CascadeTransform(std::string_view commonName, size_t iterLimit);
    CascadeTransform(const CascadeTransform &) = delete;
    CascadeTransform(CascadeTransform &&) = default;

  public:
    using Ptr = std::shared_ptr<CascadeTransform>;

    ~CascadeTransform() override = default;

    std::string_view name() const override;
    bool canRun(const Operation::Ptr &op) const override;
    void run(const Operation::Ptr &op, OptBuilder &builder) const override;
    bool recurse() const override;

    CascadeTransform &add(const BaseTransform::Ptr &transform);

    static Ptr make(std::string_view commonName, size_t iterLimit = 100U);
};

} // namespace optimizer
} // namespace optree
