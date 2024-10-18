#include <gtest/gtest.h>

#include "compiler/codegen/optree_to_llvmir/llvmir_generator.hpp"

using namespace optree;
using namespace optree::llvmir_generator;

TEST(LLVMIRGenerator, can_be_constructed) {
    LLVMIRGenerator generator("can_be_constructed");
    auto output = generator.dump();
    ASSERT_EQ("; ModuleID = 'can_be_constructed'\nsource_filename = \"can_be_constructed\"\n", output);
}
