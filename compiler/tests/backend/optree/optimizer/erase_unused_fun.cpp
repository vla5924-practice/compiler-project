#include <gtest/gtest.h>

#include <utility>

#include "compiler/backend/optree/optimizer/optimizer.hpp"
#include "compiler/backend/optree/optimizer/transform_factories.hpp"
#include "compiler/optree/adaptors.hpp"

#include "common.hpp"

using namespace optree;
using namespace optree::optimizer;

class EraseUnusedFunTest : public TransformTestBase {
    virtual void setupOptimizer(Optimizer &opt) const override {
        opt.add(createEraseUnusedFunctions());
    }

  public:
    EraseUnusedFunTest() = default;
    ~EraseUnusedFunTest() = default;
};

TEST_F(EraseUnusedFunTest, can_run_on_empty_optree) {
    runOptimizer();
    assertSameOpTree();
}

TEST_F(EraseUnusedFunTest, asasas) {
   {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("main", m.tFunc({}, m.tNone)).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(123));
        v[1] = m.opInit<FunctionCallOp>("test3", m.tNone, std::vector<Value::Ptr>());
        m.opInit<ReturnOp>();
        m.endBody();
        m.opInit<FunctionOp>("aaaa", m.tFunc({m.tI64, m.tI64}, m.tNone)).inward(v[2], 0).inward(v[3], 1).withBody();
        v[4] = m.opInit<ConstantOp>(m.tI64, int64_t(123));
        v[5] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v[2], v[3]);
        m.opInit<ReturnOp>();
        m.endBody();
        m.opInit<FunctionOp>("test3", m.tFunc({}, m.tNone)).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(123));
        v[1] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v[0], v[0]);
        v[2] = m.opInit<ArithCastOp>(ArithCastOpKind::IntToFloat, m.tF64, v[1]);
        v[3] = m.opInit<LogicBinaryOp>(LogicBinOpKind::LessEqualI, v[0], v[1]);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("main", m.tFunc({}, m.tNone)).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(123));
        v[1] = m.opInit<FunctionCallOp>("test3", m.tNone, std::vector<Value::Ptr>());
        m.opInit<ReturnOp>();
        m.endBody();
        m.opInit<FunctionOp>("test3", m.tFunc({}, m.tNone)).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(123));
        v[1] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v[0], v[0]);
        v[2] = m.opInit<ArithCastOp>(ArithCastOpKind::IntToFloat, m.tF64, v[1]);
        v[3] = m.opInit<LogicBinaryOp>(LogicBinOpKind::LessEqualI, v[0], v[1]);
        m.opInit<ReturnOp>();
        m.endBody();
    }

    runOptimizer();
    assertSameOpTree();
}