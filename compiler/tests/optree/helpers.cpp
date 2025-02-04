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

TEST_F(DeclarativeSimilarTest, arith_binary_op_similarity) {
    vFirst[0] = mFirst.op<ConstantOp>().attr(123).result(mFirst.tI64);
    vFirst[1] = mFirst.op<ConstantOp>().attr(321).result(mFirst.tI64);
    vFirst[2] = mFirst.op<ArithBinaryOp>(vFirst[0], vFirst[1]).attr(ArithBinOpKind::AddI).result(mFirst.tI64);

    vSecond[0] = mSecond.op<ConstantOp>().attr(123).result(mSecond.tI64);
    vSecond[1] = mSecond.op<ConstantOp>().attr(321).result(mSecond.tI64);
    vSecond[2] = mSecond.op<ArithBinaryOp>(vSecond[0], vSecond[1]).attr(ArithBinOpKind::AddI).result(mSecond.tI64);

    assertSimilarTrue();
}

TEST_F(DeclarativeSimilarTest, arith_binary_op_operands_attr_dissimilarity) {
    vFirst[0] = mFirst.op<ConstantOp>().attr(123).result(mFirst.tI64);
    vFirst[1] = mFirst.op<ConstantOp>().attr(321).result(mFirst.tI64);
    vFirst[2] = mFirst.op<ArithBinaryOp>(vFirst[0], vFirst[1]).attr(ArithBinOpKind::AddF).result(mFirst.tI64);

    vSecond[0] = mSecond.op<ConstantOp>().attr(123).result(mSecond.tI64);
    vSecond[1] = mSecond.op<ConstantOp>().attr(321).result(mSecond.tI64);
    vSecond[2] = mSecond.op<ArithBinaryOp>(vSecond[0], vSecond[1]).attr(ArithBinOpKind::AddI).result(mSecond.tI64);

    assertSimilarFalse();
}

TEST_F(DeclarativeSimilarTest, arith_binary_op_operands_result_dissimilarity) {
    vFirst[0] = mFirst.op<ConstantOp>().attr(123).result(mFirst.tI64);
    vFirst[1] = mFirst.op<ConstantOp>().attr(321).result(mFirst.tI64);
    vFirst[2] = mFirst.op<ArithBinaryOp>(vFirst[0], vFirst[1]).attr(ArithBinOpKind::AddI).result(mFirst.tF64);

    vSecond[0] = mSecond.op<ConstantOp>().attr(123).result(mSecond.tI64);
    vSecond[1] = mSecond.op<ConstantOp>().attr(321).result(mSecond.tI64);
    vSecond[2] = mSecond.op<ArithBinaryOp>(vSecond[0], vSecond[1]).attr(ArithBinOpKind::AddI).result(mSecond.tI64);

    assertSimilarFalse();
}

TEST_F(DeclarativeSimilarTest, arith_binary_op_operands_dissimilarity) {
    vFirst[0] = mFirst.op<ConstantOp>().attr(123).result(mFirst.tI64);
    vFirst[1] = mFirst.op<ConstantOp>().attr(321).result(mFirst.tI64);
    vFirst[2] = mFirst.op<ArithBinaryOp>(vFirst[1], vFirst[0]).attr(ArithBinOpKind::AddI).result(mFirst.tI64);

    vSecond[0] = mSecond.op<ConstantOp>().attr(123).result(mSecond.tI64);
    vSecond[1] = mSecond.op<ConstantOp>().attr(321).result(mSecond.tI64);
    vSecond[2] = mSecond.op<ArithBinaryOp>(vSecond[0], vSecond[1]).attr(ArithBinOpKind::AddI).result(mSecond.tI64);

    assertSimilarFalse();
}

TEST_F(DeclarativeSimilarTest, operation_dissimilarity) {
    vFirst[0] = mFirst.op<ConstantOp>().attr(123).result(mFirst.tI64);
    vFirst[1] = mFirst.op<ConstantOp>().attr(321).result(mFirst.tI64);
    vFirst[2] = mFirst.op<ArithBinaryOp>(vFirst[0], vFirst[1]).attr(ArithBinOpKind::AddI).result(mFirst.tI64);

    vSecond[0] = mSecond.op<ConstantOp>().attr(123).result(mSecond.tI64);
    vSecond[1] = mSecond.op<ConstantOp>().attr(321).result(mSecond.tI64);
    vSecond[2] = mSecond.op<BinaryOp>(vSecond[0], vSecond[1]).attr(LogicBinOpKind::Equal).result(mSecond.tI64);

    assertSimilarFalse();
}

TEST_F(DeclarativeSimilarTest, function_similarity) {
    mFirst.opInit<FunctionOp>("test", mFirst.tFunc({mFirst.tI64, mFirst.tI64}, mFirst.tNone))
        .inward(vFirst[0], 0)
        .inward(vFirst[1], 1)
        .withBody();
    vFirst[2] = mFirst.opInit<ConstantOp>(mFirst.tI64, 123);
    vFirst[3] = mFirst.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, vFirst[0], vFirst[1]);
    vFirst[4] = mFirst.opInit<ArithCastOp>(ArithCastOpKind::IntToFloat, mFirst.tF64, vFirst[0]);
    vFirst[5] = mFirst.opInit<LogicBinaryOp>(LogicBinOpKind::LessEqualI, vFirst[0], vFirst[1]);
    vFirst[6] = mFirst.opInit<LogicUnaryOp>(LogicUnaryOpKind::Not, vFirst[0]);
    mFirst.opInit<ReturnOp>();
    mFirst.endBody();

    mSecond.opInit<FunctionOp>("test", mSecond.tFunc({mSecond.tI64, mSecond.tI64}, mSecond.tNone))
        .inward(vSecond[0], 0)
        .inward(vSecond[1], 1)
        .withBody();
    vSecond[2] = mSecond.opInit<ConstantOp>(mSecond.tI64, 123);
    vSecond[3] = mSecond.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, vSecond[0], vSecond[1]);
    vSecond[4] = mSecond.opInit<ArithCastOp>(ArithCastOpKind::IntToFloat, mSecond.tF64, vSecond[0]);
    vSecond[5] = mSecond.opInit<LogicBinaryOp>(LogicBinOpKind::LessEqualI, vSecond[0], vSecond[1]);
    vSecond[6] = mSecond.opInit<LogicUnaryOp>(LogicUnaryOpKind::Not, vSecond[0]);
    mSecond.opInit<ReturnOp>();
    mSecond.endBody();

    assertSimilarTrue();
}