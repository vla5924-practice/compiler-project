#pragma once

#include <memory>
#include <ostream>
#include <string>

#include "compiler/ast/node.hpp"
#include "compiler/ast/node_type.hpp"
#include "compiler/ast/syntax_tree.hpp"

namespace ast {

class DeclarativeTree {
    Node::Ptr root;
    Node::Ptr parent;
    Node::Ptr current;

  public:
    DeclarativeTree(const DeclarativeTree &) = delete;
    DeclarativeTree(DeclarativeTree &&) = default;
    ~DeclarativeTree() = default;

    explicit DeclarativeTree(NodeType rootNodeType = NodeType::ProgramRoot)
        : root(std::make_shared<Node>(rootNodeType)), parent(root), current(root){};

    bool operator==(const DeclarativeTree &other) const {
        return *root == *other.root;
    }

    bool operator!=(const DeclarativeTree &other) const {
        return !(*this == other);
    }

    Node::Ptr rootNode() const {
        return root;
    }

    [[nodiscard]] SyntaxTree makeTree() const {
        SyntaxTree tree;
        tree.root = root;
        return tree;
    }

    DeclarativeTree &node(NodeType type) {
        current = std::make_shared<Node>(type, parent);
        parent->children.push_back(current);
        return *this;
    }

    template <typename VariantType>
    DeclarativeTree &node(NodeType type, const VariantType &value) {
        node(type);
        current->value = value;
        return *this;
    }

    void withChildren() {
        parent = current;
        current = nullptr;
    }

    void endChildren() {
        parent = parent->parent;
        current = nullptr;
    }

    [[nodiscard]] std::string dump() const {
        return root->dump();
    }

    void dump(std::ostream &stream) const {
        root->dump(stream);
    }
};

} // namespace ast
