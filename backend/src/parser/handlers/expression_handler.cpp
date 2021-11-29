#include "parser/handlers/expression_handler.hpp"

#include <cassert>
#include <stack>
#include <variant>

#include "ast/node_type.hpp"
#include "lexer/token_types.hpp"
#include "parser/parser_error.hpp"
#include "parser/register_handler.hpp"

using ast::BinaryOperation;
using ast::UnaryOperation;
using namespace lexer;
using namespace parser;

using TokenIterator = TokenList::const_iterator;
using SubExpression = std::variant<TokenIterator, ast::Node::Ptr>;

namespace {

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

OperationType getOperationType(const ast::Node &node) {
    switch (node.type) {
    case ast::NodeType::BinaryOperation:
        return OperationType::Binary;
    case ast::NodeType::UnaryOperation:
        return OperationType::Unary;
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

ExpressionTokenType getExpressionTokenType(const SubExpression &subexpr) {
    if (std::holds_alternative<ast::Node::Ptr>(subexpr))
        return ExpressionTokenType::Operand;
    return getExpressionTokenType(*std::get<TokenIterator>(subexpr));
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

bool isFunctionCall(const TokenIterator &tokenIter) {
    return tokenIter->type == TokenType::Identifier && std::next(tokenIter)->is(Operator::LeftBrace);
}

void buildExpressionSubtree(std::stack<SubExpression> postfixForm, ast::Node::Ptr root, ErrorBuffer &errors) {
    ast::Node::Ptr currNode = root;
    while (!postfixForm.empty()) {
        const SubExpression &subexpr = postfixForm.top();
        if (std::holds_alternative<TokenIterator>(subexpr)) {
            const Token &token = *std::get<TokenIterator>(subexpr);
            ExpressionTokenType expType = getExpressionTokenType(token);
            if (expType == ExpressionTokenType::Operation) {
                OperationType opType = getOperationType(token);
                if (opType == OperationType::Binary) {
                    currNode = ParserState::unshiftChildNode(currNode, ast::NodeType::BinaryOperation);
                    currNode->value = getBinaryOperation(token);
                } else if (opType == OperationType::Unary) {
                    currNode = ParserState::unshiftChildNode(currNode, ast::NodeType::UnaryOperation);
                } else {
                    errors.push<ParserError>(token,
                                             "Unknown operator found in expression, it must be either unary or binary");
                }
            } else if (expType == ExpressionTokenType::Operand) {
                if (token.type == TokenType::Identifier) {
                    ast::Node::Ptr node = ParserState::unshiftChildNode(currNode, ast::NodeType::VariableName);
                    node->value = token.id();
                } else if (token.type == TokenType::IntegerLiteral) {
                    ast::Node::Ptr node = ParserState::unshiftChildNode(currNode, ast::NodeType::IntegerLiteralValue);
                    node->value = std::atol(token.literal().c_str());
                } else if (token.type == TokenType::FloatingPointLiteral) {
                    ast::Node::Ptr node =
                        ParserState::unshiftChildNode(currNode, ast::NodeType::FloatingPointLiteralValue);
                    node->value = std::stod(token.literal());
                } else if (token.type == TokenType::StringLiteral) {
                    ast::Node::Ptr node = ParserState::unshiftChildNode(currNode, ast::NodeType::StringLiteralValue);
                    node->value = token.literal();
                }
            }
        } else {
            ast::Node::Ptr funcCallNode = std::get<ast::Node::Ptr>(subexpr);
            assert(funcCallNode->type == ast::NodeType::FunctionCall);
            currNode->children.push_front(funcCallNode);
        }
        while (currNode->children.size() >= getOperandCount(getOperationType(*currNode)))
            currNode = currNode->parent;
        postfixForm.pop();
    }
}

std::stack<SubExpression> generatePostfixForm(TokenIterator tokenIterBegin, TokenIterator tokenIterEnd,
                                              ErrorBuffer &errors) {
    std::stack<SubExpression> postfixForm;
    std::stack<TokenIterator> operations;
    for (auto tokenIter = tokenIterBegin; tokenIter != tokenIterEnd; tokenIter++) {
        const Token &token = *tokenIter;
        if (isFunctionCall(tokenIter)) {
            ast::Node::Ptr funcCallNode = std::make_shared<ast::Node>(ast::NodeType::FunctionCall);
            auto node = ParserState::pushChildNode(funcCallNode, ast::NodeType::FunctionName);
            node->value = token.id();
            auto argsBegin = std::next(tokenIter);
            auto it = argsBegin;
            while (!it->is(Operator::RightBrace))
                it++;
            auto argsEnd = it;
            if (std::distance(argsBegin, argsEnd) > 1) {
                auto argsNode = ParserState::pushChildNode(funcCallNode, ast::NodeType::FunctionArguments);
                auto argBegin = std::next(argsBegin);
                for (auto argsIter = argBegin; argsIter != std::next(argsEnd); argsIter++) {
                    if (!argsIter->is(Operator::Comma) && argsIter != argsEnd)
                        continue;
                    const Token &token = *argsIter;
                    std::stack<SubExpression> argPostfixForm = generatePostfixForm(argBegin, argsIter, errors);
                    auto exprNode = ParserState::pushChildNode(argsNode, ast::NodeType::Expression);
                    buildExpressionSubtree(argPostfixForm, exprNode, errors);
                    argBegin = std::next(argsIter);
                }
            }
            postfixForm.push(funcCallNode);
            tokenIter = argsEnd;
            continue;
        }
        OperationType opType = getOperationType(token);
        ExpressionTokenType expType = getExpressionTokenType(token);
        if (expType == ExpressionTokenType::Operand) {
            postfixForm.push(tokenIter);
        } else if (expType == ExpressionTokenType::OpeningBrace) {
            operations.push(tokenIter);
        } else if (expType == ExpressionTokenType::ClosingBrace) {
            bool foundBrace = false;
            while (!operations.empty()) {
                if (getExpressionTokenType(*operations.top()) != ExpressionTokenType::OpeningBrace) {
                    postfixForm.push(operations.top());
                    operations.pop();
                } else {
                    foundBrace = true;
                    break;
                }
            }
            if (!foundBrace) {
                errors.push<ParserError>(token, "Unexpected closing brance in an expression");
            }
            if (!operations.empty())
                operations.pop(); // remove opening brace
        } else if (expType == ExpressionTokenType::Operation) {
            if (operations.empty() || getOperationPriority(token) < getOperationPriority(*operations.top())) {
                operations.push(tokenIter);
            } else {
                while (!operations.empty() && getOperationPriority(*operations.top()) <= getOperationPriority(token)) {
                    postfixForm.push(operations.top());
                    operations.pop();
                }
                operations.push(tokenIter);
            }
        } else {
            errors.push<ParserError>(token, "Unexpected token inside an expression");
        }
    }
    while (!operations.empty()) {
        postfixForm.push(operations.top());
        operations.pop();
    }
    return postfixForm;
}

} // namespace

void ExpressionHandler::run(ParserState &state) {
    auto it = state.tokenIter;
    while (!it->is(Special::Colon) && !it->is(Special::EndOfExpression))
        it++;
    const auto &tokenIterBegin = state.tokenIter;
    const auto &tokenIterEnd = it;
    std::stack<SubExpression> postfixForm = generatePostfixForm(tokenIterBegin, tokenIterEnd, state.errors);
    buildExpressionSubtree(postfixForm, state.node, state.errors);
    state.tokenIter = tokenIterEnd;
    state.node = state.node->parent;
}

REGISTER_PARSING_HANDLER(ExpressionHandler, ast::NodeType::Expression);
