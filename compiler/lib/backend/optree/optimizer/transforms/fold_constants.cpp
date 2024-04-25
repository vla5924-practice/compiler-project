#include "optimizer/transform.hpp"

#include <cstdint>
#include <memory>

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/definitions.hpp"
#include "compiler/optree/helpers.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/optree/types.hpp"

#include "optimizer/opt_builder.hpp"

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
            int64_t folded = 0;
            int64_t lhs = lhsOp.value().as<int64_t>();
            int64_t rhs = rhsOp.value().as<int64_t>();
            switch (op.kind()) {
            case ArithBinOpKind::AddI:
                folded = lhs + rhs;
                break;
            case ArithBinOpKind::MulI:
                folded = lhs * rhs;
                break;
            default:
                folded = -1; // TODO: extend
            }
            auto newOp = builder.insert<ConstantOp>(op.ref(), type, folded);
            builder.replace(op, newOp);
            return;
        }
        // TODO: support other types
    }

    void run(const Operation::Ptr &op, OptBuilder &builder) const override {
        if (op->is<ArithBinaryOp>())
            foldArithBinaryOp(op->as<ArithBinaryOp>(), builder);
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
