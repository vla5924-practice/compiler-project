#include "semantizer/semantizer.hpp"

using namespace semantizer;
using namespace ast;

static std::vector<TypeId> getFunctionArguments(const std::list<Node::Ptr> &functionArguments, VariablesTable &table) {
    std::vector<TypeId> result;

    for (const auto &node : functionArguments) {
        auto children = node->children.begin();
        auto type = (*children)->typeId();
        children++;
        auto name = (*children)->str();
        result.push_back(type);
        table[name] = {type, true};
    }

    return result;
}

static void pushTypeConversion(Node::Ptr &node, TypeId type) {
    Node::Ptr conv_node = std::make_shared<Node>(NodeType::TypeConversion, node->parent);
    conv_node->children.push_front(node);
    Node::Ptr type_node = std::make_shared<Node>(type, conv_node);
    conv_node->children.push_front(type_node);
    node = conv_node;
}

static void pushTypeConversion(Node::Ptr &node, NodeType type) {
    Node::Ptr conv_node = std::make_shared<Node>(NodeType::TypeConversion, node->parent);
    conv_node->children.push_front(node);
    Node::Ptr type_node = std::make_shared<Node>(type, conv_node);
    conv_node->children.push_front(type_node);
    node = conv_node;
}

static BinaryOperation convertToFloatOperation(BinaryOperation operation) {
    switch (operation) {
    case BinaryOperation::Add:
    case BinaryOperation::FAdd:
        return BinaryOperation::FAdd;
        break;
    case BinaryOperation::Sub:
    case BinaryOperation::FSub:
        return BinaryOperation::FSub;
        break;
    case BinaryOperation::Mult:
    case BinaryOperation::FMult:
        return BinaryOperation::FMult;
        break;
    case BinaryOperation::Div:
    case BinaryOperation::FDiv:
        return BinaryOperation::FDiv;
        break;
    case BinaryOperation::And:
    case BinaryOperation::FAnd:
        return BinaryOperation::And;
        break;
    case BinaryOperation::Or:
    case BinaryOperation::FOr:
        return BinaryOperation::Or;
        break;
    case BinaryOperation::Equal:
    case BinaryOperation::FEqual:
        return BinaryOperation::FEqual;
        break;
    case BinaryOperation::NotEqual:
    case BinaryOperation::FNotEqual:
        return BinaryOperation::FNotEqual;
        break;
    case BinaryOperation::Less:
    case BinaryOperation::FLess:
        return BinaryOperation::FLess;
        break;
    case BinaryOperation::Greater:
    case BinaryOperation::FGreater:
        return BinaryOperation::FGreater;
        break;
    case BinaryOperation::LessEqual:
    case BinaryOperation::FLessEqual:
        return BinaryOperation::FLessEqual;
        break;
    case BinaryOperation::GreaterEqual:
    case BinaryOperation::FGreaterEqual:
        return BinaryOperation::FGreaterEqual;
        break;
    case BinaryOperation::Assign:
    case BinaryOperation::FAssign:
        return BinaryOperation::Assign;
        break;
    default:
        return operation;
        break;
    }
}

static void processExpression(Node::Ptr &node, TypeId var_type, SemantizerContext &ctx);

static TypeId processFunctionCall(Node::Ptr &node, SemantizerContext &ctx);

static TypeId processUnknownTypeExpression(Node::Ptr &node, NodeType type, SemantizerContext &ctx) {
    if (type == NodeType::BinaryOperation) {
        auto &first = node->firstChild();
        auto &second = node->secondChild();
        auto firstType = processUnknownTypeExpression(first, first->type, ctx);
        auto secondType = processUnknownTypeExpression(second, second->type, ctx);
        if (firstType == BuiltInTypes::FloatType || secondType == BuiltInTypes::FloatType) {
            if (firstType != BuiltInTypes::FloatType)
                pushTypeConversion(first, BuiltInTypes::FloatType);
            if (secondType != BuiltInTypes::FloatType)
                pushTypeConversion(second, BuiltInTypes::FloatType);
            node->value = convertToFloatOperation(node->binOp());
            return BuiltInTypes::FloatType;
        }
        return BuiltInTypes::IntType;
    }
    if (type == NodeType::VariableName)
        return ctx.findVariable(node);
    if (type == NodeType::FunctionCall)
        return processFunctionCall(node->firstChild(), ctx);
    if (type == NodeType::FloatingPointLiteralValue)
        return BuiltInTypes::FloatType;
    if (type == NodeType::IntegerLiteralValue)
        return BuiltInTypes::IntType;
    if (type == NodeType::StringLiteralValue)
        return BuiltInTypes::StrType;
    return BuiltInTypes::NoneType;
}

