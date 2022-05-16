#include <gtest/gtest.h>
#include <iostream>

#include "backend/lexer/lexer.hpp"
#include "backend/parser/parser.hpp"
#include "backend/semantizer/semantizer.hpp"
#include "ir_generator/ir_generator.hpp"

using namespace ast;
using namespace ir_generator;

using lexer::Lexer;
using parser::Parser;
using semantizer::Semantizer;

TEST(IRGenerator, generic_test) {
    IRGenerator generator("module");
    // clang-format off
    StringVec source = {
        "def main() -> None:",
        "    z: int = 1",
        "    print(z)",
        "    while z == 1:",
        "        z = z + 1",
    };
    // clang-format on
    auto tokens = Lexer::process(source);
    auto tree = Parser::process(tokens);
    Semantizer::process(tree);
    tree.dump(std::cout);
    generator.process(tree);
    generator.dump();
    ASSERT_TRUE(true);
}
