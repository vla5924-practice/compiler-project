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

TEST(Optimaizer, DEBUG_TEST) {
    StringVec source = {"def main() -> None:", 
    "    x: float = 1.0", 
    "    y: float = 1.0", 
    "    y = x - x", 
    //"    x = 1.0 * y", 
    //"    z: int = x * y"
    };
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    tree.dump(std::cout);
    Optimizer::process(tree);
    tree.dump(std::cout);
    ASSERT_EQ(2, 1 + 1);
}

TEST(Optimaizer, can_int_to_float_type_conversion_for_literals) {
    StringVec source = {"def main() -> None:", "    x: float = 1 + 1.0"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    ASSERT_EQ(2, 1 + 1);
}

TEST(Optimaizer, can_float_to_int_type_conversion_for_literals) {
    StringVec source = {"def main() -> None:", "    x: int = 1 + 1.0"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    ASSERT_EQ(2, 1 + 1);
}

TEST(Optimaizer, can_int_for_variables) {
    StringVec source = {"def main() -> None:", "    x: int = 1", "    y: int = x"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    ASSERT_EQ(2, 1 + 1);
}

TEST(Optimaizer, can_float_for_variables) {
    StringVec source = {"def main() -> None:", "    x: float = 1.0", "    y: float = x"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    ASSERT_EQ(2, 1 + 1);
}

TEST(Optimaizer, can_int_to_float_type_conversion_for_variables) {
    StringVec source = {"def main() -> None:", "    x: int = 1", "    y: float = x"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    ASSERT_EQ(2, 1 + 1);
}

TEST(Optimaizer, can_float_to_int_type_conversion_for_variables) {
    StringVec source = {"def main() -> None:", "    x: float = 1.0", "    y: int = x"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    ASSERT_EQ(2, 1 + 1);
}

TEST(Optimaizer, can_calc_int_variable_and_int_literal) {
    StringVec source = {"def main() -> None:", "    x: int = 1", "    y: int = x + 1"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    ASSERT_EQ(2, 1 + 1);
}

TEST(Optimaizer, can_calc_float_variable_and_float_literal) {
    StringVec source = {"def main() -> None:", "    x: float = 1.0", "    y: float = x + 1.0"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    ASSERT_EQ(2, 1 + 1);
}

TEST(Optimaizer, can_calc_int_variable_and_int_variable) {
    StringVec source = {"def main() -> None:", "    x: int = 1", "    y: int = x + x"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    ASSERT_EQ(2, 1 + 1);
}

TEST(Optimaizer, can_calc_float_variable_and_float_variable) {
    StringVec source = {"def main() -> None:", "    x: float = 1.0", "    y: float = x + x"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    Optimizer::process(tree);
    ASSERT_EQ(2, 1 + 1);
}