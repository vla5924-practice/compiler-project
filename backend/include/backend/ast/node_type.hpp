#pragma once

namespace ast {

enum class NodeType {
    BinaryOperation,
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
};

}
