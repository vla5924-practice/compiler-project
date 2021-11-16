#pragma once

#include <list>
#include <memory>
#include <string>
#include <variant>

#include "ast/node_type.hpp"
#include "ast/variables_table.hpp"

namespace ast {

struct Node {
    using Ptr = std::shared_ptr<Node>;
    using WeakPtr = std::weak_ptr<Node>;

    std::list<Ptr> children;
    Ptr parent;

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
};

} // namespace ast
