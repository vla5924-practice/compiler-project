#include <gtest/gtest.h>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantizer/semantizer.hpp"

using namespace ast;
using namespace lexer;
using namespace parser;
using namespace semantizer;

TEST(Semantizer, test) {
    StringVec source = {"def main(t:int) -> None:", "    x: int", "    y: float ", "    x = 1.0 + y", ""};
    //"def foo(t:int) -> None:",       "    z: int"
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    tree.dump(std::cout);
    ASSERT_EQ(2 + 2, 4);
}
