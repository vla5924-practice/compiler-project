#pragma once

#include "token_types.hpp"

#include <list>
#include <ostream>
#include <string>
#include <variant>

#include "compiler/utils/source_ref.hpp"

namespace lexer {

struct Token {
    TokenType type;
    std::variant<Keyword, Operator, Special, std::string> value;
    utils::SourceRef ref;

    Token() = default;
    Token(const Token &) = default;
    Token(Token &&) = default;
    ~Token() = default;

    explicit Token(Keyword kw, const utils::SourceRef &ref_ = {}) : type(TokenType::Keyword), value(kw), ref(ref_){};
    explicit Token(Operator op, const utils::SourceRef &ref_ = {}) : type(TokenType::Operator), value(op), ref(ref_){};
    explicit Token(Special spec, const utils::SourceRef &ref_ = {})
        : type(TokenType::Special), value(spec), ref(ref_){};
    Token(TokenType tokenType, const std::string &literal, const utils::SourceRef &ref_ = {})
        : type(tokenType), value(literal), ref(ref_){};

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

    bool is(const TokenType &value) const {
        return type == TokenType::Identifier;
    }

    std::string dump() const;
    void dump(std::ostream &stream) const;
};

using TokenList = std::list<Token>;
using TokenIterator = TokenList::const_iterator;

} // namespace lexer
