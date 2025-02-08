#include <deque>
#include <utility>

#include <gtest/gtest.h>

#include "compiler/backend/optree/semantizer/semantizer.hpp"
#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/declarative.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/utils/error_buffer.hpp"

#include "common.hpp"

using namespace optree;
using namespace optree::semantizer;

namespace {

class SemantizerTest : public TestWithDummyValues {
  protected:
    DeclarativeModule m;
    ValueStorage &v;

    void assertNoErrors(const Operation::Ptr &op) {
        ASSERT_NO_THROW(Semantizer::process(op));
    }

    void assertAnyErrors(const Operation::Ptr &op) {
        ASSERT_THROW(Semantizer::process(op), ErrorBuffer);
    }

  public:
    SemantizerTest() : TestWithDummyValues(), m(), v(m.values()){};
    ~SemantizerTest() = default;
};

} // namespace

TEST_F(SemantizerTest, succeeds_on_empty_module) {
    assertNoErrors(m.rootOp());
}

TEST_F(SemantizerTest, fails_on_unknown_op) {
    assertAnyErrors(Operation::make("UnknownOp"));
}

TEST_F(SemantizerTest, succeeds_on_valid_arith_binary_op) {
    m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, makeValue(m.tI64), makeValue(m.tI64));
    assertNoErrors(m.childOp());
}

TEST_F(SemantizerTest, fails_on_arith_binary_op_with_incorrect_num_operands) {
    m.op<ArithBinaryOp>(makeValue(m.tI64)).attr(ArithBinOpKind::AddI).result(m.tI64);
    assertAnyErrors(m.childOp());
}

TEST_F(SemantizerTest, succeeds_on_valid_function_with_if) {
    m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
    v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(1));
    v[1] = m.opInit<LogicBinaryOp>(LogicBinOpKind::Equal, v["x"], v[0]);
    m.op<IfOp>(v[1]).withBody();
    m.op<ThenOp>().withBody();
    v[2] = m.opInit<ConstantOp>(m.tF64, 2.3);
    m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[2]);
    m.endBody();
    m.endBody();
    m.opInit<ReturnOp>();
    m.endBody();

    assertNoErrors(m.rootOp());
}

TEST_F(SemantizerTest, succeeds_on_valid_function_with_if_elif_else) {
    m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
    v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(8));
    v[1] = m.opInit<LogicBinaryOp>(LogicBinOpKind::LessEqualI, v["x"], v[0]);
    m.op<IfOp>(v[1]).withBody();
    m.op<ThenOp>().withBody();
    v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
    m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
    m.endBody();
    m.op<ElseOp>().withBody();
    v[3] = m.opInit<ConstantOp>(m.tI64, int64_t(11));
    v[4] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualI, v["x"], v[3]);
    m.op<IfOp>(v[4]).withBody();
    m.op<ThenOp>().withBody();
    v[5] = m.opInit<ConstantOp>(m.tF64, 12.13);
    m.opInit<ArithBinaryOp>(ArithBinOpKind::MulF, v["y"], v[5]);
    m.endBody();
    m.op<ElseOp>().withBody();
    v[6] = m.opInit<ConstantOp>(m.tF64, 14.15);
    m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[6]);
    m.endBody();
    m.endBody();
    m.endBody();
    m.endBody();
    m.opInit<ReturnOp>();
    m.endBody();

    assertNoErrors(m.rootOp());
}

TEST_F(SemantizerTest, succeeds_on_valid_functions_with_function_calls) {
    m.opInit<FunctionOp>("int_to_float", m.tFunc({m.tI64}, m.tF64)).withBody();
    v[0] = m.opInit<ConstantOp>(m.tF64, 1.2);
    m.opInit<ReturnOp>(v[0]);
    m.endBody();
    m.opInit<FunctionOp>("float_to_none", m.tFunc({m.tF64}, m.tNone)).withBody();
    m.opInit<ReturnOp>();
    m.endBody();
    m.opInit<FunctionOp>("none_to_int", m.tFunc(m.tI64)).withBody();
    v[1] = m.opInit<ConstantOp>(m.tI64, int64_t(1));
    m.opInit<ReturnOp>(v[1]);
    m.endBody();
    m.opInit<FunctionOp>("many_to_int", m.tFunc({m.tI64, m.tF64, m.tStr}, m.tI64)).withBody();
    v[2] = m.opInit<ConstantOp>(m.tI64, int64_t(3));
    m.opInit<ReturnOp>(v[2]);
    m.endBody();
    m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
    v[10] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
    v[11] = m.opInit<ConstantOp>(m.tI64, int64_t(4));
    v[12] = m.opInit<FunctionCallOp>("int_to_float", m.tF64).operand(v[11]);
    v[13] = m.opInit<ConstantOp>(m.tF64, 8.9);
    v[14] = m.opInit<ArithBinaryOp>(ArithBinOpKind::DivF, v[12], v[13]);
    m.opInit<StoreOp>(v[10], v[14]);
    v[15] = m.opInit<ConstantOp>(m.tF64, 5.6);
    v[16] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[15], v["y"]);
    m.opInit<FunctionCallOp>("float_to_none", m.tNone).operand(v[16]);
    v[17] = m.opInit<FunctionCallOp>("none_to_int", m.tI64);
    v[18] = m.opInit<ConstantOp>(m.tI64, int64_t(7));
    v[19] = m.opInit<ArithBinaryOp>(ArithBinOpKind::MulI, v[18], v["x"]);
    v[20] = m.opInit<ConstantOp>(m.tStr, std::string("str"));
    v[21] = m.opInit<FunctionCallOp>("many_to_int", m.tI64).operands(v[19], v["y"], v[20]);
    v[22] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubI, v[17], v[21]);
    m.opInit<ReturnOp>();
    m.endBody();

    assertNoErrors(m.rootOp());
}

TEST_F(SemantizerTest, succeeds_on_valid_function_with_while) {
    m.opInit<FunctionOp>("test", m.tFunc({m.tI64}, m.tNone)).inward(v["x"], 0).withBody();
    m.op<WhileOp>().withBody();
    m.op<ConditionOp>().withBody();
    v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(1));
    v[1] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualI, v["x"], v[0]);
    m.endBody();
    v[2] = m.opInit<ConstantOp>(m.tI64, int64_t(2));
    m.opInit<ArithBinaryOp>(ArithBinOpKind::MulI, v[2], v["x"]);
    m.endBody();
    m.opInit<ReturnOp>();
    m.endBody();

    assertNoErrors(m.rootOp());
}
