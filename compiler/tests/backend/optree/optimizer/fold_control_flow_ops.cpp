#include <gtest/gtest.h>

#include <utility>

#include "compiler/backend/optree/optimizer/optimizer.hpp"
#include "compiler/backend/optree/optimizer/transform_factories.hpp"
#include "compiler/optree/adaptors.hpp"

#include "common.hpp"

using namespace optree;
using namespace optree::optimizer;

class FoldControlFlowTest : public TransformTestBase {
    virtual void setupOptimizer(Optimizer &opt) const override {
        opt.add(createFoldControlFlowOps());
    }

  public:
    FoldControlFlowTest() = default;
    ~FoldControlFlowTest() = default;
};

TEST_F(FoldControlFlowTest, can_run_on_empty_optree) {
    runOptimizer();
    assertSameOpTree();
}

TEST_F(FoldControlFlowTest, can_fold_control_flow_ops) {
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
