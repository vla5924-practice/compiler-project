#include "parser/register_handler.hpp"

using namespace parser;

#define INIT_PARSING_HANDLER(HandlerType)                                                                              \
    extern void registerParsingHandler__##HandlerType();                                                               \
    registerParsingHandler__##HandlerType();

HandlerRegistryImpl::HandlerRegistryImpl() {
    INIT_PARSING_HANDLER(ExpressionHandler);
    INIT_PARSING_HANDLER(FunctionArgumentsHandler);
    INIT_PARSING_HANDLER(FunctionBodyHandler);
    INIT_PARSING_HANDLER(FunctionDefinitionHandler);
    INIT_PARSING_HANDLER(ProgramRootHandler);
    INIT_PARSING_HANDLER(VariableDeclarationHandler);
}

void HandlerRegistryImpl::registerHandler(const ast::NodeType &nodeType, BaseHandler *handler) {
    (*this)[nodeType].reset(handler);
}

HandlerRegistryImpl registry;

HandlerRegistryImpl & ::parser::HandlerRegistry() {
    return registry;
}
