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
    FloatingPointLiteralValue,
    ForIterable,
    ForStatement,
    ForTargets,
    FunctionArgument,
    FunctionArguments,
    FunctionCall,
    FunctionDefinition,
    FunctionName,
    FunctionReturnType,
    IfStatement,
    IntegerLiteralValue,
    ListAccessor,
    ListStatement,
    ProgramRoot,
    ReturnStatement,
    StringLiteralValue,
    TypeConversion,
    TypeName,
    UnaryOperation,
    VariableDeclaration,
    VariableName,
    WhileStatement,
};

} // namespace ast