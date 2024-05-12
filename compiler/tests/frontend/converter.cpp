#include <gtest/gtest.h>

#include "compiler/ast/declarative.hpp"
#include "compiler/ast/node_type.hpp"
#include "compiler/ast/types.hpp"
#include "compiler/frontend/converter/converter.hpp"
#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/declarative.hpp"

using namespace optree;
using ast::DeclarativeTree;
using ast::NodeType;
using converter::Converter;

class ConverterTest : public ::testing::Test {
  protected:
    // Input syntax tree
    DeclarativeTree t;

    // Expected operation tree after conversion
    DeclarativeModule m;
    ValueStorage &v;

    // Actual operation tree after conversion
    Operation::Ptr converted;

  public:
    ConverterTest() : t(NodeType::ProgramRoot), m(), v(m.values()), converted(){};
    ~ConverterTest() = default;

    void convert() {
        converted = Converter::process(t.makeTree()).root;
    }

    auto assertCorrectConversion() const {
        ASSERT_EQ(m.dump(), converted->dump());
    }
};

TEST_F(ConverterTest, can_process_empty_program) {
    convert();
    assertCorrectConversion();
}

TEST_F(ConverterTest, can_process_function_definition) {
    // clang-format off
    t.node(NodeType::FunctionDefinition).withChildren();
        t.node(NodeType::FunctionName, "func1");
        t.node(NodeType::FunctionArguments);
        t.node(NodeType::FunctionReturnType, ast::NoneType);
        t.node(NodeType::BranchRoot).withChildren();
            t.node(NodeType::ReturnStatement);
        t.endChildren();
    t.endChildren();
    t.node(NodeType::FunctionDefinition).withChildren();
        t.node(NodeType::FunctionName, "func2");
        t.node(NodeType::FunctionArguments).withChildren();
            t.node(NodeType::FunctionArgument).withChildren();
                t.node(NodeType::TypeName, ast::IntType);
                t.node(NodeType::VariableName, "x");
            t.endChildren();
        t.endChildren();
        t.node(NodeType::FunctionReturnType, ast::FloatType);
        t.node(NodeType::BranchRoot).withChildren();
            t.node(NodeType::ReturnStatement).withChildren();
                t.node(NodeType::Expression).withChildren();
                    t.node(NodeType::FloatingPointLiteralValue, 1.2);
                t.endChildren();
            t.endChildren();
        t.endChildren();
    t.endChildren();
    t.node(NodeType::FunctionDefinition).withChildren();
        t.node(NodeType::FunctionName, "func3");
        t.node(NodeType::FunctionArguments).withChildren();
            t.node(NodeType::FunctionArgument).withChildren();
                t.node(NodeType::TypeName, ast::IntType);
                t.node(NodeType::VariableName, "x");
            t.endChildren();
            t.node(NodeType::FunctionArgument).withChildren();
                t.node(NodeType::TypeName, ast::FloatType);
                t.node(NodeType::VariableName, "y");
            t.endChildren();
            t.node(NodeType::FunctionArgument).withChildren();
                t.node(NodeType::TypeName, ast::StrType);
                t.node(NodeType::VariableName, "z");
            t.endChildren();
        t.endChildren();
        t.node(NodeType::FunctionReturnType, ast::NoneType);
        t.node(NodeType::BranchRoot).withChildren();
            t.node(NodeType::ReturnStatement);
        t.endChildren();
    t.endChildren();
    // clang-format on

    m.opInit<FunctionOp>("func1", m.tFunc(m.tNone)).withBody();
    m.opInit<ReturnOp>();
    m.endBody();
    m.opInit<FunctionOp>("func2", m.tFunc({m.tI64}, m.tF64)).withBody();
    v[0] = m.opInit<ConstantOp>(m.tF64, 1.2);
    m.opInit<ReturnOp>(v[0]);
    m.endBody();
    m.opInit<FunctionOp>("func3", m.tFunc({m.tI64, m.tF64, m.tStr}, m.tNone)).withBody();
    m.opInit<ReturnOp>();
    m.endBody();

    convert();
    assertCorrectConversion();
}

