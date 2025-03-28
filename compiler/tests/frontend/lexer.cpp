#include <gtest/gtest.h>

#include "compiler/frontend/lexer/lexer.hpp"
#include "compiler/frontend/lexer/lexer_error.hpp"
#include "compiler/frontend/lexer/token.hpp"
#include "compiler/utils/stringvec.hpp"

#define SINGLE_TOKEN_TEST_IMPL(TOKEN_STR, TOKEN_VALUE)                                                                 \
    StringVec source = {TOKEN_STR};                                                                                    \
    TokenList transformed = Lexer::process(source);                                                                    \
    TokenList expected;                                                                                                \
    expected.emplace_back(TOKEN_VALUE);                                                                                \
    expected.emplace_back(Special::EndOfExpression);                                                                   \
    ASSERT_EQ(expected, transformed);

using namespace lexer;

TEST(Lexer, can_detect_bool) {
    SINGLE_TOKEN_TEST_IMPL("bool", Keyword::Bool);
}

TEST(Lexer, can_detect_int) {
    SINGLE_TOKEN_TEST_IMPL("int", Keyword::Int);
}

TEST(Lexer, can_detect_str) {
    SINGLE_TOKEN_TEST_IMPL("str", Keyword::Str);
}

TEST(Lexer, can_detect_else) {
    SINGLE_TOKEN_TEST_IMPL("else", Keyword::Else);
}

TEST(Lexer, can_detect_for) {
    SINGLE_TOKEN_TEST_IMPL("for", Keyword::For);
}

TEST(Lexer, can_detect_import) {
    SINGLE_TOKEN_TEST_IMPL("import", Keyword::Import);
}

TEST(Lexer, can_detect_def) {
    SINGLE_TOKEN_TEST_IMPL("def", Keyword::Definition);
}

TEST(Lexer, can_detect_or) {
    SINGLE_TOKEN_TEST_IMPL("or", Keyword::Or);
}

TEST(Lexer, can_detect_not) {
    SINGLE_TOKEN_TEST_IMPL("not", Keyword::Not);
}

TEST(Lexer, can_detect_true) {
    SINGLE_TOKEN_TEST_IMPL("True", Keyword::True);
}

TEST(Lexer, can_detect_false) {
    SINGLE_TOKEN_TEST_IMPL("False", Keyword::False);
}

TEST(Lexer, can_detect_float) {
    SINGLE_TOKEN_TEST_IMPL("float", Keyword::Float);
}

TEST(Lexer, can_detect_if) {
    SINGLE_TOKEN_TEST_IMPL("if", Keyword::If);
}

TEST(Lexer, can_detect_elif) {
    SINGLE_TOKEN_TEST_IMPL("elif", Keyword::Elif);
}

TEST(Lexer, can_detect_while) {
    SINGLE_TOKEN_TEST_IMPL("while", Keyword::While);
}

TEST(Lexer, can_detect_break) {
    SINGLE_TOKEN_TEST_IMPL("break", Keyword::Break);
}

TEST(Lexer, can_detect_continue) {
    SINGLE_TOKEN_TEST_IMPL("continue", Keyword::Continue);
}

TEST(Lexer, can_detect_return) {
    SINGLE_TOKEN_TEST_IMPL("return", Keyword::Return);
}

TEST(Lexer, can_detect_and) {
    SINGLE_TOKEN_TEST_IMPL("and", Keyword::And);
}

TEST(Lexer, can_detect_in) {
    SINGLE_TOKEN_TEST_IMPL("in", Keyword::In);
}

TEST(Lexer, can_detect_none) {
    SINGLE_TOKEN_TEST_IMPL("None", Keyword::None);
}

TEST(Lexer, can_detect_pass) {
    SINGLE_TOKEN_TEST_IMPL("pass", Keyword::Pass);
}

TEST(Lexer, can_detect_comma) {
    SINGLE_TOKEN_TEST_IMPL(",", Operator::Comma);
}

TEST(Lexer, can_detect_sub) {
    SINGLE_TOKEN_TEST_IMPL("-", Operator::Sub);
}

TEST(Lexer, can_detect_equal) {
    SINGLE_TOKEN_TEST_IMPL("==", Operator::Equal);
}

TEST(Lexer, can_detect_greater) {
    SINGLE_TOKEN_TEST_IMPL(">", Operator::Greater);
}

