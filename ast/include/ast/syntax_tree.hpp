#pragma once

#include "ast/functions_table.hpp"
#include "ast/node.hpp"

namespace ast {

class SyntaxTree {
  public:
    Node::Ptr root;
    FunctionsTable functions;

    SyntaxTree() = default;
    ~SyntaxTree() = default;

    bool operator==(const SyntaxTree &other) const {
        return *root == *other.root;
    }

    bool operator!=(const SyntaxTree &other) const {
        return !(*this == other);
    }
};

} // namespace ast
