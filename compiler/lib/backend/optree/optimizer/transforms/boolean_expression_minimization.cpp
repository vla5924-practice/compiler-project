#include <string>
#include <string_view>

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/helpers.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/utils/helpers.hpp"
#include "compiler/utils/language.hpp"

#include "optimizer/opt_builder.hpp"
#include "optimizer/transform.hpp"

using namespace optree;
using namespace optree::optimizer;
#include <iostream>
namespace {

struct BooleanExpressionMinimization : public Transform<LogicBinaryOp> {
    using Transform::Transform;

    std::string_view name() const override {
        return "BooleanExpressionMinimization";
    }

    // checks pattern x op x
    static bool checkIdempotence(const LogicBinaryOp &logicOp) {
        return logicOp.lhs() == logicOp.rhs();
    }

    // checks pattern x op ~x
    static bool checkComplementation(const LogicBinaryOp &logicOp) {
        auto lhs = getValueOwnerAs<LogicUnaryOp>(logicOp.lhs());
        bool result = false;
        if (lhs) {
            result = lhs->operand(0) == logicOp.rhs();
        }
        if (!result) {
            auto rhs = getValueOwnerAs<LogicUnaryOp>(logicOp.rhs());
            if (rhs && !result) {
                result = rhs->operand(0) == logicOp.lhs();
            }
        }
        return result;
    }

    static bool proccesOperand(const LogicBinaryOp &logicOp, const ConstantOp &constOp, const Value::Ptr &secondOp, bool annihilatorValue,
                               OptBuilder &builder) {
        auto valueType = constOp->result(0)->type;
        auto replaceFunc = [&logicOp, &secondOp, &builder, annihilatorValue]<typename T>(T value) {
            bool opSwitch = annihilatorValue ? !value : value;
            if (opSwitch) {
                builder.update(logicOp, 
                    [&logicOp, &secondOp, &builder](){
                        auto &oldUses = logicOp.result()->uses;
                        for (const auto &use : oldUses) {
                            auto user = use.lock();
                            builder.update(user, [&] { user->operand(use.operandNumber) = secondOp; });
                        }
                        secondOp->uses.splice_after(secondOp->uses.before_begin(), oldUses);
                    }
                );
                builder.erase(logicOp);
            } else {
                auto newOp = builder.insert<ConstantOp>(logicOp->ref, TypeStorage::boolType(), annihilatorValue);
                builder.replace(logicOp, newOp);
            }
        };
        if (valueType->is<BoolType>()) {
            auto value = constOp.value().as<NativeBool>();
            replaceFunc(value);
            return true;
        }
        if (valueType->is<IntegerType>()) {
            auto value = constOp.value().as<NativeInt>();
            replaceFunc(value);
            return true;
        }
        if (valueType->is<FloatType>()) {
            auto value = constOp.value().as<NativeFloat>();
            replaceFunc(value);
            return true;
        }
        return false;
    }

    // checks x op patterns, annihilatorValue(f.e. x and 0 = 0, where 0 is annihilator)
    static void proccesIdentityAndAnnihilatorRules(const LogicBinaryOp &logicOp, bool annihilatorValue,
                                                   OptBuilder &builder) {
        auto constLhsOp = getValueOwnerAs<ConstantOp>(logicOp.lhs());
        bool processed = false;
        if (constLhsOp) {
            processed = proccesOperand(logicOp, constLhsOp, logicOp.rhs(), annihilatorValue, builder);
        }
        if (!processed) {
            auto constRhsOp = getValueOwnerAs<ConstantOp>(logicOp.rhs());
            if (constRhsOp) {
                proccesOperand(logicOp, constRhsOp, logicOp.lhs(), annihilatorValue, builder);
            }
        }
    }

    static void proccesAnd(const LogicBinaryOp &logicOp, OptBuilder &builder) {
        if (checkIdempotence(logicOp)) {
            builder.update(logicOp, 
                [&logicOp, &builder](){
                    auto lhsResult = logicOp.lhs();
                    auto &oldUses = logicOp.result()->uses;
                    for (const auto &use : oldUses) {
                        auto user = use.lock();
                        builder.update(user, [&] { user->operand(use.operandNumber) = lhsResult; });
                    }
                    lhsResult->uses.splice_after(lhsResult->uses.before_begin(), oldUses);
                }
            );
            builder.erase(logicOp);
            return;
        }
        if (checkComplementation(logicOp)) {
            auto newOp = builder.insert<ConstantOp>(logicOp->ref, TypeStorage::boolType(), false);
            builder.replace(logicOp, newOp);
            return;
        }
        proccesIdentityAndAnnihilatorRules(logicOp, false, builder);
    }

    static void proccesOr(const LogicBinaryOp &logicOp, OptBuilder &builder) {
        if (checkIdempotence(logicOp)) {
            builder.update(logicOp, 
                [&logicOp, &builder](){
                    auto &lhsUses = logicOp.lhs()->uses;
                    auto lhsResult = logicOp.lhs();
                    auto &oldUses = logicOp.result()->uses;
                    for (const auto &use : oldUses) {
                        auto user = use.lock();
                        builder.update(user, [&] { user->operand(use.operandNumber) = lhsResult; });
                    }
                    lhsResult->uses.splice_after(lhsResult->uses.before_begin(), oldUses);
                }
            );
            builder.erase(logicOp);
            return;
        }
        if (checkComplementation(logicOp)) {
            auto newOp = builder.insert<ConstantOp>(logicOp->ref, TypeStorage::boolType(), true);
            builder.replace(logicOp, newOp);
            return;
        }
        proccesIdentityAndAnnihilatorRules(logicOp, true, builder);
    }

    static void proccesEqual(const LogicBinaryOp &logicOp, OptBuilder &builder) {
        if (checkIdempotence(logicOp)) {
            auto newOp = builder.insert<ConstantOp>(logicOp->ref, TypeStorage::boolType(), true);
            builder.replace(logicOp, newOp);
            return;
        }
        if (checkComplementation(logicOp)) {
            auto newOp = builder.insert<ConstantOp>(logicOp->ref, TypeStorage::boolType(), false);
            builder.replace(logicOp, newOp);
            return;
        }
    }

    static void proccesNotEqual(const LogicBinaryOp &logicOp, OptBuilder &builder) {
        if (checkIdempotence(logicOp)) {
            auto newOp = builder.insert<ConstantOp>(logicOp->ref, TypeStorage::boolType(), false);
            builder.replace(logicOp, newOp);
            return;
        }
        if (checkComplementation(logicOp)) {
            auto newOp = builder.insert<ConstantOp>(logicOp->ref, TypeStorage::boolType(), true);
            builder.replace(logicOp, newOp);
            return;
        }
    }

    void run(const Operation::Ptr &op, OptBuilder &builder) const override {
        auto logicOp = op->as<LogicBinaryOp>();
        if (logicOp.kind() == LogicBinOpKind::Equal) {
            proccesEqual(logicOp, builder);
        }
        if (logicOp.kind() == LogicBinOpKind::NotEqual) {
            proccesNotEqual(logicOp, builder);
        }
        if (logicOp.kind() == LogicBinOpKind::AndI) {
            proccesAnd(logicOp, builder);
        }
        if (logicOp.kind() == LogicBinOpKind::OrI) {
            proccesOr(logicOp, builder);
        }
    }
};

} // namespace

namespace optree {
namespace optimizer {

BaseTransform::Ptr createBooleanExpressionMinimization() {
    return std::make_shared<BooleanExpressionMinimization>();
}

} // namespace optimizer
} // namespace optree
