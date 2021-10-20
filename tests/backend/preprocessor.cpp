#include <gtest/gtest.h>

#include "preprocessor/preprocessor.hpp"
#include "stringvec.hpp"

using namespace preprocessor;

TEST(Preprocessor, can_remove_comment_for_single_line) {
    StringVec source = {"x = 1 # x declaration"};
    StringVec transformed = Preprocessor::process(source);
    StringVec expected = {"x = 1 "};
    ASSERT_EQ(expected, transformed);
}
