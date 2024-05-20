#include <gtest/gtest.h>

#include "compiler/backend/optree/optimizer/optimizer.hpp"
#include "compiler/backend/optree/optimizer/transform_factories.hpp"
#include "compiler/optree/adaptors.hpp"

#include "common.hpp"

using namespace optree;
using namespace optree::optimizer;

class JoinConditionsBranchesTest : public TransformTestBase {
    virtual void setupOptimizer(Optimizer &opt) const override {
        opt.add(createJoinConditionsBranches());
    }

  public:
    JoinConditionsBranchesTest() = default;
    ~JoinConditionsBranchesTest() = default;
};

TEST_F(JoinConditionsBranchesTest, can_run_on_empty_optree) {
    runOptimizer();
    assertSameOpTree();
}

TEST_F(JoinConditionsBranchesTest, can_join_simple_if) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tF64, 4.5);
        v[1] = m.opInit<LogicBinaryOp>(LogicBinOpKind::LessF, v["y"], v[0]);
        m.op<IfOp>(v[1]).withBody();
        m.op<ThenOp>().withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, 123);
        m.endBody();
        m.op<ElseOp>().withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, 123);
        m.endBody();
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tF64, 4.5);
        v[1] = m.opInit<LogicBinaryOp>(LogicBinOpKind::LessF, v["y"], v[0]);
        m.op<IfOp>(v[1]).withBody();
        m.op<ThenOp>().withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, 123);
        m.endBody();
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }

    runOptimizer();
    assertSameOpTree();
}

TEST_F(JoinConditionsBranchesTest, can_join_complex_if) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(8));
        v[1] = m.opInit<LogicBinaryOp>(LogicBinOpKind::LessEqualI, v["x"], v[0]);
        m.op<IfOp>(v[1]).withBody();
        m.op<ThenOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["x"]);
        m.endBody();
        m.op<ElseOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["x"]);
        m.endBody();
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(8));
        v[1] = m.opInit<LogicBinaryOp>(LogicBinOpKind::LessEqualI, v["x"], v[0]);
        m.op<IfOp>(v[1]).withBody();
        m.op<ThenOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["x"]);
        m.endBody();
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }

    runOptimizer();
    assertSameOpTree();
}

TEST_F(JoinConditionsBranchesTest, can_keep_complex_if) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(8));
        v[1] = m.opInit<LogicBinaryOp>(LogicBinOpKind::LessEqualI, v["x"], v[0]);
        m.op<IfOp>(v[1]).withBody();
        m.op<ThenOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["y"]);
        m.endBody();
        m.op<ElseOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["x"]);
        m.endBody();
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(8));
        v[1] = m.opInit<LogicBinaryOp>(LogicBinOpKind::LessEqualI, v["x"], v[0]);
        m.op<IfOp>(v[1]).withBody();
        m.op<ThenOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["y"]);
        m.endBody();
        m.op<ElseOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["x"]);
        m.endBody();
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }

    runOptimizer();
    assertSameOpTree();
}

TEST_F(JoinConditionsBranchesTest, can_join_nested_if) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(8));
        v[1] = m.opInit<LogicBinaryOp>(LogicBinOpKind::LessEqualI, v["x"], v[0]);
        m.op<IfOp>(v[1]).withBody();
        m.op<ThenOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["y"]);

        m.op<IfOp>(v[1]).withBody();
        m.op<ThenOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["y"]);
        m.endBody();
        m.op<ElseOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["y"]);
        m.endBody();
        m.endBody();

        m.endBody();
        m.op<ElseOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["y"]);

        m.op<IfOp>(v[1]).withBody();
        m.op<ThenOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["y"]);
        m.endBody();
        m.op<ElseOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["y"]);
        m.endBody();
        m.endBody();

        m.endBody();
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
                m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(8));
        v[1] = m.opInit<LogicBinaryOp>(LogicBinOpKind::LessEqualI, v["x"], v[0]);
        m.op<IfOp>(v[1]).withBody();
        m.op<ThenOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["y"]);

        m.op<IfOp>(v[1]).withBody();
        m.op<ThenOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["y"]);
        m.endBody();
        m.endBody();

        m.endBody();
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }

    runOptimizer();
    assertSameOpTree();
}

TEST_F(JoinConditionsBranchesTest, can_keep_parent_if) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(8));
        v[1] = m.opInit<LogicBinaryOp>(LogicBinOpKind::LessEqualI, v["x"], v[0]);
        m.op<IfOp>(v[1]).withBody();
        m.op<ThenOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["x"]);

        m.op<IfOp>(v[1]).withBody();
        m.op<ThenOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["y"]);
        m.endBody();
        m.op<ElseOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["y"]);
        m.endBody();
        m.endBody();

        m.endBody();
        m.op<ElseOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["y"]);

        m.op<IfOp>(v[1]).withBody();
        m.op<ThenOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["y"]);
        m.endBody();
        m.op<ElseOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["y"]);
        m.endBody();
        m.endBody();

        m.endBody();
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(8));
        v[1] = m.opInit<LogicBinaryOp>(LogicBinOpKind::LessEqualI, v["x"], v[0]);
        m.op<IfOp>(v[1]).withBody();
        m.op<ThenOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["x"]);

        m.op<IfOp>(v[1]).withBody();
        m.op<ThenOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["y"]);
        m.endBody();
        m.endBody();

        m.endBody();
        m.op<ElseOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["y"]);

        m.op<IfOp>(v[1]).withBody();
        m.op<ThenOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v["y"]);
        m.endBody();
        m.endBody();

        m.endBody();
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }

    runOptimizer();
    assertSameOpTree();
}
