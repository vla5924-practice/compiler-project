#pragma once

namespace ast {

enum class NodeType {
    BinaryOperation,
    UnaryOperation,
    VariableName,
    TypeName,
    FunctionName,
    IntegerLiteralValue,
    FPointLiteralValue,
    StringLiteralValue,
    IfExpression,
    WhileExpression,
    FunctionDefinition,
    FunctionArguments,
    FunctionArgument,
    FunctionReturnType,
    FunctionBody,
    ProgramRoot,
    BranchRoot,
    Expression,
    VariableDeclaration,
};

}
