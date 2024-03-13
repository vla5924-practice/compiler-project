#include <gtest/gtest.h>

#include "compiler/ast/syntax_tree.hpp"

using namespace ast;

TEST(SyntaxTree, can_compare_same_syntax_trees) {
    SyntaxTree tree, tree1;
    tree.root = std::make_shared<Node>(NodeType::ProgramRoot);
    tree1.root = std::make_shared<Node>(NodeType::ProgramRoot);
    Node::Ptr child = std::make_shared<Node>(NodeType::FunctionDefinition, tree.root);
    tree.root->children.push_back(child);
    Node::Ptr child1 = std::make_shared<Node>(NodeType::FunctionDefinition, tree1.root);
    tree1.root->children.push_back(child1);
    ASSERT_TRUE(tree == tree1);
    ASSERT_FALSE(tree != tree1);
}

TEST(SyntaxTree, can_compare_different_syntax_trees) {
    SyntaxTree tree, tree1;
    tree.root = std::make_shared<Node>(NodeType::ProgramRoot);
    tree1.root = std::make_shared<Node>(NodeType::ProgramRoot);
    Node::Ptr child = std::make_shared<Node>(NodeType::FunctionDefinition, tree.root);
    tree.root->children.push_back(child);
    Node::Ptr child1 = std::make_shared<Node>(NodeType::FunctionReturnType, tree1.root);
    tree1.root->children.push_back(child1);
    ASSERT_TRUE(tree != tree1);
    ASSERT_FALSE(tree == tree1);
}
