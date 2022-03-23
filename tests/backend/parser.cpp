#include <gtest/gtest.h>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

using namespace ast;
using namespace lexer;
using namespace parser;

TEST(Parser, can_parse_if_statement) {
    StringVec source = {
        "def main() -> None:", "    x: int = 1 ", "    if x == 2:", "        x = 3", "    else:", "        x = 1",
    };
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression\n"
                           "          IntegerLiteralValue: 1\n"
                           "      IfStatement\n"
                           "        Expression\n"
                           "          BinaryOperation: Equal\n"
                           "            VariableName: x\n"
                           "            IntegerLiteralValue: 2\n"
                           "        BranchRoot\n"
                           "          Expression\n"
                           "            BinaryOperation: Assign\n"
                           "              VariableName: x\n"
                           "              IntegerLiteralValue: 3\n"
                           "        ElseStatement\n"
                           "          BranchRoot\n"
                           "            Expression\n"
                           "              BinaryOperation: Assign\n"
                           "                VariableName: x\n"
                           "                IntegerLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Parser, can_parse_if_elif_statement) {
    StringVec source = {
        "def main() -> None:", "    x: int = 1 ",  "    if x == 2:",
        "        x = 3",       "    elif x == 3:", "        x = 2",
    };
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression\n"
                           "          IntegerLiteralValue: 1\n"
                           "      IfStatement\n"
                           "        Expression\n"
                           "          BinaryOperation: Equal\n"
                           "            VariableName: x\n"
                           "            IntegerLiteralValue: 2\n"
                           "        BranchRoot\n"
                           "          Expression\n"
                           "            BinaryOperation: Assign\n"
                           "              VariableName: x\n"
                           "              IntegerLiteralValue: 3\n"
                           "        ElifStatement\n"
                           "          Expression\n"
                           "            BinaryOperation: Equal\n"
                           "              VariableName: x\n"
                           "              IntegerLiteralValue: 3\n"
                           "          BranchRoot\n"
                           "            Expression\n"
                           "              BinaryOperation: Assign\n"
                           "                VariableName: x\n"
                           "                IntegerLiteralValue: 2\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Parser, can_parse_while_statement) {
    StringVec source = {
        "def main() -> None:",
        "    x: int = 1 ",
        "    while x == 2:",
        "        x = 3",
    };
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression\n"
                           "          IntegerLiteralValue: 1\n"
                           "      WhileStatement\n"
                           "        Expression\n"
                           "          BinaryOperation: Equal\n"
                           "            VariableName: x\n"
                           "            IntegerLiteralValue: 2\n"
                           "        BranchRoot\n"
                           "          Expression\n"
                           "            BinaryOperation: Assign\n"
                           "              VariableName: x\n"
                           "              IntegerLiteralValue: 3\n";

    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Parser, can_parse_long_expression) {
    StringVec source = {
        "def main() -> None:",
        "    x = (y + z + 5 * 2) / 2",
    };
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      Expression\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          BinaryOperation: Div\n"
                           "            BinaryOperation: Add\n"
                           "              BinaryOperation: Add\n"
                           "                VariableName: y\n"
                           "                VariableName: z\n"
                           "              BinaryOperation: Mult\n"
                           "                IntegerLiteralValue: 5\n"
                           "                IntegerLiteralValue: 2\n"
                           "            IntegerLiteralValue: 2\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Parser, can_parse_nested_if_statement) {
    StringVec source = {
        "def main() -> None:",
        "    if x == 2:",
        "        if x > 1:",
        "            x = 3",
    };
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      IfStatement\n"
                           "        Expression\n"
                           "          BinaryOperation: Equal\n"
                           "            VariableName: x\n"
                           "            IntegerLiteralValue: 2\n"
                           "        BranchRoot\n"
                           "          IfStatement\n"
                           "            Expression\n"
                           "              BinaryOperation: Greater\n"
                           "                VariableName: x\n"
                           "                IntegerLiteralValue: 1\n"
                           "            BranchRoot\n"
                           "              Expression\n"
                           "                BinaryOperation: Assign\n"
                           "                  VariableName: x\n"
                           "                  IntegerLiteralValue: 3\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Parser, can_parse_function_call) {
    StringVec source = {
        "def main() -> None:",
        "    function(x, y, z)",
    };
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      Expression\n"
                           "        FunctionCall\n"
                           "          FunctionName: function\n"
                           "          FunctionArguments\n"
                           "            Expression\n"
                           "              VariableName: x\n"
                           "            Expression\n"
                           "              VariableName: y\n"
                           "            Expression\n"
                           "              VariableName: z\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Parser, can_parse_function_with_return) {
    StringVec source = {
        "def main() -> int:",
        "    return 1",
    };
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: IntType\n"
                           "    BranchRoot\n"
                           "      ReturnStatement\n"
                           "        Expression\n"
                           "          IntegerLiteralValue: 1\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Parser, can_parse_multiple_functions) {
    StringVec source = {
        "def foo() -> None:",
        "    y: int",
        "def bar() -> None:",
        "    x: int",
    };
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: foo\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: y\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: bar\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n";
    ASSERT_EQ(tree_str, tree.dump());
}