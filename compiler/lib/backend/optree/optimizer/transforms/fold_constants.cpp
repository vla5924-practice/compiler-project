#include <memory>

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

struct FoldConstants : public Transform<ArithBinaryOp, ArithCastOp, LogicBinaryOp, LogicUnaryOp> {
    using Transform::Transform;

    static void foldArithBinaryOp(const ArithBinaryOp &op, OptBuilder &builder) {
        auto lhsOp = getValueOwnerAs<ConstantOp>(op.lhs());
        auto rhsOp = getValueOwnerAs<ConstantOp>(op.rhs());
        if (!lhsOp || !rhsOp)
            return;
        auto type = op.result()->type;
        if (type->is<IntegerType>()) {
            NativeInt folded;
            auto lhs = lhsOp.value().as<NativeInt>();
            auto rhs = rhsOp.value().as<NativeInt>();
            switch (op.kind()) {
            case ArithBinOpKind::AddI:
                folded = lhs + rhs;
                break;
            case ArithBinOpKind::SubI:
                folded = lhs - rhs;
                break;
            case ArithBinOpKind::MulI:
                folded = lhs * rhs;
                break;
            case ArithBinOpKind::DivI:
                folded = lhs / rhs;
                break;
            default:
                COMPILER_UNREACHABLE("Unexpected op kind");
            }
            auto newOp = builder.insert<ConstantOp>(op->ref, type, folded);
            builder.replace(op, newOp);
            return;
        } else if (type->is<FloatType>()) {
            NativeFloat folded;
            auto lhs = lhsOp.value().as<NativeFloat>();
            auto rhs = rhsOp.value().as<NativeFloat>();
            switch (op.kind()) {
            case ArithBinOpKind::AddF:
                folded = lhs + rhs;
                break;
            case ArithBinOpKind::SubF:
                folded = lhs - rhs;
                break;
            case ArithBinOpKind::MulF:
                folded = lhs * rhs;
                break;
            case ArithBinOpKind::DivF:
                folded = lhs / rhs;
                break;
            default:
                COMPILER_UNREACHABLE("Unexpected op kind");
            }
            auto newOp = builder.insert<ConstantOp>(op->ref, type, folded);
            builder.replace(op, newOp);
            return;
        }
    }

    static void foldArithCastOp(const ArithCastOp &op, OptBuilder &builder) {
        auto valueOp = getValueOwnerAs<ConstantOp>(op.value());
        if (!valueOp)
            return;
        auto valueType = op.value()->type;
        auto type = op.result()->type;
        if (type->is<IntegerType>()) {
            NativeInt folded;
            if (valueType->is<IntegerType>()) {
                auto value = valueOp.value().as<NativeInt>();
                switch (op.kind()) {
                case ArithCastOpKind::ExtI:
                case ArithCastOpKind::TruncI:
                    folded = value;
                    break;
                default:
                    COMPILER_UNREACHABLE("Unexpected op kind");
                }
            } else if (valueType->is<FloatType>()) {
                auto value = valueOp.value().as<NativeFloat>();
                switch (op.kind()) {
                case ArithCastOpKind::FloatToInt:
                    folded = static_cast<NativeInt>(value);
                    break;
                default:
                    COMPILER_UNREACHABLE("Unexpected op kind");
                }
            }
            auto newOp = builder.insert<ConstantOp>(op->ref, type, folded);
            builder.replace(op, newOp);
            return;
        } else if (type->is<FloatType>()) {
            NativeFloat folded;
            if (valueType->is<FloatType>()) {
                auto value = valueOp.value().as<NativeFloat>();
                switch (op.kind()) {
                case ArithCastOpKind::ExtF:
                case ArithCastOpKind::TruncF:
                    folded = value;
                    break;
                default:
                    COMPILER_UNREACHABLE("Unexpected op kind");
                }
            } else if (valueType->is<IntegerType>()) {
                auto value = valueOp.value().as<NativeInt>();
                switch (op.kind()) {
                case ArithCastOpKind::IntToFloat:
                    folded = static_cast<NativeFloat>(value);
                    break;
                default:
                    COMPILER_UNREACHABLE("Unexpected op kind");
                }
            }
            auto newOp = builder.insert<ConstantOp>(op->ref, type, folded);
            builder.replace(op, newOp);
            return;
        }
    }

