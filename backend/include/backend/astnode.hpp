#pragma once

#include <list>
#include <memory>
#include <string>

struct ASTNode {
    using Ptr = std::shared_ptr<ASTNode>;
    using WeakPtr = std::weak_ptr<ASTNode>;

    std::list<Ptr> children;
    Ptr parent;

    enum Type {
        BinaryOperation,
        VariableIdentifier,
        IntegerLiteralValue,
        FPointLiteralValue,
        StringLiteralValue,
        IfExpression,
        WhileExpression,
        FunctionDefinition,
        FunctionArguments,
        FunctionReturnType,
        FunctionBody,
        ProgramRoot,
        BranchRoot,
    };
    Type type;

    union {
        long int intNumber;
        double fpNumber;
    } numLiteral;
    std::string strLiteral;

    ASTNode() = default;
    ASTNode(const Type &type_, Ptr parent_ = Ptr());
    ~ASTNode() = default;
};
