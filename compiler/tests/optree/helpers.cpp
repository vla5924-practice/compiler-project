#include <cstdint>

#include <gtest/gtest.h>

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/declarative.hpp"
#include "compiler/optree/helpers.hpp"

using namespace optree;

class DeclarativeSimilarTest : public ::testing::Test {
  protected:
    DeclarativeModule mFirst;
    DeclarativeModule mSecond;

    ValueStorage &vFirst;
    ValueStorage &vSecond;

  public:
    DeclarativeSimilarTest() : mFirst(), mSecond(), vFirst(mFirst.values()), vSecond(mSecond.values()){};
    ~DeclarativeSimilarTest() = default;

    auto assertSimilarTrue() const {
        ASSERT_TRUE(similar(mFirst.rootOp(), mSecond.rootOp()));
    }

    auto assertSimilarFalse() const {
        ASSERT_FALSE(similar(mFirst.rootOp(), mSecond.rootOp()));
    }
};

TEST_F(DeclarativeSimilarTest, constant_op_true_similarity) {
    vFirst[0] = mFirst.op<ConstantOp>().attr(123).result(mFirst.tI64);
    vSecond[0] = mSecond.op<ConstantOp>().attr(123).result(mSecond.tI64);

    assertSimilarTrue();
}

TEST_F(DeclarativeSimilarTest, constant_op_true_similarity_in_one_tree) {
    vFirst[0] = mFirst.op<ConstantOp>().attr(123).result(mFirst.tI64);

    ASSERT_TRUE(similar(mFirst.rootOp(), mFirst.rootOp()));
}

TEST_F(DeclarativeSimilarTest, constant_op_false_similarity) {
    vFirst[0] = mFirst.op<ConstantOp>().attr(321).result(mFirst.tI64);
    vSecond[0] = mSecond.op<ConstantOp>().attr(123).result(mSecond.tI64);

    assertSimilarFalse();
}

TEST_F(DeclarativeSimilarTest, add_similarity) {
    vFirst[0] = mFirst.op<ConstantOp>().attr(123).result(mFirst.tI64);
    vFirst[1] = mFirst.op<ConstantOp>().attr(123).result(mFirst.tI64);
    vFirst[2] = mFirst.op<ArithBinaryOp>(vFirst[0], vFirst[1]).attr(ArithBinOpKind::AddI).result(mFirst.tI64);

    vSecond[0] = mSecond.op<ConstantOp>().attr(123).result(mSecond.tI64);
    vSecond[1] = mSecond.op<ConstantOp>().attr(123).result(mSecond.tI64);
    vSecond[2] = mSecond.op<ArithBinaryOp>(vSecond[0], vSecond[1]).attr(ArithBinOpKind::AddI).result(mSecond.tI64);

    assertSimilarTrue();
}

TEST_F(DeclarativeSimilarTest, add_similarity_order_independent) {
    vFirst[0] = mFirst.op<ConstantOp>().attr(123).result(mFirst.tI64);
    vFirst[1] = mFirst.op<ConstantOp>().attr(123).result(mFirst.tI64);
    vFirst[2] = mFirst.op<ArithBinaryOp>(vFirst[0], vFirst[1]).attr(ArithBinOpKind::AddI).result(mFirst.tI64);

    vSecond[0] = mSecond.op<ConstantOp>().attr(123).result(mSecond.tI64);
    vSecond[1] = mSecond.op<ConstantOp>().attr(123).result(mSecond.tI64);
    vSecond[2] = mSecond.op<ArithBinaryOp>(vSecond[1], vSecond[0]).attr(ArithBinOpKind::AddI).result(mSecond.tI64);

    assertSimilarTrue();
}

TEST_F(DeclarativeSimilarTest, sub_similarity_order_independent) {
    vFirst[0] = mFirst.opInit<ConstantOp>(mFirst.tI64, 123);
    vFirst[1] = mFirst.opInit<ConstantOp>(mFirst.tI64, 321);
    vFirst[2] = mFirst.opInit<ArithBinaryOp>(ArithBinOpKind::SubI, vFirst[0], vFirst[1]);

    vSecond[0] = mSecond.opInit<ConstantOp>(mSecond.tI64, 123);
    vSecond[1] = mSecond.opInit<ConstantOp>(mSecond.tI64, 321);
    vSecond[2] = mSecond.opInit<ConstantOp>(mSecond.tF64, 321.0);
    vSecond[3] = mSecond.opInit<ArithBinaryOp>(ArithBinOpKind::SubI, vSecond[1], vSecond[0]);

    assertSimilarFalse();
}
