#pragma once

#include <ast/node.hpp>
#include <ast/types.hpp>

#include "lexer/tokenlist.hpp"
#include "parser/type_registry.hpp"

namespace parser {

struct ParserState {
    ast::Node::Ptr node;
    lexer::TokenList::const_iterator tokenIter;

    const lexer::Token &token() const {
        return *tokenIter;
    }

    static ast::Node::Ptr pushChildNode(ast::Node::Ptr node, const ast::NodeType &nodeType) {
        node->children.emplace_back(new ast::Node(nodeType, node));
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
