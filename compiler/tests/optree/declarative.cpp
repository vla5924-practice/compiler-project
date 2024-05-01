#include <string_view>

#include <gtest/gtest.h>

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/declarative.hpp"

using namespace optree;

class DeclarativeTest : public ::testing::Test {
  protected:
    DeclarativeModule m;
    ValueStorage &v;

  public:
    DeclarativeTest() : m(), v(m.values()){};
    ~DeclarativeTest() = default;

    auto assertDump(std::string_view str) const {
        ASSERT_EQ(str, m.dump());
    }
};

TEST_F(DeclarativeTest, can_create_module) {
    assertDump("Module () -> ()\n");
}

TEST_F(DeclarativeTest, can_insert_function_with_body) {
    // clang-format off
    m.op<FunctionOp>().attr("myfunc").attr(m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v[0], m.tI64).inward(v[1], m.tF64).withBody();
        v[2] = m.op<ConstantOp>().attr(123).result(m.tI64);
        v[3] = m.op<AllocateOp>().result(m.tPtr(m.tI64));
        v[4] = m.op<ArithBinaryOp>(v[2], v[1]).attr(ArithBinOpKind::AddI).result(m.tI64);
        m.op<StoreOp>(v[3], v[4]);
        m.op<ReturnOp>();
    m.endBody();
    // clang-format on
    assertDump("Module () -> ()\n"
               "  Function {string : myfunc, Type : func((int(64), float(64)) -> none)} () -> () [#0 : int(64), #1 : "
               "float(64)]\n"
               "    Constant {int64_t : 123} () -> (#2 : int(64))\n"
               "    Allocate () -> (#3 : ptr(int(64)))\n"
               "    ArithBinary {ArithBinOpKind : 1} (#2 : int(64), #1 : float(64)) -> (#4 : int(64))\n"
               "    Store (#3 : ptr(int(64)), #4 : int(64)) -> ()\n"
               "    Return () -> ()\n");
}

TEST_F(DeclarativeTest, can_insert_nested_operations) {
    // clang-format off
    m.op<FunctionOp>().attr("myfunc").attr(m.tFunc({m.tF64}, m.tNone)).inward(v[0], m.tF64).withBody();
        v[1] = m.op<ConstantOp>().attr(7.89).result(m.tF64);
        v[2] = m.op<AllocateOp>().result(m.tPtr(m.tF64));
        v[3] = m.op<LogicBinaryOp>(v[0], v[1]).attr(LogicBinOpKind::GreaterEqualF).result(m.tBool);
        m.op<IfOp>(v[3]).withBody();
          m.op<ThenOp>().withBody();
            v[4] = m.op<ArithBinaryOp>(v[1], v[0]).attr(ArithBinOpKind::MulF).result(m.tF64);
            m.op<StoreOp>(v[2], v[4]);
          m.endBody();
          m.op<ElseOp>().withBody();
            m.op<StoreOp>(v[2], v[0]);
          m.endBody();
        m.endBody();
        v[5] = m.op<LoadOp>(v[2]).result(m.tF64);
        m.op<PrintOp>(v[5]);
        m.op<ReturnOp>();
    m.endBody();
    // clang-format on
    assertDump("Module () -> ()\n"
               "  Function {string : myfunc, Type : func((float(64)) -> none)} () -> () [#0 : float(64)]\n"
               "    Constant {double : 7.89} () -> (#1 : float(64))\n"
               "    Allocate () -> (#2 : ptr(float(64)))\n"
               "    LogicBinary {LogicBinOpKind : 12} (#0 : float(64), #1 : float(64)) -> (#3 : int(8))\n"
               "    If (#3 : int(8)) -> ()\n"
               "      Then () -> ()\n"
               "        ArithBinary {ArithBinOpKind : 7} (#1 : float(64), #0 : float(64)) -> (#4 : float(64))\n"
               "        Store (#2 : ptr(float(64)), #4 : float(64)) -> ()\n"
               "      Else () -> ()\n"
               "        Store (#2 : ptr(float(64)), #0 : float(64)) -> ()\n"
               "    Load (#2 : ptr(float(64))) -> (#5 : float(64))\n"
               "    Print (#5 : float(64)) -> ()\n"
               "    Return () -> ()\n");
}
