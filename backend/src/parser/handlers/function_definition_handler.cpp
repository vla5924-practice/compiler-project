#include "parser/handlers/function_definition_handler.hpp"

#include "lexer/token_types.hpp"
#include "parser/register_handler.hpp"
#include "parser/type_registry.hpp"

using namespace lexer;
using namespace parser;

void FunctionDefinitionHandler::run(ParserState &state) {
    const Token &currToken = state.token();
    const Token &prevToken = *std::prev(state.tokenIter);
    // in function definition
    if (currToken.type == TokenType::Identifier && prevToken.is(Keyword::Definition)) {
        // save function name
        state.node->value = currToken.id();
    } else if (currToken.is(Operator::LeftBrace) && prevToken.type == TokenType::Identifier) {
        // start analyzing arguments
        state.node = state.pushChildNode(ast::NodeType::FunctionArguments);
        functionArgumentsEnd = true;
    } else if (currToken.is(Operator::Arrow) && functionArgumentsEnd) {
        // save return type on next step
        functionArgumentsEnd = false;
    } else if (TypeRegistry::isTypename(currToken) && prevToken.is(Operator::Arrow)) {
        auto node = state.pushChildNode(ast::NodeType::FunctionReturnType);
        node->value = TypeRegistry::typeId(currToken);
    } else if (currToken.is(Operator::Colon) && TypeRegistry::isTypename(prevToken)) {
        // end of function header
        state.node = state.pushChildNode(ast::NodeType::FunctionBody);
        functionBegin = true;
    } else {
        // semantic error
    }
    state.goNextToken();
}

void FunctionDefinitionHandler::reset() {
    functionArgumentsEnd = false;
    functionBegin = false;
}

REGISTER_PARSING_HANDLER(FunctionDefinitionHandler, ast::NodeType::FunctionDefinition);
