#pragma once

#include "parser.hpp"

namespace parser {

template <typename HandlerT>
class RegisterHandler {
  public:
    RegisterHandler(const ast::NodeType &nodeType) {
        Parser::parserHandlers.insert_or_assign(nodeType, std::make_unique<HandlerT>());
    }
};

} // namespace parser
