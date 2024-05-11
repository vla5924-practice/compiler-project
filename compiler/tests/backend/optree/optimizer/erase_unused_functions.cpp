#include <gtest/gtest.h>

#include <utility>

#include "compiler/backend/optree/optimizer/optimizer.hpp"
#include "compiler/backend/optree/optimizer/transform_factories.hpp"
#include "compiler/optree/adaptors.hpp"

#include "common.hpp"

using namespace optree;
using namespace optree::optimizer;

class EraseUnusedFunctionsTest : public TransformTestBase {
    virtual void setupOptimizer(Optimizer &opt) const override {
        opt.add(createEraseUnusedFunctions());
    }

  public:
    EraseUnusedFunctionsTest() = default;
    ~EraseUnusedFunctionsTest() = default;
};

TEST_F(EraseUnusedFunctionsTest, can_run_on_empty_optree) {
    runOptimizer();
    assertSameOpTree();
}

TEST_F(EraseUnusedFunctionsTest, can_remove_unused_function) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("main", m.tFunc({}, m.tNone)).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(123));
        v[1] = m.opInit<FunctionCallOp>("test3", m.tNone, std::vector<Value::Ptr>());
        m.opInit<ReturnOp>();
        m.endBody();
        m.opInit<FunctionOp>("unused", m.tFunc({m.tI64, m.tI64}, m.tNone)).inward(v[2], 0).inward(v[3], 1).withBody();
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

TEST_F(EraseUnusedFunctionsTest, can_remove_unused_function_with_recursion) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("main", m.tFunc({}, m.tNone)).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(123));
        v[1] = m.opInit<FunctionCallOp>("test3", m.tNone, std::vector<Value::Ptr>());
        m.opInit<ReturnOp>();
        m.endBody();
        m.opInit<FunctionOp>("unused", m.tFunc({m.tI64, m.tI64}, m.tNone)).inward(v[2], 0).inward(v[3], 1).withBody();
        v[4] = m.opInit<ConstantOp>(m.tI64, int64_t(123));
        v[5] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v[2], v[3]);
        v[0] = m.opInit<FunctionCallOp>("unused", m.tNone, std::vector<Value::Ptr>());
        m.opInit<ReturnOp>();
        m.endBody();
        m.opInit<FunctionOp>("test3", m.tFunc({}, m.tNone)).withBody();
        v[0] = m.opInit<FunctionCallOp>("test3", m.tNone, std::vector<Value::Ptr>());
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
        v[0] = m.opInit<FunctionCallOp>("test3", m.tNone, std::vector<Value::Ptr>());
        m.opInit<ReturnOp>();
        m.endBody();
    }

    runOptimizer();
    assertSameOpTree();
}

TEST_F(EraseUnusedFunctionsTest, can_remove_unused_function_calls_each_other) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("main", m.tFunc({}, m.tNone)).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(123));
        v[1] = m.opInit<FunctionCallOp>("test3", m.tNone, std::vector<Value::Ptr>());
        m.opInit<ReturnOp>();
        m.endBody();
        m.opInit<FunctionOp>("unused1", m.tFunc({m.tI64, m.tI64}, m.tNone)).inward(v[2], 0).inward(v[3], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(123));
        v[1] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v[2], v[3]);
        v[2] = m.opInit<FunctionCallOp>("unused2", m.tNone, std::vector<Value::Ptr>());
        m.opInit<ReturnOp>();
        m.endBody();
        m.opInit<FunctionOp>("unused2", m.tFunc({m.tI64, m.tI64}, m.tNone)).inward(v[2], 0).inward(v[3], 1).withBody();
        v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(123));
        v[1] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v[2], v[3]);
        v[2] = m.opInit<FunctionCallOp>("unused1", m.tNone, std::vector<Value::Ptr>());
        m.opInit<ReturnOp>();
        m.endBody();
        m.opInit<FunctionOp>("test3", m.tFunc({}, m.tNone)).withBody();
        v[0] = m.opInit<FunctionCallOp>("test3", m.tNone, std::vector<Value::Ptr>());
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
        v[0] = m.opInit<FunctionCallOp>("test3", m.tNone, std::vector<Value::Ptr>());
        m.opInit<ReturnOp>();
        m.endBody();
    }

    runOptimizer();
    assertSameOpTree();
}

