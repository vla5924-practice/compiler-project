#pragma once

#include <list>
#include <memory>
#include <ostream>
#include <string>
#include <variant>

#include <utils/source_ref.hpp>

#include "ast/node_type.hpp"
#include "ast/variables_table.hpp"

namespace ast {

struct Node {
    using Ptr = std::shared_ptr<Node>;
    using WeakPtr = std::weak_ptr<Node>;

    std::list<Ptr> children;
    Ptr parent;

    utils::SourceRef ref;

    /**
     * value type      : node type
     * long int        : IntegerLiteralValue
     * double          : FloatingPointLiteralValue
     * std::string     : StringLiteralValue, FunctionName, VariableName
     * TypeId          : TypeName
     * BinaryOperation : BinaryOperation
     * UnaryOperation  : UnaryOperation
     * VariablesTable  : BranchRoot
     * nothing         : other types
     */
    NodeType type;
    std::variant<long int, double, std::string, TypeId, BinaryOperation, UnaryOperation, VariablesTable> value;

    const long int &intNum() const {
        return std::get<long int>(value);
    }
    const double &fpNum() const {
        return std::get<double>(value);
    }
    const std::string &str() const {
        return std::get<std::string>(value);
    }
    const TypeId &typeId() const {
        return std::get<TypeId>(value);
    }
    const BinaryOperation &binOp() const {
        return std::get<BinaryOperation>(value);
    }
    const UnaryOperation &unOp() const {
        return std::get<UnaryOperation>(value);
    }
    const VariablesTable &variables() const {
        return std::get<VariablesTable>(value);
    }
    VariablesTable &variables() {
        return std::get<VariablesTable>(value);
    }

    bool operator==(const Node &other) const {
        if (children.size() != other.children.size())
            return false;
        if (children.size() > 0) {
            for (auto i = children.begin(), j = other.children.begin(); i != children.end(); i++, j++)
                if (**i != **j)
                    return false;
        }
        return type == other.type && value == other.value;
    }

    bool operator!=(const Node &other) const {
        return !(*this == other);
    }

    Node() = default;
    ~Node() = default;

    explicit Node(const NodeType &type_, Ptr parent_ = Ptr()) : type(type_), parent(parent_){};
    explicit Node(long int intNum_, Ptr parent_ = Ptr())
        : type(NodeType::IntegerLiteralValue), value(intNum_), parent(parent_){};
    explicit Node(double fpNum_, Ptr parent_ = Ptr())
        : type(NodeType::FloatingPointLiteralValue), value(fpNum_), parent(parent_){};
    Node(const NodeType &type_, const std::string &str_, Ptr parent_ = Ptr())
        : type(type_), value(str_), parent(parent_){};
    explicit Node(TypeId typeId_, Ptr parent_ = Ptr()) : type(NodeType::TypeName), value(typeId_), parent(parent_){};
    explicit Node(BinaryOperation binOp_, Ptr parent_ = Ptr())
        : type(NodeType::BinaryOperation), value(binOp_), parent(parent_){};
    explicit Node(UnaryOperation unOp_, Ptr parent_ = Ptr())
        : type(NodeType::UnaryOperation), value(unOp_), parent(parent_){};

    std::string dump(int depth = 0) const;
    void dump(std::ostream &stream, int depth = 0) const;

    Node::Ptr &firstChild() {
        return children.front();
    }

    const Node::Ptr &firstChild() const {
        return children.front();
    }

    Node::Ptr &secondChild() {
        return *std::next(children.begin());
    }

    const Node::Ptr &secondChild() const {
        return *std::next(children.begin());
    }

    Node::Ptr &lastChild() {
        return children.back();
    }

    const Node::Ptr &lastChild() const {
        return children.back();
    }
};

using NodeList = std::list<Node::Ptr>;
using NodeIterator = NodeList::iterator;

} // namespace ast
