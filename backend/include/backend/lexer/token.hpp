#pragma once

#include "token_types.hpp"
#include <string>
#include <variant>

namespace lexer {

struct Token {
    TokenType type;

    std::variant<Keyword, Operator, Special, std::string> value;

    explicit Token(Keyword kw) : type(TokenType::Keyword), value(kw){};
    explicit Token(Operator op) : type(TokenType::Operator), value(op){};
    explicit Token(Special spec) : type(TokenType::Special), value(spec){};
    Token(TokenType tokenType, const std::string &literal) : type(tokenType), value(literal){};

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

    bool operator==(const Token &other) const {
        return value == other.value && type == other.type;
    }

    bool operator!=(const Token &other) const {
        return !(*this == other);
    }

    bool is(const Keyword &value) const {
        return type == TokenType::Keyword && kw() == value;
    }

    bool is(const Operator &value) const {
        return type == TokenType::Operator && op() == value;
    }

    bool is(const Special &value) const {
        return type == TokenType::Special && spec() == value;
    }
};

} // namespace lexer
