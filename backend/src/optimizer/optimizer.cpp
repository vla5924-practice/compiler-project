#include "optimizer/optimizer.hpp"

#include <variant>

#include "optimizer/optimizer_context.hpp"

using namespace ast;
using namespace optimizer;

long int calcIntOperation(Node::Ptr &first, Node::Ptr &second, BinaryOperation operation, OptimizerContext &ctx) {
    long int firstValue =
        first->type == NodeType::VariableName ? std::get<0>(ctx.values[first->str()]) : first->intNum();
    long int secondValue =
        second->type == NodeType::VariableName ? std::get<0>(ctx.values[second->str()]) : second->intNum();
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

double calcFloatOperation(Node::Ptr &first, Node::Ptr &second, BinaryOperation operation, OptimizerContext &ctx) {
    double firstValue = first->type == NodeType::VariableName ? std::get<1>(ctx.values[first->str()]) : first->fpNum();
    double secondValue =
        second->type == NodeType::VariableName ? std::get<1>(ctx.values[second->str()]) : second->fpNum();
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

bool constantPropagation(Node::Ptr &first, Node::Ptr &second, OptimizerContext &ctx) {
    auto parent = first->parent;
    if (parent->type == NodeType::BinaryOperation && parent->binOp() == BinaryOperation::Assign)
        return false;
    if (first->type == NodeType::VariableName && ctx.findVariable(first).attributes.modified)
        return false;
    if (second->type == NodeType::VariableName && ctx.findVariable(second).attributes.modified)
        return false;
    if ((first->type == NodeType::IntegerLiteralValue ||
         (first->type == NodeType::VariableName && ctx.findVariable(first).type == BuiltInTypes::IntType)) &&
        (second->type == NodeType::IntegerLiteralValue ||
         (second->type == NodeType::VariableName && ctx.findVariable(second).type == BuiltInTypes::IntType))) {
        parent->type = NodeType::IntegerLiteralValue;
        parent->value = calcIntOperation(first, second, parent->binOp(), ctx);
        parent->children.clear();
        return true;
    }
    if ((first->type == NodeType::FloatingPointLiteralValue ||
         (first->type == NodeType::VariableName && ctx.findVariable(first).type == BuiltInTypes::FloatType)) &&
        (second->type == NodeType::FloatingPointLiteralValue ||
         (second->type == NodeType::VariableName && ctx.findVariable(second).type == BuiltInTypes::FloatType))) {
        parent->type = NodeType::FloatingPointLiteralValue;
        parent->value = calcFloatOperation(first, second, parent->binOp(), ctx);
        parent->children.clear();
        return true;
    }
    return false;
}

void processTypeConversion(Node::Ptr &node, OptimizerContext &ctx) {
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
        !ctx.findVariable(last).attributes.modified) { // procces variable in type conversion
        if (node->firstChild()->typeId() == BuiltInTypes::FloatType) {
            node->type = NodeType::FloatingPointLiteralValue;
            node->value = static_cast<float>(std::get<0>(ctx.values[last->str()]));
        } else {
            node->type = NodeType::IntegerLiteralValue;
            node->value = static_cast<long int>(std::get<1>(ctx.values[last->str()]));
        }
        node->children.clear();
    }
}

bool constantFolding(Node::Ptr &first, Node::Ptr &second, OptimizerContext &ctx) {
    auto parent = first->parent;
    if (first->type == NodeType::IntegerLiteralValue && second->type == NodeType::IntegerLiteralValue) {
        parent->type = NodeType::IntegerLiteralValue;
        parent->value = calcIntOperation(first, second, parent->binOp(), ctx);
        parent->children.clear();
        return true;
    }
    if (first->type == NodeType::FloatingPointLiteralValue && second->type == NodeType::FloatingPointLiteralValue) {
        parent->type = NodeType::FloatingPointLiteralValue;
        parent->value = calcFloatOperation(first, second, parent->binOp(), ctx);
        parent->children.clear();
        return true;
    }
    return false;
}

void variablePropagation(Node::Ptr &node, OptimizerContext &ctx) {
    if (ctx.values.find(node->str()) == ctx.values.cend())
        return;
    if (ctx.findVariable(node).type == BuiltInTypes::FloatType) {
        node->type = NodeType::FloatingPointLiteralValue;
        node->value = std::get<1>(ctx.values[node->str()]);
    } else {
        node->type = NodeType::IntegerLiteralValue;
        node->value = std::get<0>(ctx.values[node->str()]);
    }
}

void pushVariableAttribute(Node::Ptr &node, Node::Ptr &child, OptimizerContext &ctx) {
    TypeId type;
    auto parent = node->parent;
    for (auto &iter : parent->children) {
        if (iter->type == NodeType::TypeName) {
            type = iter->typeId();
        }
        if (iter->type == NodeType::VariableName) {
            if (type == BuiltInTypes::IntType)
                ctx.values.emplace(iter->str(), child->intNum());
            if (type == BuiltInTypes::FloatType)
                ctx.values.emplace(iter->str(), child->fpNum());
        }
    }
}

bool processBinaryOperation(Node::Ptr &node, OptimizerContext &ctx) {
    auto first = node->firstChild();
    auto second = node->lastChild();
    bool haveFunctionCall = false;
    if (second->type == NodeType::BinaryOperation)
        haveFunctionCall = processBinaryOperation(second, ctx);
    if (first->type == NodeType::TypeConversion)
        processTypeConversion(first, ctx);
    if (second->type == NodeType::TypeConversion)
        processTypeConversion(second, ctx);
    bool isConsExpr = constantFolding(first, second, ctx);
    bool isNotModifiedExpr = false;
    if (!isConsExpr)
        isNotModifiedExpr = constantPropagation(first, second, ctx);
    if (first->type == NodeType::FunctionCall) {
        ctx.functions.find(first->firstChild()->str())->second.useCount++;
        haveFunctionCall = true;
    }
    if (second->type == NodeType::FunctionCall) {
        ctx.functions.find(second->firstChild()->str())->second.useCount++;
        haveFunctionCall = true;
    }
    return haveFunctionCall;
}

void processExpression(Node::Ptr &node, OptimizerContext &ctx) {
    for (auto &child : node->children) {
        if (child->type == NodeType::BinaryOperation) {
            auto first = child->firstChild();
            auto second = child->lastChild();
            bool haveFunctionCall = false;
            if (second->type == NodeType::BinaryOperation)
                haveFunctionCall = processBinaryOperation(second, ctx);
            if (first->type == NodeType::TypeConversion)
                processTypeConversion(first, ctx);
            if (second->type == NodeType::TypeConversion)
                processTypeConversion(second, ctx);
            bool isConsExpr = constantFolding(first, second, ctx);
            bool isNotModifiedExpr = false;
            if (!isConsExpr)
                isNotModifiedExpr = constantPropagation(first, second, ctx);
            if (isConsExpr || isNotModifiedExpr) {
                pushVariableAttribute(node, child, ctx);
            }
            if (first->type == NodeType::FunctionCall) {
                ctx.functions.find(first->firstChild()->str())->second.useCount++;
            }
            if (second->type == NodeType::FunctionCall) {
                ctx.functions.find(second->firstChild()->str())->second.useCount++;
            }
            if (child->type == NodeType::BinaryOperation && child->binOp() == BinaryOperation::Assign &&
                haveFunctionCall) {
                ctx.findVariable(child->firstChild()).attributes.modified = true;
            }
            // if ()
            // TODO need some checks for modified variables
            continue;
        }

        if (child->type == NodeType::FunctionCall) {
            auto end_child = child->children.back();
            processExpression(end_child, ctx);
            ctx.functions.find(child->firstChild()->str())->second.useCount++;
            if (end_child->type == NodeType::FunctionName)
                continue;
        }

        if (child->type == NodeType::TypeConversion) {
            processTypeConversion(child, ctx);
            pushVariableAttribute(node, child, ctx);
            continue;
        }

        if (child->type == NodeType::FloatingPointLiteralValue || child->type == NodeType::IntegerLiteralValue) {
            pushVariableAttribute(node, child, ctx);
            continue;
        }

        if (child->type == NodeType::VariableName && !ctx.findVariable(child).attributes.modified) {
            variablePropagation(child, ctx);
            continue;
        }

        processExpression(child, ctx);
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

void changeVariablesAttributes(Node::Ptr &node, OptimizerContext &ctx) {
    for (auto &child : node->children) {
        if (child->type == NodeType::Expression) {
            auto &exprNode = child->firstChild();
            if (exprNode->type == NodeType::BinaryOperation && exprNode->binOp() == BinaryOperation::Assign) {
                ctx.findVariable(exprNode->firstChild()).attributes.modified = true;
            }
        }
    }
}

void processBranchRoot(Node::Ptr &node, OptimizerContext &ctx) {
    ctx.variables.push_front(&node->variables());
    for (auto &child : node->children) {
        if (child->type == NodeType::Expression || child->type == NodeType::VariableDeclaration) {
            processExpression(child, ctx);
        }

        if (child->type == NodeType::IfStatement) {
            processExpression(child->firstChild(), ctx);
            auto &exprResult = child->firstChild()->firstChild();
            if (isLiteral(exprResult)) {
                child->children.pop_front();
                if (exprResult->type == NodeType::IntegerLiteralValue && exprResult->intNum() == 1 ||
                    exprResult->type == NodeType::FloatingPointLiteralValue && exprResult->fpNum() == 1.0) {
                    child = child->children.front();
                    processBranchRoot(child, ctx);
                } else {
                    child->children.pop_front();
                    for (auto &ifChild : child->children) {
                        if (ifChild->type == NodeType::ElifStatement) {
                            processExpression(ifChild->firstChild(), ctx);
                        } else {
                            child = ifChild->firstChild();
                            processBranchRoot(child, ctx);
                            break;
                        }
                        auto &ifExprResult = ifChild->firstChild()->firstChild();
                        if (isLiteral(ifExprResult)) {
                            ifChild->children.pop_front();
                            if (ifExprResult->type == NodeType::IntegerLiteralValue && ifExprResult->intNum() == 1 ||
                                ifExprResult->type == NodeType::FloatingPointLiteralValue &&
                                    ifExprResult->fpNum() == 1.0) {
                                child = ifChild->children.front();
                                processBranchRoot(child, ctx);
                                break;
                            } else {
                                ifChild->children.clear();
                            }
                        }
                    }

                    child->children.remove_if(
                        [](Node::Ptr node) { return node->type == NodeType::ElifStatement && node->children.empty(); });

                    if (child->children.empty()) {
                        child->type = NodeType::BranchRoot;
                    }
                }
            } else {
                processBranchRoot(child->secondChild(), ctx);
            }
        }

        if (child->type == NodeType::WhileStatement) {
            processExpression(child->firstChild(), ctx);
            auto &exprResult = child->firstChild()->firstChild();
            if (isLiteral(exprResult)) {
                if (exprResult->type == NodeType::IntegerLiteralValue && exprResult->intNum() == 0 ||
                    exprResult->type == NodeType::FloatingPointLiteralValue && exprResult->fpNum() == 0.0) {
                    child->children.clear();
                    child->type = NodeType::BranchRoot;
                }
            } else {
                changeVariablesAttributes(child->secondChild(), ctx);
                processBranchRoot(child->secondChild(), ctx);
            }
        }
    }
}

void removeEmptyBranchRoots(Node::Ptr node) {
    for (auto &child : node->children) {
        if (!child->children.empty())
            removeEmptyBranchRoots(child);
        if (child->children.size() == 1u && child->firstChild()->type == NodeType::BranchRoot)
            child = child->firstChild();
        child->children.remove_if([](Node::Ptr node) {
            return node->type == NodeType::BranchRoot && node->children.empty() ||
                   node->type == NodeType::WhileStatement && node->children.size() == 1u ||
                   node->type == NodeType::IfStatement && node->children.size() == 1u;
        });
    }
}

void Optimizer::process(SyntaxTree &tree) {
    for (auto &node : tree.root->children) {
        if (node->type == NodeType::FunctionDefinition) {
            auto child = node->children.begin();
            std::advance(child, 3);
            OptimizerContext ctx(tree.functions);
            processBranchRoot(*child, ctx);
        }
    }
    tree.root->children.remove_if([&tree](Node::Ptr node) {
        return tree.functions.find(node->firstChild()->str())->second.useCount == 0 &&
               node->firstChild()->str() != "main";
    });
    removeEmptyBranchRoots(tree.root);
}
