#pragma once

#include <list>
#include <map>
#include <string>
#include <string_view>

#include "lexer/token.hpp"
#include "stringvec.hpp"
#include "tokenlist.hpp"


namespace lexer {

class Lexer {
    using Type = Token::Type;
    using Keyword = Token::Keyword;
    using Operator = Token::Operator;

    static std::map<std::string_view, Token::Keyword> keywords;
    static std::map<std::string_view, Token::Operator> operators;

    static TokenList processString(const std::string &str);

  public:
    Lexer() = delete;
    Lexer(const Lexer &) = delete;
    Lexer(Lexer &&) = delete;
    ~Lexer() = delete;

    static TokenList process(const StringVec &source);
};

} // namespace lexer