TEST(Lexer, can_detect_left_brace) {
    SINGLE_TOKEN_TEST_IMPL("(", Operator::LeftBrace);
}

TEST(Lexer, can_detect_rect_right_brace) {
    SINGLE_TOKEN_TEST_IMPL("]", Operator::RectRightBrace);
}

TEST(Lexer, can_detect_mod) {
    SINGLE_TOKEN_TEST_IMPL("%", Operator::Mod);
}

TEST(Lexer, can_detect_mult) {
    SINGLE_TOKEN_TEST_IMPL("*", Operator::Mult);
}

TEST(Lexer, can_detect_not_equal) {
    SINGLE_TOKEN_TEST_IMPL("!=", Operator::NotEqual);
}

TEST(Lexer, can_detect_less_equal) {
    SINGLE_TOKEN_TEST_IMPL("<=", Operator::LessEqual);
}

TEST(Lexer, can_detect_right_brace) {
    SINGLE_TOKEN_TEST_IMPL(")", Operator::RightBrace);
}

TEST(Lexer, can_detect_dot) {
    SINGLE_TOKEN_TEST_IMPL(".", Operator::Dot);
}

TEST(Lexer, can_detect_add) {
    SINGLE_TOKEN_TEST_IMPL("+", Operator::Add);
}

TEST(Lexer, can_detect_div) {
    SINGLE_TOKEN_TEST_IMPL("/", Operator::Div);
}

TEST(Lexer, can_detect_less) {
    SINGLE_TOKEN_TEST_IMPL("<", Operator::Less);
}

TEST(Lexer, can_detect_assign) {
    SINGLE_TOKEN_TEST_IMPL("=", Operator::Assign);
}

TEST(Lexer, can_detect_greater_equal) {
    SINGLE_TOKEN_TEST_IMPL(">=", Operator::GreaterEqual);
}

TEST(Lexer, can_detect_rect_left_brace) {
    SINGLE_TOKEN_TEST_IMPL("[", Operator::RectLeftBrace);
}

TEST(Lexer, can_detect_indentation) {
    StringVec source = {"if", "    int"};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(Keyword::If);
    expected.emplace_back(Special::EndOfExpression);
    expected.emplace_back(Special::Indentation);
    expected.emplace_back(Keyword::Int);
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, can_detect_arrow) {
    SINGLE_TOKEN_TEST_IMPL("->", Special::Arrow);
}

TEST(Lexer, can_detect_colon) {
    SINGLE_TOKEN_TEST_IMPL(":", Special::Colon);
}