static TypeId processFunctionCall(Node::Ptr &node, SemantizerContext &ctx) {
    const std::string &funcName = node->firstChild()->str();
    bool isPrintFunction = (funcName == "print");
    if (isPrintFunction) {
        auto exprNode = node->lastChild()->lastChild();
        auto child = exprNode->firstChild();
        auto type = child->type;
        exprNode->value = processUnknownTypeExpression(child, type, ctx);
        return BuiltInTypes::NoneType;
    }
    auto funcIter = ctx.functions.find(funcName);
    if (funcIter == ctx.functions.cend()) {
        if (funcName == "input") {
            funcIter = ctx.functions.emplace(funcName, Function(BuiltInTypes::NoneType)).first;
            TypeId type = ctx.findVariable(node->parent->firstChild());
            Node::Ptr returnTypeNode = std::make_shared<Node>(NodeType::FunctionReturnType, node);
            returnTypeNode->value = type;
            node->children.push_back(returnTypeNode);
            return funcIter->second.returnType;
        } else {
            ctx.errors.push<SemantizerError>(*node, funcName + " was not declared in this scope");
        }
    }

    if (node->children.size() >= 2u) {
        const std::vector<TypeId> &args = funcIter->second.argumentsTypes;
        size_t index = 0;
        for (auto &child : node->secondChild()->children) {
            TypeId type = isPrintFunction ? BuiltInTypes::IntType : args[index++]; // FIXME: workaround for int only
            processExpression(child, type, ctx);
        }
    }

    return funcIter->second.returnType;
}

TypeId literalNodeTypeToTypeId(NodeType type) {
    switch (type) {
    case NodeType::IntegerLiteralValue:
        return BuiltInTypes::IntType;
    case NodeType::FloatingPointLiteralValue:
        return BuiltInTypes::FloatType;
    case NodeType::StringLiteralValue:
        return BuiltInTypes::StrType;
    }
    return BuiltInTypes::UnknownType;
}

static void processExpression(Node::Ptr &node, TypeId var_type, SemantizerContext &ctx) {
    TypeId type;

    switch (var_type) {
    case BuiltInTypes::IntType:
    case BuiltInTypes::FloatType:
    case BuiltInTypes::StrType:
        type = var_type;
        break;

    default:
        ctx.errors.push<SemantizerError>(*node, "Invalid conversion from " + std::to_string(var_type));
        break;
    }

    if (node->type == NodeType::Expression && !std::holds_alternative<TypeId>(node->value))
        node->value.emplace<TypeId>(var_type);

    for (auto &child : node->children) {
        if (child->type == NodeType::UnaryOperation || child->type == NodeType::BinaryOperation) {
            processExpression(child, var_type, ctx);
            continue;
        }

        if (child->type == NodeType::FunctionCall) {
            const std::string &funcName = child->firstChild()->str();
            TypeId retType = processFunctionCall(child, ctx);
            if (retType != var_type && funcName != "input") {
                pushTypeConversion(node, var_type);
            }
            continue;
        }

        if (child->type == NodeType::VariableName) {
            auto find_type = ctx.findVariable(child);
            if (find_type != var_type) {
                if (find_type == BuiltInTypes::StrType && child->str().size() != 1u) {
                    ctx.errors.push<SemantizerError>(*node,
                                                     "Invalid conversion from string to " + std::to_string(var_type));
                }
                pushTypeConversion(child, type);
            }
            if (find_type == BuiltInTypes::FloatType && node->type != NodeType::Expression) {
                node->value = convertToFloatOperation(node->binOp());
            }
            continue;
        }

        if (literalNodeTypeToTypeId(child->type) != type) {
            pushTypeConversion(child, type);
        }

        if ((child->type == NodeType::FloatingPointLiteralValue ||
             child->type == NodeType::TypeConversion && child->firstChild()->typeId() == BuiltInTypes::FloatType) &&
            node->type == NodeType::BinaryOperation) {
            node->value = convertToFloatOperation(node->binOp());
        }
    }

    if (node->type == NodeType::VariableName) {
        auto find_type = ctx.findVariable(node);
        if (find_type != var_type) {
            pushTypeConversion(node, type);
        }
    }
}

