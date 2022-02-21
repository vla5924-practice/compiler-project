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

void Semantizer::processExpression(Node::Ptr &node, TypeId var_type, Node::Ptr &branch) {
    NodeType type;

    if (var_type == BuiltInTypes::IntType) {
        type = NodeType::IntegerLiteralValue;
    }

    if (var_type == BuiltInTypes::FloatType) {
        type = NodeType::FloatingPointLiteralValue;
    }

    for (auto &child : node->children) {
        if (child->type == NodeType::UnaryOperation || child->type == NodeType::BinaryOperation) {
            processExpression(child, var_type, branch);
            continue;
        }

        if (child->type == NodeType::VariableName) {
            auto find_type = searchVariable(branch, child->str());
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
        auto find_type = searchVariable(branch, node->str());
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

void Semantizer::processBranchRoot(Node::Ptr &node, FunctionsTable &functions) {
    for (auto &child : node->children) {
        if (child->type == NodeType::VariableDeclaration) {
            auto list_child = child->children.begin();
            auto type = (*list_child)->typeId();
            list_child++;
            auto name = (*list_child)->str();
            if (!std::holds_alternative<VariablesTable>(node->value))
                node->value.emplace<VariablesTable>();
            node->variables().emplace(name, type);
            list_child++;

            if (list_child != child->children.end() && (*list_child)->type == NodeType::Expression) {
                processExpression(*list_child, type, node);
            }

            continue;
        }

        if (child->type == NodeType::Expression) {
            Node::Ptr &expr_root = child->children.front();
            if (expr_root->type == NodeType::BinaryOperation && expr_root->binOp() == BinaryOperation::Assign) {
                auto name = expr_root->children.front()->str();
                auto type = searchVariable(node, name);
                processExpression(expr_root->children.back(), type, node);
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
                // err
            }

            continue;
        }

        processBranchRoot(child, functions);
    }
}

ast::TypeId Semantizer::searchVariable(ast::Node::Ptr &node, const std::string &name) {
    ast::VariablesTable::iterator table_name;
    TypeId type;
    if (node->type == NodeType::BranchRoot) {
        table_name = node->variables().find(name);
        if (table_name != node->variables().cend()) {
            type = table_name->second;
        } else {
            type = searchVariable(node->parent, name);
        }
    } else {
        type = searchVariable(node->parent, name);
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
            processBranchRoot(*child, functions);
        }
    }
}

void Semantizer::process(SyntaxTree &tree) {
    parseFunctions(tree.root->children, tree.functions);
}
