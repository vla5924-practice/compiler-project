#include "optimizer/optimizer.hpp"

using namespace ast;
using namespace optimizer;

Node::Ptr &firstChild(Node::Ptr &node) {
    return node->children.front();
}

Node::Ptr &lastChild(Node::Ptr &node) {
    return node->children.back();
}

void processTypeConversion(Node::Ptr &node) {
    Node::Ptr &last_child = lastChild(node);
    if (last_child->type == NodeType::IntegerLiteralValue || last_child->type == NodeType::FloatingPointLiteralValue) {
        if (firstChild(node)->typeId() == BuiltInTypes::FloatType) {
            node->type = NodeType::FloatingPointLiteralValue;
            node->value = static_cast<float>(last_child->intNum());
        } else {
            node->type = NodeType::IntegerLiteralValue;
            node->value = static_cast<long int>(last_child->fpNum());
        }
        node->children.clear();
    }
}

void Optimizer::process(SyntaxTree &tree) {
    for (auto &node : tree.root->children) {
    }
}
