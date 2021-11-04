#include "parser/handlers/program_root_handler.hpp"

#include "lexer/token_types.hpp"
#include "parser/register_handler.hpp"

using namespace lexer;
using namespace parser;

void ProgramRootHandler::run(ParserState &state) {
    const Token &currToken = state.token();
    // here can be only function definition
    if (currToken.is(Keyword::Definition)) {
        state.node = state.pushChildNode(ast::NodeType::FunctionDefinition);
    } else {
        // semantic error
    }
    state.goNextToken();
}

REGISTER_PARSING_HANDLER(ProgramRootHandler, ast::NodeType::ProgramRoot);
