#include <gtest/gtest.h>

#include "compiler/backend/optree/optimizer/optimizer.hpp"
#include "compiler/backend/optree/optimizer/transform_factories.hpp"
#include "compiler/optree/adaptors.hpp"

#include "common.hpp"

using namespace optree;
using namespace optree::optimizer;

class PropagateConstantsTest : public TransformTestBase {
    virtual void setupOptimizer(Optimizer &opt) const override {
        opt.add(createPropagateConstants());
    }

  public:
    PropagateConstantsTest() = default;
    ~PropagateConstantsTest() = default;
};

TEST_F(PropagateConstantsTest, can_run_on_empty_optree) {
    runOptimizer();
    assertSameOpTree();
}

TEST_F(PropagateConstantsTest, simple_propagation) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({}, m.tF64)).withBody();
        v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
        v[1] = m.opInit<ConstantOp>(m.tF64, 4.5);
        m.opInit<StoreOp>(v["z"], v[1]);
        v[8] = m.opInit<LoadOp>(v["z"]);
        v[9] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[8]);
        m.opInit<ReturnOp>(v[9]);
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({}, m.tF64)).withBody();
        v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
        v[1] = m.opInit<ConstantOp>(m.tF64, 4.5);
        m.opInit<StoreOp>(v["z"], v[1]);
        v[8] = m.opInit<LoadOp>(v["z"]);
        v[9] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[1]);
        m.opInit<ReturnOp>(v[9]);
        m.endBody();
    }

    runOptimizer();
    assertSameOpTree();
}

TEST_F(PropagateConstantsTest, propagate_in_down_scope) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({}, m.tF64)).withBody();
        v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
        v[1] = m.opInit<ConstantOp>(m.tF64, 4.5);
        m.opInit<StoreOp>(v["z"], v[1]);
        v[2] = m.opInit<LoadOp>(v["z"]);
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[2]);
        v[4] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualF, v[3], v[1]);
        // clang-format off
        m.op<IfOp>(v[4]).withBody();
            m.op<ThenOp>().withBody();
                v[5] = m.opInit<ConstantOp>(m.tF64, 9.10);
                m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[5], v[2]);
            m.endBody();
            m.op<ElseOp>().withBody();
                v[6] = m.opInit<ConstantOp>(m.tF64, 9.10);
                m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[6], v[2]);
            m.endBody();
        m.endBody();
        // clang-format on
        m.opInit<ReturnOp>(v[3]);
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({}, m.tF64)).withBody();
        v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
        v[1] = m.opInit<ConstantOp>(m.tF64, 4.5);
        m.opInit<StoreOp>(v["z"], v[1]);
        v[2] = m.opInit<LoadOp>(v["z"]);
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[1]);
        v[4] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualF, v[3], v[1]);
        // clang-format off
        m.op<IfOp>(v[4]).withBody();
            m.op<ThenOp>().withBody();
                v[5] = m.opInit<ConstantOp>(m.tF64, 9.10);
                m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[5], v[1]);
            m.endBody();
            m.op<ElseOp>().withBody();
                v[6] = m.opInit<ConstantOp>(m.tF64, 9.10);
                m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[6], v[1]);
            m.endBody();
        m.endBody();
        // clang-format on
        m.opInit<ReturnOp>(v[3]);
        m.endBody();
    }

    runOptimizer();
    assertSameOpTree();
}

