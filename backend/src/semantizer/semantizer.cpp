#include "semantizer/semantizer.hpp"

using namespace semantizer;
using namespace ast;

namespace {

Node::Ptr &firstChild(Node *node) {
    return node->children.front();
}

Node::Ptr &secondChild(Node *node) {
    return *std::next(node->children.begin());
}

Node::Ptr &lastChild(Node *node) {
    return node->children.back();
}

} // namespace

static std::vector<TypeId> getFunctionArguments(const std::list<Node::Ptr> &functionArguments, VariablesTable &table) {
    std::vector<TypeId> result;

    for (const auto &node : functionArguments) {
        auto children = node->children.begin();
        auto type = (*children)->typeId();
        children++;
        auto name = (*children)->str();
        result.push_back(type);
        table[name] = {type};
    }

    return result;
}

static TypeId searchVariable(const Node::Ptr &node, const std::list<VariablesTable *> &tables, ErrorBuffer &errors) {
    TypeId type = BuiltInTypes::BuiltInTypesCount;
    for (const auto &table : tables) {
        VariablesTable::const_iterator tableEntry = table->find(node->str());
        if (tableEntry != table->cend()) {
            type = tableEntry->second.type;
            break;
        }
    }

    if (type == BuiltInTypes::BuiltInTypesCount) {
        errors.push<SemantizerError>(*node, node->str() + " was not declared in this scope");
    }

    return type;
};

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

static void processExpression(Node::Ptr &node, TypeId var_type, const std::list<VariablesTable *> &tables,
                              FunctionsTable &functions, ErrorBuffer &errors);

static TypeId processFunctionCall(Node::Ptr &node, const std::list<VariablesTable *> &tables, FunctionsTable &functions,
                                  ErrorBuffer &errors) {
    auto func = functions.find((*node->children.begin())->str());

    if (func == functions.cend()) {
        if (node->str() == "print" || node->str() == "input") {
            functions.emplace(node->str(), Function(BuiltInTypes::NoneType));
        }
        errors.push<SemantizerError>(*node, node->str() + " was not declared in this scope");
    }

    auto children_it = node->children.begin();
    children_it++;

    auto args = func->second.argumentsTypes;

    if (children_it == node->children.end())
        return func->second.returnType;

    size_t index = 0;
    for (auto &it : (*children_it)->children) {
        processExpression(it, args[index++], tables, functions, errors);
    }

    return func->second.returnType;
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

static void processExpression(Node::Ptr &node, TypeId var_type, const std::list<VariablesTable *> &tables,
                              FunctionsTable &functions, ErrorBuffer &errors) {
    TypeId type;

    switch (var_type) {
    case BuiltInTypes::IntType:
    case BuiltInTypes::FloatType:
    case BuiltInTypes::StrType:
        type = var_type;
        break;

    default:
        errors.push<SemantizerError>(*node, "Invalid conversion from " + std::to_string(var_type));
        break;
    }

    if (node->type == NodeType::Expression && !std::holds_alternative<TypeId>(node->value))
        node->value.emplace<TypeId>(var_type);

    for (auto &child : node->children) {
        if (child->type == NodeType::UnaryOperation || child->type == NodeType::BinaryOperation) {
            processExpression(child, var_type, tables, functions, errors);
            continue;
        }

        if (child->type == NodeType::FunctionCall) {
            TypeId retType = processFunctionCall(child, tables, functions, errors);
            if (retType != var_type) {
                pushTypeConversion(node, var_type);
            }
            continue;
        }

        if (child->type == NodeType::VariableName) {
            auto find_type = searchVariable(child, tables, errors);
            if (find_type != var_type) {
                if (find_type == BuiltInTypes::StrType && child->str().size() != 1) {
                    errors.push<SemantizerError>(*node,
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

        if ((child->type == NodeType::FloatingPointLiteralValue && node->type == NodeType::BinaryOperation) ||
            (child->type == NodeType::TypeConversion && firstChild(child.get())->typeId() == BuiltInTypes::FloatType)) {
            node->value = convertToFloatOperation(node->binOp());
        }
    }

    if (node->type == NodeType::VariableName) {
        auto find_type = searchVariable(node, tables, errors);
        if (find_type != var_type) {
            pushTypeConversion(node, type);
        }
    }
}

static void processBranchRoot(Node::Ptr &node, FunctionsTable &functions, std::list<VariablesTable *> &tables,
                              ErrorBuffer &errors) {
    if (!std::holds_alternative<VariablesTable>(node->value))
        node->value.emplace<VariablesTable>();
    tables.push_front(&node->variables());
    for (auto &child : node->children) {
        if (child->type == NodeType::VariableDeclaration) {
            auto list_child = child->children.begin();
            auto type = (*list_child)->typeId();
            list_child++;
            auto name = (*list_child)->str();

            VariablesTable::const_iterator table_name = tables.front()->find(name);
            if (table_name != tables.front()->cend()) {
                errors.push<SemantizerError>(*child, "Redeclaration of " + name);
            }

            node->variables()[name] = {type};
            list_child++;

            if (list_child != child->children.end() && (*list_child)->type == NodeType::Expression) {
                processExpression(*list_child, type, tables, functions, errors);
            }

            continue;
        }

        if (child->type == NodeType::Expression) {
            Node::Ptr &expr_root = child->children.front();
            if (expr_root->type == NodeType::BinaryOperation && expr_root->binOp() == BinaryOperation::Assign) {
                auto name = expr_root->children.front()->str();
                auto type = searchVariable(expr_root->children.front(), tables, errors);
                processExpression(expr_root, type, tables, functions, errors);
                if (!std::holds_alternative<TypeId>(node->value))
                    child->value.emplace<TypeId>(type);
            }
            if (expr_root->type == NodeType::FunctionCall) {
                auto type = processFunctionCall(expr_root, tables, functions, errors);
                if (!std::holds_alternative<TypeId>(node->value))
                    child->value.emplace<TypeId>(type);
                continue;
            }
            continue;
        }

        processBranchRoot(child, functions, tables, errors);
    }
    tables.pop_front();
}

static void parseFunctions(const std::list<Node::Ptr> &children, FunctionsTable &functions, ErrorBuffer &errors) {
    for (auto &node : children) {
        if (node->type == NodeType::FunctionDefinition) {
            auto child = node->children.begin();
            auto name = (*child)->str();
            child++;
            VariablesTable tmp;
            auto args = getFunctionArguments((*child)->children, tmp);
            child++;
            auto ret_type = (*child)->typeId();
            functions.emplace(name, Function(ret_type, args));
            child++;
            if (!std::holds_alternative<VariablesTable>((*child)->value))
                (*child)->value.emplace<VariablesTable>(tmp);
        }
    }

    for (auto &node : children) {
        if (node->type == NodeType::FunctionDefinition) {
            auto child = node->children.begin();
            std::advance(child, 3);
            std::list<VariablesTable *> variables_table;
            processBranchRoot(*child, functions, variables_table, errors);
        }
    }

    if (functions.find("main") == functions.end())
        errors.push<SemantizerError>("Function 'main' is not declared, although it has to be");
}

void Semantizer::process(SyntaxTree &tree) {
    ErrorBuffer errors;
    parseFunctions(tree.root->children, tree.functions, errors);
    if (!errors.empty())
        throw errors;
}
