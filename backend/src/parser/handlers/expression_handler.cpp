#include "parser/handlers/expression_handler.hpp"

#include <stack>

#include "parser/register_handler.hpp"

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
    if (token.type == Token::Type::Operator) {
        const auto &op = token.op();
        switch (op) {
        case Token::Operator::Plus:
        case Token::Operator::Minus:
        case Token::Operator::Mult:
        case Token::Operator::Div:
        case Token::Operator::Equal:
        case Token::Operator::NotEqual:
        case Token::Operator::Less:
        case Token::Operator::More:
        case Token::Operator::LessEqual:
        case Token::Operator::MoreEqual:
        case Token::Operator::Assign:
            return OperationType::Binary;
        default:
            return OperationType::Unknown;
        }
    }
    if (token.type == Token::Type::Keyword) {
        const auto &kw = token.kw();
        switch (kw) {
        case Token::Keyword::And:
        case Token::Keyword::Or:
            return OperationType::Binary;
        case Token::Keyword::Not:
            return OperationType::Unary;
        default:
            return OperationType::Unknown;
        }
    }
    return OperationType::Unknown;
}

ExpressionTokenType getExpressionTokenType(const Token &token) {
    if (token.type == Token::Type::Identifier || token.type == Token::Type::IntegerLiteral ||
        token.type == Token::Type::FloatingPointLiteral || token.type == Token::Type::StringLiteral)
        return ExpressionTokenType::Operand;
    if (token.is(Token::Operator::LeftBrace))
        return ExpressionTokenType::OpeningBrace;
    if (token.is(Token::Operator::RightBrace))
        return ExpressionTokenType::ClosingBrace;
    if (getOperationType(token) != OperationType::Unknown)
        return ExpressionTokenType::Operation;
    return ExpressionTokenType::Unknown;
}

BinaryOperation getBinaryOperation(const Token &token) {
    if (token.type == Token::Type::Operator) {
        const auto &op = token.op();
        switch (op) {
        case Token::Operator::Plus:
            return BinaryOperation::Add;
        case Token::Operator::Minus:
            return BinaryOperation::Sub;
        case Token::Operator::Mult:
            return BinaryOperation::Mult;
        case Token::Operator::Div:
            return BinaryOperation::Div;
        case Token::Operator::Equal:
            return BinaryOperation::Equal;
        case Token::Operator::NotEqual:
            return BinaryOperation::NotEqual;
        case Token::Operator::Less:
            return BinaryOperation::Less;
        case Token::Operator::More:
            return BinaryOperation::Greater;
        case Token::Operator::LessEqual:
            return BinaryOperation::LessEqual;
        case Token::Operator::MoreEqual:
            return BinaryOperation::GreaterEqual;
        case Token::Operator::Assign:
            return BinaryOperation::Assign;
        default:
            return BinaryOperation::Unknown;
        }
    }
    if (token.type == Token::Type::Keyword) {
        const auto &kw = token.kw();
        switch (kw) {
        case Token::Keyword::And:
            return BinaryOperation::And;
        case Token::Keyword::Or:
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
    while (!it->is(Token::Operator::Colon) && !it->is(Token::Special::EndOfExpression))
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
            if (token.type == Token::Type::Identifier) {
                ast::Node::Ptr node = ParserState::pushChildNode(currNode, ast::NodeType::VariableName);
                node->value = token.id();
            } else if (token.type == Token::Type::IntegerLiteral) {
                ast::Node::Ptr node = ParserState::pushChildNode(currNode, ast::NodeType::IntegerLiteralValue);
                node->value = std::atol(token.literal().c_str());
            } else if (token.type == Token::Type::FloatingPointLiteral) {
                ast::Node::Ptr node = ParserState::pushChildNode(currNode, ast::NodeType::FPointLiteralValue);
                node->value = std::stod(token.literal());
            } else if (token.type == Token::Type::StringLiteral) {
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
