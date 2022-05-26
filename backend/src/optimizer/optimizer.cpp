#include "optimizer/optimizer.hpp"

#include <algorithm>
#include <variant>

#include "optimizer/optimizer_context.hpp"

using namespace ast;
using namespace optimizer;

namespace {

bool isLiteral(const Node::Ptr &node) {
    switch (node->type) {
    case NodeType::IntegerLiteralValue:
    case NodeType::FloatingPointLiteralValue:
    case NodeType::StringLiteralValue:
        return true;
    }
    return false;
}

bool isAssignment(const Node::Ptr &node) {
    return node->type == NodeType::BinaryOperation && node->binOp() == BinaryOperation::Assign;
}

bool isZeroIntLiteral(const Node::Ptr &node) {
    return node->type == NodeType::IntegerLiteralValue && node->intNum() == 0;
}

bool isNonZeroIntLiteral(const Node::Ptr &node) {
    return node->type == NodeType::IntegerLiteralValue && node->intNum() != 0;
}

bool isZeroFloatLiteral(const Node::Ptr &node) {
    return node->type == NodeType::FloatingPointLiteralValue && node->fpNum() == 0.0;
}

bool isNonZeroFloatLiteral(const Node::Ptr &node) {
    return node->type == NodeType::FloatingPointLiteralValue && node->fpNum() != 0.0;
}

bool isTruthyLiteral(const Node::Ptr &node) {
    return isNonZeroIntLiteral(node) || isNonZeroFloatLiteral(node);
}

bool isFalsyLiteral(const Node::Ptr &node) {
    return isZeroIntLiteral(node) || isZeroFloatLiteral(node);
}

bool isNumericLiteral(const Node::Ptr &node) {
    return node->type == NodeType::IntegerLiteralValue || node->type == NodeType::FloatingPointLiteralValue;
}

bool isModifiedVariable(const Node::Ptr &node, OptimizerContext &ctx) {
    return node->type == NodeType::VariableName && ctx.findVariable(node).attributes.modified;
}

bool isNonModifiedVariable(const Node::Ptr &node, OptimizerContext &ctx) {
    return node->type == NodeType::VariableName && !ctx.findVariable(node).attributes.modified &&
           node->parent->type != NodeType::VariableDeclaration;
}

bool isVariableWithType(const Node::Ptr &node, TypeId typeId, OptimizerContext &ctx) {
    return node->type == NodeType::VariableName && ctx.findVariable(node).type == typeId;
}

bool canBeConstantInt(const Node::Ptr &node, OptimizerContext &ctx) {
    return node->type == NodeType::IntegerLiteralValue || isVariableWithType(node, BuiltInTypes::IntType, ctx);
}

bool canBeConstantFloat(const Node::Ptr &node, OptimizerContext &ctx) {
    return node->type == NodeType::FloatingPointLiteralValue || isVariableWithType(node, BuiltInTypes::FloatType, ctx);
}

} // namespace

long int calculateIntOperation(Node::Ptr &first, Node::Ptr &second, BinaryOperation operation, OptimizerContext &ctx) {
    long int lhs =
        first->type == NodeType::VariableName ? std::get<0>(ctx.findVariableValue(first->str())) : first->intNum();
    long int rhs =
        second->type == NodeType::VariableName ? std::get<0>(ctx.findVariableValue(second->str())) : second->intNum();

    switch (operation) {
    case BinaryOperation::Add:
        return lhs + rhs;
    case BinaryOperation::Sub:
        return lhs - rhs;
    case BinaryOperation::Mult:
        return lhs * rhs;
    case BinaryOperation::Div:
        return lhs / rhs;
    case BinaryOperation::Equal:
        return lhs == rhs;
    case BinaryOperation::And:
        return lhs && rhs;
    case BinaryOperation::Or:
        return lhs || rhs;
    case BinaryOperation::Greater:
        return lhs > rhs;
    case BinaryOperation::GreaterEqual:
        return lhs >= rhs;
    case BinaryOperation::Less:
        return lhs < rhs;
    case BinaryOperation::LessEqual:
        return lhs <= rhs;
    case BinaryOperation::NotEqual:
        return lhs != rhs;
    }
    return 0;
}

