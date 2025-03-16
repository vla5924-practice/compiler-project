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

namespace {

struct BooleanExpressionMinimization : public Transform<LogicBinaryOp> {
    using Transform::Transform;

    std::string_view name() const override {
        return "BooleanExpressionMinimization";
    }

    // checks pattern x op x
    static bool checkIdempotence(const LogicBinaryOp &logicOp) {
        auto lhs = logicOp.lhs()->owner.lock();
        auto rhs = logicOp.rhs()->owner.lock();
        return similar(lhs, rhs);
    }

    // checks pattern x op ~x
    static bool checkComplementation(const LogicBinaryOp &logicOp) {
        auto lhs = getValueOwnerAs<UnaryOp>(logicOp.lhs());
        bool result = false; 
        if (lhs) {
            result = similar(lhs->operand(0)->owner.lock(), logicOp.rhs()->owner.lock());
        }
        auto rhs = getValueOwnerAs<UnaryOp>(logicOp.rhs());
        if (rhs && !result) {
            result = similar(rhs->operand(0)->owner.lock(), logicOp.lhs()->owner.lock());
        }
        return result;
    }

    static bool proccesOperand(const ConstantOp &constOp, const Operation::Ptr &secondOp, bool annihilatorValue,
                               OptBuilder &builder) {
        auto valueType = constOp->result(0)->type;
        auto &parent = constOp->parent;
        auto replaceFunc = [&parent, &secondOp, &builder, annihilatorValue]<typename T>(T value) {
            bool opSwitch = annihilatorValue ? !value : value;
            if (opSwitch) {
                builder.replace(parent, secondOp);
            } else {
                auto newOp = builder.insert<ConstantOp>(parent->ref, TypeStorage::boolType(), annihilatorValue);
                builder.replace(parent, newOp);
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
            processed = proccesOperand(constLhsOp, logicOp.rhs()->owner.lock(), annihilatorValue, builder);
        }
        auto constRhsOp = getValueOwnerAs<ConstantOp>(logicOp.rhs());
        if (constRhsOp && !processed) {
            proccesOperand(constRhsOp, logicOp.lhs()->owner.lock(), annihilatorValue, builder);
        }
    }

    static void proccesAnd(const LogicBinaryOp &logicOp, OptBuilder &builder) {
        if (checkIdempotence(logicOp)) {
            builder.replace(logicOp, logicOp.lhs()->owner.lock());
        } 
        if (checkComplementation(logicOp)) { 
            auto newOp = builder.insert<ConstantOp>(logicOp->ref, TypeStorage::boolType(), false);
            builder.replace(logicOp, newOp);
        }
        proccesIdentityAndAnnihilatorRules(logicOp, false, builder);
    }

    static void proccesOr(const LogicBinaryOp &logicOp, OptBuilder &builder) {
        if (checkIdempotence(logicOp)) {
            builder.replace(logicOp, logicOp.lhs()->owner.lock());
            return;
        }
        if (checkComplementation(logicOp)) { 
            auto newOp = builder.insert<ConstantOp>(logicOp->ref, TypeStorage::boolType(), true);
            builder.replace(logicOp, newOp);
        }
        proccesIdentityAndAnnihilatorRules(logicOp, true, builder);
    }

    static void proccesEqual(const LogicBinaryOp &logicOp, OptBuilder &builder) {
        if (checkIdempotence(logicOp)) {
            auto newOp = builder.insert<ConstantOp>(logicOp->ref, TypeStorage::boolType(), true);
            builder.replace(logicOp, newOp);
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
