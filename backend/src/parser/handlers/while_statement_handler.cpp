#include "parser/handlers/while_statement_handler.hpp"

#include "lexer/token_types.hpp"

using namespace lexer;
using namespace parser;

void WhileStatementHandler::run(ParserState &state) {
    const Token &currToken = state.token();

    if (currToken.is(Special::Colon)) {
        wasInExpression = false;
        state.node = state.pushChildNode(ast::NodeType::BranchRoot);
        state.goNextToken();
    } else {
        wasInExpression = true;
        state.node = state.pushChildNode(ast::NodeType::Expression);
    }
}

void WhileStatementHandler::reset() {
    wasInExpression = false;
}