    static void foldLogicBinaryOp(const LogicBinaryOp &op, OptBuilder &builder) {
        auto lhsOp = getValueOwnerAs<ConstantOp>(op.lhs());
        auto rhsOp = getValueOwnerAs<ConstantOp>(op.rhs());
        if (!lhsOp || !rhsOp)
            return;
        auto type = op.result()->type;
        auto valType = op.lhs()->type;
        NativeBool folded;
        if (valType->is<BoolType>()) {
            auto lhs = lhsOp.value().as<NativeBool>();
            auto rhs = rhsOp.value().as<NativeBool>();
            switch (op.kind()) {
            case LogicBinOpKind::AndI:
                folded = lhs && rhs;
                break;
            case LogicBinOpKind::OrI:
                folded = lhs || rhs;
                break;
            default:
                COMPILER_UNREACHABLE("Unexpected op kind");
            }
        } else if (valType->is<IntegerType>()) {
            auto lhs = lhsOp.value().as<NativeInt>();
            auto rhs = rhsOp.value().as<NativeInt>();
            switch (op.kind()) {
            case LogicBinOpKind::Equal:
                folded = lhs == rhs;
                break;
            case LogicBinOpKind::NotEqual:
                folded = lhs != rhs;
                break;
            case LogicBinOpKind::LessEqualI:
                folded = lhs <= rhs;
                break;
            case LogicBinOpKind::LessI:
                folded = lhs < rhs;
                break;
            case LogicBinOpKind::GreaterEqualI:
                folded = lhs >= rhs;
                break;
            case LogicBinOpKind::GreaterI:
                folded = lhs > rhs;
                break;
            default:
                COMPILER_UNREACHABLE("Unexpected op kind");
            }
        } else if (valType->is<FloatType>()) {
            auto lhs = lhsOp.value().as<NativeFloat>();
            auto rhs = rhsOp.value().as<NativeFloat>();
            switch (op.kind()) {
            case LogicBinOpKind::Equal:
                folded = lhs == rhs;
                break;
            case LogicBinOpKind::NotEqual:
                folded = lhs != rhs;
                break;
            case LogicBinOpKind::LessEqualF:
                folded = lhs <= rhs;
                break;
            case LogicBinOpKind::LessF:
                folded = lhs < rhs;
                break;
            case LogicBinOpKind::GreaterEqualF:
                folded = lhs >= rhs;
                break;
            case LogicBinOpKind::GreaterF:
                folded = lhs > rhs;
                break;
            default:
                COMPILER_UNREACHABLE("Unexpected op kind");
            }
        }
        auto newOp = builder.insert<ConstantOp>(op->ref, type, folded);
        builder.replace(op, newOp);
        return;
    }

    static void foldLogicUnaryOp(const LogicUnaryOp &op, OptBuilder &builder) {
        auto valueOp = getValueOwnerAs<ConstantOp>(op.value());
        if (!valueOp)
            return;
        auto type = op.result()->type;
        NativeBool folded;
        auto value = valueOp.value().as<NativeBool>();
        switch (op.kind()) {
        case LogicUnaryOpKind::Not:
            folded = !value;
            break;
        default:
            COMPILER_UNREACHABLE("Unexpected op kind");
        }
        auto newOp = builder.insert<ConstantOp>(op->ref, type, folded);
        builder.replace(op, newOp);
        return;
    }

    void run(const Operation::Ptr &op, OptBuilder &builder) const override {
        if (op->is<ArithBinaryOp>())
            foldArithBinaryOp(op->as<ArithBinaryOp>(), builder);
        else if (op->is<ArithCastOp>())
            foldArithCastOp(op->as<ArithCastOp>(), builder);
        else if (op->is<LogicBinaryOp>())
            foldLogicBinaryOp(op->as<LogicBinaryOp>(), builder);
        else if (op->is<LogicUnaryOp>())
            foldLogicUnaryOp(op->as<LogicUnaryOp>(), builder);
    }
};

} // namespace

namespace optree {
namespace optimizer {

BaseTransform::Ptr createFoldConstants() {
    return std::make_shared<FoldConstants>();
}

} // namespace optimizer
} // namespace optree
