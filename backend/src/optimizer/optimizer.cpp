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

void constantFolding(Node::Ptr &first, Node::Ptr &second) {
    if (first->type == NodeType::IntegerLiteralValue && second->type == NodeType::IntegerLiteralValue) {
        long int result = 0;
        switch (first->parent->binOp()) {
        case BinaryOperation::Add:
            result = first->intNum() + second->intNum();
            break;
        case BinaryOperation::Sub:
            result = first->intNum() - second->intNum();
            break;
        case BinaryOperation::Mult:
            result = first->intNum() * second->intNum();
            break;
        case BinaryOperation::Div:
            result = first->intNum() / second->intNum();
            break;
        default:
            return;
            break;
        }
        auto parent = first->parent;
        parent->type = NodeType::IntegerLiteralValue;
        parent->value = result;
        parent->children.clear();
    }
}

void processExpression(Node::Ptr &node, std::list<VariablesTable *> &table) {
    for (auto &child : node->children) {
        if (child->type == NodeType::TypeConversion) {
            processTypeConversion(child);
        }
    }

    for (auto &child : node->children) {
        if (child->type == NodeType::BinaryOperation) {
            auto first = firstChild(child);
            auto second = lastChild(child);
            if (first->type == NodeType::TypeConversion)
                processTypeConversion(first);
            if (second->type == NodeType::TypeConversion)
                processTypeConversion(second);
            constantFolding(first, second);
            return;
        }

        processExpression(child, table);
    }
}

void processBranchRoot(Node::Ptr &node, std::list<VariablesTable *> &table) {
    table.push_front(&node->variables());
    for (auto &child : node->children) {
        if (child->type == NodeType::Expression) {
            processExpression(node, table);
        }

        if (child->type == NodeType::VariableDeclaration) {
            processExpression(node, table);
        }
    }
}

void Optimizer::process(SyntaxTree &tree) {
    for (auto &node : tree.root->children) {
        if (node->type == NodeType::FunctionDefinition) {
            auto child = node->children.begin();
            std::advance(child, 3);
            std::list<VariablesTable *> variables_table;
            processBranchRoot(*child, variables_table);
        }
    }
}