TEST_F(ConverterTest, can_process_variable_declaration) {
    // clang-format off
    t.node(NodeType::FunctionDefinition).withChildren();
        t.node(NodeType::FunctionName, "func1");
        t.node(NodeType::FunctionArguments).withChildren();
            t.node(NodeType::FunctionArgument).withChildren();
                t.node(NodeType::TypeName, ast::IntType);
                t.node(NodeType::VariableName, "w");
            t.endChildren();
        t.endChildren();
        t.node(NodeType::FunctionReturnType, ast::NoneType);
        t.node(NodeType::BranchRoot).withChildren();
            t.node(NodeType::VariableDeclaration).withChildren();
                t.node(NodeType::TypeName, ast::IntType);
                t.node(NodeType::VariableName, "x");
            t.endChildren();
            t.node(NodeType::VariableDeclaration).withChildren();
                t.node(NodeType::TypeName, ast::FloatType);
                t.node(NodeType::VariableName, "y");
                t.node(NodeType::Expression).withChildren();
                    t.node(NodeType::FloatingPointLiteralValue, 1.2);
                t.endChildren();
            t.endChildren();
            t.node(NodeType::VariableDeclaration).withChildren();
                t.node(NodeType::TypeName, ast::IntType);
                t.node(NodeType::VariableName, "z");
                t.node(NodeType::Expression).withChildren();
                    t.node(NodeType::VariableName, "w");
                t.endChildren();
            t.endChildren();
            t.node(NodeType::ReturnStatement);
        t.endChildren();
    t.endChildren();
    // clang-format on

    m.opInit<FunctionOp>("func1", m.tFunc({m.tI64}, m.tNone)).inward(v["w"], 0).withBody();
    v["x"] = m.opInit<AllocateOp>(m.tPtr(m.tI64));
    v["y"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
    v["y_init"] = m.opInit<ConstantOp>(m.tF64, 1.2);
    m.opInit<StoreOp>(v["y"], v["y_init"]);
    v["z"] = m.opInit<AllocateOp>(m.tPtr(m.tI64));
    m.opInit<StoreOp>(v["z"], v["w"]);
    m.opInit<ReturnOp>();
    m.endBody();

    convert();
    assertCorrectConversion();
}

TEST_F(ConverterTest, can_process_expression) {
    // clang-format off
    t.node(NodeType::FunctionDefinition).withChildren();
        t.node(NodeType::FunctionName, "func1");
        t.node(NodeType::FunctionArguments).withChildren();
            t.node(NodeType::FunctionArgument).withChildren();
                t.node(NodeType::TypeName, ast::IntType);
                t.node(NodeType::VariableName, "x");
            t.endChildren();
            t.node(NodeType::FunctionArgument).withChildren();
                t.node(NodeType::TypeName, ast::IntType);
                t.node(NodeType::VariableName, "y");
            t.endChildren();
            t.node(NodeType::FunctionArgument).withChildren();
                t.node(NodeType::TypeName, ast::FloatType);
                t.node(NodeType::VariableName, "z");
            t.endChildren();
        t.endChildren();
        t.node(NodeType::FunctionReturnType, ast::NoneType);
        t.node(NodeType::BranchRoot).withChildren();
            t.node(NodeType::VariableDeclaration).withChildren();
                t.node(NodeType::TypeName, ast::IntType);
                t.node(NodeType::VariableName, "a");
                t.node(NodeType::Expression).withChildren();
                    t.node(NodeType::BinaryOperation, ast::BinaryOperation::Add).withChildren();
                        t.node(NodeType::VariableName, "x");
                        t.node(NodeType::BinaryOperation, ast::BinaryOperation::Mult).withChildren();
                            t.node(NodeType::IntegerLiteralValue, 12);
                            t.node(NodeType::VariableName, "y");
                        t.endChildren();
                    t.endChildren();
                t.endChildren();
            t.endChildren();
            t.node(NodeType::VariableDeclaration).withChildren();
                t.node(NodeType::TypeName, ast::FloatType);
                t.node(NodeType::VariableName, "b");
                t.node(NodeType::Expression).withChildren();
                    t.node(NodeType::FloatingPointLiteralValue, 3.4);
                t.endChildren();
            t.endChildren();
            t.node(NodeType::VariableDeclaration).withChildren();
                t.node(NodeType::TypeName, ast::FloatType);
                t.node(NodeType::VariableName, "c");
            t.endChildren();
            t.node(NodeType::Expression).withChildren();
                t.node(NodeType::BinaryOperation, ast::BinaryOperation::Assign).withChildren();
                    t.node(NodeType::VariableName, "c");
                    t.node(NodeType::BinaryOperation, ast::BinaryOperation::Sub).withChildren();
                        t.node(NodeType::VariableName, "b");
                        t.node(NodeType::BinaryOperation, ast::BinaryOperation::Div).withChildren();
                            t.node(NodeType::FloatingPointLiteralValue, 5.6);
                            t.node(NodeType::BinaryOperation, ast::BinaryOperation::Add).withChildren();
                                t.node(NodeType::VariableName, "z");
                                t.node(NodeType::FloatingPointLiteralValue, 7.8);
                            t.endChildren();
                        t.endChildren();
                    t.endChildren();
                t.endChildren();
            t.endChildren();
            t.node(NodeType::ReturnStatement);
        t.endChildren();
    t.endChildren();
    // clang-format on

    m.opInit<FunctionOp>("func1", m.tFunc({m.tI64, m.tI64, m.tF64}, m.tNone))
        .inward(v["x"], 0)
        .inward(v["y"], 1)
        .inward(v["z"], 2)
        .withBody();
    v["a"] = m.opInit<AllocateOp>(m.tPtr(m.tI64));
    v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(12));
    v[1] = m.opInit<ArithBinaryOp>(ArithBinOpKind::MulI, v[0], v["y"]);
    v[2] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddI, v["x"], v[1]);
    m.opInit<StoreOp>(v["a"], v[2]);
    v["b"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
    v[3] = m.opInit<ConstantOp>(m.tF64, 3.4);
    m.opInit<StoreOp>(v["b"], v[3]);
    v["c"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
    v[8] = m.opInit<LoadOp>(v["b"]);
    v[6] = m.opInit<ConstantOp>(m.tF64, 5.6);
    v[4] = m.opInit<ConstantOp>(m.tF64, 7.8);
    v[5] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["z"], v[4]);
    v[7] = m.opInit<ArithBinaryOp>(ArithBinOpKind::DivF, v[6], v[5]);
    v[9] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[8], v[7]);
    m.opInit<StoreOp>(v["c"], v[9]);
    m.opInit<ReturnOp>();
    m.endBody();

    convert();
    assertCorrectConversion();
}