TEST_F(PropagateConstantsTest, propagate_in_scope) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({}, m.tF64)).withBody();
        v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
        v[1] = m.opInit<ConstantOp>(m.tF64, 4.5);
        m.opInit<StoreOp>(v["z"], v[1]);
        v[2] = m.opInit<LoadOp>(v["z"]);
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[2]);
        v[4] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualF, v[3], v[1]);
        // clang-format off
        m.op<IfOp>(v[4]).withBody();
            m.op<ThenOp>().withBody();
                v["x"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
                v[5] = m.opInit<ConstantOp>(m.tF64, 9.10);
                m.opInit<StoreOp>(v["x"], v[5]);
                v[6] = m.opInit<LoadOp>(v["x"]);
                v[7] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[5], v[6]);
            m.endBody();
            m.op<ElseOp>().withBody();
                v["y"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
                v[8] = m.opInit<ConstantOp>(m.tF64, 3.14);
                m.opInit<StoreOp>(v["y"], v[8]);
                v[9] = m.opInit<LoadOp>(v["y"]);
                v[10] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[8], v[9]);
            m.endBody();
        m.endBody();
        // clang-format on
        m.opInit<ReturnOp>(v[3]);
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({}, m.tF64)).withBody();
        v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
        v[1] = m.opInit<ConstantOp>(m.tF64, 4.5);
        m.opInit<StoreOp>(v["z"], v[1]);
        v[2] = m.opInit<LoadOp>(v["z"]);
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[1]);
        v[4] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualF, v[3], v[1]);
        // clang-format off
        m.op<IfOp>(v[4]).withBody();
            m.op<ThenOp>().withBody();
                v["x"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
                v[5] = m.opInit<ConstantOp>(m.tF64, 9.10);
                m.opInit<StoreOp>(v["x"], v[5]);
                v[6] = m.opInit<LoadOp>(v["x"]);
                v[7] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[5], v[5]);
            m.endBody();
            m.op<ElseOp>().withBody();
                v["y"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
                v[8] = m.opInit<ConstantOp>(m.tF64, 3.14);
                m.opInit<StoreOp>(v["y"], v[8]);
                v[9] = m.opInit<LoadOp>(v["y"]);
                v[10] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[8], v[8]);
            m.endBody();
        m.endBody();
        // clang-format on
        m.opInit<ReturnOp>(v[3]);
        m.endBody();
    }

    runOptimizer();
    assertSameOpTree();
}

TEST_F(PropagateConstantsTest, invalidate_variable_after_condition) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({}, m.tF64)).withBody();
        v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
        v[1] = m.opInit<ConstantOp>(m.tF64, 4.5);
        m.opInit<StoreOp>(v["z"], v[1]);
        v[2] = m.opInit<LoadOp>(v["z"]);
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[2]);
        v[4] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualF, v[3], v[1]);
        // clang-format off
        m.op<IfOp>(v[4]).withBody();
            m.op<ThenOp>().withBody();
                v[5] = m.opInit<ConstantOp>(m.tF64, 9.10);
                m.opInit<StoreOp>(v["z"], v[5]);
                v[6] = m.opInit<LoadOp>(v["z"]);
                v[7] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[5], v[6]);
            m.endBody();
        m.endBody();
        // clang-format on
        v[8] = m.opInit<LoadOp>(v["z"]);
        v[9] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[8]);
        m.opInit<ReturnOp>(v[3]);
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({}, m.tF64)).withBody();
        v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
        v[1] = m.opInit<ConstantOp>(m.tF64, 4.5);
        m.opInit<StoreOp>(v["z"], v[1]);
        v[2] = m.opInit<LoadOp>(v["z"]);
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[1]);
        v[4] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualF, v[3], v[1]);
        // clang-format off
        m.op<IfOp>(v[4]).withBody();
            m.op<ThenOp>().withBody();
                v[5] = m.opInit<ConstantOp>(m.tF64, 9.10);
                m.opInit<StoreOp>(v["z"], v[5]);
                v[6] = m.opInit<LoadOp>(v["z"]);
                v[7] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[5], v[5]);
            m.endBody();
        m.endBody();
        // clang-format on
        v[8] = m.opInit<LoadOp>(v["z"]);
        v[9] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[8]);
        m.opInit<ReturnOp>(v[3]);
        m.endBody();
    }

    runOptimizer();
    assertSameOpTree();
}

TEST_F(PropagateConstantsTest, independ_propagate_for_if_blocks) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({}, m.tF64)).withBody();
        v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
        v[1] = m.opInit<ConstantOp>(m.tF64, 4.5);
        m.opInit<StoreOp>(v["z"], v[1]);
        v[2] = m.opInit<LoadOp>(v["z"]);
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[2]);
        v[4] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualF, v[3], v[1]);
        // clang-format off
        m.op<IfOp>(v[4]).withBody();
            m.op<ThenOp>().withBody();
                v[5] = m.opInit<ConstantOp>(m.tF64, 9.10);
                m.opInit<StoreOp>(v["z"], v[5]);
                v[6] = m.opInit<LoadOp>(v["z"]);
                v[7] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[5], v[6]);
            m.endBody();
            m.op<ElseOp>().withBody();
                v[8] = m.opInit<LoadOp>(v["z"]);
                v[9] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[1], v[8]);
            m.endBody();
        m.endBody();
        // clang-format on
        v[10] = m.opInit<LoadOp>(v["z"]);
        v[11] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[8]);
        m.opInit<ReturnOp>(v[3]);
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({}, m.tF64)).withBody();
        v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
        v[1] = m.opInit<ConstantOp>(m.tF64, 4.5);
        m.opInit<StoreOp>(v["z"], v[1]);
        v[2] = m.opInit<LoadOp>(v["z"]);
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[1]);
        v[4] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualF, v[3], v[1]);
        // clang-format off
        m.op<IfOp>(v[4]).withBody();
            m.op<ThenOp>().withBody();
                v[5] = m.opInit<ConstantOp>(m.tF64, 9.10);
                m.opInit<StoreOp>(v["z"], v[5]);
                v[6] = m.opInit<LoadOp>(v["z"]);
                v[7] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[5], v[5]);
            m.endBody();
            m.op<ElseOp>().withBody();
                v[8] = m.opInit<LoadOp>(v["z"]);
                v[9] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[1], v[1]);
            m.endBody();
        m.endBody();
        // clang-format on
        v[10] = m.opInit<LoadOp>(v["z"]);
        v[11] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[1]);
        m.opInit<ReturnOp>(v[3]);
        m.endBody();
    }

    runOptimizer();
    assertSameOpTree();
}

