#include <gtest/gtest.h>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantizer/semantizer.hpp"

using namespace ast;
using namespace lexer;
using namespace parser;
using namespace semantizer;

TEST(Semantizer, can_fill_functions_table) {
    StringVec source = {"def foo(t: int) -> None:", "def bar(x: float) -> int:", "def main() -> None:"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    FunctionsTable table;
    table.emplace("foo", Function(BuiltInTypes::NoneType, {BuiltInTypes::IntType}));
    table.emplace("bar", Function(BuiltInTypes::IntType, {BuiltInTypes::FloatType}));
    table.emplace("main", Function(BuiltInTypes::NoneType));
    ASSERT_EQ(table, tree.functions);
}

TEST(Semantizer, can_fill_variables_table) {
    StringVec source = {"def main(t: int) -> None:", "    x: int", "    y: float"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "      FunctionArgument\n"
                           "        TypeName: IntType\n"
                           "        VariableName: t\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: t:IntType x:IntType y:FloatType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: y\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Semantizer, can_process_print_function_call_with_float_as_argument) {
    StringVec source = {"def main() -> None:", "    x: int", "    y: float", "    print(1.0 + y + x * 2 / 1)"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType y:FloatType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: y\n"
                           "      Expression: NoneType\n"
                           "        FunctionCall\n"
                           "          FunctionName: print\n"
                           "          FunctionArguments\n"
                           "            Expression: FloatType\n"
                           "              BinaryOperation: FAdd\n"
                           "                BinaryOperation: FAdd\n"
                           "                  FloatingPointLiteralValue: 1\n"
                           "                  VariableName: y\n"
                           "                TypeConversion\n"
                           "                  TypeName: FloatType\n"
                           "                  BinaryOperation: Div\n"
                           "                    BinaryOperation: Mult\n"
                           "                      VariableName: x\n"
                           "                      IntegerLiteralValue: 2\n"
                           "                    IntegerLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Semantizer, can_process_print_function_call_with_int_as_argument) {
    StringVec source = {"def main() -> None:", "    x: int", "    y: int", "    print(1 + y + x * 2 / 1)"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType y:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: y\n"
                           "      Expression: NoneType\n"
                           "        FunctionCall\n"
                           "          FunctionName: print\n"
                           "          FunctionArguments\n"
                           "            Expression: IntType\n"
                           "              BinaryOperation: Add\n"
                           "                BinaryOperation: Add\n"
                           "                  IntegerLiteralValue: 1\n"
                           "                  VariableName: y\n"
                           "                BinaryOperation: Div\n"
                           "                  BinaryOperation: Mult\n"
                           "                    VariableName: x\n"
                           "                    IntegerLiteralValue: 2\n"
                           "                  IntegerLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Semantizer, can_insert_type_conversion_int_to_float) {
    StringVec source = {"def main() -> None:", "    x: int", "    y: float", "    y = x + y"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType y:FloatType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: y\n"
                           "      Expression: FloatType\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: y\n"
                           "          BinaryOperation: FAdd\n"
                           "            TypeConversion\n"
                           "              TypeName: FloatType\n"
                           "              VariableName: x\n"
                           "            VariableName: y\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Semantizer, can_insert_type_conversion_in_function_float_to_int) {
    StringVec source = {"def foo(t: int) -> None:", "    t = 1", "def main() -> None:", "    x: float", "    foo(x)"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: foo\n"
                           "    FunctionArguments\n"
                           "      FunctionArgument\n"
                           "        TypeName: IntType\n"
                           "        VariableName: t\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: t:IntType\n"
                           "      Expression: IntType\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: t\n"
                           "          IntegerLiteralValue: 1\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:FloatType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: x\n"
                           "      Expression: NoneType\n"
                           "        FunctionCall\n"
                           "          FunctionName: foo\n"
                           "          FunctionArguments\n"
                           "            Expression: IntType\n"
                           "              TypeConversion\n"
                           "                TypeName: IntType\n"
                           "                VariableName: x\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Semantizer, can_insert_type_conversion_in_function_int_to_float) {
    StringVec source = {"def foo(t: float) -> None:", "    t = 1", "def main() -> None:", "    x: int", "    foo(x)"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: foo\n"
                           "    FunctionArguments\n"
                           "      FunctionArgument\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: t\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: t:FloatType\n"
                           "      Expression: FloatType\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: t\n"
                           "          TypeConversion\n"
                           "            TypeName: FloatType\n"
                           "            IntegerLiteralValue: 1\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "      Expression: NoneType\n"
                           "        FunctionCall\n"
                           "          FunctionName: foo\n"
                           "          FunctionArguments\n"
                           "            Expression: FloatType\n"
                           "              TypeConversion\n"
                           "                TypeName: FloatType\n"
                           "                VariableName: x\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Semantizer, can_insert_type_conversion_float_to_int) {
    StringVec source = {"def main() -> None:", "    x: int", "    y: float", "    x = x + y"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType y:FloatType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: y\n"
                           "      Expression: IntType\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          BinaryOperation: FAdd\n"
                           "            VariableName: x\n"
                           "            TypeConversion\n"
                           "              TypeName: IntType\n"
                           "              VariableName: y\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Semantizer, can_insert_type_conversion_float_literal_to_int) {
    StringVec source = {"def main() -> None:", "    x: int", "    x = x + 1.0"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "      Expression: IntType\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          BinaryOperation: Add\n"
                           "            VariableName: x\n"
                           "            TypeConversion\n"
                           "              TypeName: IntType\n"
                           "              FloatingPointLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Semantizer, can_insert_type_conversion_int_literal_to_float) {
    StringVec source = {"def main() -> None:", "    y: float", "    y = y + 1"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: y:FloatType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: y\n"
                           "      Expression: FloatType\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: y\n"
                           "          BinaryOperation: FAdd\n"
                           "            VariableName: y\n"
                           "            TypeConversion\n"
                           "              TypeName: FloatType\n"
                           "              IntegerLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Semantizer, does_not_insert_type_conversion_for_ints) {
    StringVec source = {"def main() -> None:", "    y: int", "    y = y + 1"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: y:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: y\n"
                           "      Expression: IntType\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: y\n"
                           "          BinaryOperation: Add\n"
                           "            VariableName: y\n"
                           "            IntegerLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Semantizer, does_not_insert_type_conversion_for_floats) {
    StringVec source = {"def main() -> None:", "    y: float", "    y = y + 1.0"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: y:FloatType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: y\n"
                           "      Expression: FloatType\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: y\n"
                           "          BinaryOperation: FAdd\n"
                           "            VariableName: y\n"
                           "            FloatingPointLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Semantizer, can_insert_type_conversion_in_function_expression_float) {
    StringVec source = {"def foo(t: float) -> None:", "    t = 1", "def main() -> None:", "    x: float",
                        "    foo(x + 1)"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: foo\n"
                           "    FunctionArguments\n"
                           "      FunctionArgument\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: t\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: t:FloatType\n"
                           "      Expression: FloatType\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: t\n"
                           "          TypeConversion\n"
                           "            TypeName: FloatType\n"
                           "            IntegerLiteralValue: 1\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:FloatType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: x\n"
                           "      Expression: NoneType\n"
                           "        FunctionCall\n"
                           "          FunctionName: foo\n"
                           "          FunctionArguments\n"
                           "            Expression: FloatType\n"
                           "              BinaryOperation: FAdd\n"
                           "                VariableName: x\n"
                           "                TypeConversion\n"
                           "                  TypeName: FloatType\n"
                           "                  IntegerLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Semantizer, can_insert_type_conversion_in_function_expression_int) {
    StringVec source = {"def foo(t: int) -> None:", "    t = 1", "def main() -> None:", "    x: int",
                        "    foo(x + 1.0)"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: foo\n"
                           "    FunctionArguments\n"
                           "      FunctionArgument\n"
                           "        TypeName: IntType\n"
                           "        VariableName: t\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: t:IntType\n"
                           "      Expression: IntType\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: t\n"
                           "          IntegerLiteralValue: 1\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "      Expression: NoneType\n"
                           "        FunctionCall\n"
                           "          FunctionName: foo\n"
                           "          FunctionArguments\n"
                           "            Expression: IntType\n"
                           "              BinaryOperation: Add\n"
                           "                VariableName: x\n"
                           "                TypeConversion\n"
                           "                  TypeName: IntType\n"
                           "                  FloatingPointLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Semantizer, can_insert_type_conversion_in_function_with_multiple_params_expression_int) {
    StringVec source = {"def foo(t: int, n: float) -> None:",
                        "    t = 1",
                        "def main() -> None:",
                        "    x: int",
                        "    y: float",
                        "    foo(y + 1, x + 1.0)"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: foo\n"
                           "    FunctionArguments\n"
                           "      FunctionArgument\n"
                           "        TypeName: IntType\n"
                           "        VariableName: t\n"
                           "      FunctionArgument\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: n\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: n:FloatType t:IntType\n"
                           "      Expression: IntType\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: t\n"
                           "          IntegerLiteralValue: 1\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType y:FloatType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: y\n"
                           "      Expression: NoneType\n"
                           "        FunctionCall\n"
                           "          FunctionName: foo\n"
                           "          FunctionArguments\n"
                           "            Expression: IntType\n"
                           "              BinaryOperation: FAdd\n"
                           "                TypeConversion\n"
                           "                  TypeName: IntType\n"
                           "                  VariableName: y\n"
                           "                IntegerLiteralValue: 1\n"
                           "            Expression: FloatType\n"
                           "              BinaryOperation: FAdd\n"
                           "                TypeConversion\n"
                           "                  TypeName: FloatType\n"
                           "                  VariableName: x\n"
                           "                FloatingPointLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Semantizer, can_insert_type_conversion_on_int_variable_declaration) {
    StringVec source = {"def main() -> None:", "    y: int = 1.0"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: y:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: y\n"
                           "        Expression: IntType\n"
                           "          TypeConversion\n"
                           "            TypeName: IntType\n"
                           "            FloatingPointLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Semantizer, can_insert_type_conversion_on_float_variable_declaration) {
    StringVec source = {"def main() -> None:", "    y: float = 1"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: y:FloatType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: y\n"
                           "        Expression: FloatType\n"
                           "          TypeConversion\n"
                           "            TypeName: FloatType\n"
                           "            IntegerLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Semantizer, can_insert_float_operation_with_type_conversion) {
    StringVec source = {"def main() -> None:", "    y: float", "    y = 1 + 2"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: y:FloatType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: y\n"
                           "      Expression: FloatType\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: y\n"
                           "          BinaryOperation: FAdd\n"
                           "            TypeConversion\n"
                           "              TypeName: FloatType\n"
                           "              IntegerLiteralValue: 1\n"
                           "            TypeConversion\n"
                           "              TypeName: FloatType\n"
                           "              IntegerLiteralValue: 2\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Semantizer, can_declare_variables_with_same_names_in_different_scopes) {
    StringVec source = {"def main() -> None:", "    x: int", "    if(true):", "        x: float"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    ASSERT_NO_THROW(Semantizer::process(tree));
}

TEST(Semantizer, raise_error_on_undeclared_variable) {
    StringVec source = {"def main() -> None:", "    x = 1"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    ASSERT_THROW(Semantizer::process(tree), ErrorBuffer);
}

TEST(Semantizer, raise_error_on_redeclaration_of_variable_in_single_scope) {
    StringVec source = {"def main() -> None:", "    x: int", "    x: int"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    ASSERT_THROW(Semantizer::process(tree), ErrorBuffer);
}

TEST(Semantizer, raise_error_on_undefined_main_function) {
    StringVec source = {"def foo() -> None:", "    x = 1"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    ASSERT_THROW(Semantizer::process(tree), ErrorBuffer);
}