TEST_F(ConverterTest, can_process_expression_with_implicit_type_conversion) {
    // clang-format off
    t.node(NodeType::FunctionDefinition).withChildren();
        t.node(NodeType::FunctionName, "func1");
        t.node(NodeType::FunctionArguments).withChildren();
            t.node(NodeType::FunctionArgument).withChildren();
                t.node(NodeType::TypeName, ast::IntType);
                t.node(NodeType::VariableName, "x");
            t.endChildren();
            t.node(NodeType::FunctionArgument).withChildren();
                t.node(NodeType::TypeName, ast::FloatType);
                t.node(NodeType::VariableName, "y");
            t.endChildren();
        t.endChildren();
        t.node(NodeType::FunctionReturnType, ast::NoneType);
        t.node(NodeType::BranchRoot).withChildren();
            t.node(NodeType::VariableDeclaration).withChildren();
                t.node(NodeType::TypeName, ast::IntType);
                t.node(NodeType::VariableName, "a");
                t.node(NodeType::Expression).withChildren();
                    t.node(NodeType::FloatingPointLiteralValue, 3.0);
                t.endChildren();
            t.endChildren();
            t.node(NodeType::VariableDeclaration).withChildren();
                t.node(NodeType::TypeName, ast::FloatType);
                t.node(NodeType::VariableName, "b");
            t.endChildren();
            t.node(NodeType::Expression).withChildren();
                t.node(NodeType::BinaryOperation, ast::BinaryOperation::Assign).withChildren();
                    t.node(NodeType::VariableName, "b");
                    t.node(NodeType::BinaryOperation, ast::BinaryOperation::Sub).withChildren();
                        t.node(NodeType::VariableName, "x");
                        t.node(NodeType::BinaryOperation, ast::BinaryOperation::Div).withChildren();
                            t.node(NodeType::IntegerLiteralValue, 5);
                            t.node(NodeType::BinaryOperation, ast::BinaryOperation::Add).withChildren();
                                t.node(NodeType::VariableName, "x");
                                t.node(NodeType::FloatingPointLiteralValue, 7.8);
                            t.endChildren();
                        t.endChildren();
                    t.endChildren();
                t.endChildren();
            t.endChildren();
            t.node(NodeType::Expression).withChildren();
                t.node(NodeType::BinaryOperation, ast::BinaryOperation::Assign).withChildren();
                    t.node(NodeType::VariableName, "a");
                    t.node(NodeType::BinaryOperation, ast::BinaryOperation::Mult).withChildren();
                        t.node(NodeType::VariableName, "y");
                        t.node(NodeType::IntegerLiteralValue, 2);
                    t.endChildren();
                t.endChildren();
            t.endChildren();
            t.node(NodeType::ReturnStatement);
        t.endChildren();
    t.endChildren();
    // clang-format on

    m.opInit<FunctionOp>("func1", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
    v["a"] = m.opInit<AllocateOp>(m.tPtr(m.tI64));
    v[0] = m.opInit<ConstantOp>(m.tF64, 3.0);
    v[1] = m.opInit<ArithCastOp>(ArithCastOpKind::FloatToInt, m.tI64, v[0]);
    m.opInit<StoreOp>(v["a"], v[1]);
    v["b"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
    v[2] = m.opInit<ConstantOp>(m.tI64, int64_t(5));
    v[3] = m.opInit<ConstantOp>(m.tF64, 7.8);
    v[4] = m.opInit<ArithCastOp>(ArithCastOpKind::IntToFloat, m.tF64, v["x"]);
    v[5] = m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v[4], v[3]);
    v[6] = m.opInit<ArithCastOp>(ArithCastOpKind::IntToFloat, m.tF64, v[2]);
    v[7] = m.opInit<ArithBinaryOp>(ArithBinOpKind::DivF, v[6], v[5]);
    v[8] = m.opInit<ArithCastOp>(ArithCastOpKind::IntToFloat, m.tF64, v["x"]);
    v[9] = m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v[8], v[7]);
    m.opInit<StoreOp>(v["b"], v[9]);
    v[10] = m.opInit<ConstantOp>(m.tI64, int64_t(2));
    v[11] = m.opInit<ArithCastOp>(ArithCastOpKind::IntToFloat, m.tF64, v[10]);
    v[12] = m.opInit<ArithBinaryOp>(ArithBinOpKind::MulF, v["y"], v[11]);
    v[13] = m.opInit<ArithCastOp>(ArithCastOpKind::FloatToInt, m.tI64, v[12]);
    m.opInit<StoreOp>(v["a"], v[13]);
    m.opInit<ReturnOp>();
    m.endBody();

    convert();
    assertCorrectConversion();
}