TEST_F(PropagateConstantsTest, propagate_variable_in_scopes) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({}, m.tF64)).withBody();
        v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
        v[1] = m.opInit<ConstantOp>(m.tF64, 4.5);
        m.opInit<StoreOp>(v["z"], v[1]);
        v[2] = m.opInit<LoadOp>(v["z"]);
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[2]);
        v[4] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualF, v[3], v[1]);
        // clang-format off
        m.op<IfOp>(v[4]).withBody();
            m.op<ThenOp>().withBody();
                v[5] = m.opInit<ConstantOp>(m.tF64, 9.10);
                v[6] = m.opInit<LoadOp>(v["z"]);
                v[7] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[5], v[6]);
            m.endBody();
        m.endBody();
        // clang-format on
        v[8] = m.opInit<LoadOp>(v["z"]);
        v[9] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[8]);
        m.opInit<ReturnOp>(v[3]);
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({}, m.tF64)).withBody();
        v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
        v[1] = m.opInit<ConstantOp>(m.tF64, 4.5);
        m.opInit<StoreOp>(v["z"], v[1]);
        v[2] = m.opInit<LoadOp>(v["z"]);
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[1]);
        v[4] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualF, v[3], v[1]);
        // clang-format off
        m.op<IfOp>(v[4]).withBody();
            m.op<ThenOp>().withBody();
                v[5] = m.opInit<ConstantOp>(m.tF64, 9.10);
                v[6] = m.opInit<LoadOp>(v["z"]);
                v[7] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[5], v[1]);
            m.endBody();
        m.endBody();
        // clang-format on
        v[8] = m.opInit<LoadOp>(v["z"]);
        v[9] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[1]);
        m.opInit<ReturnOp>(v[3]);
        m.endBody();
    }

    runOptimizer();
    assertSameOpTree();
}

TEST_F(PropagateConstantsTest, propagate_variable_with_while) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({}, m.tF64)).withBody();
        v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
        v[1] = m.opInit<ConstantOp>(m.tF64, 4.5);
        m.opInit<StoreOp>(v["z"], v[1]);
        v[2] = m.opInit<LoadOp>(v["z"]);
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[2]);
        // clang-format off
        m.op<WhileOp>().withBody();
            m.op<ConditionOp>().withBody();
                v[4] = m.opInit<LoadOp>(v["z"]);
                v[5] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualF, v[4], v[1]);
            m.endBody();
            v[6] = m.opInit<ConstantOp>(m.tF64, 312.321);
            m.opInit<StoreOp>(v["z"], v[6]);
            v[8] = m.opInit<LoadOp>(v["z"]);
            v[9] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v[8], v[8]);
        m.endBody();
        // clang-format on
        v[10] = m.opInit<LoadOp>(v["z"]);
        v[11] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[10]);
        m.opInit<ReturnOp>(v[3]);
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({}, m.tF64)).withBody();
        v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
        v[1] = m.opInit<ConstantOp>(m.tF64, 4.5);
        m.opInit<StoreOp>(v["z"], v[1]);
        v[2] = m.opInit<LoadOp>(v["z"]);
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[1]);
        // clang-format off
        m.op<WhileOp>().withBody();
            m.op<ConditionOp>().withBody();
                v[4] = m.opInit<LoadOp>(v["z"]);
                v[5] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualF, v[4], v[1]);
            m.endBody();
            v[6] = m.opInit<ConstantOp>(m.tF64, 312.321);
            m.opInit<StoreOp>(v["z"], v[6]);
            v[8] = m.opInit<LoadOp>(v["z"]);
            v[9] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v[6], v[6]);
        m.endBody();
        // clang-format on
        v[10] = m.opInit<LoadOp>(v["z"]);
        v[11] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[10]);
        m.opInit<ReturnOp>(v[3]);
        m.endBody();
    }

    runOptimizer();
    assertSameOpTree();
}

