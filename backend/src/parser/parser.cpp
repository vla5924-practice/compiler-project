#include "parser/parser.hpp"

#include "parser/parser_state.hpp"
#include "parser/handler_registry.hpp"

using namespace ast;
using namespace lexer;
using namespace parser;

SyntaxTree Parser::process(const TokenList &tokens) {
    SyntaxTree tree;
    tree.root = std::make_shared<Node>(NodeType::ProgramRoot);

    HandlerRegistry registry;
    for (auto &[nodeType, handler] : registry) {
        handler->reset();
    }

    ParserState state = {tree.root, tokens.begin(), tokens.end()};
    while (state.tokenIter != tokens.end()) {
        registry[state.node->type]->run(state);
        if (!state.errors.empty()) {
            throw state.errors;
        }
    }

    return tree;
}