TEST_F(ConverterTest, can_process_input) {
    // clang-format off
    t.node(NodeType::FunctionDefinition).withChildren();
        t.node(NodeType::FunctionName, "func1");
        t.node(NodeType::FunctionArguments);
        t.node(NodeType::FunctionReturnType, ast::NoneType);
        t.node(NodeType::BranchRoot).withChildren();
            t.node(NodeType::VariableDeclaration).withChildren();
                t.node(NodeType::TypeName, ast::IntType);
                t.node(NodeType::VariableName, "x");
            t.endChildren();
            t.node(NodeType::VariableDeclaration).withChildren();
                t.node(NodeType::TypeName, ast::FloatType);
                t.node(NodeType::VariableName, "y");
                t.node(NodeType::Expression).withChildren();
                    t.node(NodeType::FunctionCall).withChildren();
                        t.node(NodeType::FunctionName, "input");
                    t.endChildren();
                t.endChildren();
            t.endChildren();
            t.node(NodeType::Expression).withChildren();
                t.node(NodeType::BinaryOperation, ast::BinaryOperation::Assign).withChildren();
                    t.node(NodeType::VariableName, "x");
                    t.node(NodeType::FunctionCall).withChildren();
                        t.node(NodeType::FunctionName, "input");
                    t.endChildren();
                t.endChildren();
            t.endChildren();
            t.node(NodeType::ReturnStatement);
        t.endChildren();
    t.endChildren();
    // clang-format on

    m.opInit<FunctionOp>("func1", m.tFunc(m.tNone)).withBody();
    v["x"] = m.opInit<AllocateOp>(m.tPtr(m.tI64));
    v["y"] = m.opInit<AllocateOp>(m.tPtr(m.tF64));
    m.opInit<InputOp>(v["y"]);
    m.opInit<InputOp>(v["x"]);
    m.opInit<ReturnOp>();
    m.endBody();

    convert();
    assertCorrectConversion();
}

