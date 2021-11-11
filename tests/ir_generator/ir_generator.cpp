#include <gtest/gtest.h>

#include "ir_generator/ir_generator.hpp"

using namespace ir_generator;

TEST(IRGenerator, generic_test) {
    IRGenerator generator("module");
    ast::SyntaxTree tree;
    generator.process(tree);
    ASSERT_TRUE(true);
}
