#include <gtest/gtest.h>

#include "ast/node.hpp"

using namespace ast;

TEST(Node, can_construct_integer_literal_value) {
    long value = 1l;
    Node node(value);
    ASSERT_EQ(value, node.intNum());
    ASSERT_EQ(NodeType::IntegerLiteralValue, node.type);
}

TEST(Node, can_construct_fp_literal_value) {
    float value = 1.0f;
    Node node(value);
    ASSERT_EQ(value, node.fpNum());
    ASSERT_EQ(NodeType::FloatingPointLiteralValue, node.type);
}

TEST(Node, can_construct_variable_name) {
    std::string value = "name";
    Node node(NodeType::VariableName, value);
    ASSERT_EQ(value, node.str());
    ASSERT_EQ(NodeType::VariableName, node.type);
}

TEST(Node, can_construct_type_id) {
    size_t type_id = 1;
    Node node(type_id);
    ASSERT_EQ(type_id, node.typeId());
    ASSERT_EQ(NodeType::TypeName, node.type);
}

TEST(Node, can_construct_binary_operation) {
    BinaryOperation bin_op = BinaryOperation::Add;
    Node node(bin_op);
    ASSERT_EQ(bin_op, node.binOp());
    ASSERT_EQ(NodeType::BinaryOperation, node.type);
}

TEST(Node, can_construct_unary_operation) {
    UnaryOperation un_op = UnaryOperation::Not;
    Node node(un_op);
    ASSERT_EQ(un_op, node.unOp());
    ASSERT_EQ(NodeType::UnaryOperation, node.type);
}