TEST_F(ConverterTest, can_process_if_statement) {
    // clang-format off
    t.node(NodeType::FunctionDefinition).withChildren();
        t.node(NodeType::FunctionName, "func1");
        t.node(NodeType::FunctionArguments).withChildren();
            t.node(NodeType::FunctionArgument).withChildren();
                t.node(NodeType::TypeName, ast::IntType);
                t.node(NodeType::VariableName, "x");
            t.endChildren();
            t.node(NodeType::FunctionArgument).withChildren();
                t.node(NodeType::TypeName, ast::FloatType);
                t.node(NodeType::VariableName, "y");
            t.endChildren();
        t.endChildren();
        t.node(NodeType::FunctionReturnType, ast::NoneType);
        t.node(NodeType::BranchRoot).withChildren();
            t.node(NodeType::IfStatement).withChildren();
                t.node(NodeType::Expression).withChildren();
                    t.node(NodeType::BinaryOperation, ast::BinaryOperation::Equal).withChildren();
                        t.node(NodeType::VariableName, "x");
                        t.node(NodeType::IntegerLiteralValue, 1);
                    t.endChildren();
                t.endChildren();
                t.node(NodeType::BranchRoot).withChildren();
                    t.node(NodeType::Expression).withChildren();
                        t.node(NodeType::BinaryOperation, ast::BinaryOperation::Add).withChildren();
                            t.node(NodeType::VariableName, "y");
                            t.node(NodeType::FloatingPointLiteralValue, 2.3);
                        t.endChildren();
                    t.endChildren();
                t.endChildren();
            t.endChildren();
            t.node(NodeType::ReturnStatement);
        t.endChildren();
    t.endChildren();
    // clang-format on

    m.opInit<FunctionOp>("func1", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
    v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(1));
    v[1] = m.opInit<LogicBinaryOp>(LogicBinOpKind::Equal, v["x"], v[0]);
    m.op<IfOp>(v[1]).withBody();
    m.op<ThenOp>().withBody();
    v[2] = m.opInit<ConstantOp>(m.tF64, 2.3);
    m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[2]);
    m.endBody();
    m.endBody();
    m.opInit<ReturnOp>();
    m.endBody();

    convert();
    assertCorrectConversion();
}

