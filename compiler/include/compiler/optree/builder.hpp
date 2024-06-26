#pragma once

#include "compiler/utils/source_ref.hpp"

#include "compiler/optree/operation.hpp"

namespace optree {

class Builder {
    using InsertPoint = Operation::Body::iterator;

    Operation::Ptr currentOp;
    InsertPoint insertPoint;

    Builder(const Operation::Ptr &currentOp, const InsertPoint &insertPoint)
        : currentOp(currentOp), insertPoint(insertPoint){};

  public:
    Builder() = default;
    Builder(const Builder &) = default;
    Builder(Builder &&) = default;
    virtual ~Builder() = default;

    static Builder before(const Operation::Ptr &op);
    static Builder after(const Operation::Ptr &op);
    static Builder atBodyBegin(const Operation::Ptr &op);
    static Builder atBodyEnd(const Operation::Ptr &op);

    void setInsertPointBefore(const Operation::Ptr &op);
    void setInsertPointAfter(const Operation::Ptr &op);
    void setInsertPointAtBodyBegin(const Operation::Ptr &op);
    void setInsertPointAtBodyEnd(const Operation::Ptr &op);

    virtual void insert(const Operation::Ptr &op);

    template <typename AdaptorType, typename... Args>
    AdaptorType insert(Args... args) {
        auto adaptor = Operation::make<AdaptorType>(currentOp);
        insert(adaptor.op);
        adaptor.init(std::forward<Args>(args)...);
        return adaptor;
    }

    template <typename AdaptorType, typename... Args>
    AdaptorType insert(const utils::SourceRef &ref, Args... args) {
        auto adaptor = insert<AdaptorType>(std::forward<Args>(args)...);
        adaptor.op->ref = ref;
        return adaptor;
    }
};

} // namespace optree