TEST_F(PropagateConstantsTest, propagate_variable_with_for) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({}, m.tF64)).withBody();
        v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
        v[1] = m.opInit<ConstantOp>(m.tF64, 4.5);
        m.opInit<StoreOp>(v["z"], v[1]);
        v[2] = m.opInit<LoadOp>(v["z"]);
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[2]);
        // clang-format off
        m.op<ForOp>().withBody();
            v[6] = m.opInit<ConstantOp>(m.tF64, 312.321);
            m.opInit<StoreOp>(v["z"], v[6]);
            v[8] = m.opInit<LoadOp>(v["z"]);
            v[9] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v[8], v[8]);
        m.endBody();
        // clang-format on
        v[10] = m.opInit<LoadOp>(v["z"]);
        v[11] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[10]);
        m.opInit<ReturnOp>(v[3]);
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({}, m.tF64)).withBody();
        v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
        v[1] = m.opInit<ConstantOp>(m.tF64, 4.5);
        m.opInit<StoreOp>(v["z"], v[1]);
        v[2] = m.opInit<LoadOp>(v["z"]);
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[1]);
        // clang-format off
        m.op<ForOp>().withBody();
            v[6] = m.opInit<ConstantOp>(m.tF64, 312.321);
            m.opInit<StoreOp>(v["z"], v[6]);
            v[8] = m.opInit<LoadOp>(v["z"]);
            v[9] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v[6], v[6]);
        m.endBody();
        // clang-format on
        v[10] = m.opInit<LoadOp>(v["z"]);
        v[11] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[10]);
        m.opInit<ReturnOp>(v[3]);
        m.endBody();
    }

    runOptimizer();
    assertSameOpTree();
}

TEST_F(PropagateConstantsTest, propagate_into_while) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({}, m.tF64)).withBody();
        v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
        v[1] = m.opInit<ConstantOp>(m.tF64, 4.5);
        m.opInit<StoreOp>(v["z"], v[1]);
        v[2] = m.opInit<LoadOp>(v["z"]);
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[2]);
        // clang-format off
        m.op<WhileOp>().withBody();
            m.op<ConditionOp>().withBody();
                v[4] = m.opInit<LoadOp>(v["z"]);
                v[5] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualF, v[4], v[1]);
            m.endBody();
            v[8] = m.opInit<LoadOp>(v["z"]);
            v[9] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v[8], v[8]);
        m.endBody();
        // clang-format on
        v[10] = m.opInit<LoadOp>(v["z"]);
        v[11] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[10]);
        m.opInit<ReturnOp>(v[3]);
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({}, m.tF64)).withBody();
        v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
        v[1] = m.opInit<ConstantOp>(m.tF64, 4.5);
        m.opInit<StoreOp>(v["z"], v[1]);
        v[2] = m.opInit<LoadOp>(v["z"]);
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[1]);
        // clang-format off
        m.op<WhileOp>().withBody();
            m.op<ConditionOp>().withBody();
                v[4] = m.opInit<LoadOp>(v["z"]);
                v[5] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualF, v[1], v[1]);
            m.endBody();
            v[8] = m.opInit<LoadOp>(v["z"]);
            v[9] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v[1], v[1]);
        m.endBody();
        // clang-format on
        v[10] = m.opInit<LoadOp>(v["z"]);
        v[11] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[1]);
        m.opInit<ReturnOp>(v[3]);
        m.endBody();
    }

    runOptimizer();
    assertSameOpTree();
}