TEST_F(ConverterTest, can_process_if_else_statement) {
    // clang-format off
    t.node(NodeType::FunctionDefinition).withChildren();
        t.node(NodeType::FunctionName, "func1");
        t.node(NodeType::FunctionArguments).withChildren();
            t.node(NodeType::FunctionArgument).withChildren();
                t.node(NodeType::TypeName, ast::IntType);
                t.node(NodeType::VariableName, "x");
            t.endChildren();
            t.node(NodeType::FunctionArgument).withChildren();
                t.node(NodeType::TypeName, ast::FloatType);
                t.node(NodeType::VariableName, "y");
            t.endChildren();
        t.endChildren();
        t.node(NodeType::FunctionReturnType, ast::NoneType);
        t.node(NodeType::BranchRoot).withChildren();
            t.node(NodeType::IfStatement).withChildren();
                t.node(NodeType::Expression).withChildren();
                    t.node(NodeType::BinaryOperation, ast::BinaryOperation::Less).withChildren();
                        t.node(NodeType::VariableName, "y");
                        t.node(NodeType::FloatingPointLiteralValue, 4.5);
                    t.endChildren();
                t.endChildren();
                t.node(NodeType::BranchRoot).withChildren();
                    t.node(NodeType::Expression).withChildren();
                        t.node(NodeType::BinaryOperation, ast::BinaryOperation::Sub).withChildren();
                            t.node(NodeType::VariableName, "x");
                            t.node(NodeType::IntegerLiteralValue, 6);
                        t.endChildren();
                    t.endChildren();
                t.endChildren();
                t.node(NodeType::ElseStatement).withChildren();
                    t.node(NodeType::BranchRoot).withChildren();
                        t.node(NodeType::Expression).withChildren();
                            t.node(NodeType::BinaryOperation, ast::BinaryOperation::Mult).withChildren();
                                t.node(NodeType::VariableName, "x");
                                t.node(NodeType::IntegerLiteralValue, 7);
                            t.endChildren();
                        t.endChildren();
                    t.endChildren();
                t.endChildren();
            t.endChildren();
            t.node(NodeType::ReturnStatement);
        t.endChildren();
    t.endChildren();
    // clang-format on

    m.opInit<FunctionOp>("func1", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
    v[0] = m.opInit<ConstantOp>(m.tF64, 4.5);
    v[1] = m.opInit<LogicBinaryOp>(LogicBinOpKind::LessF, v["y"], v[0]);
    m.op<IfOp>(v[1]).withBody();
    m.op<ThenOp>().withBody();
    v[2] = m.opInit<ConstantOp>(m.tI64, int64_t(6));
    m.opInit<ArithBinaryOp>(ArithBinOpKind::SubI, v["x"], v[2]);
    m.endBody();
    m.op<ElseOp>().withBody();
    v[3] = m.opInit<ConstantOp>(m.tI64, int64_t(7));
    m.opInit<ArithBinaryOp>(ArithBinOpKind::MulI, v["x"], v[3]);
    m.endBody();
    m.endBody();
    m.opInit<ReturnOp>();
    m.endBody();

    convert();
    assertCorrectConversion();
}

TEST_F(ConverterTest, can_process_if_elif_statement) {
    // clang-format off
    t.node(NodeType::FunctionDefinition).withChildren();
        t.node(NodeType::FunctionName, "func1");
        t.node(NodeType::FunctionArguments).withChildren();
            t.node(NodeType::FunctionArgument).withChildren();
                t.node(NodeType::TypeName, ast::IntType);
                t.node(NodeType::VariableName, "x");
            t.endChildren();
            t.node(NodeType::FunctionArgument).withChildren();
                t.node(NodeType::TypeName, ast::FloatType);
                t.node(NodeType::VariableName, "y");
            t.endChildren();
        t.endChildren();
        t.node(NodeType::FunctionReturnType, ast::NoneType);
        t.node(NodeType::BranchRoot).withChildren();
            t.node(NodeType::IfStatement).withChildren();
                t.node(NodeType::Expression).withChildren();
                    t.node(NodeType::BinaryOperation, ast::BinaryOperation::Greater).withChildren();
                        t.node(NodeType::VariableName, "x");
                        t.node(NodeType::IntegerLiteralValue, 8);
                    t.endChildren();
                t.endChildren();
                t.node(NodeType::BranchRoot).withChildren();
                    t.node(NodeType::Expression).withChildren();
                        t.node(NodeType::BinaryOperation, ast::BinaryOperation::Div).withChildren();
                            t.node(NodeType::VariableName, "y");
                            t.node(NodeType::FloatingPointLiteralValue, 9.10);
                        t.endChildren();
                    t.endChildren();
                t.endChildren();
                t.node(NodeType::ElifStatement).withChildren();
                    t.node(NodeType::Expression).withChildren();
                        t.node(NodeType::BinaryOperation, ast::BinaryOperation::NotEqual).withChildren();
                            t.node(NodeType::VariableName, "x");
                            t.node(NodeType::IntegerLiteralValue, 11);
                        t.endChildren();
                    t.endChildren();
                    t.node(NodeType::BranchRoot).withChildren();
                        t.node(NodeType::Expression).withChildren();
                            t.node(NodeType::BinaryOperation, ast::BinaryOperation::Add).withChildren();
                                t.node(NodeType::VariableName, "y");
                                t.node(NodeType::FloatingPointLiteralValue, 12.13);
                            t.endChildren();
                        t.endChildren();
                    t.endChildren();
                t.endChildren();
            t.endChildren();
            t.node(NodeType::ReturnStatement);
        t.endChildren();
    t.endChildren();
    // clang-format on

    m.opInit<FunctionOp>("func1", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
    v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(8));
    v[1] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterI, v["x"], v[0]);
    m.op<IfOp>(v[1]).withBody();
    m.op<ThenOp>().withBody();
    v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
    m.opInit<ArithBinaryOp>(ArithBinOpKind::DivF, v["y"], v[2]);
    m.endBody();
    m.op<ElseOp>().withBody();
    v[3] = m.opInit<ConstantOp>(m.tI64, int64_t(11));
    v[4] = m.opInit<LogicBinaryOp>(LogicBinOpKind::NotEqual, v["x"], v[3]);
    m.op<IfOp>(v[4]).withBody();
    m.op<ThenOp>().withBody();
    v[5] = m.opInit<ConstantOp>(m.tF64, 12.13);
    m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[5]);
    m.endBody();
    m.endBody();
    m.endBody();
    m.endBody();
    m.opInit<ReturnOp>();
    m.endBody();

    convert();
    assertCorrectConversion();
}

