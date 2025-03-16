#pragma once

#include <cstdint>
#include <list>
#include <memory>
#include <ostream>
#include <string>
#include <variant>

#include "compiler/utils/source_ref.hpp"

#include "compiler/ast/node_type.hpp"
#include "compiler/ast/types.hpp"
#include "compiler/ast/variables_table.hpp"

namespace ast {

struct Node {
    using Ptr = std::shared_ptr<Node>;
    using ValueStorage =
        std::variant<int64_t, double, bool, std::string, TypeId, BinaryOperation, UnaryOperation, VariablesTable>;

    std::list<Ptr> children;
    Ptr parent;
    utils::SourceRef ref;

    /**
     * value type      : node type
     * int64_t         : IntegerLiteralValue
     * double          : FloatingPointLiteralValue
     * bool            : BooleanLiteralValue
     * std::string     : StringLiteralValue, FunctionName, VariableName
     * TypeId          : TypeName
     * BinaryOperation : BinaryOperation
     * UnaryOperation  : UnaryOperation
     * VariablesTable  : BranchRoot
     * nothing         : other types
     */
    NodeType type;
    ValueStorage value;

    Node() = default;
    ~Node() = default;

    explicit Node(const NodeType &type, const Ptr &parent = {});
    explicit Node(int64_t intNum, const Ptr &parent = {});
    explicit Node(double fpNum, const Ptr &parent = {});
    explicit Node(const NodeType &type, const std::string &str, const Ptr &parent = {});
    explicit Node(TypeId typeId, const Ptr &parent = {});
    explicit Node(BinaryOperation binOp, const Ptr &parent = {});
    explicit Node(UnaryOperation unOp, const Ptr &parent = {});

    const int64_t &intNum() const;
    const double &fpNum() const;
    const bool &boolean() const;
    const std::string &str() const;
    const TypeId &typeId() const;
    const BinaryOperation &binOp() const;
    const UnaryOperation &unOp() const;
    const VariablesTable &variables() const;
    VariablesTable &variables();

    bool operator==(const Node &other) const;
    bool operator!=(const Node &other) const;

    Ptr &firstChild();
    const Ptr &firstChild() const;
    Ptr &secondChild();
    const Ptr &secondChild() const;
    Ptr &lastChild();
    const Ptr &lastChild() const;
    size_t numChildren() const;

    std::string dump(int depth = 0) const;
    void dump(std::ostream &stream, int depth = 0) const;
};

} // namespace ast
