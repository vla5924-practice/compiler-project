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

TEST_F(DeclarativeSimilarTest, constant_op_false_similarity) {
    vFirst[0] = mFirst.op<ConstantOp>().attr(321).result(mFirst.tI64);
    vSecond[0] = mSecond.op<ConstantOp>().attr(123).result(mSecond.tI64);

    assertSimilarFalse();
}
