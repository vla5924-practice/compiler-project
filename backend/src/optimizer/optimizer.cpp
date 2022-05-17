#include "optimizer/optimizer.hpp"

#include <variant>

using namespace ast;
using namespace optimizer;

using VariablesValue = std::map<std::string, std::variant<long int, double>>; // TODO std::map -> std::list<std::map>

Variable &getVariable(const Node::Ptr &node,
                      const std::list<VariablesTable *> &tables) { // TODO merge this function with getVariableAttribute
    VariablesTable::iterator tableEntry;
    for (const auto &table : tables) {
        tableEntry = table->find(node->str());
        if (tableEntry != table->cend()) {
            break;
        }
    }
    return tableEntry->second;
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

long int calcIntOperation(Node::Ptr &first, Node::Ptr &second, BinaryOperation operation,
                          VariablesValue *variablesValue = nullptr) {
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
    case BinaryOperation::Equal:
        return firstValue == secondValue;
        break;
    case BinaryOperation::And:
        return firstValue && secondValue;
        break;
    case BinaryOperation::Or:
        return firstValue || secondValue;
        break;
    case BinaryOperation::Greater:
        return firstValue > secondValue;
        break;
    case BinaryOperation::GreaterEqual:
        return firstValue >= secondValue;
        break;
    case BinaryOperation::Less:
        return firstValue < secondValue;
        break;
    case BinaryOperation::LessEqual:
        return firstValue <= secondValue;
        break;
    case BinaryOperation::NotEqual:
        return firstValue != secondValue;
        break;
    default:
        return 0;
        break;
    }
}

double calcFloatOperation(Node::Ptr &first, Node::Ptr &second, BinaryOperation operation,
                          VariablesValue *variablesValue = nullptr) {
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
    case BinaryOperation::FEqual:
        return firstValue == secondValue;
        break;
    case BinaryOperation::FAnd:
        return firstValue && secondValue;
        break;
    case BinaryOperation::FOr:
        return firstValue || secondValue;
        break;
    case BinaryOperation::FGreater:
        return firstValue > secondValue;
        break;
    case BinaryOperation::FGreaterEqual:
        return firstValue >= secondValue;
        break;
    case BinaryOperation::FLess:
        return firstValue < secondValue;
        break;
    case BinaryOperation::FLessEqual:
        return firstValue <= secondValue;
        break;
    case BinaryOperation::FNotEqual:
        return firstValue != secondValue;
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
    if (first->type == NodeType::VariableName && getVariableAttribute(first, tables))
        return false;
    if (second->type == NodeType::VariableName && getVariableAttribute(second, tables))
        return false;
    if ((first->type == NodeType::IntegerLiteralValue ||
         (first->type == NodeType::VariableName && getVariable(first, tables).type == BuiltInTypes::IntType)) &&
        (second->type == NodeType::IntegerLiteralValue ||
         (second->type == NodeType::VariableName && getVariable(second, tables).type == BuiltInTypes::IntType))) {
        parent->type = NodeType::IntegerLiteralValue;
        parent->value = calcIntOperation(first, second, parent->binOp(), &variablesValue);
        parent->children.clear();
        return true;
    }
    if ((first->type == NodeType::FloatingPointLiteralValue ||
         (first->type == NodeType::VariableName && getVariable(first, tables).type == BuiltInTypes::FloatType)) &&
        (second->type == NodeType::FloatingPointLiteralValue ||
         (second->type == NodeType::VariableName && getVariable(second, tables).type == BuiltInTypes::FloatType))) {
        parent->type = NodeType::FloatingPointLiteralValue;
        parent->value = calcFloatOperation(first, second, parent->binOp(), &variablesValue);
        parent->children.clear();
        return true;
    }
    return false;
}

