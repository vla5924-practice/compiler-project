#include <gtest/gtest.h>

#include "lexer/lexer.hpp"
#include "optimizer/optimizer.hpp"
#include "parser/parser.hpp"
#include "semantizer/semantizer.hpp"
#include "stringvec.hpp"

using namespace ast;
using namespace lexer;
using namespace parser;
using namespace semantizer;
using namespace optimizer;

TEST(Optimizer, can_convert_float_literal_to_int) {
    StringVec source = {"def main() -> None:", "    x: float = 1 + 1.0"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:FloatType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: x\n"
                           "        Expression: FloatType\n"
                           "          FloatingPointLiteralValue: 2\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_convert_int_literal_to_float) {
    StringVec source = {"def main() -> None:", "    x: int = 1 + 1.0"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 2\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_substitute_int_constant_in_int_variable_declaration) {
    StringVec source = {"def main() -> None:", "    x: int = 1", "    y: int = x"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType y:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 1\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: y\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_correct_assign_with_function_call) {
    StringVec source = {"def foo() -> int:", "    return 1", "def main() -> None:", "    x: int = 1", "    y: int = x",
                        "    x = foo()",     "    y = x"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: foo\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: IntType\n"
                           "    BranchRoot:\n"
                           "      ReturnStatement\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 1\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType y:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 1\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: y\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 1\n"
                           "      Expression: IntType\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          IntegerLiteralValue: 1\n"
                           "      Expression: IntType\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: y\n"
                           "          VariableName: x\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_substitute_float_constant_in_float_variable_declaration) {
    StringVec source = {"def main() -> None:", "    x: float = 1.0", "    y: float = x"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:FloatType y:FloatType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: x\n"
                           "        Expression: FloatType\n"
                           "          FloatingPointLiteralValue: 1\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: y\n"
                           "        Expression: FloatType\n"
                           "          FloatingPointLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_substitute_int_constant_in_float_variable_declaration) {
    StringVec source = {"def main() -> None:", "    x: int = 1", "    y: float = x"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType y:FloatType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 1\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: y\n"
                           "        Expression: FloatType\n"
                           "          FloatingPointLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_substitute_float_constant_in_int_variable_declaration) {
    StringVec source = {"def main() -> None:", "    x: float = 1.0", "    y: int = x"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:FloatType y:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: x\n"
                           "        Expression: FloatType\n"
                           "          FloatingPointLiteralValue: 1\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: y\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_substitute_int_constant_in_int_variable_declaration_with_int_literal) {
    StringVec source = {"def main() -> None:", "    x: int = 1", "    y: int = x + 1"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType y:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 1\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: y\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 2\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_substitute_float_constant_in_float_variable_declaration_with_float_literal) {
    StringVec source = {"def main() -> None:", "    x: float = 1.0", "    y: float = x + 1.0"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:FloatType y:FloatType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: x\n"
                           "        Expression: FloatType\n"
                           "          FloatingPointLiteralValue: 1\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: y\n"
                           "        Expression: FloatType\n"
                           "          FloatingPointLiteralValue: 2\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_substitute_int_constant_in_int_variable_declaration_with_expression) {
    StringVec source = {"def main() -> None:", "    x: int = 1", "    y: int = x + x"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType y:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 1\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: y\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 2\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_substitute_float_constant_in_float_variable_declaration_with_expression) {
    StringVec source = {"def main() -> None:", "    x: float = 1.0", "    y: float = x + x"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:FloatType y:FloatType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: x\n"
                           "        Expression: FloatType\n"
                           "          FloatingPointLiteralValue: 1\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: y\n"
                           "        Expression: FloatType\n"
                           "          FloatingPointLiteralValue: 2\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_deal_with_variable_scopes) {
    StringVec source = {"def main() -> None:", "    x: int = 1", "    y: int = x", "    if 1:",
                        "        x: int = 2",  "        y = x",  "    y = x"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType y:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 1\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: y\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 1\n"
                           "      BranchRoot: x:IntType\n"
                           "        VariableDeclaration\n"
                           "          TypeName: IntType\n"
                           "          VariableName: x\n"
                           "          Expression: IntType\n"
                           "            IntegerLiteralValue: 2\n"
                           "        Expression: IntType\n"
                           "          BinaryOperation: Assign\n"
                           "            VariableName: y\n"
                           "            IntegerLiteralValue: 2\n"
                           "      Expression: IntType\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: y\n"
                           "          IntegerLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_remove_unused_function) {
    StringVec source = {"def main() -> None:", "    x: float = 1", "def killMe() -> None:", "    y: float = 1"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:FloatType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: x\n"
                           "        Expression: FloatType\n"
                           "          FloatingPointLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_remove_while_loop_with_always_false_condition) {
    StringVec source = {"def main() -> None:", "    x: int = 1", "    while 0:", "        x = 2"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_join_up_if_with_always_true_condition) {
    StringVec source = {"def main() -> None:", "    x: int = 1", "    if 1:", "        x = 2"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 1\n"
                           "      BranchRoot:\n"
                           "        Expression: IntType\n"
                           "          BinaryOperation: Assign\n"
                           "            VariableName: x\n"
                           "            IntegerLiteralValue: 2\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_join_up_if_with_always_false_condition) {
    StringVec source = {"def main() -> None:", "    x: int = 1", "    if 0:", "        x = 2"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_remove_if_false_and_join_up_else) {
    StringVec source = {
        "def main() -> None:", "    x: int = 1", "    if 0:", "        x = 2", "    else:", "        x = 4"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 1\n"
                           "      BranchRoot:\n"
                           "        Expression: IntType\n"
                           "          BinaryOperation: Assign\n"
                           "            VariableName: x\n"
                           "            IntegerLiteralValue: 4\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_remove_if_false_and_join_up_elif_true) {
    StringVec source = {
        "def main() -> None:", "    x: int = 1", "    if 0:", "        x = 2", "    elif 1:", "        x = 3"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 1\n"
                           "      BranchRoot:\n"
                           "        Expression: IntType\n"
                           "          BinaryOperation: Assign\n"
                           "            VariableName: x\n"
                           "            IntegerLiteralValue: 3\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_remove_if_and_elif_with_always_false_conditions) {
    StringVec source = {
        "def main() -> None:", "    x: int = 1", "    if 0:", "        x = 2", "    elif 0:", "        x = 3"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_remove_if_false_elif_false_and_join_up_else) {
    StringVec source = {"def main() -> None:", "    x: int = 1", "    if 0:", "        x = 2",
                        "    elif 0:",         "        x = 3",  "    else:", "        x = 4"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 1\n"
                           "      BranchRoot:\n"
                           "        Expression: IntType\n"
                           "          BinaryOperation: Assign\n"
                           "            VariableName: x\n"
                           "            IntegerLiteralValue: 4\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_remove_if_false_with_else_and_join_up_elif_true) {
    StringVec source = {"def main() -> None:", "    x: int = 1", "    if 0:", "        x = 2",
                        "    elif 1:",         "        x = 5",  "    else:", "        x = 4"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 1\n"
                           "      BranchRoot:\n"
                           "        Expression: IntType\n"
                           "          BinaryOperation: Assign\n"
                           "            VariableName: x\n"
                           "            IntegerLiteralValue: 5\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_join_up_first_elif_with_true_condition_in_sequence) {
    StringVec source = {"def main() -> None:", "    x: int = 1", "    if 0:",   "        x = 2",
                        "    elif 1:",         "        x = 3",  "    elif 1:", "        x = 5"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 1\n"
                           "      BranchRoot:\n"
                           "        Expression: IntType\n"
                           "          BinaryOperation: Assign\n"
                           "            VariableName: x\n"
                           "            IntegerLiteralValue: 3\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_remove_inaccessible_code_after_return) {
    StringVec source = {"def main() -> int:", "    x: int = 1", "    return 1", "    x = 1"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: IntType\n"
                           "    BranchRoot: x:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 1\n"
                           "      ReturnStatement\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_remove_inaccessible_code_after_infinite_loop) {
    StringVec source = {"def main() -> None:", "    x: int", "    if 1:", "        while 1:",
                        "            x = 2",   "    x = 3 ", "    x = 4 "};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "      BranchRoot:\n"
                           "        WhileStatement\n"
                           "          Expression\n"
                           "            IntegerLiteralValue: 1\n"
                           "          BranchRoot:\n"
                           "            Expression: IntType\n"
                           "              BinaryOperation: Assign\n"
                           "                VariableName: x\n"
                           "                IntegerLiteralValue: 2\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_insert_inline_function) {
    StringVec source = {"def foo(x: int, y: int, z: int) -> int:", "    return x + y",
                        "def main() -> None:", "    x: int = 2", "    x = foo(x + 1, 1, 3) + foo(1, 1, 1)"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: foo\n"
                           "    FunctionArguments\n"
                           "      FunctionArgument\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "      FunctionArgument\n"
                           "        TypeName: IntType\n"
                           "        VariableName: y\n"
                           "      FunctionArgument\n"
                           "        TypeName: IntType\n"
                           "        VariableName: z\n"
                           "    FunctionReturnType: IntType\n"
                           "    BranchRoot: x:IntType y:IntType z:IntType\n"
                           "      ReturnStatement\n"
                           "        Expression: IntType\n"
                           "          BinaryOperation: Add\n"
                           "            VariableName: x\n"
                           "            VariableName: y\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 2\n"
                           "      Expression: IntType\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          IntegerLiteralValue: 6\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Optimizer, can_insert_inline_function_with_type_conversion) {
    StringVec source = {"def foo(x: int, y: int, z: int) -> int:",
                        "    return x + y",
                        "def main() -> None:",
                        "    x: int = 2",
                        "    y: float = 1.0",
                        "    x = foo(x + 1, y, 3)"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: foo\n"
                           "    FunctionArguments\n"
                           "      FunctionArgument\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "      FunctionArgument\n"
                           "        TypeName: IntType\n"
                           "        VariableName: y\n"
                           "      FunctionArgument\n"
                           "        TypeName: IntType\n"
                           "        VariableName: z\n"
                           "    FunctionReturnType: IntType\n"
                           "    BranchRoot: x:IntType y:IntType z:IntType\n"
                           "      ReturnStatement\n"
                           "        Expression: IntType\n"
                           "          BinaryOperation: Add\n"
                           "            VariableName: x\n"
                           "            VariableName: y\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot: x:IntType y:FloatType\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression: IntType\n"
                           "          IntegerLiteralValue: 2\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: y\n"
                           "        Expression: FloatType\n"
                           "          FloatingPointLiteralValue: 1\n"
                           "      Expression: IntType\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          IntegerLiteralValue: 4\n";
    ASSERT_EQ(tree_str, tree.dump());
}
