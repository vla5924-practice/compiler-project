#include "optimizer/transform.hpp"

#include <memory>

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/helpers.hpp"
#include "compiler/optree/operation.hpp"

#include "optimizer/opt_builder.hpp"

using namespace optree;
using namespace optree::optimizer;

namespace {

struct FoldControlFlowOps : public Transform<IfOp, WhileOp> {
    using Transform::Transform;

    static void hoistBody(const Operation::Ptr &op, OptBuilder &builder) {
        if (!op)
            return;
        for (const auto &childOp : op->body)
            builder.replace(childOp, builder.clone(childOp));
    }

    static void processIfOp(const IfOp &op, OptBuilder &builder) {
        auto conditionOp = getValueOwnerAs<ConstantOp>(op.cond());
        if (!conditionOp)
            return;
        bool condition = conditionOp.value().as<bool>();
        if (condition) {
            hoistBody(op.thenOp(), builder);
        } else {
            hoistBody(op.elseOp(), builder);
        }
        builder.erase(op);
    }

    static void processWhileOp(const WhileOp &op, OptBuilder &builder) {
        auto conditionOp = getValueOwnerAs<ConstantOp>(op.conditionOp().terminator());
        if (!conditionOp)
            return;
        bool condition = conditionOp.value().as<bool>();
        if (!condition) {
            builder.erase(op);
        }
    }

    void run(const Operation::Ptr &op, OptBuilder &builder) const override {
        if (auto ifOp = op->as<IfOp>())
            processIfOp(ifOp, builder);
        else if (auto whileOp = op->as<WhileOp>())
            processWhileOp(whileOp, builder);
        builder.erase(op);
    }
};

} // namespace

namespace optree {
namespace optimizer {

BaseTransform::Ptr createFoldControlFlowOps() {
    return std::make_shared<FoldControlFlowOps>();
}

} // namespace optimizer
} // namespace optree
