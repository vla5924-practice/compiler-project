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
        m.op<StoreOp>(v[3], v[2]);
    m.endBody();
    // clang-format on
    assertDump("Module () -> ()\n"
               "  Function {string : myfunc, Type : func((int(64), float(64)) -> none)} () -> () [#0 : int(64), #1 : "
               "float(64)]\n"
               "    Constant {int64_t : 123} () -> (#2 : int(64))\n"
               "    Allocate () -> (#3 : ptr(int(64)))\n"
               "    Store (#3 : ptr(int(64)), #2 : int(64)) -> ()\n");
}
