#pragma once

#include "tokens_types.hpp"
#include <string>
#include <variant>

namespace lexer {

struct Token {
    TokenType type;

    std::variant<Keyword, Operator, Special, std::string> value;

    Token(TokenType tokenType, Keyword keyword) : type(tokenType), value(keyword) {
    }
    Token(TokenType tokenType, Operator oper) : type(tokenType), value(oper) {
    }
    Token(TokenType tokenType, Special special) : type(tokenType), value(special) {
    }
    Token(TokenType tokenType, std::string &string) : type(tokenType), value(string) {
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
};

} // namespace lexer
