#include <algorithm>
#include <memory>
#include <string_view>
#include <unordered_map>

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/definitions.hpp"
#include "compiler/optree/helpers.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/optree/types.hpp"
#include "compiler/utils/helpers.hpp"

#include "optimizer/opt_builder.hpp"
#include "optimizer/transform.hpp"

using namespace optree;
using namespace optree::optimizer;

namespace {

struct ConstantPropagationContext {

    ConstantPropagationContext(OptBuilder &builder) : builder{builder} {};
    using ScopesStack = std::deque<std::unordered_map<Value::Ptr, Value::Ptr>>;

    void setValueAttribute(const StoreOp &op) {
        auto storeValue = op.valueToStore();
        auto valueOwner = storeValue->owner.lock();

        if (valueOwner->as<ConstantOp>()) {
            scopes.front()[op.dst()] = storeValue;
        } else {
            return;
        }
    }

    void replaceValue(const LoadOp &op) {
        auto scr = op.src();
        Value::Ptr value;
        for (const auto &scope : scopes) {
            if (const auto it = scope.find(scr); it != scope.end()) {
                value = it->second;
                break;
            } else {
                return;
            }
        }
        auto &result = op->result(0);
        for (auto &use : result->uses) {
            auto user = use.lock();

            builder.update(user, [&user, &value, &operandNumber = use.operandNumber]() {
                auto &targetOperand = user->operand(operandNumber);
                targetOperand = value;
            });
        }
    }

    void traverseOps(const Operation::Ptr &op) {
        if (utils::isAny<FunctionOp, IfOp, ThenOp, ElseOp, ForOp, WhileOp>(op)) {
            scopes.emplace_front();

            for (const auto &child : op->body) {
                if (auto storeOp = child->as<StoreOp>()) {
                    setValueAttribute(storeOp);
                    continue;
                }
                if (auto loadOp = child->as<LoadOp>()) {
                    replaceValue(loadOp);
                }
                traverseOps(child);
            }
            scopes.pop_front();
        }
    }

    ScopesStack scopes;
    OptBuilder &builder;
};

struct ConstantPropagation : public Transform<FunctionOp> {
    using Transform::Transform;

    std::string_view name() const override {
        return "ConstantPropagation";
    }

    // using ExpressionPair = std::pair<Operation::Ptr, std::vector<Value::Ptr>>;

    void run(const Operation::Ptr &op, OptBuilder &builder) const override {
        auto propagationContext = ConstantPropagationContext{builder};
        propagationContext.traverseOps(op);
    }
};

} // namespace

namespace optree {
namespace optimizer {

BaseTransform::Ptr createConstantPropagation() {
    return std::make_shared<ConstantPropagation>();
}

} // namespace optimizer
} // namespace optree
