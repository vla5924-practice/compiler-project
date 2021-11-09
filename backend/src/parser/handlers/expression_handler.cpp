#include "parser/handlers/expression_handler.hpp"

#include <stack>

#include "lexer/token_types.hpp"
#include "parser/register_handler.hpp"

using namespace lexer;
using namespace parser;

namespace {

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

enum class UnaryOperation {
    Unknown,
    Not,
};

enum class OperationType {
    Unknown,
    Unary,
    Binary,
};

enum class ExpressionTokenType {
    Unknown,
    Operation,
    Operand,
    OpeningBrace,
    ClosingBrace,
};

OperationType getOperationType(const Token &token) {
    if (token.type == TokenType::Operator) {
        const auto &op = token.op();
        switch (op) {
        case Operator::Add:
        case Operator::Sub:
        case Operator::Mult:
        case Operator::Div:
        case Operator::Equal:
        case Operator::NotEqual:
        case Operator::Less:
        case Operator::Greater:
        case Operator::LessEqual:
        case Operator::GreaterEqual:
        case Operator::Assign:
            return OperationType::Binary;
        default:
            return OperationType::Unknown;
        }
    }
    if (token.type == TokenType::Keyword) {
        const auto &kw = token.kw();
        switch (kw) {
        case Keyword::And:
        case Keyword::Or:
            return OperationType::Binary;
        case Keyword::Not:
            return OperationType::Unary;
        default:
            return OperationType::Unknown;
        }
    }
    return OperationType::Unknown;
}

ExpressionTokenType getExpressionTokenType(const Token &token) {
    if (token.type == TokenType::Identifier || token.type == TokenType::IntegerLiteral ||
        token.type == TokenType::FloatingPointLiteral || token.type == TokenType::StringLiteral)
        return ExpressionTokenType::Operand;
    if (token.is(Operator::LeftBrace))
        return ExpressionTokenType::OpeningBrace;
    if (token.is(Operator::RightBrace))
        return ExpressionTokenType::ClosingBrace;
    if (getOperationType(token) != OperationType::Unknown)
        return ExpressionTokenType::Operation;
    return ExpressionTokenType::Unknown;
}

BinaryOperation getBinaryOperation(const Token &token) {
    if (token.type == TokenType::Operator) {
        const auto &op = token.op();
        switch (op) {
        case Operator::Add:
            return BinaryOperation::Add;
        case Operator::Sub:
            return BinaryOperation::Sub;
        case Operator::Mult:
            return BinaryOperation::Mult;
        case Operator::Div:
            return BinaryOperation::Div;
        case Operator::Equal:
            return BinaryOperation::Equal;
        case Operator::NotEqual:
            return BinaryOperation::NotEqual;
        case Operator::Less:
            return BinaryOperation::Less;
        case Operator::Greater:
            return BinaryOperation::Greater;
        case Operator::LessEqual:
            return BinaryOperation::LessEqual;
        case Operator::GreaterEqual:
            return BinaryOperation::GreaterEqual;
        case Operator::Assign:
            return BinaryOperation::Assign;
        default:
            return BinaryOperation::Unknown;
        }
    }
    if (token.type == TokenType::Keyword) {
        const auto &kw = token.kw();
        switch (kw) {
        case Keyword::And:
            return BinaryOperation::And;
        case Keyword::Or:
            return BinaryOperation::Or;
        default:
            return BinaryOperation::Unknown;
        }
    }
    return BinaryOperation::Unknown;
}

size_t getOperationPriority(const Token &token) {
    BinaryOperation op = getBinaryOperation(token);
    switch (op) {
    case BinaryOperation::Mult:
    case BinaryOperation::Div:
        return 10;
    case BinaryOperation::Add:
    case BinaryOperation::Sub:
        return 20;
    case BinaryOperation::Less:
    case BinaryOperation::LessEqual:
    case BinaryOperation::Greater:
    case BinaryOperation::GreaterEqual:
        return 30;
    case BinaryOperation::And:
        return 40;
    case BinaryOperation::Or:
        return 50;
    case BinaryOperation::Assign:
        return 60;
    default:
        return -1;
    }
    return -1;
}

size_t getOperandCount(OperationType type) {
    if (type == OperationType::Binary)
        return 2u;
    if (type == OperationType::Unary)
        return 1u;
    return -1;
}

} // namespace

