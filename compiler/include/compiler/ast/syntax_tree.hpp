#pragma once

#include <ostream>

#include "compiler/ast/functions_table.hpp"
#include "compiler/ast/node.hpp"

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

    std::string dump() const;
    void dump(std::ostream &stream) const;
};

} // namespace ast
