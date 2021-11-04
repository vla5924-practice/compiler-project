#pragma once

#include "ast/functions_table.hpp"
#include "ast/node.hpp"

namespace ast {

class SyntaxTree {
  public:
    SyntaxTree() = default;
    ~SyntaxTree() = default;

    Node::Ptr root;
    FunctionsTable functions;
};

} // namespace ast
