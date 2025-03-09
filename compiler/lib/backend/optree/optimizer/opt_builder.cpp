#include "optimizer/opt_builder.hpp"

#include <functional>

#include "compiler/optree/builder.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/utils/debug.hpp"
#include "compiler/utils/helpers.hpp"

using namespace optree;
using namespace optree::optimizer;

using dbg = utils::DebugPrinter;

namespace {

void notifyInsertRecursively(const Operation::Ptr &op, const OptBuilder::Notifier &notifier) {
    for (const auto &nestedOp : op->body) {
        notifyInsertRecursively(nestedOp, notifier);
        notifier.onInsert(nestedOp);
    }
}

} // namespace

OptBuilder::Notifier::Notifier(const Callback &onInsert, const Callback &onUpdate, const Callback &onErase)
    : onInsert(onInsert), onUpdate(onUpdate), onErase(onErase) {
}

void OptBuilder::Notifier::notifyInsert(const Operation::Ptr &op) const {
    if (onInsert)
        onInsert(op);
}

void OptBuilder::Notifier::notifyUpdate(const Operation::Ptr &op) const {
    if (onUpdate)
        onUpdate(op);
}

void OptBuilder::Notifier::notifyErase(const Operation::Ptr &op) const {
    if (onErase)
        onErase(op);
}

void OptBuilder::insert(const Operation::Ptr &op) {
    COMPILER_DEBUG(dbg::get() << "  Insert " << op->name << '\n');
    Builder::insert(op);
    notifier.notifyInsert(op);
}

Operation::Ptr OptBuilder::clone(const Operation::Ptr &op) {
    COMPILER_DEBUG(dbg::get() << "  Clone " << op->name << "{\n");
    auto newOp = op->clone();
    notifyInsertRecursively(newOp, notifier);
    insert(newOp);
    COMPILER_DEBUG(dbg::get() << "  }\n");
    return newOp;
}

void OptBuilder::erase(const Operation::Ptr &op) {
    if (op->parent)
        setInsertPointAfter(op);
    while (!op->body.empty()) {
        erase(op->body.back());
    }
    COMPILER_DEBUG(dbg::get() << "  Erase " << op->name << '\n');
    notifier.notifyErase(op);
    op->erase();
}

void OptBuilder::update(const Operation::Ptr &op, const std::function<void()> &actor) {
    COMPILER_DEBUG(dbg::get() << "  Update " << op->name << '\n');
    if (actor)
        actor();
    notifier.notifyUpdate(op);
}

void OptBuilder::replace(const Operation::Ptr &op, const Operation::Ptr &newOp) {
    COMPILER_DEBUG(dbg::get() << "  Replace " << op->name << '\n');
    for (const auto &[oldResult, newResult] : utils::zip(op->results, newOp->results)) {
        for (const auto &use : oldResult->uses) {
            auto user = use.lock();
            update(user, [&] { user->operand(use.operandNumber) = newResult; });
        }
        newResult->uses.splice_after(newResult->uses.before_begin(), oldResult->uses);
    }
    erase(op);
}