void processTypeConversion(Node::Ptr &node, std::list<VariablesTable *> &table, VariablesValue &variablesValue) {
    Node::Ptr &last = node->lastChild();
    if (last->type == NodeType::IntegerLiteralValue || last->type == NodeType::FloatingPointLiteralValue) {
        if (node->firstChild()->typeId() == BuiltInTypes::FloatType) {
            node->type = NodeType::FloatingPointLiteralValue;
            node->value = static_cast<float>(last->intNum());
        } else {
            node->type = NodeType::IntegerLiteralValue;
            node->value = static_cast<long int>(last->fpNum());
        }
        node->children.clear();
        return;
    }

    if (last->type == NodeType::VariableName &&
        !getVariableAttribute(last, table)) { // procces variable in type conversion
        if (node->firstChild()->typeId() == BuiltInTypes::FloatType) {
            node->type = NodeType::FloatingPointLiteralValue;
            node->value = static_cast<float>(std::get<0>(variablesValue[last->str()]));
        } else {
            node->type = NodeType::IntegerLiteralValue;
            node->value = static_cast<long int>(std::get<1>(variablesValue[last->str()]));
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

void variablePropagation(Node::Ptr &node, std::list<VariablesTable *> &table, VariablesValue &variablesValue) {
    if (variablesValue.find(node->str()) == variablesValue.cend())
        return;
    if (getVariable(node, table).type == BuiltInTypes::FloatType) {
        node->type = NodeType::FloatingPointLiteralValue;
        node->value = std::get<1>(variablesValue[node->str()]);
    } else {
        node->type = NodeType::IntegerLiteralValue;
        node->value = std::get<0>(variablesValue[node->str()]);
    }
}

void pushVariableAttribute(Node::Ptr &node, Node::Ptr &child,
                           VariablesValue &variablesValue) { // TODO upgrade VariablesValue
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

void processBinaryOperation(Node::Ptr &node, std::list<VariablesTable *> &table, VariablesValue &variablesValue,
                            FunctionsTable &functions) {
    auto first = node->firstChild();
    auto second = node->lastChild();
    if (second->type == NodeType::BinaryOperation)
        processBinaryOperation(second, table, variablesValue, functions);
    if (first->type == NodeType::TypeConversion)
        processTypeConversion(first, table, variablesValue);
    if (second->type == NodeType::TypeConversion)
        processTypeConversion(second, table, variablesValue);
    bool isConsExpr = constantFolding(first, second);
    bool isNotModifiedExpr = false;
    if (!isConsExpr)
        isNotModifiedExpr = constantPropagation(first, second, table, variablesValue);
    if (first->type == NodeType::FunctionCall) {
        functions.find(first->firstChild()->str())->second.useCount++;
    }
    if (second->type == NodeType::FunctionCall) {
        functions.find(second->firstChild()->str())->second.useCount++;
    }
}

void processExpression(Node::Ptr &node, std::list<VariablesTable *> &table, VariablesValue &variablesValue,
                       FunctionsTable &functions) {
    for (auto &child : node->children) {
        if (child->type == NodeType::BinaryOperation) {
            auto first = child->firstChild();
            auto second = child->lastChild();
            if (second->type == NodeType::BinaryOperation)
                processBinaryOperation(second, table, variablesValue, functions);
            if (first->type == NodeType::TypeConversion)
                processTypeConversion(first, table, variablesValue);
            if (second->type == NodeType::TypeConversion)
                processTypeConversion(second, table, variablesValue);
            bool isConsExpr = constantFolding(first, second);
            bool isNotModifiedExpr = false;
            if (!isConsExpr)
                isNotModifiedExpr = constantPropagation(first, second, table, variablesValue);
            if (isConsExpr || isNotModifiedExpr) {
                pushVariableAttribute(node, child, variablesValue);
            }
            if (first->type == NodeType::FunctionCall) {
                functions.find(first->firstChild()->str())->second.useCount++;
            }
            if (second->type == NodeType::FunctionCall) {
                functions.find(second->firstChild()->str())->second.useCount++;
            }
            // if ()
            // TODO need some checks for modified variables
            continue;
        }

        if (child->type == NodeType::FunctionCall) {
            auto end_child = child->children.back();
            processExpression(end_child, table, variablesValue, functions);
            functions.find(child->firstChild()->str())->second.useCount++;
            if (end_child->type == NodeType::FunctionName)
                continue;
        }

        if (child->type == NodeType::TypeConversion) {
            processTypeConversion(child, table, variablesValue);
            pushVariableAttribute(node, child, variablesValue);
            continue;
        }

        if (child->type == NodeType::FloatingPointLiteralValue || child->type == NodeType::IntegerLiteralValue) {
            pushVariableAttribute(node, child, variablesValue);
            continue;
        }

        if (child->type == NodeType::VariableName && !getVariableAttribute(child, table)) {
            variablePropagation(child, table, variablesValue);
            continue;
        }

        processExpression(child, table, variablesValue, functions);
    }
}

bool isLiteral(Node::Ptr &node) {
    switch (node->type) {
    case NodeType::IntegerLiteralValue:
    case NodeType::FloatingPointLiteralValue:
        return true;
    }
    return false;
}

void processBranchRoot(Node::Ptr &node, std::list<VariablesTable *> &table, VariablesValue &variablesValue,
                       FunctionsTable &functions) {
    table.push_front(&node->variables());
    for (auto &child : node->children) {
        if (child->type == NodeType::Expression || child->type == NodeType::VariableDeclaration) {
            processExpression(child, table, variablesValue, functions);
        }

        if (child->type == NodeType::IfStatement) {
            processExpression(child->firstChild(), table, variablesValue, functions);
            auto &exprResult = child->firstChild()->firstChild();
            if (isLiteral(exprResult)) {
                child->children.pop_front();
                if (exprResult->type == NodeType::IntegerLiteralValue && exprResult->intNum() == 1 ||
                    exprResult->type == NodeType::FloatingPointLiteralValue && exprResult->fpNum() == 1.0) {
                    child = child->children.front();
                } else {
                    if (child->children.size() > 1u) {
                        if (child->secondChild()->type == NodeType::ElseStatement) {
                            child = child->children.back()->firstChild();
                        } else {
                            // TODO: Elif Statement
                        }

                    } else {
                        child->children.clear();
                        child->type = NodeType::BranchRoot;
                    }
                }
            } // TODO @arteboss удалить ненужную ветку в иф выражении
        }

        if (child->type == NodeType::WhileStatement) {
            processExpression(child->firstChild(), table, variablesValue, functions);
            auto &exprResult = child->firstChild()->firstChild();
            if (isLiteral(exprResult)) {
                if (exprResult->type == NodeType::IntegerLiteralValue && exprResult->intNum() == 0 ||
                    exprResult->type == NodeType::FloatingPointLiteralValue && exprResult->fpNum() == 0.0) {
                    child->children.clear();
                    child->type = NodeType::BranchRoot;
                }
            } // TODO @arteboss удалить ненужную ветку в иф выражении
        }     // сделать аналогично if
    }
}

void Optimizer::process(SyntaxTree &tree) {
    for (auto &node : tree.root->children) {
        if (node->type == NodeType::FunctionDefinition) {
            auto child = node->children.begin();
            std::advance(child, 3);
            std::list<VariablesTable *> variablesTable;
            VariablesValue variablesValue;
            processBranchRoot(*child, variablesTable, variablesValue, tree.functions);
        }
    }
    // for (auto &function : tree.functions) {
    //     if (function.second.useCount == 0) {
    //         tree.functions.erase(function.first);
    //     }
    // }
    tree.root->children.remove_if([&tree](Node::Ptr node) {
        return tree.functions.find(node->firstChild()->str())->second.useCount == 0 &&
               node->firstChild()->str() != "main";
    });
}
