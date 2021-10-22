#include "parser/handlers/function_definition_handler.hpp"

#include "parser/register_handler.hpp"
#include "parser/type_registry.hpp"

using namespace parser;

void FunctionDefinitionHandler::run(ParserState &state) {
    const Token &currToken = state.token();
    const Token &prevToken = *std::prev(state.tokenIter);
    // in function definition
    if (currToken.type == Token::Type::Identifier && prevToken.is(Token::Keyword::Definition)) {
        // save function name
        state.node->strLiteral = currToken.id();
    } else if (currToken.is(Token::Operator::LeftBrace) && prevToken.type == Token::Type::Identifier) {
        // start analyzing arguments
        state.node = state.pushChildNode(ast::NodeType::FunctionArguments);
    } else if (currToken.is(Token::Operator::Arrow) && functionArgumentsEnd) {
        // save return type on next step
    } else if (TypeRegistry::isTypename(currToken) && prevToken.is(Token::Operator::Arrow)) {
        auto node = state.pushChildNode(ast::NodeType::FunctionReturnType);
        node->uid() = TypeRegistry::typeId(currToken);
    } else if (currToken.is(Token::Operator::Colon) && TypeRegistry::isTypename(prevToken)) {
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
