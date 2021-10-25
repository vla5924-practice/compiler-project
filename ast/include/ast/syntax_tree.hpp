#pragma once

#include "ast/node.hpp"

namespace ast {

class SyntaxTree {
  public:
    SyntaxTree() = default;
    ~SyntaxTree() = default;

    Node::Ptr root;
};

} // namespace ast
