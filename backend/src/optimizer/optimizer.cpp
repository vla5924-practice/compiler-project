#include "optimizer/optimizer.hpp"
#include <variant>

using namespace ast;
using namespace optimizer;

using VariablesValue = std::map<std::string, std::variant<long int, double>>; // TODO std::map -> std::list<std::map>

Node::Ptr &firstChild(Node::Ptr &node) {
    return node->children.front();
}

Node::Ptr &lastChild(Node::Ptr &node) {
    return node->children.back();
}

bool &getVariableAttribute(const Node::Ptr &node, const std::list<VariablesTable *> &tables) {
    VariablesTable::iterator tableEntry;
    for (const auto &table : tables) {
        tableEntry = table->find(node->str());
        if (tableEntry != table->cend()) {
            break;
        }
    }
    return tableEntry->second.attributes.modified;
}

long int calcIntOperation(Node::Ptr &first, Node::Ptr &second, BinaryOperation operation, VariablesValue *variablesValue = nullptr) {
    long int firstValue =
        first->type == NodeType::VariableName ? std::get<0>((*variablesValue)[first->str()]) : first->intNum();
    long int secondValue =
        second->type == NodeType::VariableName ? std::get<0>((*variablesValue)[second->str()]) : second->intNum();
    switch (operation) {
    case BinaryOperation::Add:
        return firstValue + secondValue;
        break;
    case BinaryOperation::Sub:
        return firstValue - secondValue;
        break;
    case BinaryOperation::Mult:
        return firstValue * secondValue;
        break;
    case BinaryOperation::Div:
        return firstValue / secondValue;
        break;
    default:
        return 0;
        break;
    }
}

double calcFloatOperation(Node::Ptr &first, Node::Ptr &second, BinaryOperation operation, VariablesValue *variablesValue = nullptr) {
    double firstValue =
        first->type == NodeType::VariableName ? std::get<1>((*variablesValue)[first->str()]) : first->fpNum();
    double secondValue =
        second->type == NodeType::VariableName ? std::get<1>((*variablesValue)[second->str()]) : second->fpNum();
    switch (operation) {
    case BinaryOperation::FAdd:
        return firstValue + secondValue;
        break;
    case BinaryOperation::FSub:
        return firstValue - secondValue;
        break;
    case BinaryOperation::FMult:
        return firstValue * secondValue;
        break;
    case BinaryOperation::FDiv:
        return firstValue / secondValue;
        break;
    default:
        return 0.0;
        break;
    }
}

bool constantPropagation(Node::Ptr &first, Node::Ptr &second, std::list<VariablesTable *> tables,
                         VariablesValue &variablesValue) {
    auto parent = first->parent;
    if (parent->type == NodeType::BinaryOperation && parent->binOp() == BinaryOperation::Assign)
        return false;
    if (first->type == NodeType::VariableName && getVariableAttribute(first, tables) == true)
        return false;
    if (second->type == NodeType::VariableName && getVariableAttribute(second, tables) == true)
        return false;
    if ((first->type == NodeType::IntegerLiteralValue || first->type == NodeType::VariableName) &&
        (second->type == NodeType::IntegerLiteralValue || second->type == NodeType::VariableName)) {
        parent->type = NodeType::IntegerLiteralValue;
        parent->value = calcIntOperation(first, second, parent->binOp(), &variablesValue);
        parent->children.clear();
        return true;
    }
    if ((first->type == NodeType::FloatingPointLiteralValue || first->type == NodeType::VariableName) &&
        (second->type == NodeType::FloatingPointLiteralValue || second->type == NodeType::VariableName)) {
        parent->type = NodeType::FloatingPointLiteralValue;
        parent->value = calcFloatOperation(first, second, parent->binOp(), &variablesValue);;
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
        parent->type = NodeType::IntegerLiteralValue;
        parent->value = calcIntOperation(first, second, parent->binOp());
        parent->children.clear();
        return true;
    }
    if (first->type == NodeType::FloatingPointLiteralValue && second->type == NodeType::FloatingPointLiteralValue) {
        parent->type = NodeType::FloatingPointLiteralValue;
        parent->value = calcFloatOperation(first, second, parent->binOp());
        parent->children.clear();
        return true;
    }
    return false;
}

void pushVariableAttribute(Node::Ptr &node, Node::Ptr &child, VariablesValue &variablesValue) { // TODO upgrade VariablesValue
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
            // TODO need some checks for modified variables
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
