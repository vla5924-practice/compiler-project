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

    static inline std::array<ArithBinOpKind, 4> commutativityBinaryOps{ArithBinOpKind::AddI, ArithBinOpKind::AddF,
                                                                       ArithBinOpKind::MulI, ArithBinOpKind::MulF};

    static void sortArithBinaryOprands(const ArithBinaryOp &op, OptBuilder &builder) {
        if (std::find(commutativityBinaryOps.cbegin(), commutativityBinaryOps.cend(), op.kind()) ==
            commutativityBinaryOps.end()) {
            return;
        }
        if (auto lhs = getValueOwnerAs<ConstantOp>(op.lhs())) {
            if (auto rhs = getValueOwnerAs<ConstantOp>(op.rhs())) {
                if (lhs->attributes[0].storage > rhs->attributes[0].storage)
                    std::swap(op->operands[0], op->operands[1]);
            }
            if (auto rhs = getValueOwnerAs<LoadOp>(op.rhs())) {
                std::swap(op->operands[0], op->operands[1]);
            }
        }
        if (auto lhs = getValueOwnerAs<LoadOp>(op.lhs())) {
            if (auto rhs = getValueOwnerAs<LoadOp>(op.rhs())) {
                if (lhs > rhs)
                    std::swap(op->operands[0], op->operands[1]);
            }
        }
        if (auto lhs = getValueOwnerAs<FunctionCallOp>(op.lhs())) {
            if (auto rhs = getValueOwnerAs<ConstantOp>(op.rhs())) {
                std::swap(op->operands[0], op->operands[1]);
            }
            if (auto rhs = getValueOwnerAs<LoadOp>(op.rhs())) {
                std::swap(op->operands[0], op->operands[1]);
            }
            if (auto rhs = getValueOwnerAs<FunctionCallOp>(op.rhs())) {
                if (std::lexicographical_compare(lhs->name.begin(), lhs->name.end(), rhs->name.begin(),
                                                 rhs->name.end())) {
                    std::swap(op->operands[0], op->operands[1]);
                } else {
                    // TODO check function argument order
                }
            }
        }
    }

    static void sortLogicBinaryOprands(const LogicBinaryOp &op, OptBuilder &builder) {
    }

    void run(const Operation::Ptr &op, OptBuilder &builder) const override {
        if (op->is<ArithBinaryOp>())
            sortArithBinaryOprands(op->as<ArithBinaryOp>(), builder);
        else if (op->is<LogicBinaryOp>())
            sortLogicBinaryOprands(op->as<LogicBinaryOp>(), builder);
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
