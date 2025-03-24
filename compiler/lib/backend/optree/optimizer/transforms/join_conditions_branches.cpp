#include <memory>
#include <string_view>

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/helpers.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/optree/value.hpp"
#include "compiler/utils/helpers.hpp"
#include "optimizer/opt_builder.hpp"
#include "optimizer/transform.hpp"

using namespace optree;
using namespace optree::optimizer;

namespace {

struct JoinConditionsBranches : public Transform<IfOp> {
    using Transform::Transform;

    std::string_view name() const override {
        return "JoinConditionsBranches";
    }

    void run(const Operation::Ptr &op, OptBuilder &builder) const override {
        auto ifOp = op->as<IfOp>();
        if (auto elseOp = ifOp.elseOp()) {
            auto thenOp = ifOp.thenOp();
            if (elseOp->body.size() == thenOp->body.size()) {
                for (const auto &[thenElem, elseElem] : utils::zip(thenOp->body, elseOp->body)) {
                    if (!similar(thenElem, elseElem)) {
                        return;
                    }
                }
                builder.erase(elseOp);
                builder.setInsertPointBefore(op);
                for (const auto &childOp : utils::advanceEarly(thenOp->body)) {
                    auto cloned = builder.clone(childOp);
                    builder.replace(childOp, cloned);
                    builder.setInsertPointAfter(cloned);
                }
                builder.erase(op);
            }
        }
    }
};

} // namespace

namespace optree {
namespace optimizer {

BaseTransform::Ptr createJoinConditionsBranches() {
    return std::make_shared<JoinConditionsBranches>();
}

} // namespace optimizer
} // namespace optree
