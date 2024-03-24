#pragma once

#include "compiler/optree/operation.hpp"

namespace optree {

namespace converter {

struct ConverterContext {
    Operation::Ptr op;

    template <typename AdaptorType>
    std::pair<Operation::Ptr, AdaptorType> addToBody() {
        auto newOp = Operation::make<AdaptorType>(op);
        op->addToBody(newOp);
        AdaptorType adaptor(newOp);
        adaptor.init();
        return std::make_pair(newOp, adaptor);
    }

    void goParent() {
        op = op->parent;
    }
};

} // namespace converter

} // namespace optree
