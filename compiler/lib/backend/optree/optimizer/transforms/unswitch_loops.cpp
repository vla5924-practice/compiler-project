#include "optimizer/transform.hpp"

#include <memory>
#include <string_view>
#include <unordered_set>

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/optree/value.hpp"
#include "compiler/utils/helpers.hpp"

#include "optimizer/opt_builder.hpp"

using namespace optree;
using namespace optree::optimizer;

namespace {

struct UnswitchLoops : public Transform<WhileOp, ForOp> {
    using Transform::Transform;

    std::string_view name() const override {
        return "UnswitchLoops";
    }

    using LoopValues = std::unordered_set<Value::Ptr>;

    static bool isInvariant(const Operation::Ptr &op, const LoopValues &values) {
        bool result = false;
        for (const auto &operand : op->operands) {
            result |= values.contains(operand);
        }
        return !result;
    }

    static void hoistBody(const Operation::Ptr &op, OptBuilder &builder) {
        if (!op)
            return;
        builder.setInsertPointBefore(op->parent);
        for (const auto &childOp : utils::advanceEarly(op->body)) {
            auto cloned = builder.clone(childOp);
            builder.replace(childOp, cloned);
            builder.setInsertPointAfter(cloned);
        }
    }

    void run(const Operation::Ptr &op, OptBuilder &builder) const override {
        LoopValues values;
        for (const auto &childOp : op->body) {
            for (const auto &result : childOp->results) {
                values.insert(result);
            }
            if (childOp->as<StoreOp>()) {
                for (const auto &operand : childOp->operands) {
                    values.insert(operand);
                }
            }
        }

        auto invariant_if_it = op->body.end();

        for (auto childIt = op->body.begin(); childIt != op->body.end(); ++childIt) {

            if ((*childIt)->as<IfOp>() && isInvariant(*childIt, values)) {
                invariant_if_it = childIt;
                break;
            }
        }

        if (invariant_if_it == op->body.end()) {
            return;
        }

        auto cloned_if = builder.clone(*invariant_if_it);
        auto cloned_loop_if = builder.clone(op);
        for (auto& child : cloned_loop_if->body) {
            if (child == *invariant_if_it) {
                hoistBody(child->as<IfOp>().thenOp(), builder);
                builder.erase(child);
                break;
            }
        }

        auto cloned_loop_else = builder.clone(op);
        for (auto& child : cloned_loop_else->body) {
            if (child == *invariant_if_it) {
                hoistBody(child->as<IfOp>().elseOp(), builder);
                builder.erase(child);
                break;
            }
        }
        builder.update(cloned_if, [&] { 
            cloned_if->as<IfOp>().thenOp()->body.clear();
            cloned_if->as<IfOp>().thenOp()->addToBody(cloned_loop_if);
            cloned_if->as<IfOp>().elseOp()->body.clear();
            cloned_if->as<IfOp>().elseOp()->addToBody(cloned_loop_else); });

        builder.replace(op, cloned_if);
    }
};

} // namespace

namespace optree {
namespace optimizer {

BaseTransform::Ptr createUnswitchLoops() {
    return std::make_shared<UnswitchLoops>();
}

} // namespace optimizer
} // namespace optree
