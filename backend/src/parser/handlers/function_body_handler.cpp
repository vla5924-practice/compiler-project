#include "parser/handlers/function_body_handler.hpp"

#include "parser/register_handler.hpp"
#include "parser/type_registry.hpp"

using namespace parser;

namespace {

bool isVariableDeclaration(TokenList::const_iterator tokenIter) {
    const Token &varName = *tokenIter;
    const Token &colon = *std::next(tokenIter);
    const Token &varType = *std::next(tokenIter, 2);
    return varName.type == Token::Type::Identifier && colon.is(Token::Operator::Colon) ||
           TypeRegistry::isTypename(varType);
}

} // namespace

void FunctionBodyHandler::run(ParserState &state) {
    const Token &currToken = state.token();
    const Token &prevToken = *std::prev(state.tokenIter);
    if (prevToken.is(Token::Special::Indentation)) {
        nestingLevel++;
    }
    if (currToken.is(Token::Special::Indentation)) {
        state.goNextToken();
        return;
    }
    if (nestingLevel > 1) {
        // syntax error: extra indentations
        return;
    }
    if (currToken.is(Token::Keyword::If) && prevToken.is(Token::Special::EndOfExpression)) {
        // start of conditional expression
        state.node = state.pushChildNode(ast::NodeType::IfExpression);
    } else if (currToken.is(Token::Keyword::While) && prevToken.is(Token::Special::EndOfExpression)) {
        // start of cycle expression
        state.node = state.pushChildNode(ast::NodeType::WhileExpression);
    } else if (isVariableDeclaration(state.tokenIter)) {
        state.node = state.pushChildNode(ast::NodeType::VariableDeclaration);
    } else {
        // try to parse arithmetic/logic expression using postfix form
    }
    // TODO: add range-based for
    // TODO: add errors handling
    state.goNextToken();
}

void FunctionBodyHandler::reset() {
    nestingLevel = 0;
}

REGISTER_PARSING_HANDLER(FunctionBodyHandler, ast::NodeType::FunctionBody);
