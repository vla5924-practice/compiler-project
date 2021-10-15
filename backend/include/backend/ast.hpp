#pragma once

#include <memory>

#include "astnode.hpp"

class AST {
  public:
    AST() = default;
    ~AST() = default;

    ASTNode::Ptr root;
};
