#pragma once

#include "compiler/optree/operation.hpp"

namespace optree {

namespace converter {

struct ConverterContext {
    Operation::Ptr op;

    template <typename AdaptorType>
    Operation::Ptr addToBody() {
        auto newOp = Operation::make<AdaptorType>(op);
        op->addToBody(newOp);
        return newOp;
    }

    void goParent() {
        op = op->parent;
    }
};

} // namespace converter

} // namespace optree
