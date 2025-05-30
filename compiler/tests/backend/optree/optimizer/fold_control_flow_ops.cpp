#include <gtest/gtest.h>

#include "compiler/backend/optree/optimizer/optimizer.hpp"
#include "compiler/backend/optree/optimizer/transform_factories.hpp"
#include "compiler/optree/adaptors.hpp"

#include "common.hpp"

using namespace optree;
using namespace optree::optimizer;

class FoldControlFlowTest : public TransformTestBase {
    virtual void setupOptimizer(Optimizer &opt) const override {
        auto transform = CascadeTransform::make("FoldControlFlowTest");
        transform->add(createFoldControlFlowOps());
        transform->add(createFoldConstants());
        opt.add(transform);
    }

  public:
    FoldControlFlowTest() = default;
    ~FoldControlFlowTest() = default;
};

TEST_F(FoldControlFlowTest, can_run_on_empty_optree) {
    runOptimizer();
    assertSameOpTree();
}

TEST_F(FoldControlFlowTest, can_fold_control_flow_ops_if_true) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tBool, true);
        m.op<IfOp>(v[0]).withBody();
        m.op<ThenOp>().withBody();
        v[1] = m.opInit<ConstantOp>(m.tF64, 2.3);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
        m.endBody();
        m.op<ElseOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tI64, int64_t(2));
        m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v["x"], v[2]);
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
        m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    assertSameOpTree();
}

TEST_F(FoldControlFlowTest, can_fold_control_flow_ops_if_false) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tBool, false);
        m.op<IfOp>(v[0]).withBody();
        m.op<ThenOp>().withBody();
        v[1] = m.opInit<ConstantOp>(m.tF64, 2.3);
        m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[1]);
        m.endBody();
        m.op<ElseOp>().withBody();
        v[2] = m.opInit<ConstantOp>(m.tI64, int64_t(2));
        m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v["x"], v[2]);
        m.endBody();
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tBool, false);
        v[2] = m.opInit<ConstantOp>(m.tI64, int64_t(2));
        m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v["x"], v[2]);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    assertSameOpTree();
}

TEST_F(FoldControlFlowTest, can_fold_control_flow_ops_while_false) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(2));
        m.opInit<ArithBinaryOp>(ArithBinOpKind::MulI, v[0], v["x"]);
        m.op<WhileOp>().withBody();
        m.op<ConditionOp>().withBody();
        v[1] = m.opInit<LogicBinaryOp>(LogicBinOpKind::NotEqual, v[0], v[0]);
        m.endBody();
        v[2] = m.opInit<ConstantOp>(m.tI64, int64_t(2));
        m.opInit<ArithBinaryOp>(ArithBinOpKind::MulI, v[2], v["x"]);
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(2));
        m.opInit<ArithBinaryOp>(ArithBinOpKind::MulI, v[0], v["x"]);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    assertSameOpTree();
}
