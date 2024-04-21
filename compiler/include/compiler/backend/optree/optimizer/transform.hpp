#pragma once

#include <memory>

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

    virtual bool canRun(const Operation::Ptr &op) const = 0;
    virtual void run(const Operation::Ptr &op, OptBuilder &builder) const = 0;
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

} // namespace optimizer
} // namespace optree
