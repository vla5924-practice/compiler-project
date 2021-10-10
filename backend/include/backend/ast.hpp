#pragma once

#include <memory>

#include "astnode.hpp"

class AST {
    ASTNode::Ptr root_;

  public:
    AST() = default;
    ~AST() = default;

    ASTNode::Ptr root() const;
};
