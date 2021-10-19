#pragma once

#include <list>
#include <memory>
#include <string>

#include "ast/node_type.hpp"

namespace ast {

struct Node {
    using Ptr = std::shared_ptr<Node>;
    using WeakPtr = std::weak_ptr<Node>;

    std::list<Ptr> children;
    Ptr parent;

    NodeType type;

    union {
        long int intNumber;
        double fpNumber;
    } numLiteral;
    std::string strLiteral;

    Node() = default;
    Node(const NodeType &type_, Ptr parent_ = Ptr()) : type(type_), parent(parent_){};
    ~Node() = default;
};

} // namespace ast
