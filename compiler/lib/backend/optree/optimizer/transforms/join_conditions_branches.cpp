#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <ranges>

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/utils/helpers.hpp"
#include "compiler/utils/language.hpp"
#include "optimizer/opt_builder.hpp"
#include "optimizer/transform.hpp"

using namespace optree;
using namespace optree::optimizer;

namespace {

struct JoinConditionsBranches : public Transform<IfOp> {
    using Transform::Transform;

    static inline bool compareOperations(const Operation::Ptr &lhs, const Operation::Ptr &rhs) {
        auto attrEqual = [](const Attribute &lhs, const Attribute &rhs) { return lhs.storage == rhs.storage; };
        bool attr = std::ranges::equal(lhs->attributes, rhs->attributes, attrEqual);
        auto valueEqual = [](const Value::Ptr &lhs, const Value::Ptr &rhs) { return lhs->type == rhs->type; };
        bool operands = std::ranges::equal(lhs->operands, rhs->operands, valueEqual);
        bool inwards = std::ranges::equal(lhs->inwards, rhs->inwards, valueEqual);
        bool results = std::ranges::equal(lhs->results, rhs->results, valueEqual);
        bool body = std::ranges::equal(lhs->body, rhs->body, JoinConditionsBranches::compareOperations);

        return attr && operands && inwards && results && body;
    }

    void run(const Operation::Ptr &op, OptBuilder &builder) const override {
        auto ifOp = op->as<IfOp>();
        if (auto elseOp = ifOp.elseOp()) {
            auto thenOp = ifOp.thenOp();
            if (elseOp->body.size() == thenOp->body.size()) {
                for (const auto &[thenElem, elseElem] : utils::zip(thenOp->body, elseOp->body)) {
                    if (!compareOperations(thenElem, elseElem)) {
                        return;
                    }
                }
                builder.erase(elseOp);
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
