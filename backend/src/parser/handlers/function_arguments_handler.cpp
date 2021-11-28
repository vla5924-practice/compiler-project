#include "parser/handlers/function_arguments_handler.hpp"

#include "lexer/token_types.hpp"
#include "parser/parser_error.hpp"
#include "parser/register_handler.hpp"
#include "parser/type_registry.hpp"

using namespace lexer;
using namespace parser;

void FunctionArgumentsHandler::run(ParserState &state) {
    while (!state.token().is(Operator::RightBrace)) {
        const Token &argName = *state.tokenIter;
        const Token &colon = *std::next(state.tokenIter);
        const Token &argType = *std::next(state.tokenIter, 2);
        if (argName.type != TokenType::Identifier || !colon.is(Special::Colon) || !TypeRegistry::isTypename(argType)) {
            state.errors.push<ParserError>(argName, "Function argument declaration is ill-formed");
            while (!state.token().is(Operator::RightBrace) && !state.token().is(Special::Colon))
                state.goNextToken();
            break;
        }
        auto node = state.pushChildNode(ast::NodeType::FunctionArgument);
        auto argTypeNode = ParserState::pushChildNode(node, ast::NodeType::TypeName);
        argTypeNode->value = TypeRegistry::typeId(argType);
        auto argNameNode = ParserState::pushChildNode(node, ast::NodeType::VariableName);
        argNameNode->value = argName.id();

        const Token &last = *std::next(state.tokenIter, 3);
        if (last.is(Operator::Comma))
            std::advance(state.tokenIter, 4);
        else
            std::advance(state.tokenIter, 3);
    }
    state.node = state.node->parent;
    state.goNextToken();
}

REGISTER_PARSING_HANDLER(FunctionArgumentsHandler, ast::NodeType::FunctionArguments);
