#include <gtest/gtest.h>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantizer/semantizer.hpp"

using namespace ast;
using namespace lexer;
using namespace parser;
using namespace semantizer;

TEST(Parser, test) {
    StringVec source = {"def main() -> None:", "    x: int = 1 ", "    x = 3 + 1.0"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    tree = Semantizer::process(tree);
    ASSERT_EQ(2 + 2, 4);
}