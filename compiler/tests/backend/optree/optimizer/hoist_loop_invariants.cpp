#include <gtest/gtest.h>

#include "compiler/backend/optree/optimizer/optimizer.hpp"
#include "compiler/backend/optree/optimizer/transform_factories.hpp"
#include "compiler/optree/adaptors.hpp"

#include "common.hpp"

using namespace optree;
using namespace optree::optimizer;

class HoistLoopInvariantsTest : public TransformTestBase {
    virtual void setupOptimizer(Optimizer &opt) const override {
        auto transform = CascadeTransform::make("HoistLoopInvariantsTest");
        transform->add(createHoistLoopInvariants());
        opt.add(transform);
    }

  public:
    HoistLoopInvariantsTest() = default;
    ~HoistLoopInvariantsTest() = default;
};

TEST_F(HoistLoopInvariantsTest, can_run_on_empty_optree) {
    runOptimizer();
    assertSameOpTree();
}

TEST_F(HoistLoopInvariantsTest, can_hoist_loop_invariants) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(1));
        m.op<WhileOp>().withBody();
        m.op<ConditionOp>().withBody();
        v[1] = m.opInit<LogicBinaryOp>(LogicBinOpKind::NotEqual, v["x"], v[0]);
        m.endBody();
        v[2] = m.opInit<ConstantOp>(m.tI64, int64_t(2));
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v["x"], v[2]);
        m.opInit<StoreOp>(v["x"], v[3]);
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(1));
        v[2] = m.opInit<ConstantOp>(m.tI64, int64_t(2));
        m.op<WhileOp>().withBody();
        m.op<ConditionOp>().withBody();
        v[1] = m.opInit<LogicBinaryOp>(LogicBinOpKind::NotEqual, v["x"], v[0]);
        m.endBody();
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v["x"], v[2]);
        m.opInit<StoreOp>(v["x"], v[3]);
        m.endBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    assertSameOpTree();
}