TEST_F(ConverterTest, can_process_if_elif_else_statement) {
    // clang-format off
    t.node(NodeType::FunctionDefinition).withChildren();
        t.node(NodeType::FunctionName, "func1");
        t.node(NodeType::FunctionArguments).withChildren();
            t.node(NodeType::FunctionArgument).withChildren();
                t.node(NodeType::TypeName, ast::IntType);
                t.node(NodeType::VariableName, "x");
            t.endChildren();
            t.node(NodeType::FunctionArgument).withChildren();
                t.node(NodeType::TypeName, ast::FloatType);
                t.node(NodeType::VariableName, "y");
            t.endChildren();
        t.endChildren();
        t.node(NodeType::FunctionReturnType, ast::NoneType);
        t.node(NodeType::BranchRoot).withChildren();
            t.node(NodeType::IfStatement).withChildren();
                t.node(NodeType::Expression).withChildren();
                    t.node(NodeType::BinaryOperation, ast::BinaryOperation::LessEqual).withChildren();
                        t.node(NodeType::VariableName, "x");
                        t.node(NodeType::IntegerLiteralValue, 8);
                    t.endChildren();
                t.endChildren();
                t.node(NodeType::BranchRoot).withChildren();
                    t.node(NodeType::Expression).withChildren();
                        t.node(NodeType::BinaryOperation, ast::BinaryOperation::Sub).withChildren();
                            t.node(NodeType::VariableName, "y");
                            t.node(NodeType::FloatingPointLiteralValue, 9.10);
                        t.endChildren();
                    t.endChildren();
                t.endChildren();
                t.node(NodeType::ElifStatement).withChildren();
                    t.node(NodeType::Expression).withChildren();
                        t.node(NodeType::BinaryOperation, ast::BinaryOperation::GreaterEqual).withChildren();
                            t.node(NodeType::VariableName, "x");
                            t.node(NodeType::IntegerLiteralValue, 11);
                        t.endChildren();
                    t.endChildren();
                    t.node(NodeType::BranchRoot).withChildren();
                        t.node(NodeType::Expression).withChildren();
                            t.node(NodeType::BinaryOperation, ast::BinaryOperation::Mult).withChildren();
                                t.node(NodeType::VariableName, "y");
                                t.node(NodeType::FloatingPointLiteralValue, 12.13);
                            t.endChildren();
                        t.endChildren();
                    t.endChildren();
                t.endChildren();
                t.node(NodeType::ElseStatement).withChildren();
                    t.node(NodeType::BranchRoot).withChildren();
                        t.node(NodeType::Expression).withChildren();
                            t.node(NodeType::BinaryOperation, ast::BinaryOperation::Add).withChildren();
                                t.node(NodeType::VariableName, "y");
                                t.node(NodeType::FloatingPointLiteralValue, 14.15);
                            t.endChildren();
                        t.endChildren();
                    t.endChildren();
                t.endChildren();
            t.endChildren();
            t.node(NodeType::ReturnStatement);
        t.endChildren();
    t.endChildren();
    // clang-format on

    m.opInit<FunctionOp>("func1", m.tFunc({m.tI64, m.tF64}, m.tNone)).inward(v["x"], 0).inward(v["y"], 1).withBody();
    v[0] = m.opInit<ConstantOp>(m.tI64, int64_t(8));
    v[1] = m.opInit<LogicBinaryOp>(LogicBinOpKind::LessEqualI, v["x"], v[0]);
    m.op<IfOp>(v[1]).withBody();
    m.op<ThenOp>().withBody();
    v[2] = m.opInit<ConstantOp>(m.tF64, 9.10);
    m.opInit<ArithBinaryOp>(ArithBinOpKind::SubF, v["y"], v[2]);
    m.endBody();
    m.op<ElseOp>().withBody();
    v[3] = m.opInit<ConstantOp>(m.tI64, int64_t(11));
    v[4] = m.opInit<LogicBinaryOp>(LogicBinOpKind::GreaterEqualI, v["x"], v[3]);
    m.op<IfOp>(v[4]).withBody();
    m.op<ThenOp>().withBody();
    v[5] = m.opInit<ConstantOp>(m.tF64, 12.13);
    m.opInit<ArithBinaryOp>(ArithBinOpKind::MulF, v["y"], v[5]);
    m.endBody();
    m.op<ElseOp>().withBody();
    v[6] = m.opInit<ConstantOp>(m.tF64, 14.15);
    m.opInit<ArithBinaryOp>(ArithBinOpKind::AddF, v["y"], v[6]);
    m.endBody();
    m.endBody();
    m.endBody();
    m.endBody();
    m.opInit<ReturnOp>();
    m.endBody();

    convert();
    assertCorrectConversion();
}
