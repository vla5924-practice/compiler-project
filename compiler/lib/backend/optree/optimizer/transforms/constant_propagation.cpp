#include "optimizer/transform.hpp"

#include <deque>
#include <memory>
#include <set>
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
    using Scope = std::unordered_map<Value::Ptr, Value::Ptr>;
    using Scopes = std::deque<Scope>;

    void setValueAttribute(const StoreOp &op, bool invalidateDeps = true) {
        auto storeValue = op.valueToStore();
        auto valueOwner = storeValue->owner.lock();
        auto dst = op.dst();
        if (valueOwner->as<ConstantOp>()) {
            scopes.front()[dst] = storeValue;
        } else {
            scopes.front()[dst] = nullptr;
        }

        if (invalidateDeps) {
            auto beginIt = scopes.begin();
            beginIt++;
            for (; beginIt != scopes.end(); beginIt++) {
                auto &scope = *beginIt;
                if (scope.contains(dst)) {
                    scope[dst] = nullptr;
                }
            }
        }
    }

    void replaceValue(const LoadOp &op) {
        auto scr = op.src();
        Value::Ptr value = nullptr;
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

    std::set<Value::Ptr> getStoresForBlock(const Operation::Ptr &op) {
        std::set<Value::Ptr> stores;
        for (const auto &child : op->body) {
            if (auto storeOp = child->as<StoreOp>()) {
                stores.emplace(storeOp.dst());
            }
            stores.merge(getStoresForBlock(child));
        }
        return stores;
    }

    void iterateThrowChildrens(const Operation::Ptr &op, bool invalidateDeps = true) {
        scopes.emplace_front();
        for (const auto &child : op->body) {
            if (auto storeOp = child->as<StoreOp>()) {
                setValueAttribute(storeOp, invalidateDeps);
                continue;
            }
            if (auto loadOp = child->as<LoadOp>()) {
                replaceValue(loadOp);
            }
            traverseOps(child);
        }
        scopes.pop_front();
    }

    void traverseOps(const Operation::Ptr &op) {
        if (op->is<IfOp>() && op->numChildren() == 2) {
            auto ifOp = op->as<IfOp>();
            auto thenOp = ifOp.thenOp();
            auto elseOp = ifOp.elseOp();
            auto mergedValues = getStoresForBlock(thenOp);
            mergedValues.merge(getStoresForBlock(elseOp));
            iterateThrowChildrens(thenOp, false);
            iterateThrowChildrens(elseOp, false);
            for (auto &scope : scopes)
                for (const auto &value : mergedValues)
                    if (auto it = scope.find(value); it != scope.end()) {
                        it->second = nullptr;
                    }
        } else if (utils::isAny<ForOp, WhileOp>(op)) {
            auto values = getStoresForBlock(op);
            for (auto &scope : scopes)
                for (const auto &value : values)
                    if (auto it = scope.find(value); it != scope.end()) {
                        it->second = nullptr;
                    }
            iterateThrowChildrens(op, false);
        } else if (utils::isAny<FunctionOp, IfOp, ThenOp, ConditionOp>(op)) {
            iterateThrowChildrens(op);
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
