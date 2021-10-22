#pragma once

#include <string>
#include <type_traits>
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

    std::variant<Keyword, Operator, Special, std::string> value;

    Keyword &kw() {
        return std::get<Keyword>(value);
    }
    std::string &id() {
        return std::get<std::string>(value);
    }
    Operator &op() {
        return std::get<Operator>(value);
    }
    Special &spec() {
        return std::get<Special>(value);
    }
    std::string &literal() {
        return std::get<std::string>(value);
    }

    const Keyword &kw() const {
        return std::get<Keyword>(value);
    }
    const std::string &id() const {
        return std::get<std::string>(value);
    }
    const Operator &op() const {
        return std::get<Operator>(value);
    }
    const Special &spec() const {
        return std::get<Special>(value);
    }
    const std::string &literal() const {
        return std::get<std::string>(value);
    }

    template <Type TokenType, typename ValueType>
    static Token make(const ValueType &value) {
        return Token{TokenType, value};
    }

    bool is(const Keyword &value) const {
        return type == Type::Keyword && kw() == value;
    }

    bool is(const Operator &value) const {
        return type == Type::Operator && op() == value;
    }

    bool is(const Special &value) const {
        return type == Type::Special && spec() == value;
    }
};
