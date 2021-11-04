#include "parser/handlers/elif_statement_handler.hpp"

#include "lexer/token_types.hpp"
#include "parser/register_handler.hpp"

using namespace lexer;
using namespace parser;

void ElifStatementHandler::run(ParserState &state) {
    const Token &currToken = state.token();

    if (currToken.is(Operator::Colon)) {
        wasInExpression = false;
        state.node = state.pushChildNode(ast::NodeType::BranchRoot);
    } else {
        wasInExpression = true;
        state.node = state.pushChildNode(ast::NodeType::Expression);
    }
    state.goNextToken();
}

void ElifStatementHandler::reset() {
    wasInExpression = false;
}

REGISTER_PARSING_HANDLER(ElifStatementHandler, ast::NodeType::ElifStatement);
