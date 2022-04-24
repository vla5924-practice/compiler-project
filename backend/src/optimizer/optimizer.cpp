#include "optimizer/optimizer.hpp"
#include <variant>

using namespace ast;
using namespace optimizer;

using VariablesValue = std::map<std::string, std::variant<long int, double>>;

Node::Ptr &firstChild(Node::Ptr &node) {
    return node->children.front();
}

Node::Ptr &lastChild(Node::Ptr &node) {
    return node->children.back();
}

bool &checkVariableAttribute(const Node::Ptr &node, const std::list<VariablesTable *> &tables) {
    VariablesTable::iterator tableEntry;
    for (const auto &table : tables) {
        tableEntry = table->find(node->str());
        if (tableEntry != table->cend()) {
            break;
        }
    }
    return tableEntry->second.attributes.modified;
}

bool constantPropagation(Node::Ptr &first, Node::Ptr &second, std::list<VariablesTable *> tables,
                         VariablesValue &variablesValue) {
    if (first->type == NodeType::VariableName && checkVariableAttribute(first, tables) == true)
        return false;
    if (second->type == NodeType::VariableName && checkVariableAttribute(second, tables) == true)
        return false;
    auto parent = first->parent;
    if ((first->type == NodeType::IntegerLiteralValue || first->type == NodeType::VariableName) &&
        (second->type == NodeType::IntegerLiteralValue || second->type == NodeType::VariableName)) {
        long int result = 0;
        switch (parent->binOp()) {
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
            return false;
            break;
        }
        parent->type = NodeType::IntegerLiteralValue;
        parent->value = result;
        parent->children.clear();
        return true;
    }
    if ((first->type == NodeType::FloatingPointLiteralValue || first->type == NodeType::VariableName) &&
        (second->type == NodeType::FloatingPointLiteralValue || second->type == NodeType::VariableName)) {
        double result = 0.0;
        double firstValue =
            first->type == NodeType::VariableName ? std::get<1>(variablesValue[first->str()]) : first->fpNum();
        double secondValue =
            second->type == NodeType::VariableName ? std::get<1>(variablesValue[second->str()]) : second->fpNum();
        switch (parent->binOp()) {
        case BinaryOperation::FAdd:
            result = firstValue + secondValue;
            break;
        case BinaryOperation::FSub:
            result = firstValue - secondValue;
            break;
        case BinaryOperation::FMult:
            result = firstValue * secondValue;
            break;
        case BinaryOperation::FDiv:
            result = firstValue / secondValue;
            break;
        default:
            return false;
            break;
        }
        parent->type = NodeType::FloatingPointLiteralValue;
        parent->value = result;
        parent->children.clear();
        return true;
    }
    return false;
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

bool constantFolding(Node::Ptr &first, Node::Ptr &second) {
    auto parent = first->parent;
    if (first->type == NodeType::IntegerLiteralValue && second->type == NodeType::IntegerLiteralValue) {
        long int result = 0;
        switch (parent->binOp()) {
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
            return false;
            break;
        }
        parent->type = NodeType::IntegerLiteralValue;
        parent->value = result;
        parent->children.clear();
        return true;
    }

    if (first->type == NodeType::FloatingPointLiteralValue && second->type == NodeType::FloatingPointLiteralValue) {
        double result = 0.0;
        switch (parent->binOp()) {
        case BinaryOperation::FAdd:
            result = first->fpNum() + second->fpNum();
            break;
        case BinaryOperation::FSub:
            result = first->fpNum() - second->fpNum();
            break;
        case BinaryOperation::FMult:
            result = first->fpNum() * second->fpNum();
            break;
        case BinaryOperation::FDiv:
            result = first->fpNum() / second->fpNum();
            break;
        default:
            return false;
            break;
        }
        parent->type = NodeType::FloatingPointLiteralValue;
        parent->value = result;
        parent->children.clear();
        return true;
    }
    return false;
}

void pushVariableAttribute(Node::Ptr &node, Node::Ptr &child, VariablesValue &variablesValue) {
    TypeId type;
    auto parent = node->parent;
    for (auto &iter : parent->children) {
        if (iter->type == NodeType::TypeName) {
            type = iter->typeId();
        }
        if (iter->type == NodeType::VariableName) {
            if (type == BuiltInTypes::IntType)
                variablesValue.emplace(iter->str(), child->intNum());
            if (type == BuiltInTypes::FloatType)
                variablesValue.emplace(iter->str(), child->fpNum());
        }
    }
}

void processBinaryOperation(Node::Ptr &node, std::list<VariablesTable *> &table, VariablesValue &variablesValue) {
    auto first = firstChild(node);
    auto second = lastChild(node);
    if (second->type == NodeType::BinaryOperation)
        processBinaryOperation(second, table, variablesValue);
    if (first->type == NodeType::TypeConversion)
        processTypeConversion(first);
    if (second->type == NodeType::TypeConversion)
        processTypeConversion(second);
    bool isConsExpr = constantFolding(first, second);
    bool isNotModifiedExpr = false;
    if (!isConsExpr)
        isNotModifiedExpr = constantPropagation(first, second, table, variablesValue);
}

void processExpression(Node::Ptr &node, std::list<VariablesTable *> &table, VariablesValue &variablesValue) {
    for (auto &child : node->children) {
        if (child->type == NodeType::BinaryOperation) {
            auto first = firstChild(child);
            auto second = lastChild(child);
            if (second->type == NodeType::BinaryOperation)
                processBinaryOperation(second, table, variablesValue);
            if (first->type == NodeType::TypeConversion)
                processTypeConversion(first);
            if (second->type == NodeType::TypeConversion)
                processTypeConversion(second);
            bool isConsExpr = constantFolding(first, second);
            bool isNotModifiedExpr = false;
            if (!isConsExpr)
                isNotModifiedExpr = constantPropagation(first, second, table, variablesValue);
            if (isConsExpr || isNotModifiedExpr) {
                pushVariableAttribute(node, child, variablesValue);
            }
            // if (!isConsExpr && child->binOp() == BinaryOperation::Assign)
            // checkVariableAttribute(first, table);
            continue;
        }

        if (child->type == NodeType::TypeConversion) {
            processTypeConversion(child);
            pushVariableAttribute(node, child, variablesValue);
            continue;
        }

        processExpression(child, table, variablesValue);
    }
}

void processBranchRoot(Node::Ptr &node, std::list<VariablesTable *> &table, VariablesValue &variablesValue) {
    table.push_front(&node->variables());
    for (auto &child : node->children) {
        if (child->type == NodeType::Expression) {
            processExpression(child, table, variablesValue);
        }

        if (child->type == NodeType::VariableDeclaration) {
            processExpression(child, table, variablesValue);
            // pushVariableAttribute(child, table, variablesValue);
        }
    }
}

void Optimizer::process(SyntaxTree &tree) {
    for (auto &node : tree.root->children) {
        if (node->type == NodeType::FunctionDefinition) {
            auto child = node->children.begin();
            std::advance(child, 3);
            std::list<VariablesTable *> variablesTable;
            VariablesValue variablesValue;
            processBranchRoot(*child, variablesTable, variablesValue);
        }
    }
}