TEST(Lexer, can_detect_identifier) {
    StringVec source = {"x"};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(TokenType::Identifier, "x");
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, can_detect_identifier_with_other) {
    StringVec source = {"int x = 6"};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(Keyword::Int);
    expected.emplace_back(TokenType::Identifier, "x");
    expected.emplace_back(Operator::Assign);
    expected.emplace_back(TokenType::IntegerLiteral, "6");
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, can_detect_identifier_with_numbers) {
    StringVec source = {"int x5924 = 6"};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(Keyword::Int);
    expected.emplace_back(TokenType::Identifier, "x5924");
    expected.emplace_back(Operator::Assign);
    expected.emplace_back(TokenType::IntegerLiteral, "6");
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, can_detect_integer_literal) {
    StringVec source = {"6"};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(TokenType::IntegerLiteral, "6");
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, can_detect_integer_literal_with_operators_no_spaces) {
    StringVec source = {"6+6"};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(TokenType::IntegerLiteral, "6");
    expected.emplace_back(Operator::Add);
    expected.emplace_back(TokenType::IntegerLiteral, "6");
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, can_detect_float_literal) {
    StringVec source = {"6.5"};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(TokenType::FloatingPointLiteral, "6.5");
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, can_detect_float_literal_without_fractional_part) {
    StringVec source = {"6."};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(TokenType::FloatingPointLiteral, "6.");
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, can_detect_string_literal) {
    StringVec source = {"\"string\""};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(TokenType::StringLiteral, "string");
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, can_detect_string_literal_with_arrow_colon) {
    StringVec source = {"\"string : ->\""};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(TokenType::StringLiteral, "string : ->");
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, can_detect_double_indentation) {
    StringVec source = {"        int"};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(Special::Indentation);
    expected.emplace_back(Special::Indentation);
    expected.emplace_back(Keyword::Int);
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, can_detect_tokens_with_spaces) {
    StringVec source = {"int   float  ", "6  ==  7.99  "};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(Keyword::Int);
    expected.emplace_back(Keyword::Float);
    expected.emplace_back(Special::EndOfExpression);
    expected.emplace_back(TokenType::IntegerLiteral, "6");
    expected.emplace_back(Operator::Equal);
    expected.emplace_back(TokenType::FloatingPointLiteral, "7.99");
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, can_skip_empty_line) {
    StringVec source = {"int", "          "};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(Keyword::Int);
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, can_detect_operators_in_a_row) {
    StringVec source = {"+-+-+-+-+-"};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(Operator::Add);
    expected.emplace_back(Operator::Sub);
    expected.emplace_back(Operator::Add);
    expected.emplace_back(Operator::Sub);
    expected.emplace_back(Operator::Add);
    expected.emplace_back(Operator::Sub);
    expected.emplace_back(Operator::Add);
    expected.emplace_back(Operator::Sub);
    expected.emplace_back(Operator::Add);
    expected.emplace_back(Operator::Sub);
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, can_detect_id_in_braces_without_spaces) {
    StringVec source = {"main(a)"};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(TokenType::Identifier, "main");
    expected.emplace_back(Operator::LeftBrace);
    expected.emplace_back(TokenType::Identifier, "a");
    expected.emplace_back(Operator::RightBrace);
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, can_can_detect_id_in_rect_braces_no_spaces) {
    StringVec source = {"main[a]"};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(TokenType::Identifier, "main");
    expected.emplace_back(Operator::RectLeftBrace);
    expected.emplace_back(TokenType::Identifier, "a");
    expected.emplace_back(Operator::RectRightBrace);
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, can_detect_id_with_operators_no_spaces) {
    StringVec source = {"a+b"};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(TokenType::Identifier, "a");
    expected.emplace_back(Operator::Add);
    expected.emplace_back(TokenType::Identifier, "b");
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, can_detect_declarations_no_spaces) {
    StringVec source = {"abc:int,abc1:int"};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(TokenType::Identifier, "abc");
    expected.emplace_back(Special::Colon);
    expected.emplace_back(Keyword::Int);
    expected.emplace_back(Operator::Comma);
    expected.emplace_back(TokenType::Identifier, "abc1");
    expected.emplace_back(Special::Colon);
    expected.emplace_back(Keyword::Int);
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, can_detect_first_symbol_of_id_is_underscore) {
    StringVec source = {"_id"};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(TokenType::Identifier, "_id");
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, can_detect_on_symbol_of_id_is_underscore) {
    StringVec source = {"i_d"};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(TokenType::Identifier, "i_d");
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, raise_error_on_id_starting_with_number) {
    StringVec source = {"int 1xx"};
    ASSERT_THROW(Lexer::process(source), ErrorBuffer);
}

TEST(Lexer, raise_error_on_id_containing_special_symbol_commat) {
    StringVec source = {"int x@x"};
    ASSERT_THROW(Lexer::process(source), ErrorBuffer);
}

TEST(Lexer, raise_error_on_id_containing_special_symbol_dollar) {
    StringVec source = {"int x$x"};
    ASSERT_THROW(Lexer::process(source), ErrorBuffer);
}

TEST(Lexer, raise_error_on_extra_spaces) {
    StringVec source = {"  int"};
    ASSERT_THROW(Lexer::process(source), ErrorBuffer);
}

TEST(Lexer, raise_error_on_extra_spaces_with_several_lines) {
    StringVec source = {"  int", " int"};
    source[0].ref.line = 1;
    source[1].ref.line = 2;
    ErrorBuffer expected;
    utils::SourceRef ref;
    expected.push<LexerError>(ref.inSameFile(1u, 1u), "Extra spaces at the begining of line are not allowed");
    expected.push<LexerError>(ref.inSameFile(2u, 1u), "Extra spaces at the begining of line are not allowed");
    try {
        Lexer::process(source);
    } catch (const ErrorBuffer &errors) {
        ASSERT_EQ(expected.message(), errors.message());
        return;
    }
    FAIL() << "No expected errors were raised.";
}

TEST(Lexer, raise_error_on_literal_without_closing_quote) {
    StringVec source = {"\"quote"};
    ASSERT_THROW(Lexer::process(source), ErrorBuffer);
}

