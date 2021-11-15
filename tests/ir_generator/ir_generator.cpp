#include <gtest/gtest.h>

#include "ir_generator/ir_generator.hpp"

using namespace ast;
using namespace ir_generator;

TEST(IRGenerator, generic_test) {
    IRGenerator generator("module");
    SyntaxTree tree1;
    Function fn;
    fn.returnType = NoneType;
    tree1.functions.insert_or_assign("main", fn);
    tree1.root = std::make_shared<Node>(NodeType::ProgramRoot);
    Node::Ptr child = std::make_shared<Node>(NodeType::FunctionDefinition, tree1.root);
    tree1.root->children.push_back(child);
    Node::Ptr chil1 = std::make_shared<Node>(NodeType::FunctionName, child);
    chil1->value = "main";
    Node::Ptr chil2 = std::make_shared<Node>(NodeType::FunctionArguments, child);
    Node::Ptr chil3 = std::make_shared<Node>(NodeType::FunctionReturnType, child);
    chil3->value = NoneType;
    Node::Ptr chil4 = std::make_shared<Node>(NodeType::BranchRoot, child);
    child->children.push_back(chil1);
    child->children.push_back(chil2);
    child->children.push_back(chil3);
    child->children.push_back(chil4);
    Node::Ptr chi1 = std::make_shared<Node>(NodeType::VariableDeclaration, chil4);
    Node::Ptr chi2 = std::make_shared<Node>(NodeType::IfStatement, chil4);
    chil4->children.push_back(chi1);
    chil4->children.push_back(chi2);
    Node::Ptr ch1 = std::make_shared<Node>(NodeType::TypeName, chi1);
    ch1->value = IntType;
    Node::Ptr ch2 = std::make_shared<Node>(NodeType::VariableName, chi1);
    ch2->value = "x";
    Node::Ptr ch3 = std::make_shared<Node>(NodeType::Expression, chi1);
    chi1->children.push_back(ch1);
    chi1->children.push_back(ch2);
    chi1->children.push_back(ch3);
    Node::Ptr c1 = std::make_shared<Node>(NodeType::IntegerLiteralValue, ch3);
    c1->value = 1l;
    ch3->children.push_back(c1);
    Node::Ptr ch4 = std::make_shared<Node>(NodeType::Expression, chil4);
    Node::Ptr ch5 = std::make_shared<Node>(NodeType::BranchRoot, chil4);
    chi2->children.push_back(ch4);
    chi2->children.push_back(ch5);
    Node::Ptr c2 = std::make_shared<Node>(NodeType::BinaryOperation, ch4);
    c2->value = BinaryOperation::Equal;
    ch4->children.push_back(c2);
    Node::Ptr _child1 = std::make_shared<Node>(NodeType::VariableName, c2);
    _child1->value = "x";
    Node::Ptr _child2 = std::make_shared<Node>(NodeType::IntegerLiteralValue, c2);
    _child2->value = 2l;
    c2->children.push_back(_child1);
    c2->children.push_back(_child2);
    Node::Ptr c3 = std::make_shared<Node>(NodeType::Expression, ch5);
    ch5->children.push_back(c3);
    Node::Ptr _child3 = std::make_shared<Node>(NodeType::BinaryOperation, c3);
    _child3->value = BinaryOperation::Assign;
    c3->children.push_back(_child3);
    Node::Ptr _chil1 = std::make_shared<Node>(NodeType::VariableName, _child3);
    _chil1->value = "x";
    Node::Ptr _chil2 = std::make_shared<Node>(NodeType::IntegerLiteralValue, _child3);
    _chil2->value = 3l;
    _child3->children.push_back(_chil1);
    _child3->children.push_back(_chil2);
    generator.process(tree1);
    generator.dump();
    ASSERT_TRUE(true);
}
