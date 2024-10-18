#include <gtest/gtest.h>

#include "compiler/ast/declarative.hpp"
#include "compiler/ast/node_type.hpp"
#include "compiler/ast/types.hpp"

using namespace ast;

TEST(Declarative, can_create_with_program_root) {
    DeclarativeTree t;
    ASSERT_EQ(t.dump(), "ProgramRoot\n");
}

TEST(Declarative, can_create_with_given_type) {
    DeclarativeTree t(NodeType::Expression);
    ASSERT_EQ(t.dump(), "Expression\n");
}

TEST(Declarative, can_create_and_build_tree) {
    DeclarativeTree t;
    // clang-format off
    t.node(NodeType::FunctionDefinition).withChildren();
        t.node(NodeType::FunctionName, "main");
        t.node(NodeType::FunctionArguments).withChildren();
            t.node(NodeType::FunctionArgument).withChildren();
                t.node(NodeType::TypeName, IntType);
                t.node(NodeType::VariableName, "x");
            t.endChildren();
        t.endChildren();
        t.node(NodeType::FunctionReturnType, NoneType);
        t.node(NodeType::BranchRoot).withChildren();
            t.node(NodeType::VariableDeclaration).withChildren();
                t.node(NodeType::TypeName, IntType);
                t.node(NodeType::VariableName, "y");
                t.node(NodeType::Expression).withChildren();
                    t.node(NodeType::IntegerLiteralValue, 1);
                t.endChildren();
            t.endChildren();
            t.node(NodeType::IfStatement).withChildren();
                t.node(NodeType::Expression).withChildren();
                    t.node(NodeType::BinaryOperation, BinaryOperation::Equal).withChildren();
                        t.node(NodeType::VariableName, "x");
                        t.node(NodeType::IntegerLiteralValue, 2);
                    t.endChildren();
                t.endChildren(); 
                t.node(NodeType::BranchRoot).withChildren();
                    t.node(NodeType::Expression).withChildren();
                        t.node(NodeType::BinaryOperation, BinaryOperation::Assign).withChildren();
                            t.node(NodeType::VariableName, "y");
                            t.node(NodeType::BinaryOperation, BinaryOperation::Add).withChildren();
                                t.node(NodeType::VariableName, "x");
                                t.node(NodeType::IntegerLiteralValue, 3);
                            t.endChildren();
                        t.endChildren();
                    t.endChildren(); 
                t.endChildren();
            t.endChildren();
        t.endChildren();
    t.endChildren();
    // clang-format on
    ASSERT_EQ(t.dump(), R"(ProgramRoot
  FunctionDefinition
    FunctionName: main
    FunctionArguments
      FunctionArgument
        TypeName: IntType
        VariableName: x
    FunctionReturnType: NoneType
    BranchRoot
      VariableDeclaration
        TypeName: IntType
        VariableName: y
        Expression
          IntegerLiteralValue: 1
      IfStatement
        Expression
          BinaryOperation: Equal
            VariableName: x
            IntegerLiteralValue: 2
        BranchRoot
          Expression
            BinaryOperation: Assign
              VariableName: y
              BinaryOperation: Add
                VariableName: x
                IntegerLiteralValue: 3
)");
}
