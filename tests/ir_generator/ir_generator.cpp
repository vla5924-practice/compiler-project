#include <gtest/gtest.h>
#include <iostream>

#include <backend/lexer/lexer.hpp>
#include <backend/parser/parser.hpp>
#include <backend/semantizer/semantizer.hpp>
#include <ir_generator/ir_generator.hpp>
#include <utils/source_files.hpp>

using namespace ast;
using namespace ir_generator;

using lexer::Lexer;
using parser::Parser;
using semantizer::Semantizer;
using utils::SourceFile;

TEST(IRGenerator, can_be_constructed) {
    ASSERT_NO_THROW(IRGenerator generator("module"));
}

TEST(IRGenerator, can_run_generation) {
    SourceFile source = {
        "def main() -> None:",
        "    z: int = 1",
        "    print(z)",
    };
    auto tokens = Lexer::process(source);
    auto tree = Parser::process(tokens);
    Semantizer::process(tree);

    IRGenerator generator("module");
    ASSERT_NO_THROW(generator.process(tree));
}

TEST(IRGenerator, can_generate_and_dump_ir) {
    SourceFile source = {
        "def foo(x: int) -> float:",
        "    z: int = 5 + x",
        "    return 3.5 + z",
        "def main() -> None:",
        "    x: int = 1",
        "    y: float",
        "    y = input()",
        "    print(\"hi\")",
        "    if y > 1.5:",
        "        z: float = foo(x) * 2.5",
        "    else:",
        "        y = 6",
        "        while x < 10:",
        "            x = x + 1",
        "    print(y)",
    };
    auto tokens = Lexer::process(source);
    auto tree = Parser::process(tokens);
    Semantizer::process(tree);
    tree.dump(std::cout);

    IRGenerator generator("module");
    generator.process(tree);

    std::string ir = "";
    generator.dump(std::cout);
    ASSERT_EQ(ir, generator.dump());
}
