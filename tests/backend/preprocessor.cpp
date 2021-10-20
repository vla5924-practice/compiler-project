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

TEST(Preprocessor, can_remove_comment_for_multiple_line) {
    StringVec source = {"x = 1 # x declaration", "y = 2 # y declaration", "z = 3 # z declaration"};
    StringVec transformed = Preprocessor::process(source);
    StringVec expected = {"x = 1 ", "y = 2 ", "z = 3 "};
    ASSERT_EQ(expected, transformed);
}

TEST(Preprocessor, can_skip_str_without_comment) {
    StringVec source = {"x = 1"};
    StringVec transformed = Preprocessor::process(source);
    ASSERT_EQ(source, transformed);
}

TEST(Preprocessor, can_remove_skip_comment_symbol_in_string) {
    StringVec source = {"x = \"#x\""};
    StringVec transformed = Preprocessor::process(source);
    StringVec expected = {"x = \"#x\""};
    ASSERT_EQ(expected, transformed);
}


TEST(Preprocessor, can_remove_comment_in_str_with_string_with_comment_symbol) {
    StringVec source = {"x = \"#x\" # x declaration"};
    StringVec transformed = Preprocessor::process(source);
    StringVec expected = {"x = \"#x\" "};
    ASSERT_EQ(expected, transformed);
}
