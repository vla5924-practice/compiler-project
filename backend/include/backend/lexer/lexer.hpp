#pragma once

#include <list>
#include <map>
#include <string>
#include <string_view>

#include "lexer/token.hpp"
#include "lexer/tokenlist.hpp"
#include "stringvec.hpp"

namespace lexer {

class Lexer {
    static std::map<std::string_view, Keyword> keywords;
    static std::map<std::string_view, Operator> operators;

    static TokenList processString(const std::string &str);

  public:
    Lexer() = delete;
    Lexer(const Lexer &) = delete;
    Lexer(Lexer &&) = delete;
    ~Lexer() = delete;

    static TokenList process(const StringVec &source);
};

} // namespace lexer
