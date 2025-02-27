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

void OptBuilder::insert(const Operation::Ptr &op) {
    COMPILER_DEBUG(dbg::get() << "  Insert " << op->name << '\n');
    Builder::insert(op);
    notifier.onInsert(op);
}

Operation::Ptr OptBuilder::clone(const Operation::Ptr &op) {
    COMPILER_DEBUG(dbg::get() << "  Clone " << op->name << '\n');
    auto newOp = op->clone();
    notifyInsertRecursively(newOp, notifier);
    insert(newOp);
    return newOp;
}

void OptBuilder::erase(const Operation::Ptr &op) {
    if (op->parent)
        setInsertPointAfter(op);
    while (!op->body.empty()) {
        erase(op->body.back());
    }
    COMPILER_DEBUG(dbg::get() << "  Erase " << op->name << '\n');
    notifier.onErase(op);
    op->erase();
}

void OptBuilder::update(const Operation::Ptr &op, const std::function<void()> &actor) {
    COMPILER_DEBUG(dbg::get() << "  Update " << op->name << '\n');
    if (actor)
        actor();
    notifier.onUpdate(op);
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
