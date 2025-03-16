#include <algorithm>
#include <array>
#include <memory>
#include <string_view>

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

struct OrderingCommutativityOps : public Transform<ArithBinaryOp, LogicBinaryOp> {
    using Transform::Transform;

    std::string_view name() const override {
        return "OrderingCommutativityOps";
    }
    // clang-format off
    static inline std::array<ArithBinOpKind, 4> commutativityArithOps{
        ArithBinOpKind::AddI,
        ArithBinOpKind::AddF,
        ArithBinOpKind::MulI,
        ArithBinOpKind::MulF
    };

    static inline std::array<LogicBinOpKind, 4> commutativityLogicOps{
        LogicBinOpKind::AndI,
        LogicBinOpKind::Equal,
        LogicBinOpKind::OrI,
        LogicBinOpKind::NotEqual
    };
    // clang-format on
    static void sortBinaryOprands(const BinaryOp &op, OptBuilder &builder) {
        if (auto lhs = getValueOwnerAs<ConstantOp>(op.lhs())) {
            if (auto rhs = getValueOwnerAs<ConstantOp>(op.rhs()))
                if (lhs->attributes[0].storage > rhs->attributes[0].storage)
                    builder.update(op, [&op, &lhs, &rhs]() { std::swap(op->operands[0], op->operands[1]); });
            if (auto rhs = getValueOwnerAs<LoadOp>(op.rhs()))
                builder.update(op, [&op, &lhs, &rhs]() { std::swap(op->operands[0], op->operands[1]); });
        }
        if (auto lhs = getValueOwnerAs<LoadOp>(op.lhs())) {
            if (auto rhs = getValueOwnerAs<LoadOp>(op.rhs()))
                if (lhs > rhs)
                    builder.update(op, [&op, &lhs, &rhs]() { std::swap(op->operands[0], op->operands[1]); });
        }
        if (auto lhs = getValueOwnerAs<FunctionCallOp>(op.lhs())) {
            if (auto rhs = getValueOwnerAs<ConstantOp>(op.rhs()))
                builder.update(op, [&op, &lhs, &rhs]() { std::swap(op->operands[0], op->operands[1]); });
            if (auto rhs = getValueOwnerAs<LoadOp>(op.rhs()))
                builder.update(op, [&op, &lhs, &rhs]() { std::swap(op->operands[0], op->operands[1]); });
            if (auto rhs = getValueOwnerAs<FunctionCallOp>(op.rhs())) {
                if (std::lexicographical_compare(lhs->name.begin(), lhs->name.end(), rhs->name.begin(),
                                                 rhs->name.end()))
                    builder.update(op, [&op, &lhs, &rhs]() { std::swap(op->operands[0], op->operands[1]); });
                else {
                    // TODO check function argument order
                }
            }
        }
    }

    void run(const Operation::Ptr &op, OptBuilder &builder) const override {
        if (op->is<ArithBinaryOp>()) {
            auto arithOp = op->as<ArithBinaryOp>();
            if (std::find(commutativityArithOps.cbegin(), commutativityArithOps.cend(), arithOp.kind()) ==
                commutativityArithOps.end()) {
                return;
            }
            sortBinaryOprands(op->as<BinaryOp>(), builder);
        } else if (op->is<LogicBinaryOp>()) {
            auto logicOp = op->as<LogicBinaryOp>();
            if (std::find(commutativityLogicOps.cbegin(), commutativityLogicOps.cend(), logicOp.kind()) ==
                commutativityLogicOps.end()) {
                return;
            }
            sortBinaryOprands(op->as<BinaryOp>(), builder);
        }
    }
};

} // namespace

namespace optree {
namespace optimizer {

BaseTransform::Ptr createOrderingCommutativityOps() {
    return std::make_shared<OrderingCommutativityOps>();
}

} // namespace optimizer
} // namespace optree
