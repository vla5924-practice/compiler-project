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
                           "        Expression\n"
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
                           "          FunctionCall\n"
                           "            FunctionName: foo\n"
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

TEST(Optimizer, correct_work_with_scopes) {
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

TEST(Optimizer, DCE_for_function) {
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

TEST(Optimizer, DCE_for_while_0) {
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

TEST(Optimizer, DCE_for_if_1) {
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

TEST(Optimizer, DCE_for_if_0) {
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

TEST(Optimizer, DCE_for_if_0_else) {
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

TEST(Optimizer, DCE_for_if_0_elif_1) {
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

TEST(Optimizer, DCE_for_if_0_elif_0) {
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

TEST(Optimizer, DCE_for_if_0_elif_0_else) {
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

TEST(Optimizer, DCE_for_if_0_elif_1_else) {
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

TEST(Optimizer, DCE_for_if_0_elif_1_elif_1) {
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

TEST(Optimizer, DCE_return_statement) {
    StringVec source = {"def main() -> None:", "    x: int = 1", "    return 1", "    x = 1"};
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
                           "      ReturnStatement\n"
                           "        Expression\n"
                           "          IntegerLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

// TEST(Optimizer, DCE_while_1) {
//     StringVec source = {"def main() -> None:", "    x: int", "    if 1:", "        while 1:", "            x = 2", "
//     x = 3", "    x = 4"}; TokenList token_list = Lexer::process(source); SyntaxTree tree =
//     Parser::process(token_list); Semantizer::process(tree); Optimizer::process(tree); tree.dump(std::cout);
//     std::string tree_str = "ProgramRoot\n"
//                            "  FunctionDefinition\n"
//                            "    FunctionName: main\n"
//                            "    FunctionArguments\n"
//                            "    FunctionReturnType: NoneType\n"
//                            "    BranchRoot: x:IntType\n"
//                            "      VariableDeclaration\n"
//                            "        TypeName: IntType\n"
//                            "        VariableName: x\n"
//                            "        Expression: IntType\n"
//                            "          IntegerLiteralValue: 1\n"
//                            "      ReturnStatement\n"
//                            "        Expression\n"
//                            "          IntegerLiteralValue: 1\n";
//     ASSERT_EQ(tree_str, tree.dump());
// }
//