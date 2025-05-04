#include <gtest/gtest.h>

#include "compiler/backend/optree/optimizer/optimizer.hpp"
#include "compiler/backend/optree/optimizer/transform_factories.hpp"
#include "compiler/optree/adaptors.hpp"

#include "common.hpp"

using namespace optree;
using namespace optree::optimizer;

class SinkControlFlowOpsTest : public TransformTestBase {
    virtual void setupOptimizer(Optimizer &opt) const override {
        auto transform = CascadeTransform::make("SinkControlFlowOpsTest");
        transform->add(createSinkControlFlowOps());
        opt.add(transform);
    }

  public:
    SinkControlFlowOpsTest() = default;
    ~SinkControlFlowOpsTest() = default;
};

TEST_F(SinkControlFlowOpsTest, can_run_on_empty_optree) {
    runOptimizer();
    assertSameOpTree();
}

// clang-format off
TEST_F(SinkControlFlowOpsTest, can_move_operator_to_then) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tBool, true);
        v[1] = m.opInit<ConstantOp>(m.tF64, 2.3);
        m.op<IfOp>(v[0]).withBody();
            m.op<ThenOp>().withBody();
                m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
            m.endBody();
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tBool, true);
        m.op<IfOp>(v[0]).withBody();
            m.op<ThenOp>().withBody();
                v[1] = m.opInit<ConstantOp>(m.tF64, 2.3);
                m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
            m.endBody();
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    assertSameOpTree();
}

TEST_F(SinkControlFlowOpsTest, can_keep_operation_using_in_base_region) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tBool, true);
        v[1] = m.opInit<ConstantOp>(m.tF64, 2.3);
        m.op<IfOp>(v[0]).withBody();
            m.op<ThenOp>().withBody();
                v[2] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
            m.endBody();
        m.endBody();
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tBool, true);
        v[1] = m.opInit<ConstantOp>(m.tF64, 2.3);

        m.op<IfOp>(v[0]).withBody();
            m.op<ThenOp>().withBody();
                v[2] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
            m.endBody();
        m.endBody();
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);

        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    assertSameOpTree();
}

TEST_F(SinkControlFlowOpsTest, cannot_move_operation_to_then_else_regions) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tBool, true);
        v[1] = m.opInit<ConstantOp>(m.tF64, 2.3);

        m.op<IfOp>(v[0]).withBody();
            m.op<ThenOp>().withBody();
                v[2] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
            m.endBody();
            m.op<ElseOp>().withBody();
                v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
            m.endBody();
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tBool, true);
        v[1] = m.opInit<ConstantOp>(m.tF64, 2.3);
        m.op<IfOp>(v[0]).withBody();
            m.op<ThenOp>().withBody();
                v[2] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
            m.endBody();
            m.op<ElseOp>().withBody();
                v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
            m.endBody();
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    assertSameOpTree();
}

TEST_F(SinkControlFlowOpsTest, can_move_to_then_else_diff_operations) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tBool, true);
        v[1] = m.opInit<ConstantOp>(m.tF64, 2.3);
        v[2] = m.opInit<ConstantOp>(m.tF64, 2.3);

        m.op<IfOp>(v[0]).withBody();
            m.op<ThenOp>().withBody();
                v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
            m.endBody();
            m.op<ElseOp>().withBody();
                v[4] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[2]);
            m.endBody();
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tBool, true);
        
        m.op<IfOp>(v[0]).withBody();
            m.op<ThenOp>().withBody();
                v[1] = m.opInit<ConstantOp>(m.tF64, 2.3);
                v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
            m.endBody();
            m.op<ElseOp>().withBody();
                v[2] = m.opInit<ConstantOp>(m.tF64, 2.3);
                v[4] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[2]);
            m.endBody();
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    assertSameOpTree();
}

TEST_F(SinkControlFlowOpsTest, chain_moving) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tBool, true);
        v[1] = m.opInit<ConstantOp>(m.tF64, 2.3);
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
        m.op<IfOp>(v[0]).withBody();
            m.op<ThenOp>().withBody();
                v[4] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[3]);
            m.endBody();
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tBool, true);
        m.op<IfOp>(v[0]).withBody();
            m.op<ThenOp>().withBody();
                v[1] = m.opInit<ConstantOp>(m.tF64, 2.3);
                v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
                v[4] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[3]);
            m.endBody();
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    assertSameOpTree();
}

TEST_F(SinkControlFlowOpsTest, can_move_operators_in_inner_regions) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tBool, true);
        m.op<IfOp>(v[0]).withBody();
            m.op<ThenOp>().withBody();
                v[1] = m.opInit<ConstantOp>(m.tF64, 2.3);
                m.op<IfOp>(v[0]).withBody();
                    m.op<ThenOp>().withBody();
                        v[2] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
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
        v[0] = m.opInit<ConstantOp>(m.tBool, true);
        m.op<IfOp>(v[0]).withBody();
            m.op<ThenOp>().withBody();
                m.op<IfOp>(v[0]).withBody();
                    m.op<ThenOp>().withBody();
                        v[1] = m.opInit<ConstantOp>(m.tF64, 2.3);
                        v[2] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
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

TEST_F(SinkControlFlowOpsTest, skip_while) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(2));
        v[1] = m.opInit<ConstantOp>(m.tI64, int64_t(2));
        m.op<WhileOp>().withBody();
            m.op<ConditionOp>().withBody();
                v[3] = m.opInit<LogicBinaryOp>(LogicBinOpKind::NotEqual, v[0], v[0]);
            m.endBody();
            v[4] = m.opInit<ArithBinaryOp>(ArithBinOpKind::MulI, v[1], v["x"]);
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(2));
        v[1] = m.opInit<ConstantOp>(m.tI64, int64_t(2));
        m.op<WhileOp>().withBody();
            m.op<ConditionOp>().withBody();
                v[3] = m.opInit<LogicBinaryOp>(LogicBinOpKind::NotEqual, v[0], v[0]);
            m.endBody();
            v[4] = m.opInit<ArithBinaryOp>(ArithBinOpKind::MulI, v[1], v["x"]);
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    assertSameOpTree();
}

TEST_F(SinkControlFlowOpsTest, two_ifs) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tBool, true);
        v[1] = m.opInit<ConstantOp>(m.tF64, 2.3);
        m.op<IfOp>(v[0]).withBody();
            m.op<ThenOp>().withBody();
                m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
            m.endBody();
        m.endBody();
        m.op<IfOp>(v[0]).withBody();
            m.op<ThenOp>().withBody();
                m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
            m.endBody();
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tBool, true);
        v[1] = m.opInit<ConstantOp>(m.tF64, 2.3);
        m.op<IfOp>(v[0]).withBody();
            m.op<ThenOp>().withBody();
                m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
            m.endBody();
        m.endBody();
        m.op<IfOp>(v[0]).withBody();
            m.op<ThenOp>().withBody();
                m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
            m.endBody();
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    assertSameOpTree();
}
// clang-format on
