#pragma once

#include <cstdlib>
#include <string>
#include <string_view>

#include "token.hpp"

class Parser {
    template <typename RType>
    static RType parseLiteral(const Token &token);

  public:
    Parser() = delete;
    Parser(const Parser &) = delete;
    Parser(Parser &&) = delete;
    ~Parser() = delete;
};
