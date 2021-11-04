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
    FunctionDefinition,
    FunctionArguments,
    FunctionArgument,
    FunctionReturnType,
    ProgramRoot,
    BranchRoot,
    Expression,
    VariableDeclaration,
    IfStatement,
    ElifStatement,
    WhileStatement,
};

}