TEST_F(EraseUnusedFunctionsTest, can_keep_complex_used_functions) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("main", m.tFunc({}, m.tNone)).withBody();
        v[0] = m.opInit<FunctionCallOp>("used1", m.tNone, std::vector<Value::Ptr>());
        m.opInit<ReturnOp>();
        m.endBody();
        m.opInit<FunctionOp>("used1", m.tFunc({m.tI64, m.tI64}, m.tNone)).inward(v[0], 0).inward(v[1], 1).withBody();
        v[2] = m.opInit<FunctionCallOp>("used2", m.tNone, std::vector<Value::Ptr>());
        m.opInit<ReturnOp>();
        m.endBody();
        m.opInit<FunctionOp>("used2", m.tFunc({m.tI64, m.tI64}, m.tNone)).inward(v[2], 0).inward(v[3], 1).withBody();
        v[2] = m.opInit<FunctionCallOp>("used3", m.tNone, std::vector<Value::Ptr>());
        m.opInit<ReturnOp>();
        m.endBody();
        m.opInit<FunctionOp>("used3", m.tFunc({}, m.tNone)).withBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("main", m.tFunc({}, m.tNone)).withBody();
        v[0] = m.opInit<FunctionCallOp>("used1", m.tNone, std::vector<Value::Ptr>());
        m.opInit<ReturnOp>();
        m.endBody();
        m.opInit<FunctionOp>("used1", m.tFunc({m.tI64, m.tI64}, m.tNone)).inward(v[0], 0).inward(v[1], 1).withBody();
        v[2] = m.opInit<FunctionCallOp>("used2", m.tNone, std::vector<Value::Ptr>());
        m.opInit<ReturnOp>();
        m.endBody();
        m.opInit<FunctionOp>("used2", m.tFunc({m.tI64, m.tI64}, m.tNone)).inward(v[2], 0).inward(v[3], 1).withBody();
        v[2] = m.opInit<FunctionCallOp>("used3", m.tNone, std::vector<Value::Ptr>());
        m.opInit<ReturnOp>();
        m.endBody();
        m.opInit<FunctionOp>("used3", m.tFunc({}, m.tNone)).withBody();
        m.opInit<ReturnOp>();
    }

    runOptimizer();
    assertSameOpTree();
}

TEST_F(EraseUnusedFunctionsTest, can_keep_several_used_functions) {
    {
        auto &&[m, v] = getActual();
        m.opInit<FunctionOp>("main", m.tFunc({}, m.tNone)).withBody();
        v[0] = m.opInit<FunctionCallOp>("used1", m.tNone, std::vector<Value::Ptr>());
        v[1] = m.opInit<FunctionCallOp>("used2", m.tNone, std::vector<Value::Ptr>());
        v[2] = m.opInit<FunctionCallOp>("used3", m.tNone, std::vector<Value::Ptr>());
        m.opInit<ReturnOp>();
        m.endBody();
        m.opInit<FunctionOp>("unused", m.tFunc({m.tI64, m.tI64}, m.tNone)).inward(v[2], 0).inward(v[3], 1).withBody();
        v[4] = m.opInit<ConstantOp>(m.tI64, int64_t(123));
        v[5] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v[2], v[3]);
        m.opInit<ReturnOp>();
        m.endBody();
        m.opInit<FunctionOp>("used1", m.tFunc({m.tI64, m.tI64}, m.tNone)).inward(v[0], 0).inward(v[1], 1).withBody();
        v[2] = m.opInit<FunctionCallOp>("used2", m.tNone, std::vector<Value::Ptr>());
        m.opInit<ReturnOp>();
        m.endBody();
        m.opInit<FunctionOp>("used2", m.tFunc({m.tI64, m.tI64}, m.tNone)).inward(v[2], 0).inward(v[3], 1).withBody();
        v[2] = m.opInit<FunctionCallOp>("used3", m.tNone, std::vector<Value::Ptr>());
        m.opInit<ReturnOp>();
        m.endBody();
        m.opInit<FunctionOp>("used3", m.tFunc({}, m.tNone)).withBody();
        m.opInit<ReturnOp>();
        m.endBody();
    }
    {
        auto &&[m, v] = getExpected();
        m.opInit<FunctionOp>("main", m.tFunc({}, m.tNone)).withBody();
        v[0] = m.opInit<FunctionCallOp>("used1", m.tNone, std::vector<Value::Ptr>());
        v[1] = m.opInit<FunctionCallOp>("used2", m.tNone, std::vector<Value::Ptr>());
        v[2] = m.opInit<FunctionCallOp>("used3", m.tNone, std::vector<Value::Ptr>());
        m.opInit<ReturnOp>();
        m.endBody();
        m.opInit<FunctionOp>("used1", m.tFunc({m.tI64, m.tI64}, m.tNone)).inward(v[0], 0).inward(v[1], 1).withBody();
        v[2] = m.opInit<FunctionCallOp>("used2", m.tNone, std::vector<Value::Ptr>());
        m.opInit<ReturnOp>();
        m.endBody();
        m.opInit<FunctionOp>("used2", m.tFunc({m.tI64, m.tI64}, m.tNone)).inward(v[2], 0).inward(v[3], 1).withBody();
        v[2] = m.opInit<FunctionCallOp>("used3", m.tNone, std::vector<Value::Ptr>());
        m.opInit<ReturnOp>();
        m.endBody();
        m.opInit<FunctionOp>("used3", m.tFunc({}, m.tNone)).withBody();
        m.opInit<ReturnOp>();
    }

    runOptimizer();
    assertSameOpTree();
}
