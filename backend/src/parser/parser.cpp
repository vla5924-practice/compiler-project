#include "parser/parser.hpp"

using namespace ast;
using namespace parser;

std::unordered_map<ast::NodeType, std::unique_ptr<parser::BaseHandler>> Parser::parserHandlers = {};

template <>
int Parser::parseLiteral(const Token &token) {
    return atoi(token.literal().c_str());
}

template <>
long Parser::parseLiteral(const Token &token) {
    return atol(token.literal().c_str());
}

template <>
double Parser::parseLiteral(const Token &token) {
    return atof(token.literal().c_str());
}

template <>
std::string_view Parser::parseLiteral(const Token &token) {
    return token.literal();
}

template <>
std::string Parser::parseLiteral(const Token &token) {
    return token.literal();
}

ast::SyntaxTree Parser::process(const TokenList &tokens) {
    SyntaxTree tree;
    tree.root = std::make_shared<Node>(NodeType::ProgramRoot);

    parser::ParserState state = {tree.root, tokens.begin()};
    while (state.tokenIter != tokens.end()) {
        parserHandlers[state.node->type]->run(state);
    }
    return tree;
}
