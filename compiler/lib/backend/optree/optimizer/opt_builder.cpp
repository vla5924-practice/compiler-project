#include "optimizer/opt_builder.hpp"

#include <functional>

#include "compiler/optree/builder.hpp"
#include "compiler/optree/operation.hpp"

using namespace optree;
using namespace optree::optimizer;

void OptBuilder::insert(const Operation::Ptr &op) {
    Builder::insert(op);
    notifier.onInsert(op);
}

Operation::Ptr OptBuilder::clone(const Operation::Ptr &op) {
    // TODO
    notifier.onInsert(op);
    return op;
}

void OptBuilder::erase(const Operation::Ptr &op) {
    if (op->parent)
        setInsertPointAfter(op);
    for (const auto &nestedOp : op->body)
        erase(nestedOp);
    op->erase();
    notifier.onErase(op);
}

void OptBuilder::update(const Operation::Ptr &op, const std::function<void()> &actor) {
    actor();
    notifier.onUpdate(op);
}
