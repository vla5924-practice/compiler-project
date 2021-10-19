#pragma once

#include <cstdlib>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "ast/syntax_tree.hpp"
#include "token.hpp"
#include "tokenlist.hpp"

#include "parser/handlers/base_handler.hpp"

namespace parser {

class Parser {
    template <typename RType>
    static RType parseLiteral(const Token &token);

  public:
    template <typename HandlerT>
    friend class RegisterHandler;
    static std::unordered_map<ast::NodeType, std::unique_ptr<BaseHandler>> parserHandlers;

  public:
    Parser() = delete;
    Parser(const Parser &) = delete;
    Parser(Parser &&) = delete;
    ~Parser() = delete;

    static ast::SyntaxTree process(const TokenList &tokens);
};

} // namespace parser
