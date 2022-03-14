#include "parser/handlers/if_statement_handler.hpp"

#include "lexer/token_types.hpp"
#include "parser/parser_error.hpp"

using namespace lexer;
using namespace parser;

void IfStatementHandler::run(ParserState &state) {
    const Token &currToken = state.token();

    if (currToken.is(Special::Colon)) {
        wasInExpression = false;
        state.node = state.pushChildNode(ast::NodeType::BranchRoot);
    } else if (currToken.is(Keyword::Elif)) {
        if (branch == Branch::If) {
            wasInExpression = true;
            branch = Branch::Elif;
            state.node = state.pushChildNode(ast::NodeType::ElifStatement);
        } else {
            state.errors.push<ParserError>(currToken, "elif is not allowed here");
        }
        state.goNextToken();
    } else if (currToken.is(Keyword::Else)) {
        if (branch == Branch::If || branch == Branch::Elif) {
            branch = Branch::Else;
            state.node = state.pushChildNode(ast::NodeType::BranchRoot);
        } else {
            state.errors.push<ParserError>(currToken, "else is not allowed here");
        }
        state.goNextToken();
    } else {
        wasInExpression = true;
        state.node = state.pushChildNode(ast::NodeType::Expression);
    }
}

void IfStatementHandler::reset() {
    branch = Branch::If;
    wasInExpression = false;
}
