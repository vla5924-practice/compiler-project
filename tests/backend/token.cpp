#include <gtest/gtest.h>

#include "lexer/token.hpp"

using namespace lexer;

TEST(Token, can_construct_keyword) {
    Keyword kw = Keyword::If;
    Token token(kw);
    ASSERT_EQ(token.type, TokenType::Keyword);
    ASSERT_EQ(token.kw(), kw);
}

TEST(Token, can_construct_special) {
    Special spec = Special::Indentation;
    Token token(spec);
    ASSERT_EQ(token.type, TokenType::Special);
    ASSERT_EQ(token.spec(), spec);
}

TEST(Token, can_construct_operator) {
    Operator op = Operator::Add;
    Token token(op);
    ASSERT_EQ(token.type, TokenType::Operator);
    ASSERT_EQ(token.op(), op);
}

TEST(Token, can_construct_identifier) {
    TokenType type = TokenType::Identifier;
    std::string id = "identifier";
    Token token(type, id);
    ASSERT_EQ(token.type, type);
    ASSERT_EQ(token.id(), id);
}

TEST(Token, can_construct_integer_literal) {
    TokenType type = TokenType::IntegerLiteral;
    std::string literal = "5924";
    Token token(type, literal);
    ASSERT_EQ(token.type, type);
    ASSERT_EQ(token.literal(), literal);
}

TEST(Token, can_construct_float_literal) {
    TokenType type = TokenType::FloatingPointLiteral;
    std::string literal = "1.0";
    Token token(type, literal);
    ASSERT_EQ(token.type, type);
    ASSERT_EQ(token.literal(), literal);
}

TEST(Token, can_construct_string_literal) {
    TokenType type = TokenType::StringLiteral;
    std::string literal = "string";
    Token token(type, literal);
    ASSERT_EQ(token.type, type);
    ASSERT_EQ(token.literal(), literal);
}

TEST(Token, can_compare_equal) {
    Token token1(Keyword::Float);
    Token token2(Keyword::Float);
    ASSERT_TRUE(token1 == token2);
}

TEST(Token, can_compare_not_equal) {
    Token token1(Keyword::Float);
    Token token2(Keyword::Int);
    ASSERT_TRUE(token1 != token2);
}

TEST(Token, can_check_is_keyword) {
    Token token(Keyword::Float);
    ASSERT_TRUE(token.is(Keyword::Float));
}

TEST(Token, can_check_is_operator) {
    Token token(Operator::Add);
    ASSERT_TRUE(token.is(Operator::Add));
}

TEST(Token, can_check_is_special) {
    Token token(Special::EndOfExpression);
    ASSERT_TRUE(token.is(Special::EndOfExpression));
}
