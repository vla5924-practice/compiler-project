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
    FAdd,
    FSub,
    FMult,
    FDiv,
    FAnd,
    FOr,
    FEqual,
    FNotEqual,
    FLess,
    FGreater,
    FLessEqual,
    FGreaterEqual,
    FAssign,
};

enum class UnaryOperation {
    Unknown,
    Not,
    Negative,
};

enum class NodeType {
    BinaryOperation,
    BooleanLiteralValue,
    BranchRoot,
    ElifStatement,
    ElseStatement,
    Expression,
    FunctionArgument,
    FunctionArguments,
    FunctionCall,
    FunctionDefinition,
    FunctionName,
    FunctionReturnType,
    FloatingPointLiteralValue,
    IfStatement,
    IntegerLiteralValue,
    ProgramRoot,
    ReturnStatement,
    StringLiteralValue,
    TypeConversion,
    TypeName,
    UnaryOperation,
    VariableDeclaration,
    VariableName,
    WhileStatement,
    ListStatement,
    ListAccessor,
};

} // namespace ast
