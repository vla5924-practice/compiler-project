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

void runTransform(const BaseTransform::Ptr &transform, const Operation::Ptr &op) {
    for (const auto &childOp : utils::advanceEarly(op->body)) {
        runTransform(transform, childOp);
        if (transform->canRun(childOp)) {
            OptBuilder builder;
            builder.setInsertPointBefore(childOp);
            COMPILER_DEBUG(dbg::get() << "Run " << transform->name() << " on " << op->dump() << "{\n");
            transform->run(childOp, builder);
            COMPILER_DEBUG(dbg::get() << "}\n\n");
        }
    }
}

} // namespace

Optimizer &Optimizer::add(const BaseTransform::Ptr &transform) {
    transforms.emplace_back(transform);
    return *this;
}

void Optimizer::process(const Operation::Ptr &op) const {
    for (const auto &transform : transforms)
        runTransform(transform, op);
}

void Optimizer::process(Program &program) const {
    process(program.root);
}
