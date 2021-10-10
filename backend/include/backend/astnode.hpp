#pragma once

#include <memory>
#include <string>

struct Literal {
    union {
        long int Integer;
        double FPoint;
        std::string String;
    };

    ~Literal() = default;
};

struct ASTNode {
    using Ptr = std::shared_ptr<ASTNode>;
    using WeakPtr = std::weak_ptr<ASTNode>;

    Ptr left;
    Ptr right;
    WeakPtr parent;

    enum Type {
        BinaryOperation,
        VariableIdentifier,
        IntegerLiteralValue,
        FPointLiteralValue,
        StringLiteralValue,
        BranchRoot
    };
    Type type;
    Literal literal;

    ~ASTNode() = default;
};