TEST_F(PropagateConstantsTest, propagate_into_while_if) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({}, m.tF64)).withBody();
        v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
        v[1] = m.opInit<ConstantOp>(m.tF64, 4.5);
        m.opInit<StoreOp>(v["z"], v[1]);
        v[2] = m.opInit<LoadOp>(v["z"]);
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[2]);
        // clang-format off
        m.op<WhileOp>().withBody();
            m.op<ConditionOp>().withBody();
                v[4] = m.opInit<LoadOp>(v["z"]);
                v[5] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualF, v[4], v[1]);
            m.endBody();
            m.op<IfOp>(v[1]).withBody();
                m.op<ThenOp>().withBody();
                    v[6] = m.opInit<ConstantOp>(m.tF64, 9.10);
                    m.opInit<StoreOp>(v["z"], v[6]);
                    v[8] = m.opInit<LoadOp>(v["z"]);
                    v[9] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[8], v[6]);
                m.endBody();
                m.op<ElseOp>().withBody();
                    v[10] = m.opInit<LoadOp>(v["z"]);
                    v[11] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[1], v[10]);
                m.endBody();
            m.endBody();
        m.endBody();
        // clang-format on
        v[12] = m.opInit<LoadOp>(v["z"]);
        v[13] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[12]);
        m.opInit<ReturnOp>(v[3]);
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({}, m.tF64)).withBody();
        v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
        v[1] = m.opInit<ConstantOp>(m.tF64, 4.5);
        m.opInit<StoreOp>(v["z"], v[1]);
        v[2] = m.opInit<LoadOp>(v["z"]);
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[1]);
        // clang-format off
        m.op<WhileOp>().withBody();
            m.op<ConditionOp>().withBody();
                v[4] = m.opInit<LoadOp>(v["z"]);
                v[5] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualF, v[4], v[1]);
            m.endBody();
            m.op<IfOp>(v[1]).withBody();
                m.op<ThenOp>().withBody();
                    v[6] = m.opInit<ConstantOp>(m.tF64, 9.10);
                    m.opInit<StoreOp>(v["z"], v[6]);
                    v[8] = m.opInit<LoadOp>(v["z"]);
                    v[9] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[6], v[6]);
                m.endBody();
                m.op<ElseOp>().withBody();
                    v[10] = m.opInit<LoadOp>(v["z"]);
                    v[11] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[1], v[10]);
                m.endBody();
            m.endBody();
        m.endBody();
        // clang-format on
        v[12] = m.opInit<LoadOp>(v["z"]);
        v[13] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[12]);
        m.opInit<ReturnOp>(v[3]);
        m.endBody();
    }

    runOptimizer();
    assertSameOpTree();
}

TEST_F(PropagateConstantsTest, propagate_into_while_if_store_in_else) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("test", m.tFunc({}, m.tF64)).withBody();
        v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
        v[1] = m.opInit<ConstantOp>(m.tF64, 4.5);
        m.opInit<StoreOp>(v["z"], v[1]);
        v[2] = m.opInit<LoadOp>(v["z"]);
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[2]);
        // clang-format off
        m.op<WhileOp>().withBody();
            m.op<ConditionOp>().withBody();
                v[4] = m.opInit<LoadOp>(v["z"]);
                v[5] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualF, v[4], v[1]);
            m.endBody();
            m.op<IfOp>(v[1]).withBody();
                m.op<ThenOp>().withBody();
                    v[6] = m.opInit<LoadOp>(v["z"]);
                    v[7] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[1], v[6]);
                m.endBody();
                m.op<ElseOp>().withBody();
                    v[8] = m.opInit<ConstantOp>(m.tF64, 9.10);
                    m.opInit<StoreOp>(v["z"], v[8]);
                    v[10] = m.opInit<LoadOp>(v["z"]);
                    v[11] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[1], v[10]);
                m.endBody();
            m.endBody();
        m.endBody();
        // clang-format on
        v[12] = m.opInit<LoadOp>(v["z"]);
        v[13] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[12]);
        m.opInit<ReturnOp>(v[3]);
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("test", m.tFunc({}, m.tF64)).withBody();
        v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
        v[1] = m.opInit<ConstantOp>(m.tF64, 4.5);
        m.opInit<StoreOp>(v["z"], v[1]);
        v[2] = m.opInit<LoadOp>(v["z"]);
        v[3] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[1]);
        // clang-format off
        m.op<WhileOp>().withBody();
            m.op<ConditionOp>().withBody();
                v[4] = m.opInit<LoadOp>(v["z"]);
                v[5] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualF, v[4], v[1]);
            m.endBody();
            m.op<IfOp>(v[1]).withBody();
                m.op<ThenOp>().withBody();
                    v[6] = m.opInit<LoadOp>(v["z"]);
                    v[7] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[1], v[6]);
                m.endBody();
                m.op<ElseOp>().withBody();
                    v[8] = m.opInit<ConstantOp>(m.tF64, 9.10);
                    m.opInit<StoreOp>(v["z"], v[8]);
                    v[10] = m.opInit<LoadOp>(v["z"]);
                    v[11] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[1], v[8]);
                m.endBody();
            m.endBody();
        m.endBody();
        // clang-format on
        v[12] = m.opInit<LoadOp>(v["z"]);
        v[13] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[1], v[12]);
        m.opInit<ReturnOp>(v[3]);
        m.endBody();
    }

    runOptimizer();
    assertSameOpTree();
}
