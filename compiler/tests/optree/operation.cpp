#include <gtest/gtest.h>

#include "compiler/optree/base_adaptor.hpp"
#include "compiler/optree/operation.hpp"

using namespace optree;

namespace {

struct FirstOp : public Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "First")
};

struct SecondOp : public Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Second")
};

} // namespace

TEST(Operation, make_without_spec_is_unknown) {
    auto op = Operation::make("UnknownOp");
    ASSERT_TRUE(op->isUnknown());
}

TEST(Operation, can_assign_immediate_parent) {
    auto op1 = Operation::make<FirstOp>();
    auto op2 = Operation::make<SecondOp>(op1);
    ASSERT_EQ(op1.op, op2->parent);
}

TEST(Operation, can_find_parent_if_in_tree) {
    auto op1 = Operation::make<FirstOp>();
    auto op2 = Operation::make<SecondOp>(op1);
    ASSERT_EQ(op1.op, op2->findParent<FirstOp>().op);
}

TEST(Operation, can_find_parent_not_self) {
    auto op1 = Operation::make<FirstOp>();
    auto op2 = Operation::make<SecondOp>(op1);
    auto op3 = Operation::make<FirstOp>(op2);
    auto op4 = Operation::make<FirstOp>(op3);
    auto op5 = Operation::make<SecondOp>(op4);
    ASSERT_EQ(op2.op, op5->findParent<SecondOp>().op);
}
