#include "semantizer/semantizer.hpp"

using namespace semantizer;
using namespace ast;

std::vector<TypeId> Semantizer::getFunctionArguments(std::list<Node::Ptr> &children) {
    std::vector<TypeId> result;

    for (const auto &node : children) {
        result.push_back(node->typeId());
    }

    return result;
}

void Semantizer::processExpression(Node::Ptr &node, TypeId var_type, Node::Ptr &branch,
                                   const std::list<VariablesTable *> &tables) {
    NodeType type;

    switch (var_type) {
    case BuiltInTypes::IntType:
        type = NodeType::IntegerLiteralValue;
        break;

    case BuiltInTypes::FloatType:
        type = NodeType::FloatingPointLiteralValue;
        break;

    default:
        // err
        break;
    }

    for (auto &child : node->children) {
        if (child->type == NodeType::UnaryOperation || child->type == NodeType::BinaryOperation) {
            processExpression(child, var_type, branch, tables);
            continue;
        }

        if (child->type == NodeType::VariableName) {
            auto find_type = searchVariable(child->str(), tables);
            if (find_type != var_type) {
                pushTypeConversion(child, type);
            }
            continue;
        }

        if (child->type != type) {
            pushTypeConversion(child, type);
        }
    }

    if (node->type == NodeType::VariableName) {
        auto find_type = searchVariable(node->str(), tables);
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

void Semantizer::processBranchRoot(Node::Ptr &node, FunctionsTable &functions, std::list<VariablesTable *> &tables) {
    if (!std::holds_alternative<VariablesTable>(node->value))
        node->value.emplace<VariablesTable>();
    tables.push_front(&node->variables());
    for (auto &child : node->children) {
        if (child->type == NodeType::VariableDeclaration) {
            auto list_child = child->children.begin();
            auto type = (*list_child)->typeId();
            list_child++;
            auto name = (*list_child)->str();
            node->variables().emplace(name, type);
            list_child++;

            if (list_child != child->children.end() && (*list_child)->type == NodeType::Expression) {
                processExpression(*list_child, type, node, tables);
            }

            continue;
        }

        if (child->type == NodeType::Expression) {
            Node::Ptr &expr_root = child->children.front();
            if (expr_root->type == NodeType::BinaryOperation && expr_root->binOp() == BinaryOperation::Assign) {
                auto name = expr_root->children.front()->str();
                auto type = searchVariable(name, tables);
                processExpression(expr_root->children.back(), type, node, tables);
            }

            continue;
        }

        if (child->type == NodeType::FunctionCall) {
            auto funct = functions.find(child->str());

            if (funct == functions.cend()) {
                if (child->str() == "print" || child->str() == "input") {
                    functions.emplace(child->str(), Function(BuiltInTypes::NoneType));
                    continue;
                }
                // errors.push<SemantizerError>();
            }

            continue;
        }

        processBranchRoot(child, functions, tables);
    }
    tables.pop_front();
}

ast::TypeId Semantizer::searchVariable(const std::string &name, const std::list<VariablesTable *> &tables) {
    TypeId type = BuiltInTypes::BuiltInTypesCount;
    for (const auto &table : tables) {
        ast::VariablesTable::const_iterator table_name = table->find(name);
        if (table_name != table->cend()) {
            type = table_name->second;
            break;
        }
    }

    if (type == BuiltInTypes::BuiltInTypesCount) {
        // err
    }

    return type;
}

void Semantizer::parseFunctions(std::list<Node::Ptr> &children, FunctionsTable &functions) {
    for (auto &node : children) {
        if (node->type == NodeType::FunctionDefinition) {
            auto child = node->children.begin();
            auto name = (*child)->str();
            child++;
            auto args = getFunctionArguments((*child)->children);
            child++;
            auto ret_type = (*child)->typeId();
            functions.emplace(name, Function(ret_type, args));
            child++;
            std::list<VariablesTable *> variables_table;
            processBranchRoot(*child, functions, variables_table);
        }
    }
}

void Semantizer::process(SyntaxTree &tree) {
    ErrorBuffer errors;
    parseFunctions(tree.root->children, tree.functions);
}