TEST(Lexer, raise_error_on_float_literal_with_alpha) {
    StringVec source = {"6.5A"};
    ASSERT_THROW(Lexer::process(source), ErrorBuffer);
}

TEST(Lexer, raise_error_on_id_starting_with_special) {
    StringVec source = {"int @x"};
    ASSERT_THROW(Lexer::process(source), ErrorBuffer);
}

TEST(Lexer, rect_brace_expression) {
    StringVec source = {"[]"};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(Operator::RectLeftBrace);
    expected.emplace_back(Operator::RectRightBrace);
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, rect_brace_expression_with_values) {
    StringVec source = {"[1, 3.0, Z]"};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(Operator::RectLeftBrace);
    expected.emplace_back(TokenType::IntegerLiteral, "1");
    expected.emplace_back(Operator::Comma);
    expected.emplace_back(TokenType::FloatingPointLiteral, "3.0");
    expected.emplace_back(Operator::Comma);
    expected.emplace_back(TokenType::Identifier, "Z");
    expected.emplace_back(Operator::RectRightBrace);
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, list_expression) {
    StringVec source = {"mylist: list[int] = [1, 2, 3]"};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(TokenType::Identifier, "mylist");
    expected.emplace_back(Special::Colon);
    expected.emplace_back(Keyword::List);
    expected.emplace_back(Operator::RectLeftBrace);
    expected.emplace_back(Keyword::Int);
    expected.emplace_back(Operator::RectRightBrace);
    expected.emplace_back(Operator::Assign);
    expected.emplace_back(Operator::RectLeftBrace);
    expected.emplace_back(TokenType::IntegerLiteral, "1");
    expected.emplace_back(Operator::Comma);
    expected.emplace_back(TokenType::IntegerLiteral, "2");
    expected.emplace_back(Operator::Comma);
    expected.emplace_back(TokenType::IntegerLiteral, "3");
    expected.emplace_back(Operator::RectRightBrace);
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, for_range_and_enumerate_expression) {
    StringVec source = {"for i in range(10)", "for i in enumerate(mylist)"};
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(Keyword::For);
    expected.emplace_back(TokenType::Identifier, "i");
    expected.emplace_back(Keyword::In);
    expected.emplace_back(TokenType::Identifier, "range");
    expected.emplace_back(Operator::LeftBrace);
    expected.emplace_back(TokenType::IntegerLiteral, "10");
    expected.emplace_back(Operator::RightBrace);
    expected.emplace_back(Special::EndOfExpression);
    expected.emplace_back(Keyword::For);
    expected.emplace_back(TokenType::Identifier, "i");
    expected.emplace_back(Keyword::In);
    expected.emplace_back(TokenType::Identifier, "enumerate");
    expected.emplace_back(Operator::LeftBrace);
    expected.emplace_back(TokenType::Identifier, "mylist");
    expected.emplace_back(Operator::RightBrace);
    expected.emplace_back(Special::EndOfExpression);
    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, can_parse_scientific_notation_float_literal) {
    StringVec source = {
        "1e-0", "1.e-1", "1.0e-0", "154e+1", "332.e+23", "1.65e+412",
    };
    TokenList transformed = Lexer::process(source);
    TokenList expected;
    expected.emplace_back(TokenType::FloatingPointLiteral, "1e-0");
    expected.emplace_back(Special::EndOfExpression);
    expected.emplace_back(TokenType::FloatingPointLiteral, "1.e-1");
    expected.emplace_back(Special::EndOfExpression);
    expected.emplace_back(TokenType::FloatingPointLiteral, "1.0e-0");
    expected.emplace_back(Special::EndOfExpression);
    expected.emplace_back(TokenType::FloatingPointLiteral, "154e+1");
    expected.emplace_back(Special::EndOfExpression);
    expected.emplace_back(TokenType::FloatingPointLiteral, "332.e+23");
    expected.emplace_back(Special::EndOfExpression);
    expected.emplace_back(TokenType::FloatingPointLiteral, "1.65e+412");
    expected.emplace_back(Special::EndOfExpression);

    ASSERT_EQ(expected, transformed);
}

TEST(Lexer, raise_error_on_scientific_notation_literal_with_alpha) {
    StringVec source = {"6.5e-0a"};
    ASSERT_THROW(Lexer::process(source), ErrorBuffer);
}
