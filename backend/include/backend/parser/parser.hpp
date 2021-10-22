#pragma once

#include "ast/syntax_tree.hpp"
#include "token.hpp"
#include "tokenlist.hpp"

namespace parser {

class Parser {
    template <typename RType>
    static RType parseLiteral(const Token &token);

  public:
    Parser() = delete;
    Parser(const Parser &) = delete;
    Parser(Parser &&) = delete;
    ~Parser() = delete;

    static ast::SyntaxTree process(const TokenList &tokens);
};

} // namespace parser
