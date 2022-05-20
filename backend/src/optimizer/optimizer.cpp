#include "optimizer/optimizer.hpp"

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

bool isModifiedVariable(const ast::Node::Ptr &node, OptimizerContext &ctx) {
    return node->type == NodeType::VariableName && ctx.findVariable(node).attributes.modified;
}

bool isNonModifiedVariable(const ast::Node::Ptr &node, OptimizerContext &ctx) {
    return node->type == NodeType::VariableName && !ctx.findVariable(node).attributes.modified;
}

bool isVariableWithType(const ast::Node::Ptr &node, ast::TypeId typeId, OptimizerContext &ctx) {
    return node->type == NodeType::VariableName && ctx.findVariable(node).type == typeId;
}

bool canBeConstantInt(const ast::Node::Ptr &node, OptimizerContext &ctx) {
    return node->type == NodeType::IntegerLiteralValue || isVariableWithType(node, BuiltInTypes::IntType, ctx);
}

bool canBeConstantFloat(const ast::Node::Ptr &node, OptimizerContext &ctx) {
    return node->type == NodeType::FloatingPointLiteralValue || isVariableWithType(node, BuiltInTypes::FloatType, ctx);
}

} // namespace

long int calculateIntOperation(Node::Ptr &first, Node::Ptr &second, BinaryOperation operation, OptimizerContext &ctx) {
    long int lhs = first->type == NodeType::VariableName ? std::get<0>(ctx.values[first->str()]) : first->intNum();
    long int rhs = second->type == NodeType::VariableName ? std::get<0>(ctx.values[second->str()]) : second->intNum();
    switch (operation) {
    case BinaryOperation::Add:
        return lhs + rhs;
        break;
    case BinaryOperation::Sub:
        return lhs - rhs;
        break;
    case BinaryOperation::Mult:
        return lhs * rhs;
        break;
    case BinaryOperation::Div:
        return lhs / rhs;
        break;
    case BinaryOperation::Equal:
        return lhs == rhs;
        break;
    case BinaryOperation::And:
        return lhs && rhs;
        break;
    case BinaryOperation::Or:
        return lhs || rhs;
        break;
    case BinaryOperation::Greater:
        return lhs > rhs;
        break;
    case BinaryOperation::GreaterEqual:
        return lhs >= rhs;
        break;
    case BinaryOperation::Less:
        return lhs < rhs;
        break;
    case BinaryOperation::LessEqual:
        return lhs <= rhs;
        break;
    case BinaryOperation::NotEqual:
        return lhs != rhs;
        break;
    }
    return 0;
}

double calculateFloatOperation(Node::Ptr &first, Node::Ptr &second, BinaryOperation operation, OptimizerContext &ctx) {
    double lhs = first->type == NodeType::VariableName ? std::get<1>(ctx.values[first->str()]) : first->fpNum();
    double rhs = second->type == NodeType::VariableName ? std::get<1>(ctx.values[second->str()]) : second->fpNum();
    switch (operation) {
    case BinaryOperation::FAdd:
        return lhs + rhs;
        break;
    case BinaryOperation::FSub:
        return lhs - rhs;
        break;
    case BinaryOperation::FMult:
        return lhs * rhs;
        break;
    case BinaryOperation::FDiv:
        return lhs / rhs;
        break;
    case BinaryOperation::FEqual:
        return lhs == rhs;
        break;
    case BinaryOperation::FAnd:
        return lhs && rhs;
        break;
    case BinaryOperation::FOr:
        return lhs || rhs;
        break;
    case BinaryOperation::FGreater:
        return lhs > rhs;
        break;
    case BinaryOperation::FGreaterEqual:
        return lhs >= rhs;
        break;
    case BinaryOperation::FLess:
        return lhs < rhs;
        break;
    case BinaryOperation::FLessEqual:
        return lhs <= rhs;
        break;
    case BinaryOperation::FNotEqual:
        return lhs != rhs;
        break;
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

    if (isNonModifiedVariable(last, ctx)) { // procces variable in type conversion
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
    for (auto &child : node->children) {
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