double calculateFloatOperation(Node::Ptr &first, Node::Ptr &second, BinaryOperation operation, OptimizerContext &ctx) {
    double lhs = first->type == NodeType::VariableName ? std::get<1>(ctx.values.front()[first->str()]) : first->fpNum();
    double rhs =
        second->type == NodeType::VariableName ? std::get<1>(ctx.values.front()[second->str()]) : second->fpNum();

    switch (operation) {
    case BinaryOperation::FAdd:
        return lhs + rhs;
    case BinaryOperation::FSub:
        return lhs - rhs;
    case BinaryOperation::FMult:
        return lhs * rhs;
    case BinaryOperation::FDiv:
        return lhs / rhs;
    case BinaryOperation::FEqual:
        return lhs == rhs;
    case BinaryOperation::FAnd:
        return lhs && rhs;
    case BinaryOperation::FOr:
        return lhs || rhs;
    case BinaryOperation::FGreater:
        return lhs > rhs;
    case BinaryOperation::FGreaterEqual:
        return lhs >= rhs;
    case BinaryOperation::FLess:
        return lhs < rhs;
    case BinaryOperation::FLessEqual:
        return lhs <= rhs;
    case BinaryOperation::FNotEqual:
        return lhs != rhs;
    }
    return 0.0;
}

bool constantPropagation(Node::Ptr &first, Node::Ptr &second, OptimizerContext &ctx) {
    auto parent = first->parent;
    if (isAssignment(parent) || isModifiedVariable(first, ctx) || isModifiedVariable(second, ctx))
        return false;
    if (canBeConstantInt(first, ctx) && canBeConstantInt(second, ctx)) {
        parent->type = NodeType::IntegerLiteralValue;
        parent->value = calculateIntOperation(first, second, parent->binOp(), ctx);
        parent->children.clear();
        return true;
    }
    if (canBeConstantFloat(first, ctx) && canBeConstantFloat(second, ctx)) {
        parent->type = NodeType::FloatingPointLiteralValue;
        parent->value = calculateFloatOperation(first, second, parent->binOp(), ctx);
        parent->children.clear();
        return true;
    }
    return false;
}

void processTypeConversion(Node::Ptr &node, OptimizerContext &ctx) {
    Node::Ptr &operand = node->lastChild();
    if (isNumericLiteral(operand)) {
        if (node->firstChild()->typeId() == BuiltInTypes::FloatType) {
            node->type = NodeType::FloatingPointLiteralValue;
            node->value = static_cast<double>(operand->intNum());
        } else {
            node->type = NodeType::IntegerLiteralValue;
            node->value = static_cast<long int>(operand->fpNum());
        }
        node->children.clear();
        return;
    }

    if (isNonModifiedVariable(operand, ctx)) {
        const std::string &varName = operand->str();
        if (node->firstChild()->typeId() == BuiltInTypes::FloatType) {
            node->type = NodeType::FloatingPointLiteralValue;
            node->value = static_cast<double>(std::get<long int>(ctx.findVariableValue(varName)));
        } else {
            node->type = NodeType::IntegerLiteralValue;
            node->value = static_cast<long int>(std::get<double>(ctx.findVariableValue(varName)));
        }
        node->children.clear();
    }
}

bool constantFolding(Node::Ptr &first, Node::Ptr &second, OptimizerContext &ctx) {
    auto parent = first->parent;
    if (first->type == NodeType::IntegerLiteralValue && second->type == NodeType::IntegerLiteralValue) {
        parent->type = NodeType::IntegerLiteralValue;
        parent->value = calculateIntOperation(first, second, parent->binOp(), ctx);
        parent->children.clear();
        return true;
    }
    if (first->type == NodeType::FloatingPointLiteralValue && second->type == NodeType::FloatingPointLiteralValue) {
        parent->type = NodeType::FloatingPointLiteralValue;
        parent->value = calculateFloatOperation(first, second, parent->binOp(), ctx);
        parent->children.clear();
        return true;
    }
    return false;
}

void variablePropagation(Node::Ptr &node, OptimizerContext &ctx) {
    const std::string &varName = node->str();
    if (!ctx.hasVariable(varName)) // fix me
        return;
    auto variableIter = ctx.findVariableValue(varName);
    if (ctx.findVariable(node).type == BuiltInTypes::FloatType) {
        node->type = NodeType::FloatingPointLiteralValue;
        node->value = std::get<double>(variableIter);
    } else {
        node->type = NodeType::IntegerLiteralValue;
        node->value = std::get<long int>(variableIter);
    }
}

