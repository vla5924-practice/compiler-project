#include "optimizer/opt_builder.hpp"

#include <algorithm>
#include <functional>

#include "compiler/optree/builder.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/utils/helpers.hpp"

using namespace optree;
using namespace optree::optimizer;

namespace {

void notifyInsertRecursively(const Operation::Ptr &op, const OptBuilder::Notifier &notifier) {
    for (const auto &nestedOp : op->body) {
        notifyInsertRecursively(nestedOp, notifier);
        notifier.onInsert(nestedOp);
    }
}

} // namespace

void OptBuilder::insert(const Operation::Ptr &op) {
    Builder::insert(op);
    notifier.onInsert(op);
}

Operation::Ptr OptBuilder::clone(const Operation::Ptr &op) {
    auto newOp = op->clone();
    notifyInsertRecursively(op, notifier);
    insert(op);
    return op;
}

void OptBuilder::erase(const Operation::Ptr &op) {
    if (op->parent)
        setInsertPointAfter(op);
    for (auto &op : utils::reversed(op->body))
        erase(op);
    op->erase();
    notifier.onErase(op);
}

void OptBuilder::update(const Operation::Ptr &op, const std::function<void()> &actor) {
    actor();
    notifier.onUpdate(op);
}

void OptBuilder::replace(const Operation::Ptr &op, const Operation::Ptr &newOp) {
    for (const auto &[oldResult, newResult] : utils::zip(op->results, newOp->results)) {
        for (const auto &use : oldResult->uses) {
            auto user = use.lock();
            update(user, [&] { user->operand(use.operandNumber) = newResult; });
        }
        newResult->uses.splice_after(newResult->uses.before_begin(), oldResult->uses);
    }
    erase(op);
}
