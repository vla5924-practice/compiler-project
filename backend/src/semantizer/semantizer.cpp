#include "semantizer/semantizer.hpp"

using namespace semantizer;
using namespace ast;

std::vector<TypeId> Semantizer::getFunctionArguments(std::list<Node::Ptr> &functionArguments) {
    std::vector<TypeId> result;

    for (const auto &node: functionArguments) {
        result.push_back((*node->children.cbegin())->typeId());
    }

    return result;
}

void Semantizer::processExpression(Node::Ptr &node, TypeId var_type,
                                   const std::list<VariablesTable *> &tables, FunctionsTable& functions, ErrorBuffer &errors) {
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
            continue;
        }

        if (child->type != type) {
            pushTypeConversion(child, type);
        }
    }

    if (node->type == NodeType::VariableName) {
        auto find_type = searchVariable(node, tables, errors);
        if (find_type != var_type) {
            pushTypeConversion(node, type);
        }
    }
}

void Semantizer::pushTypeConversion(Node::Ptr &node, TypeId type) {
    Node::Ptr conv_node = std::make_shared<Node>(NodeType::TypeConversion, node->parent);
    conv_node->children.push_front(node);
    Node::Ptr type_node = std::make_shared<Node>(type, conv_node);
    conv_node->children.push_front(type_node);
    node = conv_node;
}

void Semantizer::pushTypeConversion(Node::Ptr &node, NodeType type) {
    Node::Ptr conv_node = std::make_shared<Node>(NodeType::TypeConversion, node->parent);
    conv_node->children.push_front(node);
    Node::Ptr type_node = std::make_shared<Node>(type, conv_node);
    conv_node->children.push_front(type_node);
    node = conv_node;
}

TypeId Semantizer::processFunctionCall(Node::Ptr &node,
                        const std::list<VariablesTable *> &tables, FunctionsTable& functions, ErrorBuffer &errors) {
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
    for (auto &it: (*children_it)->children) {
        processExpression(it, args[index++], tables, functions, errors);
    }

    return func->second.returnType;
}

void Semantizer::processBranchRoot(Node::Ptr &node, FunctionsTable &functions, std::list<VariablesTable *> &tables,
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

            ast::VariablesTable::const_iterator table_name = tables.front()->find(name);
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
            }
            if (expr_root->type == NodeType::FunctionCall) {
                processFunctionCall(expr_root, tables, functions, errors);
                continue;
            }
            continue;
        }

        processBranchRoot(child, functions, tables, errors);
    }
    tables.pop_front();
}

ast::TypeId Semantizer::searchVariable(const Node::Ptr &node, const std::list<VariablesTable *> &tables,
                                       ErrorBuffer &errors) {
    TypeId type = BuiltInTypes::BuiltInTypesCount;
    for (const auto &table : tables) {
        ast::VariablesTable::const_iterator table_name = table->find(node->str());
        if (table_name != table->cend()) {
            type = table_name->second;
            break;
        }
    }

    if (type == BuiltInTypes::BuiltInTypesCount) {
        errors.push<SemantizerError>(*node, node->str() + " was not declared in this scope");
    }

    return type;
}

void Semantizer::parseFunctions(std::list<Node::Ptr> &children, FunctionsTable &functions, ErrorBuffer &errors) {
    for (auto &node : children) {
        if (node->type == NodeType::FunctionDefinition) {
            auto child = node->children.begin();
            auto name = (*child)->str();
            child++;
            auto args = getFunctionArguments((*child)->children);
            child++;
            auto ret_type = (*child)->typeId();
            functions.emplace(name, Function(ret_type, args));
            //child++;
            //std::list<VariablesTable *> variables_table;
            //processBranchRoot(*child, functions, variables_table, errors);
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
