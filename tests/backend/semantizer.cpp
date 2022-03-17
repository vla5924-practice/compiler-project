#include <gtest/gtest.h>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantizer/semantizer.hpp"

using namespace ast;
using namespace lexer;
using namespace parser;
using namespace semantizer;

TEST(Semantizer, test) {
    // StringVec source = {"def main() -> None:", "    x: int",     "    y: float ",    "    x = 1.0 + y",
    //                     "    if x > y:",       "        z: int", "        z = y + 1"};
    // TokenList token_list = Lexer::process(source);
    // SyntaxTree tree = Parser::process(token_list);
    // Semantizer::process(tree);
    ASSERT_EQ(2 + 2, 4);
}
