#include <gtest/gtest.h>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantizer/semantizer.hpp"

using namespace ast;
using namespace lexer;
using namespace parser;
using namespace semantizer;

TEST(Semantizer, can_fill_functions_table) {
    StringVec source = {"def foo(t: int) -> None:", "def bar(x: float) -> int:", "def foobar() -> None:"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    // tree.dump(std::cout);
    FunctionsTable table;
    table.emplace("foo", Function(BuiltInTypes::NoneType, {BuiltInTypes::IntType}));
    table.emplace("bar", Function(BuiltInTypes::IntType, {BuiltInTypes::FloatType}));
    table.emplace("foobar", Function(BuiltInTypes::NoneType));
    ASSERT_EQ(table, tree.functions);
}

TEST(Semantizer, can_fill_variables_table) {
    StringVec source = {"def foo(t: int) -> None:", "    x: int", "    y: float"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    tree.dump(std::cout);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: foo\n"
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
    FAIL();
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

TEST(Semantizer, can_insert_type_conversion_in_function_int_to_float) {
    StringVec source = {"def foo(t: float) -> None:", "    t = 1", "def main() -> None:", "    x: int", "    foo(x)"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    tree.dump(std::cout);
    FAIL();
}

TEST(Semantizer, can_insert_type_conversion_float_to_int) {
    StringVec source = {"def main() -> None:", "    x: int", "    y: float", "    x = x + y"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    tree.dump(std::cout);
    FAIL();
}

TEST(Semantizer, can_insert_type_conversion_float_constant_to_int) {
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

TEST(Semantizer, can_insert_type_conversion_int_constant_to_float) {
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

TEST(Semantizer, dont_insert_type_conversion_int_int) {
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

TEST(Semantizer, dont_insert_type_conversion_float_float) {
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
                           "          BinaryOperation: Unknown\n"
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
    tree.dump(std::cout);
    FAIL();
}

TEST(Semantizer, can_insert_type_conversion_in_function_expression_int) {
    StringVec source = {"def foo(t: int) -> None:", "    t = 1", "def main() -> None:", "    x: int",
                        "    foo(x + 1.0)"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    tree.dump(std::cout);
    FAIL();
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
    tree.dump(std::cout);
    FAIL();
}
