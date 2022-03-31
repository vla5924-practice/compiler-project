#pragma once

#include <functional>
#include <stack>
#include <unordered_map>

#include <ast/node.hpp>
#include <ast/types.hpp>

#include "error_buffer.hpp"
#include "lexer/token_types.hpp"
#include "lexer/tokenlist.hpp"
#include "parser/parser_error.hpp"

namespace parser {

struct ParserContext {
    std::unordered_map<ast::NodeType, std::function<void(ParserContext &)>> &subparsers;
    ast::Node::Ptr node;
    lexer::TokenList::const_iterator tokenIter;
    lexer::TokenList::const_iterator tokenEnd;
    int nestingLevel;
    ErrorBuffer errors;

    const lexer::Token &token() const {
        return *tokenIter;
    }

    static ast::Node::Ptr pushChildNode(ast::Node::Ptr node, const ast::NodeType &nodeType) {
        node->children.emplace_back(new ast::Node(nodeType, node));
        return node->children.back();
    }

    static ast::Node::Ptr unshiftChildNode(ast::Node::Ptr node, const ast::NodeType &nodeType) {
        node->children.emplace_front(new ast::Node(nodeType, node));
        return node->children.front();
    }

    ast::Node::Ptr pushChildNode(const ast::NodeType &nodeType) {
        return pushChildNode(node, nodeType);
    }

    void goNextToken() {
        tokenIter++;
    }

    void propagate() {
        subparsers[node->type](*this);
    }

    void pushError(const std::string &message) {
        errors.push<ParserError>(token(), message);
    }

    void goNextExpression() {
        while (!token().is(lexer::Special::EndOfExpression)) {
            goNextToken();
            if (tokenIter == tokenEnd)
                return;
        }
        goNextToken();
    }

    void goParentNode() {
        node = node->parent;
    }
};

} // namespace parser