void ExpressionHandler::run(ParserState &state) {
    // find expression end (EndOfExpression or Colon)
    auto it = state.tokenIter;
    while (!it->is(Special::Colon) && !it->is(Special::EndOfExpression))
        it++;
    const auto &tokenIterBegin = state.tokenIter;
    const auto &tokenIterEnd = it;

    std::stack<TokenList::const_iterator> postfixForm;
    std::stack<TokenList::const_iterator> operations;
    for (auto tokenIter = tokenIterBegin; tokenIter != tokenIterEnd; tokenIter++) {
        const Token &token = *tokenIter;
        OperationType opType = getOperationType(token);
        ExpressionTokenType expType = getExpressionTokenType(token);
        if (expType == ExpressionTokenType::Operand)
            postfixForm.push(tokenIter);
        else if (expType == ExpressionTokenType::OpeningBrace) {
            operations.push(tokenIter);
        } else if (expType == ExpressionTokenType::ClosingBrace) {
            while (getExpressionTokenType(*operations.top()) != ExpressionTokenType::OpeningBrace) {
                postfixForm.push(operations.top());
                operations.pop();
            }
            if (!operations.empty())
                operations.pop(); // remove opening brace
        } else if (expType == ExpressionTokenType::Operation) {
            if (operations.empty() || getOperationPriority(*operations.top()) <= getOperationPriority(token))
                operations.push(tokenIter);
            else {
                while (!operations.empty() && getOperationPriority(*operations.top()) >= getOperationPriority(token)) {
                    postfixForm.push(operations.top());
                    operations.pop();
                }
                operations.push(tokenIter);
            }
        } else
            throw -1; // something unknown
    }
    while (!operations.empty()) {
        postfixForm.push(operations.top());
        operations.pop();
    }
    // build expression tree based on reversed postfix form
    ast::Node::Ptr currNode = state.node;
    while (!postfixForm.empty()) {
        const Token &token = *postfixForm.top();
        ExpressionTokenType expType = getExpressionTokenType(token);
        size_t operandMaxCount = -1;
        if (expType == ExpressionTokenType::Operation) {
            OperationType opType = getOperationType(token);
            operandMaxCount = getOperandCount(opType);
            if (opType == OperationType::Binary) {
                currNode = ParserState::pushChildNode(currNode, ast::NodeType::BinaryOperation);
            } else {
                currNode = ParserState::pushChildNode(currNode, ast::NodeType::UnaryOperation);
            }
        } else if (expType == ExpressionTokenType::Operand) {
            if (token.type == TokenType::Identifier) {
                ast::Node::Ptr node = ParserState::pushChildNode(currNode, ast::NodeType::VariableName);
                node->value = token.id();
            } else if (token.type == TokenType::IntegerLiteral) {
                ast::Node::Ptr node = ParserState::pushChildNode(currNode, ast::NodeType::IntegerLiteralValue);
                node->value = std::atol(token.literal().c_str());
            } else if (token.type == TokenType::FloatingPointLiteral) {
                ast::Node::Ptr node = ParserState::pushChildNode(currNode, ast::NodeType::FPointLiteralValue);
                node->value = std::stod(token.literal());
            } else if (token.type == TokenType::StringLiteral) {
                ast::Node::Ptr node = ParserState::pushChildNode(currNode, ast::NodeType::StringLiteralValue);
                node->value = token.literal();
            }
        }
        while (currNode->children.size() >= operandMaxCount)
            currNode = currNode->parent;
        postfixForm.pop();
    }
    state.tokenIter = tokenIterEnd;
    state.node = state.node->parent;
}

REGISTER_PARSING_HANDLER(ExpressionHandler, ast::NodeType::Expression);
