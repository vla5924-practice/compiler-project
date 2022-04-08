#include "semantizer/semantizer.hpp"

using namespace semantizer;
using namespace ast;

static std::vector<TypeId> getFunctionArguments(const std::list<Node::Ptr> &functionArguments) {
    std::vector<TypeId> result;

    for (const auto &node : functionArguments) {
        result.push_back((*node->children.cbegin())->typeId());
    }

    return result;
}

static TypeId searchVariable(const Node::Ptr &node, const std::list<VariablesTable *> &tables, ErrorBuffer &errors) {
    TypeId type = BuiltInTypes::BuiltInTypesCount;
    for (const auto &table : tables) {
        VariablesTable::const_iterator table_name = table->find(node->str());
        if (table_name != table->cend()) {
            type = table_name->second;
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
        return BinaryOperation::FAdd;
        break;
    case BinaryOperation::Sub:
        return BinaryOperation::FSub;
        break;
    case BinaryOperation::Mult:
        return BinaryOperation::FMult;
        break;
    case BinaryOperation::Div:
        return BinaryOperation::FDiv;
        break;
    case BinaryOperation::And:
        return BinaryOperation::FAnd;
        break;
    case BinaryOperation::Or:
        return BinaryOperation::FOr;
        break;
    case BinaryOperation::Equal:
        return BinaryOperation::FEqual;
        break;
    case BinaryOperation::NotEqual:
        return BinaryOperation::FNotEqual;
        break;
    case BinaryOperation::Less:
        return BinaryOperation::FLess;
        break;
    case BinaryOperation::Greater:
        return BinaryOperation::FGreater;
        break;
    case BinaryOperation::LessEqual:
        return BinaryOperation::FLessEqual;
        break;
    case BinaryOperation::GreaterEqual:
        return BinaryOperation::FGreaterEqual;
        break;
    case BinaryOperation::Assign:
        return BinaryOperation::FAssign;
        break;
    }
    return BinaryOperation::Unknown;
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

    size_t index = 0;
    for (auto &it : (*children_it)->children) {
        processExpression(it, args[index++], tables, functions, errors);
    }

    return func->second.returnType;
}

static void processExpression(Node::Ptr &node, TypeId var_type, const std::list<VariablesTable *> &tables,
                              FunctionsTable &functions, ErrorBuffer &errors) {
    NodeType type;

    switch (var_type) {
    case BuiltInTypes::IntType:
        type = NodeType::IntegerLiteralValue;
        break;

    case BuiltInTypes::FloatType:
        type = NodeType::FloatingPointLiteralValue;
        break;

    case BuiltInTypes::StrType:
        type = NodeType::StringLiteralValue;
        break;

    default:
        errors.push<SemantizerError>(*node, "Invalid conversion from " + var_type);
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
                    errors.push<SemantizerError>(*node, "Invalid conversion from string to " + var_type);
                }
                pushTypeConversion(child, type);
            }
            if (find_type == BuiltInTypes::FloatType) {
                node->value = convertToFloatOperation(node->binOp());
            }
            continue;
        }

        if (child->type != type) {
            pushTypeConversion(child, type);
        }

        if (child->type == NodeType::FloatingPointLiteralValue) {
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

            node->variables().emplace(name, type);
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
                processExpression(expr_root->children.back(), type, tables, functions, errors);
                if (!std::holds_alternative<TypeId>(node->value))
                    child->value.emplace<TypeId>(type);
            }
            if (expr_root->type == NodeType::FunctionCall) {
                processFunctionCall(expr_root, tables, functions, errors);
                if (!std::holds_alternative<TypeId>(node->value))
                    child->value.emplace<TypeId>(expr_root->children.front()->typeId());
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
            auto args = getFunctionArguments((*child)->children);
            child++;
            auto ret_type = (*child)->typeId();
            functions.emplace(name, Function(ret_type, args));
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
}

void Semantizer::process(SyntaxTree &tree) {
    ErrorBuffer errors;
    parseFunctions(tree.root->children, tree.functions, errors);
    if (!errors.empty())
        throw errors;
}
