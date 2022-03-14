#include "parser/handlers/variable_declaration_handler.hpp"

#include "lexer/token_types.hpp"
#include "parser/parser_error.hpp"
#include "parser/type_registry.hpp"

using namespace lexer;
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
    node->value = varName.id();

    auto endOfDecl = std::next(state.tokenIter, 2);
    if (endOfDecl->is(Special::EndOfExpression)) {
        // declaration without definition
        std::advance(state.tokenIter, 3);
        state.node = state.node->parent;
    } else if (endOfDecl->is(Operator::Assign)) {
        // declaration with definition
        state.node = state.pushChildNode(ast::NodeType::Expression);
        wasInDefinition = true;
        std::advance(state.tokenIter, 3);
    } else {
        state.errors.push<ParserError>(*endOfDecl, "Definition expression or line break was expected");
    }
}

void VariableDeclarationHandler::reset() {
    wasInDefinition = false;
}
