#include "optimizer/transform.hpp"

#include <memory>
#include <string_view>
#include <unordered_set>

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/helpers.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/optree/types.hpp"
#include "compiler/utils/helpers.hpp"

#include "optimizer/opt_builder.hpp"

using namespace optree;
using namespace optree::optimizer;

namespace {

struct HoistLoopInvariants : public Transform<WhileOp, ForOp> {
    using Transform::Transform;

    std::string_view name() const override {
        return "HoistLoopInvariants";
    }

    using LoopValues = std::unordered_set<Value::Ptr>;

    static bool isInvariant(const Operation::Ptr op, const LoopValues &values) {
        bool result = false;
        for (auto operand : op->operands) {
            result |= values.count(operand);
        }
        return !result;
    }

    void run(const Operation::Ptr &op, OptBuilder &builder) const override {
        LoopValues values;
        for (const auto &childOp : op->body) {
            for (auto result : childOp->results) {
                values.insert(result);
            }
        }

        for (const auto &childOp : utils::advanceEarly(op->body)) {
            if (utils::isAny<WhileOp, ForOp>(childOp)) {
                continue;
            }

            if (childOp->as<LoadOp>()) {
                continue;
            }

            if (isInvariant(childOp, values)) {
                builder.setInsertPointBefore(op);
                auto cloned = builder.clone(childOp);
                builder.replace(childOp, cloned);
                builder.setInsertPointAfter(cloned);
            }
        }
    }
};

} // namespace

namespace optree {
namespace optimizer {

BaseTransform::Ptr createHoistLoopInvariants() {
    return std::make_shared<HoistLoopInvariants>();
}

} // namespace optimizer
} // namespace optree
