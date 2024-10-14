#include "optimizer/transform.hpp"

#include <memory>

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/operation.hpp"

#include "optimizer/opt_builder.hpp"

using namespace optree;
using namespace optree::optimizer;

namespace {

struct EraseUnusedOps : public Transform<ConstantOp, ArithBinaryOp, ArithCastOp, LogicBinaryOp, LogicUnaryOp> {
    using Transform::Transform;

    std::string_view name() const override {
        return "EraseUnusedOps";
    }

    void run(const Operation::Ptr &op, OptBuilder &builder) const override {
        bool unused = true;
        for (const auto &result : op->results)
            unused &= result->uses.empty();
        if (unused)
            builder.erase(op);
    }
};

} // namespace

namespace optree {
namespace optimizer {

BaseTransform::Ptr createEraseUnusedOps() {
    return std::make_shared<EraseUnusedOps>();
}

} // namespace optimizer
} // namespace optree
