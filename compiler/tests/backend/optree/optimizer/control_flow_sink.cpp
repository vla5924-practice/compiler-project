#include <gtest/gtest.h>

#include "compiler/backend/optree/optimizer/optimizer.hpp"
#include "compiler/backend/optree/optimizer/transform_factories.hpp"
#include "compiler/optree/adaptors.hpp"

#include "common.hpp"

using namespace optree;
using namespace optree::optimizer;

class ControlFlowSinkOpsTest : public TransformTestBase {
    virtual void setupOptimizer(Optimizer &opt) const override {
        auto transform = CascadeTransform::make("ControlFlowSinkOpsTest");
        transform->add(createControlFlowSinkOps());
        opt.add(transform);
    }

  public:
    ControlFlowSinkOpsTest() = default;
    ~ControlFlowSinkOpsTest() = default;
};

TEST_F(ControlFlowSinkOpsTest, can_run_on_empty_optree) {
    runOptimizer();
    assertSameOpTree();
}

TEST_F(ControlFlowSinkOpsTest, can_move_operator_to_then) {
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
    auto &&[m, v] = getActual();
    m.dump(std::cout);
    assertSameOpTree();
}

TEST_F(ControlFlowSinkOpsTest, can_keep_operation_using_in_base_region) {
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
    auto &&[m, v] = getActual();
    m.dump(std::cout);
    assertSameOpTree();
}

TEST_F(ControlFlowSinkOpsTest, aaa2) {
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
    auto &&[m, v] = getActual();
    m.dump(std::cout);
    assertSameOpTree();
}

TEST_F(ControlFlowSinkOpsTest, aaa3) {
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
                v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[0]);
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
                v[2] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
            m.endBody();
            m.op<ElseOp>().withBody();
                v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[0]);
            m.endBody();
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    auto &&[m, v] = getActual();
    m.dump(std::cout);
    assertSameOpTree();
}

