#pragma once

#include "ast/node.hpp"
#include "tokenlist.hpp"

namespace parser {

struct ParserState {
    ast::Node::Ptr node;
    TokenList::const_iterator tokenIter;

    const Token &token() const {
        return *tokenIter;
    }

    static ast::Node::Ptr pushChildNode(ast::Node::Ptr node, const ast::NodeType &nodeType) {
        node->children.push_back(std::make_shared<ast::Node>(ast::NodeType::FunctionArguments, node));
        return node->children.back();
    }

    ast::Node::Ptr pushChildNode(const ast::NodeType &nodeType) {
        return pushChildNode(node, nodeType);
    }

    void goNextToken() {
        tokenIter++;
    }
};

} // namespace parser
