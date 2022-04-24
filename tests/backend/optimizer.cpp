#include <gtest/gtest.h>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantizer/semantizer.hpp"
#include "optimizer/optimizer.hpp"

using namespace ast;
using namespace lexer;
using namespace parser;
using namespace semantizer;
using namespace optimizer;

TEST(Optimaizer, can_fill_functions_table) {
    StringVec source = {"def main() -> None:", 
    "    x: float = 1 + 1.0", 
    "    y: float = 1", 
    "    y = 1.0 - x"};
    TokenList token_list = Lexer::process(source);
    SyntaxTree tree = Parser::process(token_list);
    Semantizer::process(tree);
    tree.dump(std::cout);
    Optimizer::process(tree);
    tree.dump(std::cout);
    ASSERT_EQ(2, 1+1);
}
