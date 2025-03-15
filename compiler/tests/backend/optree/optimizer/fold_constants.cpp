#include <gtest/gtest.h>

#include "compiler/backend/optree/optimizer/optimizer.hpp"
#include "compiler/backend/optree/optimizer/transform.hpp"
#include "compiler/backend/optree/optimizer/transform_factories.hpp"
#include "compiler/optree/adaptors.hpp"

#include "common.hpp"

using namespace optree;
using namespace optree::optimizer;

class FoldConstantsTest : public TransformTestBase {
    virtual void setupOptimizer(Optimizer &opt) const override {
        auto transform = CascadeTransform::make("FoldConstantsTest");
        transform->add(createFoldConstants());
        opt.add(transform);
    }

  public:
    FoldConstantsTest() = default;
    ~FoldConstantsTest() = default;
};

TEST_F(FoldConstantsTest, can_run_on_empty_optree) {
    runOptimizer();
    assertSameOpTree();
}

TEST_F(FoldConstantsTest, can_fold_constants_arith_binary_integer_op) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc(m.tNone)).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, 6);
        v[1] = m.opInit<ConstantOp>(m.tI64, 2);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v[0], v[1]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubI, v[0], v[1]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::MulI, v[0], v[1]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::DivI, v[0], v[1]);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc(m.tNone)).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, 6);
        v[1] = m.opInit<ConstantOp>(m.tI64, 2);
        m.opInit<ConstantOp>(m.tI64, 8);
        m.opInit<ConstantOp>(m.tI64, 4);
        m.opInit<ConstantOp>(m.tI64, 12);
        m.opInit<ConstantOp>(m.tI64, 3);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    assertSameOpTree();
}

TEST_F(FoldConstantsTest, can_fold_constants_arith_cast_integer_op) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc(m.tNone)).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, 134);
        m.opInit<ArithCastOp>(ArithCastOpKind::IntToFloat, m.tF64, v[0]);
        m.opInit<ArithCastOp>(ArithCastOpKind::ExtI, m.tI64, v[0]);
        m.opInit<ArithCastOp>(ArithCastOpKind::TruncI, m.tI64, v[0]);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc(m.tNone)).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, 134);
        m.opInit<ConstantOp>(m.tF64, 134.0);
        m.opInit<ConstantOp>(m.tI64, 134);
        m.opInit<ConstantOp>(m.tI64, 134);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    assertSameOpTree();
}

TEST_F(FoldConstantsTest, can_fold_constants_logic_binary_integer_op) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc(m.tNone)).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, 6);
        v[1] = m.opInit<ConstantOp>(m.tI64, 2);
        m.opInit<LogicBinaryOp>(LogicBinOpKind::Equal, v[0], v[1]);
        m.opInit<LogicBinaryOp>(LogicBinOpKind::NotEqual, v[0], v[1]);
        m.opInit<LogicBinaryOp>(LogicBinOpKind::LessEqualI, v[0], v[1]);
        m.opInit<LogicBinaryOp>(LogicBinOpKind::LessI, v[0], v[1]);
        m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualI, v[0], v[1]);
        m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterI, v[0], v[1]);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc(m.tNone)).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, 6);
        v[1] = m.opInit<ConstantOp>(m.tI64, 2);
        m.opInit<ConstantOp>(m.tBool, false);
        m.opInit<ConstantOp>(m.tBool, true);
        m.opInit<ConstantOp>(m.tBool, false);
        m.opInit<ConstantOp>(m.tBool, false);
        m.opInit<ConstantOp>(m.tBool, true);
        m.opInit<ConstantOp>(m.tBool, true);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    assertSameOpTree();
}

TEST_F(FoldConstantsTest, can_fold_constants_logic_unary_integer_op) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc(m.tNone)).withBody();
        v[0] = m.opInit<ConstantOp>(m.tBool, true);
        m.opInit<LogicUnaryOp>(LogicUnaryOpKind::Not, v[0]);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc(m.tNone)).withBody();
        m.opInit<ConstantOp>(m.tBool, true);
        m.opInit<ConstantOp>(m.tBool, false);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    assertSameOpTree();
}

