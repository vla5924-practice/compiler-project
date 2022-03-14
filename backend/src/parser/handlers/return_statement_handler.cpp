#include "parser/handlers/return_statement_handler.hpp"

#include "lexer/token_types.hpp"
#include "parser/parser_error.hpp"

using namespace lexer;
using namespace parser;

void ReturnStatementHandler::run(ParserState &state) {
    const Token &currToken = state.token();
    if (currToken.is(Special::EndOfExpression)) {
        // function returns None or value was already parsed
        state.node = state.node->parent;
        state.goNextToken();
        return;
    }
    if (currToken.type != TokenType::FloatingPointLiteral && currToken.type != TokenType::Identifier &&
        currToken.type != TokenType::IntegerLiteral && currToken.type != TokenType::StringLiteral &&
        !currToken.is(Operator::LeftBrace)) {
        state.errors.push<ParserError>(currToken, "Expression as function return value was expected");
        while (!state.token().is(Special::EndOfExpression)) {
            state.goNextToken();
        }
        return;
    }
    state.node = state.pushChildNode(ast::NodeType::Expression);
}
