#include "optimizer/transform.hpp"

#include <cassert>
#include <cstdint>
#include <memory>

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/definitions.hpp"
#include "compiler/optree/helpers.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/optree/types.hpp"

#include "optimizer/opt_builder.hpp"

#if __has_builtin(__builtin_unreachable)
#define UNREACHABLE(MSG)                                                                                               \
    assert(false && (MSG));                                                                                            \
    __builtin_unreachable()
#else
#define UNREACHABLE(MSG) assert(false && (MSG))
#endif

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
            int64_t folded;
            int64_t lhs = lhsOp.value().as<int64_t>();
            int64_t rhs = rhsOp.value().as<int64_t>();
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
                UNREACHABLE("Unexpected op kind");
            }
            auto newOp = builder.insert<ConstantOp>(op->ref, type, folded);
            builder.replace(op, newOp);
            return;
        } else if (type->is<FloatType>()) {
            double folded;
            double lhs = lhsOp.value().as<double>();
            double rhs = rhsOp.value().as<double>();
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
                UNREACHABLE("Unexpected op kind");
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
            int64_t folded;
            if (valueType->is<IntegerType>()) {
                int64_t value = valueOp.value().as<int64_t>();
                switch (op.kind()) {
                case ArithCastOpKind::ExtI:
                case ArithCastOpKind::TruncI:
                    folded = value;
                    break;
                default:
                    UNREACHABLE("Unexpected op kind");
                }
            } else if (valueType->is<FloatType>()) {
                double value = valueOp.value().as<double>();
                switch (op.kind()) {
                case ArithCastOpKind::FloatToInt:
                    folded = static_cast<int64_t>(value);
                    break;
                default:
                    UNREACHABLE("Unexpected op kind");
                }
            }
            auto newOp = builder.insert<ConstantOp>(op->ref, type, folded);
            builder.replace(op, newOp);
            return;
        } else if (type->is<FloatType>()) {
            double folded;
            if (valueType->is<FloatType>()) {
                double value = valueOp.value().as<double>();
                switch (op.kind()) {
                case ArithCastOpKind::ExtF:
                case ArithCastOpKind::TruncF:
                    folded = value;
                    break;
                default:
                    UNREACHABLE("Unexpected op kind");
                }
            } else if (valueType->is<IntegerType>()) {
                int64_t value = valueOp.value().as<int64_t>();
                switch (op.kind()) {
                case ArithCastOpKind::IntToFloat:
                    folded = static_cast<double>(value);
                    break;
                default:
                    UNREACHABLE("Unexpected op kind");
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
        bool folded;
        if (valType->is<BoolType>()) {
            bool lhs = lhsOp.value().as<bool>();
            bool rhs = rhsOp.value().as<bool>();
            switch (op.kind()) {
            case LogicBinOpKind::AndI:
                folded = lhs && rhs;
                break;
            case LogicBinOpKind::OrI:
                folded = lhs || rhs;
                break;
            default:
                UNREACHABLE("Unexpected op kind");
            }
        } else if (valType->is<IntegerType>()) {
            int64_t lhs = lhsOp.value().as<int64_t>();
            int64_t rhs = rhsOp.value().as<int64_t>();
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
                UNREACHABLE("Unexpected op kind");
            }
        } else if (valType->is<FloatType>()) {
            double lhs = lhsOp.value().as<double>();
            double rhs = rhsOp.value().as<double>();
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
                UNREACHABLE("Unexpected op kind");
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
        bool folded;
        bool value = valueOp.value().as<bool>();
        switch (op.kind()) {
        case LogicUnaryOpKind::Not:
            folded = !value;
            break;
        default:
            UNREACHABLE("Unexpected op kind");
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
