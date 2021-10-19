#include "parser/handlers/function_body_handler.hpp"

#include "parser/register_handler.hpp"

using namespace parser;

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

parser::RegisterHandler<FunctionBodyHandler> handler(ast::NodeType::FunctionDefinition);
