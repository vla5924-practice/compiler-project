#pragma once

namespace ast {

enum class BinaryOperation {
    Unknown,
    Add,
    Sub,
    Mult,
    Div,
    And,
    Or,
    Equal,
    NotEqual,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,
    Assign,
};

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

} // namespace ast
