#include <gtest/gtest.h>

#include "preprocessor/preprocessor.hpp"
#include "stringvec.hpp"

using namespace preprocessor;

TEST(Preprocessor, can_remove_full_comment_line) {
    StringVec source = {"x = 1 # x declaration", "# y = 2", "z = 3 # z declaration"};
    StringVec transformed = Preprocessor::process(source);
    StringVec expected = {"x = 1 ", "z = 3 "};
    ASSERT_EQ(expected, transformed);
}

TEST(Preprocessor, can_remove_comment_for_single_line) {
    StringVec source = {"x = 1 # x declaration"};
    StringVec transformed = Preprocessor::process(source);
    StringVec expected = {"x = 1 "};
    ASSERT_EQ(expected, transformed);
}

TEST(Preprocessor, can_skip_multiple_literals) {
    StringVec source = {"x = \"#x\", \"#a\" # x a"};
    StringVec transformed = Preprocessor::process(source);
    StringVec expected = {"x = \"#x\", \"#a\" "};
    ASSERT_EQ(expected, transformed);
}

TEST(Preprocessor, can_remove_comment_with_comment_symbol_for_single_line) {
    StringVec source = {"x = 1 # x # declaration"};
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

TEST(Preprocessor, can_skip_line_without_comment) {
    StringVec source = {"x = 1"};
    StringVec transformed = Preprocessor::process(source);
    ASSERT_EQ(source, transformed);
}

TEST(Preprocessor, can_remove_skip_comment_symbol_in_literal) {
    StringVec source = {"x = \"#x\""};
    StringVec transformed = Preprocessor::process(source);
    StringVec expected = {"x = \"#x\""};
    ASSERT_EQ(expected, transformed);
}

TEST(Preprocessor, can_remove_comment_in_line_with_literal_with_comment_symbol) {
    StringVec source = {"x = \"#x\" # x declaration"};
    StringVec transformed = Preprocessor::process(source);
    StringVec expected = {"x = \"#x\" "};
    ASSERT_EQ(expected, transformed);
}
