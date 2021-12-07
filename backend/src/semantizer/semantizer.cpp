#include "semantizer/semantizer.hpp"
#include "iostream"

using namespace semantizer;
using namespace ast;

std::vector<TypeId> Semantizer::getFunctionArguments(std::list<ast::Node::Ptr> &children) {
    std::vector<TypeId> result;

    for (auto &node : children) {
        result.push_back(node->typeId());
    }

    return result;
}

void Semantizer::parseExpression(ast::Node::Ptr &node, TypeId var_type, ast::Node::Ptr &branch) {
    NodeType type;

    if (var_type == BuiltInTypes::IntType) {
        type = NodeType::IntegerLiteralValue;
    }

    if (var_type == BuiltInTypes::FloatType) {
        type = NodeType::FloatingPointLiteralValue;
    }

    for (auto &it : node->children) {
        if (it->type == NodeType::UnaryOperation || it->type == NodeType::BinaryOperation) {
            parseExpression(it, var_type, branch);
            continue;
        }

        if (it->type == NodeType::VariableName) {
            auto table_var = branch->variables().find(it->str());

            if (table_var == branch.get()->variables().cend()) {
                // error
            }

            if (table_var->second != var_type) {
                pushTypeConversion(it, type);
            }
            continue;
        }

        if (it->type != type) {
            pushTypeConversion(it, type);
        }
    }
}

void Semantizer::pushTypeConversion(ast::Node::Ptr &node, TypeId type) {
    Node::Ptr conv_node = std::make_shared<Node>(Node(NodeType::TypeConversion, node->parent));
    conv_node->children.push_front(node);
    Node::Ptr type_node = std::make_shared<Node>(Node(type, conv_node));
    conv_node->children.push_front(type_node);
    node = conv_node;
}

void Semantizer::pushTypeConversion(ast::Node::Ptr &node, NodeType type) {
    Node::Ptr conv_node = std::make_shared<Node>(Node(NodeType::TypeConversion, node->parent));
    conv_node->children.push_front(node);
    Node::Ptr type_node = std::make_shared<Node>(Node(type, conv_node));
    conv_node->children.push_front(type_node);
    node = conv_node;
}

void Semantizer::parseBranchRoot(ast::Node::Ptr &node, ast::FunctionsTable &functions) {

    for (auto &it : node->children) {
        if (it->type == NodeType::VariableDeclaration) {
            auto list_it = it->children.begin();
            auto type = list_it->get()->typeId();
            list_it++;
            auto name = list_it->get()->str();
            if (!std::holds_alternative<VariablesTable>(node->value))
                node->value.emplace<VariablesTable>();
            node->variables().emplace(name, type);
            list_it++;

            if (list_it != it->children.end() && list_it->get()->type == NodeType::Expression) {
                parseExpression(*list_it, type, node);
            }

            continue;
        }

        if (it->type == NodeType::Expression) {
            auto list_it = it->children.begin();
            if (list_it->get()->type == NodeType::BinaryOperation &&
                list_it->get()->binOp() == BinaryOperation::Assign) {
                auto a = list_it->get()->children.begin();
                auto var_table = node.get()->variables().find(a->get()->str());
                if (var_table == node.get()->variables().cend()) {
                    // error
                }
                auto type = var_table->second;
                a++;
                parseExpression(*a, type, node);
                continue;
            }

            continue;
        }

        if (it->type == NodeType::FunctionCall) {
            auto funct = functions.find(it->str());

            if (funct == functions.cend()) {
                if (it->str() == "print" || it->str() == "input") {
                    functions.emplace(it->str(), Function(BuiltInTypes::NoneType));
                    continue;
                }
                // err
            }

            continue;
        }

        parseBranchRoot(it, functions);
    }
};

void Semantizer::parseFunctions(std::list<Node::Ptr> &children, ast::FunctionsTable &functions) {
    for (auto &node : children) {
        if (node->type == NodeType::FunctionDefinition) {
            auto it = node->children.begin();
            auto name = it->get()->str();
            it++;
            auto args = getFunctionArguments((it)->get()->children);
            it++;
            auto ret_type = (it)->get()->typeId();
            functions.emplace(name, Function(ret_type, args));
            it++;
            parseBranchRoot(*it, functions);
        }
    }
}

ast::SyntaxTree &Semantizer::process(ast::SyntaxTree &tree) {
    parseFunctions(tree.root->children, tree.functions);

    return tree;
}