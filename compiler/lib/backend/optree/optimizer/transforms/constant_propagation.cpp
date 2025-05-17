#include "optimizer/transform.hpp"

#include <deque>
#include <memory>
#include <string_view>
#include <unordered_map>

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/optree/value.hpp"
#include "compiler/utils/helpers.hpp"

#include "optimizer/opt_builder.hpp"

using namespace optree;
using namespace optree::optimizer;

namespace {

struct ConstantPropagationContext {

    ConstantPropagationContext(OptBuilder &builder) : builder{builder} {};
    using Scopes = std::deque<std::unordered_map<Value::Ptr, Value::Ptr>>;

    void setValueAttribute(const StoreOp &op) {
        auto storeValue = op.valueToStore();
        auto valueOwner = storeValue->owner.lock();
        auto dst = op.dst();
        if (valueOwner->as<ConstantOp>())
            scopes.front()[dst] = storeValue;
        auto beginIt = scopes.begin();
        beginIt++;
        for (; beginIt != scopes.end(); beginIt++) {
            auto &scope = *beginIt;
            if (scope.count(dst) != 0) {
                scope[dst] = nullptr;
            }
        }
    }

    void replaceValue(const LoadOp &op) {
        auto scr = op.src();
        Value::Ptr value = nullptr;
        auto tmp_scope = scopes.front();
        for (const auto &scope : scopes) {
            if (const auto it = scope.find(scr); it != scope.end()) {
                value = it->second;
                break;
            }
        }
        if (!value) {
            return;
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

    Scopes scopes;
    OptBuilder &builder;
};

struct ConstantPropagation : public Transform<FunctionOp> {
    using Transform::Transform;

    std::string_view name() const override {
        return "ConstantPropagation";
    }

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
