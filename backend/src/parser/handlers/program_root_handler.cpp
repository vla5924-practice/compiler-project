#include "parser/handlers/program_root_handler.hpp"

#include "parser/register_handler.hpp"

using namespace parser;

void ProgramRootHandler::run(ParserState &state) {
    const Token &currToken = state.token();
    // here can be only function definition
    if (currToken.is(Token::Keyword::Definition)) {
        state.node = state.pushChildNode(ast::NodeType::FunctionDefinition);
    } else {
        // semantic error
    }
    state.goNextToken();
}

REGISTER_PARSING_HANDLER(ProgramRootHandler, ast::NodeType::ProgramRoot);
