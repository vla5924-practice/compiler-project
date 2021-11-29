#include "parser/handlers/function_definition_handler.hpp"

#include "lexer/token_types.hpp"
#include "parser/parser_error.hpp"
#include "parser/register_handler.hpp"
#include "parser/type_registry.hpp"

using namespace lexer;
using namespace parser;

void FunctionDefinitionHandler::run(ParserState &state) {
    const Token &currToken = state.token();
    const Token &prevToken = *std::prev(state.tokenIter);
    if (currToken.type == TokenType::Identifier && prevToken.is(Keyword::Definition)) {
        auto node = state.pushChildNode(ast::NodeType::FunctionName);
        node->value = currToken.id();
    } else if (currToken.is(Operator::LeftBrace) && prevToken.type == TokenType::Identifier) {
        state.node = state.pushChildNode(ast::NodeType::FunctionArguments);
        functionArgumentsEnd = true;
    } else if (currToken.is(Special::Arrow) && functionArgumentsEnd) {
        functionArgumentsEnd = false;
    } else if (TypeRegistry::isTypename(currToken) && prevToken.is(Special::Arrow)) {
        auto node = state.pushChildNode(ast::NodeType::FunctionReturnType);
        node->value = TypeRegistry::typeId(currToken);
    } else if (currToken.is(Special::Colon) && TypeRegistry::isTypename(prevToken)) {
        state.node = state.pushChildNode(ast::NodeType::BranchRoot);
        functionBegin = true;
    } else {
        state.errors.push<ParserError>(currToken, "Given token is not allowed here in function definition");
    }
    state.goNextToken();
}

void FunctionDefinitionHandler::reset() {
    functionArgumentsEnd = false;
    functionBegin = false;
}

REGISTER_PARSING_HANDLER(FunctionDefinitionHandler, ast::NodeType::FunctionDefinition);