static void processBranchRoot(Node::Ptr &node, SemantizerContext &ctx) {
    if (!std::holds_alternative<VariablesTable>(node->value))
        node->value.emplace<VariablesTable>();
    ctx.variables.push_front(&node->variables());
    for (auto &child : node->children) {
        if (child->type == NodeType::VariableDeclaration) {
            auto list_child = child->children.begin();
            auto type = (*list_child)->typeId();
            list_child++;
            auto name = (*list_child)->str();

            auto table_name = ctx.variables.front()->find(name);
            if (table_name != ctx.variables.front()->cend()) {
                ctx.errors.push<SemantizerError>(*child, "Redeclaration of " + name);
            }

            node->variables()[name] = {type};
            list_child++;

            if (list_child != child->children.end() && (*list_child)->type == NodeType::Expression) {
                processExpression(*list_child, type, ctx);
            }

            continue;
        }

        if (child->type == NodeType::Expression) {
            Node::Ptr &expr_root = child->children.front();
            if (expr_root->type == NodeType::BinaryOperation && expr_root->binOp() == BinaryOperation::Assign) {
                auto name = expr_root->children.front()->str();
                auto type = ctx.findVariable(expr_root->children.front());
                processExpression(expr_root, type, ctx);
                if (!std::holds_alternative<TypeId>(node->value))
                    child->value.emplace<TypeId>(type);
            }
            if (expr_root->type == NodeType::FunctionCall) {
                auto type = processFunctionCall(expr_root, ctx);
                if (!std::holds_alternative<TypeId>(node->value))
                    child->value.emplace<TypeId>(type);
            }
            continue;
        }

        if (child->type == NodeType::ReturnStatement) {
            if (ctx.currentFunctionType == BuiltInTypes::NoneType && !child->children.empty()) {
                ctx.errors.push<SemantizerError>(*node, "Return statement with a value, in function returning None");
            } else {
                processExpression(child->firstChild(), ctx.currentFunctionType, ctx);
            }
            continue;
        }

        if (child->type == NodeType::IfStatement || child->type == NodeType::WhileStatement ||
            child->type == NodeType::ElifStatement) {
            Node::Ptr &expr_root = child->firstChild()->firstChild();
            processUnknownTypeExpression(expr_root, expr_root->type, ctx);
        }

        if (child->type == NodeType::ElseStatement) {
            processBranchRoot(child->firstChild(), ctx);
            continue;
        }

        processBranchRoot(child, ctx);
    }
    ctx.variables.pop_front();
}

static void parseFunctions(const std::list<Node::Ptr> &children, SemantizerContext &ctx) {
    for (auto &node : children) {
        if (node->type == NodeType::FunctionDefinition) {
            auto child = node->children.begin();
            auto name = (*child)->str();
            child++;
            VariablesTable tmp;
            auto args = getFunctionArguments((*child)->children, tmp);
            child++;
            auto ret_type = (*child)->typeId();
            ctx.functions.emplace(name, Function(ret_type, args));
            child++;
            if (!std::holds_alternative<VariablesTable>((*child)->value))
                (*child)->value.emplace<VariablesTable>(tmp);
        }
    }

    for (auto &node : children) {
        if (node->type == NodeType::FunctionDefinition) {
            auto child = node->children.begin();
            std::advance(child, 2);
            ctx.currentFunctionType = (*child)->typeId();
            processBranchRoot(*std::next(child), ctx);
        }
    }

    if (ctx.functions.find("main") == ctx.functions.end())
        ctx.errors.push<SemantizerError>("Function 'main' is not declared, although it has to be");
}

void Semantizer::process(SyntaxTree &tree) {
    SemantizerContext ctx(tree.functions);
    parseFunctions(tree.root->children, ctx);
    if (!ctx.errors.empty())
        throw ctx.errors;
}
