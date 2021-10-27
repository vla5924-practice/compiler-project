#include "parser/parser.hpp"

#include "parser/parser_state.hpp"
#include "parser/register_handler.hpp"

using namespace ast;
using namespace lexer;
using namespace parser;

SyntaxTree Parser::process(const TokenList &tokens) {
    SyntaxTree tree;
    tree.root = std::make_shared<Node>(NodeType::ProgramRoot);

    for (auto &[nodeType, handler] : HandlerRegistry()) {
        handler->reset();
    }

    ParserState state = {tree.root, tokens.begin()};
    while (state.tokenIter != tokens.end()) {
        HandlerRegistry()[state.node->type]->run(state);
    }
    return tree;
}
