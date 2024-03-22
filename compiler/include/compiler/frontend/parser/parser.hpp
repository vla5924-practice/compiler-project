#pragma once

#include "compiler/ast/syntax_tree.hpp"

#include "compiler/frontend/lexer/token.hpp"

namespace parser {

class Parser {
  public:
    Parser() = delete;
    Parser(const Parser &) = delete;
    Parser(Parser &&) = delete;
    ~Parser() = delete;

    static ast::SyntaxTree process(const lexer::TokenList &tokens);
};

} // namespace parser
