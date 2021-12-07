#include "semantizer/semantizer.hpp"
#include "iostream"

using namespace semantizer;
using namespace ast;

bool Semantizer::ch(NodeType type, TypeId type_id) {

    switch (type_id) {
    case BuiltInTypes::IntType:
        if (type == NodeType::IntegerLiteralValue) {
            return true;
        } else {
            // add TypeConverting node
        }
        break;

    case BuiltInTypes::FloatType:
        if (type == NodeType::FloatingPointLiteralValue) {
            return true;
        } else {
            // add TypeConverting node
        }
        break;

    case BuiltInTypes::StrType:
        if (type == NodeType::FloatingPointLiteralValue) {
            return true;
        } else {
            // add TypeConverting node
        }
        break;

    default:
        break;
    }

    return true;
}

std::vector<TypeId> Semantizer::test1(std::list<ast::Node::Ptr> &children) {
    std::vector<TypeId> result;

    for (auto &node : children) {
        result.push_back(node->typeId());
    }

    return result;
}

void Semantizer::test3(ast::Node::Ptr &node, TypeId var_type) {
    NodeType type;

    if (var_type == BuiltInTypes::IntType) {
        type = NodeType::IntegerLiteralValue;
    }

    if (var_type == BuiltInTypes::FloatType) {
        type = NodeType::FloatingPointLiteralValue;
    }

    for (auto &it : node->children) {
        if (it->type == NodeType::UnaryOperation || it->type == NodeType::BinaryOperation) {
            test3(it, var_type);
            continue;
        }

        if (it->type != type) {
            type_conv(it, type);
        }
    }
}

void Semantizer::type_conv(ast::Node::Ptr &node, NodeType type) {
    Node::Ptr type_node = std::make_shared<Node>(Node(NodeType::TypeConversion, node->parent));
    type_node->children.push_front(node);
    // push other node
    node = type_node;
}

void Semantizer::test2(ast::Node::Ptr &node) {

    for (auto &it : node->children) {
        if (it->type == NodeType::VariableDeclaration) {
            auto list_it = it->children.begin();
            auto type = list_it->get()->typeId();
            list_it++;
            auto name = list_it->get()->str();
            node->value.emplace<VariablesTable>();
            node->variables().emplace(std::make_pair(name, type));
            list_it++;

            if (list_it != it->children.end() && list_it->get()->type == NodeType::Expression) {
                test3(*list_it, type);
            }

            // test2(it);
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
                test3(*a, type);
                continue;
            }

            std::cout << "111" << std::endl;
            continue;
        }
        test2(it);
    }
};

void Semantizer::test(std::list<Node::Ptr> &children, ast::FunctionsTable &functions) {
    for (auto &node : children) {
        if (node->type == NodeType::FunctionDefinition) {
            auto it = node->children.begin();
            auto name = it->get()->str();
            it++;
            auto args = test1((it)->get()->children);
            it++;
            auto ret_type = (it)->get()->typeId();
            functions.emplace(std::make_pair(name, Function(ret_type, args)));
            it++;
            test2(*it);
        }
    }
}

ast::SyntaxTree &Semantizer::process(ast::SyntaxTree &tree) {
    test(tree.root->children, tree.functions);

    return tree;
}