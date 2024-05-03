#include <gtest/gtest.h>

#include <utility>

#include "compiler/backend/optree/optimizer/optimizer.hpp"
#include "compiler/backend/optree/optimizer/transform_factories.hpp"
#include "compiler/optree/adaptors.hpp"

#include "common.hpp"

using namespace optree;
using namespace optree::optimizer;

class EraseUnusedOpsTest : public TransformTestBase {
    virtual void setupOptimizer(Optimizer &opt) const override {
        opt.add(createEraseUnusedOps());
    }

  public:
    EraseUnusedOpsTest() = default;
    ~EraseUnusedOpsTest() = default;
};

TEST_F(EraseUnusedOpsTest, can_run_on_empty_optree) {
    runOptimizer();
    assertSameOpTree();
}

TEST_F(EraseUnusedOpsTest, can_erase_unused_ops) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tI64}, m.tNone)).inward(v[0], 0).inward(v[1], 1).withBody();
        v[2] = m.opInit<ConstantOp>(m.tI64, int64_t(123));
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v[0], v[1]);
        v[4] = m.opInit<ArithCastOp>(ArithCastOpKind::IntToFloat, m.tF64, v[0]);
        v[5] = m.opInit<LogicBinaryOp>(LogicBinOpKind::LessEqualI, v[0], v[1]);
        v[6] = m.opInit<LogicUnaryOp>(LogicUnaryOpKind::Not, v[0]);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tI64}, m.tNone)).inward(v[0], 0).inward(v[1], 1).withBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    assertSameOpTree();
}

TEST_F(EraseUnusedOpsTest, can_erase_chain_of_unused_ops) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64}, m.tNone)).inward(v[0], 0).withBody();
        v[1] = m.opInit<ConstantOp>(m.tI64, int64_t(123));
        v[2] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v[0], v[1]);
        v[3] = m.opInit<ArithCastOp>(ArithCastOpKind::IntToFloat, m.tF64, v[2]);
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({m.tI64}, m.tNone)).inward(v[0], 0).withBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    runOptimizer();
    assertSameOpTree();
}

TEST_F(EraseUnusedOpsTest, can_keep_used_ops) {
    auto &&[m, v] = getActual();

    m.opInit<FunctionOp>("test", m.tFunc({m.tI64, m.tI64}, m.tNone)).inward(v[0], 0).inward(v[1], 1).withBody();
    v[2] = m.opInit<ConstantOp>(m.tI64, int64_t(123));
    v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v[0], v[1]);
    v[4] = m.opInit<ArithCastOp>(ArithCastOpKind::IntToFloat, m.tF64, v[0]);
    v[5] = m.opInit<LogicBinaryOp>(LogicBinOpKind::LessEqualI, v[0], v[1]);
    v[6] = m.opInit<LogicUnaryOp>(LogicUnaryOpKind::Not, v[0]);
    m.opInit<PrintOp>(v[2]);
    m.opInit<PrintOp>(v[3]);
    m.opInit<PrintOp>(v[4]);
    m.opInit<PrintOp>(v[5]);
    m.opInit<PrintOp>(v[6]);
    m.opInit<ReturnOp>();
    m.endBody();

    saveActualAsExpected();
    runOptimizer();
    assertSameOpTree();
}

TEST_F(EraseUnusedOpsTest, can_keep_irrelevant_ops) {
    auto &&[m, v] = getActual();

    m.opInit<FunctionOp>("test", m.tFunc({m.tPtr(m.tI64)}, m.tNone)).inward(v[0], 0).withBody();
    v[1] = m.opInit<AllocateOp>(m.tPtr(m.tI64));
    v[2] = m.opInit<LoadOp>(v[0]);
    v[3] = m.opInit<InputOp>(m.tI64);
    m.opInit<ReturnOp>();
    m.endBody();

    saveActualAsExpected();
    runOptimizer();
    assertSameOpTree();
}
