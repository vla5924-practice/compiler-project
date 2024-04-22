#pragma once

#include "compiler/utils/source_ref.hpp"

#include "compiler/optree/operation.hpp"

namespace optree {

class Builder {
    using InsertPoint = Operation::Body::iterator;

    Operation *currentOp;
    InsertPoint insertPoint;

    Builder(Operation *currentOp, const InsertPoint &insertPoint) : currentOp(currentOp), insertPoint(insertPoint){};

  public:
    Builder() = default;
    Builder(const Builder &) = default;
    Builder(Builder &&) = default;
    virtual ~Builder() = default;

    static Builder before(Operation *op);
    static Builder after(Operation *op);
    static Builder atBodyBegin(Operation *op);
    static Builder atBodyEnd(Operation *op);

    void setInsertPointBefore(Operation *op);
    void setInsertPointAfter(Operation *op);
    void setInsertPointAtBodyBegin(Operation *op);
    void setInsertPointAtBodyEnd(Operation *op);

    virtual void insert(Operation *op);

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