TEST_F(FoldConstantsTest, can_fold_constants_arith_binary_float_op) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc(m.tNone)).withBody();
        v[0] = m.opInit<ConstantOp>(m.tF64, 6.0);
        v[1] = m.opInit<ConstantOp>(m.tF64, 2.0);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[0], v[1]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[0], v[1]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::MulF, v[0], v[1]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::DivF, v[0], v[1]);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc(m.tNone)).withBody();
        v[0] = m.opInit<ConstantOp>(m.tF64, 6.0);
        v[1] = m.opInit<ConstantOp>(m.tF64, 2.0);
        m.opInit<ConstantOp>(m.tF64, 8.0);
        m.opInit<ConstantOp>(m.tF64, 4.0);
        m.opInit<ConstantOp>(m.tF64, 12.0);
        m.opInit<ConstantOp>(m.tF64, 3.0);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    assertSameOpTree();
}

TEST_F(FoldConstantsTest, can_fold_constants_arith_cast_float_op) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc(m.tNone)).withBody();
        v[0] = m.opInit<ConstantOp>(m.tF64, 7.77);
        m.opInit<ArithCastOp>(ArithCastOpKind::FloatToInt, m.tI64, v[0]);
        m.opInit<ArithCastOp>(ArithCastOpKind::ExtF, m.tF64, v[0]);
        m.opInit<ArithCastOp>(ArithCastOpKind::TruncF, m.tF64, v[0]);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc(m.tNone)).withBody();
        v[0] = m.opInit<ConstantOp>(m.tF64, 7.77);
        m.opInit<ConstantOp>(m.tI64, 7);
        m.opInit<ConstantOp>(m.tF64, 7.77);
        m.opInit<ConstantOp>(m.tF64, 7.77);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    assertSameOpTree();
}

TEST_F(FoldConstantsTest, can_fold_constants_logic_binary_float_op) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc(m.tNone)).withBody();
        v[0] = m.opInit<ConstantOp>(m.tF64, 6.0);
        v[1] = m.opInit<ConstantOp>(m.tF64, 2.0);
        m.opInit<LogicBinaryOp>(LogicBinOpKind::Equal, v[0], v[1]);
        m.opInit<LogicBinaryOp>(LogicBinOpKind::NotEqual, v[0], v[1]);
        m.opInit<LogicBinaryOp>(LogicBinOpKind::LessEqualF, v[0], v[1]);
        m.opInit<LogicBinaryOp>(LogicBinOpKind::LessF, v[0], v[1]);
        m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualF, v[0], v[1]);
        m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterF, v[0], v[1]);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc(m.tNone)).withBody();
        v[0] = m.opInit<ConstantOp>(m.tF64, 6.0);
        v[1] = m.opInit<ConstantOp>(m.tF64, 2.0);
        m.opInit<ConstantOp>(m.tBool, false);
        m.opInit<ConstantOp>(m.tBool, true);
        m.opInit<ConstantOp>(m.tBool, false);
        m.opInit<ConstantOp>(m.tBool, false);
        m.opInit<ConstantOp>(m.tBool, true);
        m.opInit<ConstantOp>(m.tBool, true);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    assertSameOpTree();
}

TEST_F(FoldConstantsTest, can_fold_constants_logic_binary_bool_op) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc(m.tNone)).withBody();
        v[0] = m.opInit<ConstantOp>(m.tBool, true);
        v[1] = m.opInit<ConstantOp>(m.tBool, false);
        m.opInit<LogicBinaryOp>(LogicBinOpKind::AndI, v[0], v[1]);
        m.opInit<LogicBinaryOp>(LogicBinOpKind::OrI, v[0], v[1]);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc(m.tNone)).withBody();
        m.opInit<ConstantOp>(m.tBool, true);
        m.opInit<ConstantOp>(m.tBool, false);
        m.opInit<ConstantOp>(m.tBool, false);
        m.opInit<ConstantOp>(m.tBool, true);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    assertSameOpTree();
}

TEST_F(FoldConstantsTest, can_fold_constants_in_multiple_ops) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc(m.tNone)).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, 2);
        v[1] = m.opInit<ConstantOp>(m.tI64, 3);
        v[2] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v[0], v[1]);
        v[3] = m.opInit<ArithCastOp>(ArithCastOpKind::IntToFloat, m.tF64, v[2]);
        v[4] = m.opInit<ConstantOp>(m.tF64, 5.0);
        m.opInit<LogicBinaryOp>(LogicBinOpKind::Equal, v[3], v[4]);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc(m.tNone)).withBody();
        m.opInit<ConstantOp>(m.tI64, 2);
        m.opInit<ConstantOp>(m.tI64, 3);
        m.opInit<ConstantOp>(m.tI64, 5);
        m.opInit<ConstantOp>(m.tF64, 5.0);
        m.opInit<ConstantOp>(m.tF64, 5.0);
        m.opInit<ConstantOp>(m.tBool, true);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    assertSameOpTree();
}
