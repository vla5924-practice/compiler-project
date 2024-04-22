#include "optimizer/opt_builder.hpp"

#include <functional>

#include "compiler/optree/builder.hpp"
#include "compiler/optree/operation.hpp"

using namespace optree;
using namespace optree::optimizer;

void OptBuilder::insert(Operation *op) {
    Builder::insert(op);
    notifier.onInsert(op);
}

Operation *OptBuilder::clone(Operation *op) {
    // TODO
    notifier.onInsert(op);
    return op;
}

void OptBuilder::erase(Operation *op) {
    if (op->parent)
        setInsertPointAfter(op);
    for (const auto &nestedOp : op->body)
        erase(nestedOp);
    op->erase();
    notifier.onErase(op);
}

void OptBuilder::update(Operation *op, const std::function<void()> &actor) {
    actor();
    notifier.onUpdate(op);
}
