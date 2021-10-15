#pragma once

#include <cstdlib>
#include <string>
#include <string_view>
#include <unordered_set>

#include "ast.hpp"
#include "token.hpp"
#include "tokenlist.hpp"

class Parser {
    static std::unordered_set<std::string> userDefinedTypes;

    template <typename RType>
    static RType parseLiteral(const Token &token);

    static bool isTypename(const Token &token);

  public:
    Parser() = delete;
    Parser(const Parser &) = delete;
    Parser(Parser &&) = delete;
    ~Parser() = delete;

    static AST process(const TokenList &tokens);
};
