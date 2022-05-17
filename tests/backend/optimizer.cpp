#include <gtest/gtest.h>

#include "lexer/lexer.hpp"
#include "optimizer/optimizer.hpp"
#include "parser/parser.hpp"
#include "semantizer/semantizer.hpp"

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

TEST(Optimizer, DCE_for_function) {
    StringVec source = {"def main() -> None:", "    x: float = 1", "def killMe() -> None:", "    y: float = 1"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    tree.dump(std::cout);
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
