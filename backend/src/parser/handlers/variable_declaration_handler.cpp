#include "parser/handlers/variable_declaration_handler.hpp"

#include "parser/register_handler.hpp"
#include "parser/type_registry.hpp"

using namespace parser;

void VariableDeclarationHandler::run(ParserState &state) {
    if (wasInDefinition) {
        wasInDefinition = false;
        state.node = state.node->parent;
        state.goNextToken();
        return;
    }
    const Token &colon = state.token();
    const Token &varName = *std::prev(state.tokenIter);
    const Token &varType = *std::next(state.tokenIter);

    auto node = state.pushChildNode(ast::NodeType::TypeName);
    node->value = TypeRegistry::typeId(varType);
    node = state.pushChildNode(ast::NodeType::VariableName);
    node->value = varType.id();

    auto endOfDecl = std::next(state.tokenIter, 2);
    if (endOfDecl->is(Token::Special::EndOfExpression)) {
        // declaration without definition
    } else if (endOfDecl->is(Token::Operator::Assign)) {
        // declaration with definition
        state.node = state.pushChildNode(ast::NodeType::Expression);
        wasInDefinition = true;
    } else {
        // semantic error
    }
    std::advance(state.tokenIter, 3);
}

void VariableDeclarationHandler::reset() {
    wasInDefinition = false;
}

REGISTER_PARSING_HANDLER(VariableDeclarationHandler, ast::NodeType::VariableDeclaration);
