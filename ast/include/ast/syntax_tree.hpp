#pragma once

#include <ostream>

#include "ast/functions_table.hpp"
#include "ast/node.hpp"

namespace ast {

class SyntaxTree {
  public:
    SyntaxTree() = default;
    ~SyntaxTree() = default;

    Node::Ptr root;
    FunctionsTable functions;

    std::string dump() const;
    void dump(std::ostream &stream) const;
};

} // namespace ast
