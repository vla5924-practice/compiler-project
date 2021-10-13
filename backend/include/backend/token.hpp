#pragma once

#include <string>
#include <variant>

struct Token {
    enum class Keyword {
        Indentation,
        Bool,
        Int,
        Float,
        String,
        If,
        Else,
        Elif,
        Range,
        While,
        For,
        Break,
        Import,
        Continue,
        Definition,
        Return,
        Or,
        And,
        Not,
        In,
        True,
        None,
        False
    };

    enum class Operator {
        Colon,
        Semicolon,
        Dot,
        Comma,
        Assign,
        Plus,
        Minus,
        Mult,
        Div,
        Equal,
        NotEqual,
        Less,
        More,
        LessEqual,
        MoreEqual,
        LeftBrace,
        RightBrace,
        RectLeftBrace,
        RectRightBrace,
        Apostrophe,
        Mod
    };

    enum class Type { Keyword, Identifier, Operator, IntegerLiteral, FloatingPointLiteral, StringLiteral };
    Type type;

    std::variant<Keyword, Operator, std::string> kwValue, opValue, strValue; // idValue???

    /*Token() = default;
    Token(const Token&) = default;
    Token(Token&&) = default;
    ~Token() = default;*/

    Keyword &kw() {
        return std::get<0>(kwValue);
    }
    std::string &id() {
        return std::get<2>(strValue);
    }
    Operator &op() {
        return std::get<1>(opValue);
    }
    std::string &literal() {
        return std::get<2>(strValue);
    }

    const Keyword &kw() const {
        return std::get<0>(kwValue);
    }
    const std::string &id() const {
        return std::get<2>(strValue);
    }
    const Operator &op() const {
        return std::get<1>(opValue);
    }
    const std::string &literal() const {
        return std::get<2>(strValue);
    }

    template <Type TokenType, typename ValueType>
    static Token make(const ValueType &value) {
        return Token{TokenType, value};
    }
};