void pushVariableAttribute(Node::Ptr &node, Node::Ptr &child, OptimizerContext &ctx) {
    TypeId type;
    auto parent = node->parent;
    for (auto &iter : parent->children) {
        if (iter->type == NodeType::TypeName) {
            type = iter->typeId();
            continue;
        }
        if (iter->type == NodeType::VariableName) {
            const std::string &varName = iter->str();
            if (type == BuiltInTypes::IntType)
                ctx.values.front().emplace(varName, child->intNum());
            else if (type == BuiltInTypes::FloatType)
                ctx.values.front().emplace(varName, child->fpNum());
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
            if (child->binOp() == BinaryOperation::Assign) {
                if (isNonModifiedVariable(second, ctx)) {
                    variablePropagation(second, ctx);
                }
            }
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
                haveFunctionCall = true;
            }
            if (second->type == NodeType::FunctionCall) {
                ctx.functions.find(second->firstChild()->str())->second.useCount++;
                haveFunctionCall = true;
            }
            if (isAssignment(child) && haveFunctionCall) {
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

        if (isNumericLiteral(child)) {
            pushVariableAttribute(node, child, ctx);
            continue;
        }

        if (isNonModifiedVariable(child, ctx)) {
            variablePropagation(child, ctx);
            continue;
        }

        processExpression(child, ctx);
    }
}

void changeVariablesAttributes(Node::Ptr &node, OptimizerContext &ctx) {
    for (auto &child : node->children) {
        if (child->type == NodeType::Expression) {
            auto &exprNode = child->firstChild();
            if (isAssignment(exprNode)) {
                ctx.findVariable(exprNode->firstChild()).attributes.modified = true;
            }
        }
    }
}

void processBranchRoot(Node::Ptr &node, OptimizerContext &ctx) {
    ctx.variables.push_front(&node->variables());
    ctx.values.emplace_front();
    for (auto childIter = node->children.begin(); childIter != node->children.end(); childIter++) {
        Node::Ptr &child = *childIter;
        if (child->type == NodeType::Expression || child->type == NodeType::VariableDeclaration) {
            processExpression(child, ctx);
        }

        if (child->type == NodeType::IfStatement) {
            processExpression(child->firstChild(), ctx);
            auto &exprResult = child->firstChild()->firstChild();
            if (isNumericLiteral(exprResult)) {
                child->children.pop_front();
                if (isTruthyLiteral(exprResult)) {
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
                        if (isNumericLiteral(ifExprResult)) {
                            ifChild->children.pop_front();
                            if (isTruthyLiteral(ifExprResult)) {
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
            if (isNumericLiteral(exprResult)) {
                if (isFalsyLiteral(exprResult)) {
                    child->children.clear();
                    child->type = NodeType::BranchRoot;
                } else if (isTruthyLiteral(exprResult)) {
                    auto currNode = child->parent;
                    auto prevNode = child;
                    while (currNode->parent->type != NodeType::FunctionDefinition) {
                        if ((currNode->type == NodeType::IfStatement || currNode->type == NodeType::ElifStatement ||
                             currNode->type == NodeType::WhileStatement) &&
                            isNumericLiteral(currNode->firstChild()->firstChild()) &&
                            !isTruthyLiteral(currNode->firstChild()->firstChild()))
                            break;
                        prevNode = currNode;
                        currNode = currNode->parent;
                    }
                    if (currNode->parent->type == NodeType::FunctionDefinition) {
                        auto iter =
                            std::find_if(currNode->children.begin(), currNode->children.end(),
                                         [&prevNode](const Node::Ptr &node) { return node.get() == prevNode.get(); });
                        currNode->parent->children.erase(std::next(iter), currNode->parent->children.end());
                        break;
                    }
                }
            } else {
                changeVariablesAttributes(child->secondChild(), ctx);
                processBranchRoot(child->secondChild(), ctx);
            }
        }

        if (child->type == NodeType::ReturnStatement) {
            processExpression(child->firstChild(), ctx);
            if (node->parent->type == NodeType::FunctionDefinition) {
                node->children.erase(std::next(childIter), node->children.end());
                break;
            }
        }
    }
    ctx.values.pop_front();
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

void removeUnusedFunctions(SyntaxTree &tree) {
    tree.root->children.remove_if([&functions = tree.functions](Node::Ptr node) {
        const std::string &funcName = node->firstChild()->str();
        return functions[funcName].useCount == 0 && funcName != "main";
    });
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
    removeUnusedFunctions(tree);
    removeEmptyBranchRoots(tree.root);
}
