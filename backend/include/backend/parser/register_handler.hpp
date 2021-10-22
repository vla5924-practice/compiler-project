#pragma once

#include <map>
#include <memory>

#include "ast/node_type.hpp"
#include "parser/handlers/base_handler.hpp"

#define REGISTER_PARSING_HANDLER(HandlerType, AstNodeType)                                                             \
    extern void registerParsingHandler__##HandlerType();                                                               \
    void registerParsingHandler__##HandlerType() {                                                                     \
        static parser::RegisterHandler<HandlerType> handler(AstNodeType);                                              \
    }

namespace parser {

class HandlerRegistryImpl : public std::map<ast::NodeType, std::unique_ptr<BaseHandler>> {
    template <typename HandlerT>
    friend class RegisterHandler;

    void registerHandler(const ast::NodeType &nodeType, BaseHandler *handler);

  public:
    HandlerRegistryImpl();
};

HandlerRegistryImpl &HandlerRegistry();

template <typename HandlerT>
class RegisterHandler {
  public:
    RegisterHandler(const ast::NodeType &nodeType) {
        HandlerRegistry().registerHandler(nodeType, new HandlerT);
    }
};

} // namespace parser
