#include "optimizer/optimizer.hpp"

#include "compiler/optree/operation.hpp"
#include "compiler/optree/program.hpp"
#include "compiler/utils/debug.hpp"
#include "compiler/utils/helpers.hpp"

#include "optimizer/opt_builder.hpp"
#include "optimizer/transform.hpp"

using namespace optree;
using namespace optree::optimizer;

using dbg = utils::DebugPrinter;

namespace {

void runTransform(const BaseTransform::Ptr &transform, const OptBuilder::Notifier &notifier, const Operation::Ptr &op) {
    bool canRun = transform->canRun(op);
    if (transform->recurse() || (!canRun && !transform->recurse())) {
        for (const auto &childOp : utils::advanceEarly(op->body)) {
            runTransform(transform, notifier, childOp);
        }
    }
    if (!canRun)
        return;
    OptBuilder builder(notifier);
    builder.setInsertPointBefore(op);
    COMPILER_DEBUG(dbg::get() << "Run " << transform->name() << " on " << op->dump() << "{\n");
    transform->run(op, builder);
    COMPILER_DEBUG(dbg::get() << "}\n\n");
}

} // namespace

Optimizer &Optimizer::add(const BaseTransform::Ptr &transform) {
    transforms.emplace_back(transform);
    return *this;
}

void Optimizer::process(const Operation::Ptr &op) const {
    OptBuilder::Notifier empty;
    for (const auto &transform : transforms)
        runTransform(transform, empty, op);
}

void Optimizer::process(Program &program) const {
    process(program.root);
}
