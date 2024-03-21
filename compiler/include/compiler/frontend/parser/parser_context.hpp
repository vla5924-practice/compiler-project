#pragma once

#include <functional>
#include <stack>
#include <unordered_map>

#include "compiler/ast/node.hpp"
#include "compiler/ast/types.hpp"
#include "compiler/utils/error_buffer.hpp"
#include "compiler/utils/source_ref.hpp"

#include "compiler/frontend/lexer/token.hpp"
#include "compiler/frontend/lexer/token_types.hpp"
#include "compiler/frontend/parser/parser_error.hpp"

namespace parser {

struct ParserContext {
    std::unordered_map<ast::NodeType, std::function<void(ParserContext &)>> &subparsers;
    ast::Node::Ptr node;
    lexer::TokenIterator tokenIter;
    lexer::TokenIterator tokenEnd;
    int nestingLevel;
    ErrorBuffer errors;

    const lexer::Token &token() const {
        return *tokenIter;
    }

    static ast::Node::Ptr pushChildNode(ast::Node::Ptr node, const ast::NodeType &nodeType,
                                        const utils::SourceRef &ref) {
        auto &childNode = node->children.emplace_back(new ast::Node(nodeType, node));
        childNode->ref = ref;
        return childNode;
    }

    static ast::Node::Ptr unshiftChildNode(ast::Node::Ptr node, const ast::NodeType &nodeType,
                                           const utils::SourceRef &ref) {
        auto &childNode = node->children.emplace_front(new ast::Node(nodeType, node));
        childNode->ref = ref;
        return childNode;
    }

    ast::Node::Ptr pushChildNode(const ast::NodeType &nodeType) {
        return pushChildNode(node, nodeType, tokenIter->ref);
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
