#pragma once

#include <map>
#include <memory>

#include <ast/node_type.hpp>

#include "backend/parser/handlers/base_handler.hpp"

namespace parser {

class HandlerRegistry : public std::map<ast::NodeType, std::unique_ptr<BaseHandler>> {
    void registerHandler(const ast::NodeType &nodeType, BaseHandler *handler);

  public:
    HandlerRegistry();
};

} // namespace parser
