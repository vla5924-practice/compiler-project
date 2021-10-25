#pragma once

#include <list>
#include <memory>
#include <string>
#include <variant>

#include "ast/node_type.hpp"

namespace ast {

struct Node {
    using Ptr = std::shared_ptr<Node>;
    using WeakPtr = std::weak_ptr<Node>;

    std::list<Ptr> children;
    Ptr parent;

    NodeType type;

    std::variant<long int, double, std::string, size_t> value;

    const long int &intNum() const {
        return std::get<long int>(value);
    }
    const double &fpNum() const {
        return std::get<double>(value);
    }
    const std::string &str() const {
        return std::get<std::string>(value);
    }
    const std::string &id() const {
        return std::get<std::string>(value);
    }
    const size_t &uid() const {
        return std::get<size_t>(value);
    }

    Node() = default;
    Node(const NodeType &type_, Ptr parent_ = Ptr()) : type(type_), parent(parent_){};
    ~Node() = default;
};

} // namespace ast
