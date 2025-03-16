#include <gtest/gtest.h>

#include <utility>

#include "compiler/backend/optree/optimizer/optimizer.hpp"
#include "compiler/backend/optree/optimizer/transform_factories.hpp"
#include "compiler/optree/adaptors.hpp"

#include "common.hpp"

using namespace optree;
using namespace optree::optimizer;

class BooleanExpressionMinimizationTest : public TransformTestBase {
    virtual void setupOptimizer(Optimizer &opt) const override {
        opt.add(createBooleanExpressionMinimization());
    }

  public:
    BooleanExpressionMinimizationTest() = default;
    ~BooleanExpressionMinimizationTest() = default;
};

TEST_F(BooleanExpressionMinimizationTest, can_run_on_empty_optree) {
    runOptimizer();
    assertSameOpTree();
}

TEST_F(BooleanExpressionMinimizationTest, minimize_or) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tI64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[2] = m.opInit<ConstantOp>(m.tBool, false);
        v[3] = m.opInit<ConstantOp>(m.tBool, true);
        v[4] = m.opInit<LogicBinaryOp>(LogicBinOpKind::OrI, v["x"], v["y"]);
        v[5] = m.opInit<LogicBinaryOp>(LogicBinOpKind::OrI, v["x"], v["x"]);
        v[6] = m.opInit<LogicBinaryOp>(LogicBinOpKind::OrI, v[5], v["y"]);
        v[7] = m.opInit<LogicBinaryOp>(LogicBinOpKind::OrI, v[3], v["x"]);
        v[8] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterI, v[7], v["x"]);


        // v[4] = m.opInit<LogicBinaryOp>(LogicBinOpKind::OrI, v["y"], v["y"]);
        // v[5] = m.opInit<LogicBinaryOp>(LogicBinOpKind::OrI, v[2], v["x"]);
        // v[6] = m.opInit<LogicBinaryOp>(LogicBinOpKind::OrI, v[3], v["x"]);
        // v[7] = m.opInit<LogicBinaryOp>(LogicBinOpKind::OrI, v["x"], v[2]);
        // v[8] = m.opInit<LogicBinaryOp>(LogicBinOpKind::OrI, v["x"], v[3]);
        m.opInit<ReturnOp>();
        m.endBody();
        m.dump(std::cout);
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tI64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
        v[2] = m.opInit<ConstantOp>(m.tBool, false);
        v[3] = m.opInit<ConstantOp>(m.tBool, true);
        v[4] = m.opInit<LogicBinaryOp>(LogicBinOpKind::OrI, v["x"], v["y"]);
        v[6] = m.opInit<LogicBinaryOp>(LogicBinOpKind::OrI, v["x"], v["y"]);
        v[7] = m.opInit<ConstantOp>(m.tBool, true);
        v[8] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterI, v[7], v["x"]);

        m.opInit<ReturnOp>();
        m.endBody();
        m.dump(std::cout);

    }
    runOptimizer();
    assertSameOpTree();
}
