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
        False,
    };

    enum class Operator {
        Colon,
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
        Mod,
        Arrow, // TODO: add "->" to lexer
    };

    enum class Special {
        Indentation,
        EndOfExpression,
    };

    enum class Type {
        Keyword,
        Identifier,
        Operator,
        Special,
        IntegerLiteral,
        FloatingPointLiteral,
        StringLiteral,
    };
    Type type;

    std::variant<Keyword, Operator, Special, std::string> kwValue, opValue, specValue, strValue;

    Keyword &kw() {
        return std::get<0>(kwValue);
    }
    std::string &id() {
        return std::get<3>(strValue);
    }
    Operator &op() {
        return std::get<1>(opValue);
    }
    Special &spec() {
        return std::get<2>(specValue);
    }
    std::string &literal() {
        return std::get<3>(strValue);
    }

    const Keyword &kw() const {
        return std::get<0>(kwValue);
    }
    const std::string &id() const {
        return std::get<3>(strValue);
    }
    const Operator &op() const {
        return std::get<1>(opValue);
    }
    const Special &spec() const {
        return std::get<2>(specValue);
    }
    const std::string &literal() const {
        return std::get<3>(strValue);
    }

    template <Type TokenType, typename ValueType>
    static Token make(const ValueType &value) {
        return Token{TokenType, value};
    }
};
