#include "parser/handlers/variable_declaration_handler.hpp"

#include "parser/register_handler.hpp"
#include "parser/type_registry.hpp"

using namespace parser;

void VariableDeclarationHandler::run(ParserState &state) {
    const Token &colon = state.token();
    const Token &varName = *std::prev(state.tokenIter);
    const Token &varType = *std::next(state.tokenIter);

    auto node = state.pushChildNode(ast::NodeType::TypeName);
    node->uid() = TypeRegistry::typeId(varType);
    node = state.pushChildNode(ast::NodeType::VariableName);
    node->strLiteral = varType.id();

    auto endOfDecl = std::next(state.tokenIter, 2);
    if (endOfDecl->is(Token::Special::EndOfExpression)) {
        // declaration without definition
    } else if (endOfDecl->is(Token::Operator::Assign)) {
        // declaration with definition
        state.node = state.pushChildNode(ast::NodeType::Expression);
    } else {
        // semantic error
    }
    std::advance(state.tokenIter, 3);
}

parser::RegisterHandler<VariableDeclarationHandler> handler(ast::NodeType::VariableDeclaration);
