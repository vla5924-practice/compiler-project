#pragma once

#include <cstdlib>
#include <string>
#include <string_view>

#include "lexer/token.hpp"

class Parser {
    template <typename RType>
    static RType parseLiteral(const lexer::Token &token);

  public:
    Parser() = delete;
    Parser(const Parser &) = delete;
    Parser(Parser &&) = delete;
    ~Parser() = delete;
};
