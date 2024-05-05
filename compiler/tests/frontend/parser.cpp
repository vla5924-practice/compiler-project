#include <gtest/gtest.h>

#include "compiler/frontend/lexer/lexer.hpp"
#include "compiler/frontend/parser/parser.hpp"
#include "compiler/utils/stringvec.hpp"

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
        "    function(x, y + 1, z + x)",
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
                           "              BinaryOperation: Add\n"
                           "                VariableName: y\n"
                           "                IntegerLiteralValue: 1\n"
                           "            Expression\n"
                           "              BinaryOperation: Add\n"
                           "                VariableName: z\n"
                           "                VariableName: x\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Parser, can_parse_function_definition_with_parameters) {
    StringVec source = {
        "def foo(x: int, y: float, z: bool) -> None:",
        "    return",
    };
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: foo\n"
                           "    FunctionArguments\n"
                           "      FunctionArgument\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "      FunctionArgument\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: y\n"
                           "      FunctionArgument\n"
                           "        TypeName: BoolType\n"
                           "        VariableName: z\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      ReturnStatement\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Parser, can_parse_function_with_return) {
    StringVec source = {
        "def main() -> float:",
        "    return 1 / x",
    };
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: FloatType\n"
                           "    BranchRoot\n"
                           "      ReturnStatement\n"
                           "        Expression\n"
                           "          BinaryOperation: Div\n"
                           "            IntegerLiteralValue: 1\n"
                           "            VariableName: x\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Parser, can_parse_multiple_functions) {
    StringVec source = {
        "def func1() -> None:",  "    x: int",     "    return",
        "def func2() -> int:",   "    y: int = 1", "    return y",
        "def func3() -> float:", "    return 2.3", "    z: int",
        "def func4() -> None:",  "    x: float",
    };
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    std::string tree_str = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: func1\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "      ReturnStatement\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: func2\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: IntType\n"
                           "    BranchRoot\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: y\n"
                           "        Expression\n"
                           "          IntegerLiteralValue: 1\n"
                           "      ReturnStatement\n"
                           "        Expression\n"
                           "          VariableName: y\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: func3\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: FloatType\n"
                           "    BranchRoot\n"
                           "      ReturnStatement\n"
                           "        Expression\n"
                           "          FloatingPointLiteralValue: 2.3\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: z\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: func4\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      VariableDeclaration\n"
                           "        TypeName: FloatType\n"
                           "        VariableName: x\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Parser, can_parse_nested_function_call_alone) {
    StringVec source = {
        "def main() -> None:",
        "    x = foo(bar())",
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
                           "          FunctionCall\n"
                           "            FunctionName: foo\n"
                           "            FunctionArguments\n"
                           "              Expression\n"
                           "                FunctionCall\n"
                           "                  FunctionName: bar\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Parser, can_parse_nested_function_call_inside_expression) {
    StringVec source = {
        "def main() -> None:",
        "    x = foo(1 + bar())",
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
                           "          FunctionCall\n"
                           "            FunctionName: foo\n"
                           "            FunctionArguments\n"
                           "              Expression\n"
                           "                BinaryOperation: Add\n"
                           "                  IntegerLiteralValue: 1\n"
                           "                  FunctionCall\n"
                           "                    FunctionName: bar\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Parser, can_parse_multiple_nested_function_calls) {
    StringVec source = {
        "def main() -> None:",
        "    x = foo(bar(), 1 + baz())",
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
                           "          FunctionCall\n"
                           "            FunctionName: foo\n"
                           "            FunctionArguments\n"
                           "              Expression\n"
                           "                FunctionCall\n"
                           "                  FunctionName: bar\n"
                           "              Expression\n"
                           "                BinaryOperation: Add\n"
                           "                  IntegerLiteralValue: 1\n"
                           "                  FunctionCall\n"
                           "                    FunctionName: baz\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Parser, can_parse_nested_function_call_with_arguments) {
    StringVec source = {
        "def main() -> None:",
        "    x = foo(bar(y))",
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
                           "          FunctionCall\n"
                           "            FunctionName: foo\n"
                           "            FunctionArguments\n"
                           "              Expression\n"
                           "                FunctionCall\n"
                           "                  FunctionName: bar\n"
                           "                  FunctionArguments\n"
                           "                    Expression\n"
                           "                      VariableName: y\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Parser, can_parse_complex_nested_function_call) {
    StringVec source = {
        "def main() -> None:",
        "    x = foo(bar(fun()) + 1, baz(y), z)",
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
                           "          FunctionCall\n"
                           "            FunctionName: foo\n"
                           "            FunctionArguments\n"
                           "              Expression\n"
                           "                BinaryOperation: Add\n"
                           "                  FunctionCall\n"
                           "                    FunctionName: bar\n"
                           "                    FunctionArguments\n"
                           "                      Expression\n"
                           "                        FunctionCall\n"
                           "                          FunctionName: fun\n"
                           "                  IntegerLiteralValue: 1\n"
                           "              Expression\n"
                           "                FunctionCall\n"
                           "                  FunctionName: baz\n"
                           "                  FunctionArguments\n"
                           "                    Expression\n"
                           "                      VariableName: y\n"
                           "              Expression\n"
                           "                VariableName: z\n";
    ASSERT_EQ(tree_str, tree.dump());
}

TEST(Parser, can_parse_bool) {
    StringVec source = {
        "def main() -> None:",
        "    x: bool = True",
        "    x = (3 == 4) and False",
    };
    TokenList tokens = Lexer::process(source);

    SyntaxTree tree = Parser::process(tokens);
    std::string expected = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      VariableDeclaration\n"
                           "        TypeName: BoolType\n"
                           "        VariableName: x\n"
                           "        Expression\n"
                           "          BooleanLiteralValue: True\n"
                           "      Expression\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          BinaryOperation: And\n"
                           "            BinaryOperation: Equal\n"
                           "              IntegerLiteralValue: 3\n"
                           "              IntegerLiteralValue: 4\n"
                           "            BooleanLiteralValue: False\n";
    ASSERT_EQ(expected, tree.dump());
}

TEST(Parser, can_parse_list_declaration) {
    StringVec source = {
        "def main() -> None:",
        "    mylist : list[int]",
    };
    TokenList tokens = Lexer::process(source);
    SyntaxTree tree = Parser::process(tokens);
    std::string expected = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      VariableDeclaration\n"
                           "        TypeName: ListType\n"
                           "          TypeName: IntType\n"
                           "        VariableName: mylist\n";
    ASSERT_EQ(expected, tree.dump());
}

TEST(Parser, can_parse_list_definition) {
    StringVec source = {
        "def main() -> None:",
        "    mylist : list[int] = [1 + 1, 1 + 2, 2 + 3]",
    };
    TokenList tokens = Lexer::process(source);
    SyntaxTree tree = Parser::process(tokens);
    std::string expected = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      VariableDeclaration\n"
                           "        TypeName: ListType\n"
                           "          TypeName: IntType\n"
                           "        VariableName: mylist\n"
                           "        Expression\n"
                           "          ListStatement\n"
                           "            Expression\n"
                           "              BinaryOperation: Add\n"
                           "                IntegerLiteralValue: 1\n"
                           "                IntegerLiteralValue: 1\n"
                           "            Expression\n"
                           "              BinaryOperation: Add\n"
                           "                IntegerLiteralValue: 1\n"
                           "                IntegerLiteralValue: 2\n"
                           "            Expression\n"
                           "              BinaryOperation: Add\n"
                           "                IntegerLiteralValue: 2\n"
                           "                IntegerLiteralValue: 3\n";
    ASSERT_EQ(expected, tree.dump());
}

TEST(Parser, can_parse_list_access) {
    StringVec source = {
        "def main() -> None:",
        "    mylist : list[int] = [1, 2, 3]",
        "    x : int = 1 + mylist[0] - (mylist[1 + 2 * 3] * 2 * mylist[2])",
    };
    TokenList tokens = Lexer::process(source);
    SyntaxTree tree = Parser::process(tokens);
    std::string expected = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      VariableDeclaration\n"
                           "        TypeName: ListType\n"
                           "          TypeName: IntType\n"
                           "        VariableName: mylist\n"
                           "        Expression\n"
                           "          ListStatement\n"
                           "            Expression\n"
                           "              IntegerLiteralValue: 1\n"
                           "            Expression\n"
                           "              IntegerLiteralValue: 2\n"
                           "            Expression\n"
                           "              IntegerLiteralValue: 3\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression\n"
                           "          BinaryOperation: Sub\n"
                           "            BinaryOperation: Add\n"
                           "              IntegerLiteralValue: 1\n"
                           "              ListAccessor\n"
                           "                VariableName: mylist\n"
                           "                Expression\n"
                           "                  IntegerLiteralValue: 0\n"
                           "            BinaryOperation: Mult\n"
                           "              BinaryOperation: Mult\n"
                           "                ListAccessor\n"
                           "                  VariableName: mylist\n"
                           "                  Expression\n"
                           "                    BinaryOperation: Add\n"
                           "                      IntegerLiteralValue: 1\n"
                           "                      BinaryOperation: Mult\n"
                           "                        IntegerLiteralValue: 2\n"
                           "                        IntegerLiteralValue: 3\n"
                           "                IntegerLiteralValue: 2\n"
                           "              ListAccessor\n"
                           "                VariableName: mylist\n"
                           "                Expression\n"
                           "                  IntegerLiteralValue: 2\n";
    ASSERT_EQ(expected, tree.dump());
}

TEST(Parser, can_parse_complex_nested_list) {
    StringVec source = {
        "def main() -> None:",
        "    mylist : list[int] = [1, 2, 3]",
        "    x : int = mylist[mylist[1]]",
        "    y : int = mylist[mylist[1] + mylist[mylist[mylist[1]]]]",
    };
    TokenList tokens = Lexer::process(source);
    SyntaxTree tree = Parser::process(tokens);
    std::string expected = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      VariableDeclaration\n"
                           "        TypeName: ListType\n"
                           "          TypeName: IntType\n"
                           "        VariableName: mylist\n"
                           "        Expression\n"
                           "          ListStatement\n"
                           "            Expression\n"
                           "              IntegerLiteralValue: 1\n"
                           "            Expression\n"
                           "              IntegerLiteralValue: 2\n"
                           "            Expression\n"
                           "              IntegerLiteralValue: 3\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: x\n"
                           "        Expression\n"
                           "          ListAccessor\n"
                           "            VariableName: mylist\n"
                           "            Expression\n"
                           "              ListAccessor\n"
                           "                VariableName: mylist\n"
                           "                Expression\n"
                           "                  IntegerLiteralValue: 1\n"
                           "      VariableDeclaration\n"
                           "        TypeName: IntType\n"
                           "        VariableName: y\n"
                           "        Expression\n"
                           "          ListAccessor\n"
                           "            VariableName: mylist\n"
                           "            Expression\n"
                           "              BinaryOperation: Add\n"
                           "                ListAccessor\n"
                           "                  VariableName: mylist\n"
                           "                  Expression\n"
                           "                    IntegerLiteralValue: 1\n"
                           "                ListAccessor\n"
                           "                  VariableName: mylist\n"
                           "                  Expression\n"
                           "                    ListAccessor\n"
                           "                      VariableName: mylist\n"
                           "                      Expression\n"
                           "                        ListAccessor\n"
                           "                          VariableName: mylist\n"
                           "                          Expression\n"
                           "                            IntegerLiteralValue: 1\n";
    ASSERT_EQ(expected, tree.dump());
}

TEST(Parser, can_parse_list_access_with_nested_function_call) {
    StringVec source = {
        "def main() -> None:",
        "    x = mylist[foo(bar(y))]",
    };
    TokenList tokens = Lexer::process(source);
    SyntaxTree tree = Parser::process(tokens);
    std::string expected = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      Expression\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          ListAccessor\n"
                           "            VariableName: mylist\n"
                           "            Expression\n"
                           "              FunctionCall\n"
                           "                FunctionName: foo\n"
                           "                FunctionArguments\n"
                           "                  Expression\n"
                           "                    FunctionCall\n"
                           "                      FunctionName: bar\n"
                           "                      FunctionArguments\n"
                           "                        Expression\n"
                           "                          VariableName: y\n";
    ASSERT_EQ(expected, tree.dump());
}

TEST(Parser, can_parse_simple_unary_expressions) {
    StringVec source = {
        "def main() -> None:", "    x = (-a)", "    x = -(a)", "    x = -a", "    x = -(a+a)",
    };
    TokenList tokens = Lexer::process(source);
    SyntaxTree tree = Parser::process(tokens);
    std::string expected = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      Expression\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          UnaryOperation: Negative\n"
                           "            Expression\n"
                           "              VariableName: a\n"
                           "      Expression\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          UnaryOperation: Negative\n"
                           "            Expression\n"
                           "              VariableName: a\n"
                           "      Expression\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          UnaryOperation: Negative\n"
                           "            Expression\n"
                           "              VariableName: a\n"
                           "      Expression\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          UnaryOperation: Negative\n"
                           "            Expression\n"
                           "              BinaryOperation: Add\n"
                           "                VariableName: a\n"
                           "                VariableName: a\n";
    ASSERT_EQ(expected, tree.dump());
}

TEST(Parser, can_parse_complex_unary_expressions) {
    StringVec source = {
        "def main() -> None:",
        "    x = -(x + (-a)) * s",
        "    x = -(x + 1 + (-a)) * -1",
        "    x = -(x + 1 + (-a)) * (-1)",
    };
    TokenList tokens = Lexer::process(source);
    SyntaxTree tree = Parser::process(tokens);
    std::string expected = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      Expression\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          BinaryOperation: Mult\n"
                           "            UnaryOperation: Negative\n"
                           "              Expression\n"
                           "                BinaryOperation: Add\n"
                           "                  VariableName: x\n"
                           "                  UnaryOperation: Negative\n"
                           "                    Expression\n"
                           "                      VariableName: a\n"
                           "            VariableName: s\n"
                           "      Expression\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          BinaryOperation: Mult\n"
                           "            UnaryOperation: Negative\n"
                           "              Expression\n"
                           "                BinaryOperation: Add\n"
                           "                  BinaryOperation: Add\n"
                           "                    VariableName: x\n"
                           "                    IntegerLiteralValue: 1\n"
                           "                  UnaryOperation: Negative\n"
                           "                    Expression\n"
                           "                      VariableName: a\n"
                           "            UnaryOperation: Negative\n"
                           "              Expression\n"
                           "                IntegerLiteralValue: 1\n"
                           "      Expression\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          BinaryOperation: Mult\n"
                           "            UnaryOperation: Negative\n"
                           "              Expression\n"
                           "                BinaryOperation: Add\n"
                           "                  BinaryOperation: Add\n"
                           "                    VariableName: x\n"
                           "                    IntegerLiteralValue: 1\n"
                           "                  UnaryOperation: Negative\n"
                           "                    Expression\n"
                           "                      VariableName: a\n"
                           "            UnaryOperation: Negative\n"
                           "              Expression\n"
                           "                IntegerLiteralValue: 1\n";
    ASSERT_EQ(expected, tree.dump());
}

TEST(Parser, can_parse_complex_unary_expressions_with_functions) {
    StringVec source = {
        "def main() -> None:",
        "    x = -(x + fun(-a)) * s",
        "    x = -fun(x) + -fun(x) * foo(-bar(), -bar(-a))",
        "    x = -(x + 1 + (-a)) * fun(-1)",
    };
    TokenList tokens = Lexer::process(source);
    SyntaxTree tree = Parser::process(tokens);
    std::string expected = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      Expression\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          BinaryOperation: Mult\n"
                           "            UnaryOperation: Negative\n"
                           "              Expression\n"
                           "                BinaryOperation: Add\n"
                           "                  VariableName: x\n"
                           "                  FunctionCall\n"
                           "                    FunctionName: fun\n"
                           "                    FunctionArguments\n"
                           "                      Expression\n"
                           "                        UnaryOperation: Negative\n"
                           "                          Expression\n"
                           "                            VariableName: a\n"
                           "            VariableName: s\n"
                           "      Expression\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          BinaryOperation: Add\n"
                           "            UnaryOperation: Negative\n"
                           "              Expression\n"
                           "                FunctionCall\n"
                           "                  FunctionName: fun\n"
                           "                  FunctionArguments\n"
                           "                    Expression\n"
                           "                      VariableName: x\n"
                           "            BinaryOperation: Mult\n"
                           "              UnaryOperation: Negative\n"
                           "                Expression\n"
                           "                  FunctionCall\n"
                           "                    FunctionName: fun\n"
                           "                    FunctionArguments\n"
                           "                      Expression\n"
                           "                        VariableName: x\n"
                           "              FunctionCall\n"
                           "                FunctionName: foo\n"
                           "                FunctionArguments\n"
                           "                  Expression\n"
                           "                    UnaryOperation: Negative\n"
                           "                      Expression\n"
                           "                        FunctionCall\n"
                           "                          FunctionName: bar\n"
                           "                  Expression\n"
                           "                    UnaryOperation: Negative\n"
                           "                      Expression\n"
                           "                        FunctionCall\n"
                           "                          FunctionName: bar\n"
                           "                          FunctionArguments\n"
                           "                            Expression\n"
                           "                              UnaryOperation: Negative\n"
                           "                                Expression\n"
                           "                                  VariableName: a\n"
                           "      Expression\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          BinaryOperation: Mult\n"
                           "            UnaryOperation: Negative\n"
                           "              Expression\n"
                           "                BinaryOperation: Add\n"
                           "                  BinaryOperation: Add\n"
                           "                    VariableName: x\n"
                           "                    IntegerLiteralValue: 1\n"
                           "                  UnaryOperation: Negative\n"
                           "                    Expression\n"
                           "                      VariableName: a\n"
                           "            FunctionCall\n"
                           "              FunctionName: fun\n"
                           "              FunctionArguments\n"
                           "                Expression\n"
                           "                  UnaryOperation: Negative\n"
                           "                    Expression\n"
                           "                      IntegerLiteralValue: 1\n";
    ASSERT_EQ(expected, tree.dump());
}

TEST(Parser, can_parse_complex_unary_expressions_with_lists) {
    StringVec source = {
        "def main() -> None:",
        "    x = -(x + fun[-a]) * s",
        "    x = -fun[x] + -fun[x] * foo[-bar[-a]]",
        "    x = -(x + 1 + mylist[-a]) * fun[-1]",
    };
    TokenList tokens = Lexer::process(source);
    SyntaxTree tree = Parser::process(tokens);
    std::string expected = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      Expression\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          BinaryOperation: Mult\n"
                           "            UnaryOperation: Negative\n"
                           "              Expression\n"
                           "                BinaryOperation: Add\n"
                           "                  VariableName: x\n"
                           "                  ListAccessor\n"
                           "                    VariableName: fun\n"
                           "                    Expression\n"
                           "                      UnaryOperation: Negative\n"
                           "                        Expression\n"
                           "                          VariableName: a\n"
                           "            VariableName: s\n"
                           "      Expression\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          BinaryOperation: Add\n"
                           "            UnaryOperation: Negative\n"
                           "              Expression\n"
                           "                ListAccessor\n"
                           "                  VariableName: fun\n"
                           "                  Expression\n"
                           "                    VariableName: x\n"
                           "            BinaryOperation: Mult\n"
                           "              UnaryOperation: Negative\n"
                           "                Expression\n"
                           "                  ListAccessor\n"
                           "                    VariableName: fun\n"
                           "                    Expression\n"
                           "                      VariableName: x\n"
                           "              ListAccessor\n"
                           "                VariableName: foo\n"
                           "                Expression\n"
                           "                  UnaryOperation: Negative\n"
                           "                    Expression\n"
                           "                      ListAccessor\n"
                           "                        VariableName: bar\n"
                           "                        Expression\n"
                           "                          UnaryOperation: Negative\n"
                           "                            Expression\n"
                           "                              VariableName: a\n"
                           "      Expression\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          BinaryOperation: Mult\n"
                           "            UnaryOperation: Negative\n"
                           "              Expression\n"
                           "                BinaryOperation: Add\n"
                           "                  BinaryOperation: Add\n"
                           "                    VariableName: x\n"
                           "                    IntegerLiteralValue: 1\n"
                           "                  ListAccessor\n"
                           "                    VariableName: mylist\n"
                           "                    Expression\n"
                           "                      UnaryOperation: Negative\n"
                           "                        Expression\n"
                           "                          VariableName: a\n"
                           "            ListAccessor\n"
                           "              VariableName: fun\n"
                           "              Expression\n"
                           "                UnaryOperation: Negative\n"
                           "                  Expression\n"
                           "                    IntegerLiteralValue: 1\n";
    ASSERT_EQ(expected, tree.dump());
}

TEST(Parser, can_parse_simple_unary_bool_expressions) {
    StringVec source = {
        "def main() -> None:",  "    x: bool = not a", "    x = not(a)",      "    x = (not a)",
        "    x = not (a or a)", "    x = not a or a",  "    x = not a and a",
    };
    TokenList tokens = Lexer::process(source);
    SyntaxTree tree = Parser::process(tokens);
    std::string expected = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      VariableDeclaration\n"
                           "        TypeName: BoolType\n"
                           "        VariableName: x\n"
                           "        Expression\n"
                           "          UnaryOperation: Not\n"
                           "            Expression\n"
                           "              VariableName: a\n"
                           "      Expression\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          UnaryOperation: Not\n"
                           "            Expression\n"
                           "              VariableName: a\n"
                           "      Expression\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          UnaryOperation: Not\n"
                           "            Expression\n"
                           "              VariableName: a\n"
                           "      Expression\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          UnaryOperation: Not\n"
                           "            Expression\n"
                           "              BinaryOperation: Or\n"
                           "                VariableName: a\n"
                           "                VariableName: a\n"
                           "      Expression\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          BinaryOperation: Or\n"
                           "            UnaryOperation: Not\n"
                           "              Expression\n"
                           "                VariableName: a\n"
                           "            VariableName: a\n"
                           "      Expression\n"
                           "        BinaryOperation: Assign\n"
                           "          VariableName: x\n"
                           "          BinaryOperation: And\n"
                           "            UnaryOperation: Not\n"
                           "              Expression\n"
                           "                VariableName: a\n"
                           "            VariableName: a\n";
    ASSERT_EQ(expected, tree.dump());
}

TEST(Parser, can_parse_unary_in_if) {
    StringVec source = {
        "def main() -> None:", "    if not x and not True:", "        x = 1", "    if not (x and False):",
        "        x = 1",
    };
    TokenList tokens = Lexer::process(source);
    SyntaxTree tree = Parser::process(tokens);
    std::string expected = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      IfStatement\n"
                           "        Expression\n"
                           "          BinaryOperation: And\n"
                           "            UnaryOperation: Not\n"
                           "              Expression\n"
                           "                VariableName: x\n"
                           "            UnaryOperation: Not\n"
                           "              Expression\n"
                           "                BooleanLiteralValue: True\n"
                           "        BranchRoot\n"
                           "          Expression\n"
                           "            BinaryOperation: Assign\n"
                           "              VariableName: x\n"
                           "              IntegerLiteralValue: 1\n"
                           "      IfStatement\n"
                           "        Expression\n"
                           "          UnaryOperation: Not\n"
                           "            Expression\n"
                           "              BinaryOperation: And\n"
                           "                VariableName: x\n"
                           "                BooleanLiteralValue: False\n"
                           "        BranchRoot\n"
                           "          Expression\n"
                           "            BinaryOperation: Assign\n"
                           "              VariableName: x\n"
                           "              IntegerLiteralValue: 1\n";
    ASSERT_EQ(expected, tree.dump());
}

TEST(Parser, can_parse_for_range_and_enumerate) {
    StringVec source = {
        "def main() -> None:", "    for i in range(0, 10, 1):",
        "        x = 1",       "    for i, value in enumerate(mylist):",
        "        x = 1",
    };
    TokenList tokens = Lexer::process(source);
    SyntaxTree tree = Parser::process(tokens);
    std::string expected = "ProgramRoot\n"
                           "  FunctionDefinition\n"
                           "    FunctionName: main\n"
                           "    FunctionArguments\n"
                           "    FunctionReturnType: NoneType\n"
                           "    BranchRoot\n"
                           "      ForStatement\n"
                           "        ForTargetList\n"
                           "          VariableName: i\n"
                           "        ForStartedList\n"
                           "          Expression\n"
                           "            FunctionCall\n"
                           "              FunctionName: range\n"
                           "              FunctionArguments\n"
                           "                Expression\n"
                           "                  IntegerLiteralValue: 0\n"
                           "                Expression\n"
                           "                  IntegerLiteralValue: 10\n"
                           "                Expression\n"
                           "                  IntegerLiteralValue: 1\n"
                           "        BranchRoot\n"
                           "          Expression\n"
                           "            BinaryOperation: Assign\n"
                           "              VariableName: x\n"
                           "              IntegerLiteralValue: 1\n"
                           "      ForStatement\n"
                           "        ForTargetList\n"
                           "          VariableName: i\n"
                           "          VariableName: value\n"
                           "        ForStartedList\n"
                           "          Expression\n"
                           "            FunctionCall\n"
                           "              FunctionName: enumerate\n"
                           "              FunctionArguments\n"
                           "                Expression\n"
                           "                  VariableName: mylist\n"
                           "        BranchRoot\n"
                           "          Expression\n"
                           "            BinaryOperation: Assign\n"
                           "              VariableName: x\n"
                           "              IntegerLiteralValue: 1\n";
    ASSERT_EQ(expected, tree.dump());
}
