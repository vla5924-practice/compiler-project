#include "parser/parser.hpp"

#include <cassert>
#include <functional>
#include <iterator>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>

#include "compiler/ast/node.hpp"
#include "compiler/ast/node_type.hpp"
#include "compiler/ast/types.hpp"
#include "compiler/utils/error_buffer.hpp"

#include "lexer/token.hpp"
#include "lexer/token_types.hpp"
#include "parser/parser_context.hpp"
#include "parser/parser_error.hpp"
#include "parser/type_registry.hpp"

using namespace ast;
using namespace lexer;
using namespace parser;

namespace {

using SubExpression = std::variant<TokenIterator, Node::Ptr>;

bool isVariableDeclaration(const TokenIterator &tokenIter, const TokenIterator &tokenEnd) {
    if (tokenIter == tokenEnd || std::next(tokenIter) == tokenEnd || std::next(tokenIter, 2) == tokenEnd)
        return false;

    const Token &varName = *tokenIter;
    const Token &colon = *std::next(tokenIter);
    const Token &varType = *std::next(tokenIter, 2);
    return varName.type == TokenType::Identifier && colon.is(Special::Colon) || TypeRegistry::isTypename(varType);
}

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
    RectBrace,
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

OperationType getOperationType(const Node &node) {
    switch (node.type) {
    case NodeType::BinaryOperation:
        return OperationType::Binary;
    case NodeType::UnaryOperation:
        return OperationType::Unary;
    default:
        return OperationType::Unknown;
    }
}

ExpressionTokenType getExpressionTokenType(const Token &token) {
    if (token.type == TokenType::Identifier || token.type == TokenType::IntegerLiteral ||
        token.type == TokenType::FloatingPointLiteral || token.type == TokenType::StringLiteral ||
        token.is(Keyword::True) || token.is(Keyword::False))
        return ExpressionTokenType::Operand;
    if (token.is(Operator::LeftBrace))
        return ExpressionTokenType::OpeningBrace;
    if (token.is(Operator::RightBrace))
        return ExpressionTokenType::ClosingBrace;
    if (token.is(Operator::RectLeftBrace) || token.is(Operator::RectRightBrace))
        return ExpressionTokenType::RectBrace;
    if (getOperationType(token) != OperationType::Unknown)
        return ExpressionTokenType::Operation;
    return ExpressionTokenType::Unknown;
}

ExpressionTokenType getExpressionTokenType(const SubExpression &subexpr) {
    if (std::holds_alternative<Node::Ptr>(subexpr))
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

UnaryOperation getUnaryOperation(const Token &token) {
    if (token.type == TokenType::Operator) {
        const auto &op = token.op();
        switch (op) {
        case Operator::Sub:
            return UnaryOperation::Negative;
        default:
            return UnaryOperation::Unknown;
        }
    }
    if (token.type == TokenType::Keyword) {
        const auto &kw = token.kw();
        switch (kw) {
        case Keyword::Not:
            return UnaryOperation::Not;
        default:
            return UnaryOperation::Unknown;
        }
    }
    return UnaryOperation::Unknown;
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
    case BinaryOperation::Equal:
    case BinaryOperation::NotEqual:
        return 35;
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
        return 2U;
    if (type == OperationType::Unary)
        return 1U;
    return -1;
}

bool isFunctionCall(const TokenIterator &tokenIter) {
    return tokenIter->type == TokenType::Identifier && std::next(tokenIter)->is(Operator::LeftBrace);
}

bool isListAccessor(const TokenIterator &tokenIter) {
    return tokenIter->type == TokenType::Identifier && std::next(tokenIter)->is(Operator::RectLeftBrace);
}

bool canBeUnaryOperation(const TokenIterator &tokenIter) {
    if (tokenIter->type == TokenType::Operator) {
        return tokenIter->op() == Operator::Sub || tokenIter->op() == Operator::Add;
    }
    if (tokenIter->type == TokenType::Keyword) {
        return tokenIter->kw() == Keyword::Not;
    }
    return false;
}

bool isUnaryOperation(const TokenIterator &tokenIter, const ExpressionTokenType &prevTokenIter) {
    return canBeUnaryOperation(tokenIter) &&
           (prevTokenIter == ExpressionTokenType::Operation || prevTokenIter == ExpressionTokenType::OpeningBrace);
}

std::string unescapeStringLiteral(const std::string &str) {
    std::string result;
    result.reserve(str.size());
    std::string::value_type prev = '\0';
    bool backslash = false;
    for (auto c : str) {
        if (!backslash && prev == '\\') {
            backslash = false;
            if (c == '\\')
                backslash = true;
            else if (c == 'n')
                result.back() = '\n';
            else if (c == 'r')
                result.back() = '\r';
            else if (c == 't')
                result.back() = '\t';
            else
                result.push_back(c);
        } else {
            result.push_back(c);
        }
        prev = c;
    }
    result.shrink_to_fit();
    return result;
}

void buildExpressionSubtree(std::stack<SubExpression> &postfixForm, const Node::Ptr &root, ErrorBuffer &errors) {
    Node::Ptr currNode = root;
    while (!postfixForm.empty()) {
        const SubExpression &subexpr = postfixForm.top();
        if (std::holds_alternative<TokenIterator>(subexpr)) {
            const Token &token = *std::get<TokenIterator>(subexpr);
            ExpressionTokenType expType = getExpressionTokenType(token);
            if (expType == ExpressionTokenType::Operation) {
                OperationType opType = getOperationType(token);
                if (opType == OperationType::Binary) {
                    currNode = ParserContext::unshiftChildNode(currNode, NodeType::BinaryOperation, token.ref);
                    currNode->value = getBinaryOperation(token);
                } else if (opType == OperationType::Unary) {
                    currNode = ParserContext::unshiftChildNode(currNode, NodeType::UnaryOperation, token.ref);
                } else {
                    errors.push<ParserError>(token,
                                             "Unknown operator found in expression, it must be either unary or binary");
                }
            } else if (expType == ExpressionTokenType::Operand) {
                if (token.type == TokenType::Identifier) {
                    Node::Ptr node = ParserContext::unshiftChildNode(currNode, NodeType::VariableName, token.ref);
                    node->value = token.id();
                } else if (token.type == TokenType::IntegerLiteral) {
                    Node::Ptr node =
                        ParserContext::unshiftChildNode(currNode, NodeType::IntegerLiteralValue, token.ref);
                    try {
                        node->value = std::stol(token.literal().c_str());
                    } catch (const std::out_of_range &) {
                        errors.push<ParserError>(token,
                                                 "Failed to convert integer literal. The number is out of range");
                    }
                } else if (token.type == TokenType::FloatingPointLiteral) {
                    Node::Ptr node =
                        ParserContext::unshiftChildNode(currNode, NodeType::FloatingPointLiteralValue, token.ref);
                    try {
                        node->value = std::stod(token.literal());
                    } catch (const std::out_of_range &) {
                        errors.push<ParserError>(token, "Failed to convert float literal. The number is out of range");
                    }
                } else if (token.type == TokenType::StringLiteral) {
                    Node::Ptr node = ParserContext::unshiftChildNode(currNode, NodeType::StringLiteralValue, token.ref);
                    node->value = unescapeStringLiteral(token.literal());
                } else if (token.is(Keyword::False) || token.is(Keyword::True)) {
                    Node::Ptr node =
                        ParserContext::unshiftChildNode(currNode, NodeType::BooleanLiteralValue, token.ref);
                    if (token.is(Keyword::True))
                        node->value = true;
                    else if (token.is(Keyword::False))
                        node->value = false;
                }
            }
        } else {
            // can be FunctionCall node and list ListAccessor
            Node::Ptr callNode = std::get<Node::Ptr>(subexpr);
            assert(callNode->type == NodeType::FunctionCall or callNode->type == NodeType::ListAccessor or
                   callNode->type == NodeType::UnaryOperation);
            callNode->parent = currNode;
            currNode->children.push_front(callNode);
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
    ExpressionTokenType prevExpType = ExpressionTokenType::Operation;
    for (auto tokenIter = tokenIterBegin; tokenIter != tokenIterEnd; tokenIter++) {
        const Token &token = *tokenIter;
        if (isFunctionCall(tokenIter)) {
            Node::Ptr funcCallNode = std::make_shared<Node>(NodeType::FunctionCall);
            auto node = ParserContext::pushChildNode(funcCallNode, NodeType::FunctionName, token.ref);
            node->value = token.id();
            auto argsBegin = std::next(tokenIter);
            auto it = argsBegin;
            unsigned nestingLevel = 0;
            do {
                if (it->is(Operator::RightBrace))
                    nestingLevel--;
                else if (it->is(Operator::LeftBrace))
                    nestingLevel++;
                it++;
            } while (nestingLevel > 0);
            auto argsEnd = std::prev(it);
            if (std::distance(argsBegin, argsEnd) > 1) {
                auto argsNode = ParserContext::pushChildNode(funcCallNode, NodeType::FunctionArguments, token.ref);
                auto argBegin = std::next(argsBegin);
                for (auto argsIter = argBegin; argsIter != std::next(argsEnd); argsIter++) {
                    if (!argsIter->is(Operator::Comma) && argsIter != argsEnd)
                        continue;
                    const Token &token = *argsIter;
                    std::stack<SubExpression> argPostfixForm = generatePostfixForm(argBegin, argsIter, errors);
                    auto exprNode = ParserContext::pushChildNode(argsNode, NodeType::Expression, token.ref);
                    buildExpressionSubtree(argPostfixForm, exprNode, errors);
                    argBegin = std::next(argsIter);
                }
            }
            postfixForm.emplace(funcCallNode);
            tokenIter = argsEnd;
            prevExpType = ExpressionTokenType::Operand;
            continue;
        }
        if (isListAccessor(tokenIter)) {
            Node::Ptr listAccessorNode = std::make_shared<Node>(NodeType::ListAccessor);
            auto node = ParserContext::pushChildNode(listAccessorNode, NodeType::VariableName, token.ref);
            node->value = token.id();
            auto exprBegin = std::next(tokenIter);
            auto it = exprBegin;
            unsigned nestingLevel = 0;
            do {
                if (it->is(Operator::RectRightBrace))
                    nestingLevel--;
                else if (it->is(Operator::RectLeftBrace))
                    nestingLevel++;
                it++;
            } while (nestingLevel > 0);
            std::stack<SubExpression> argPostfixForm = generatePostfixForm(exprBegin, it, errors);
            auto exprNode = ParserContext::pushChildNode(listAccessorNode, NodeType::Expression, token.ref);
            buildExpressionSubtree(argPostfixForm, exprNode, errors);
            postfixForm.emplace(listAccessorNode);
            tokenIter = std::prev(it);
            prevExpType = ExpressionTokenType::Operand;
            continue;
        }
        if (isUnaryOperation(tokenIter, prevExpType)) {
            auto unaryNode = std::make_shared<Node>(NodeType::UnaryOperation);
            unaryNode->value = getUnaryOperation(*tokenIter);
            auto argsBegin = std::next(tokenIter);
            auto it = argsBegin;
            unsigned nestingLevel = 0;
            if (isFunctionCall(it) || isListAccessor(it)) {
                nestingLevel++;
                std::advance(it, 2);
            }
            do {
                if (it->is(Operator::RightBrace) or it->is(Operator::RectRightBrace))
                    nestingLevel--;
                else if (it->is(Operator::LeftBrace) or it->is(Operator::RectLeftBrace))
                    nestingLevel++;
                it++;
            } while (nestingLevel > 0);
            std::stack<SubExpression> argPostfixForm = generatePostfixForm(argsBegin, it, errors);
            auto exprNode = ParserContext::pushChildNode(unaryNode, NodeType::Expression, token.ref);
            buildExpressionSubtree(argPostfixForm, exprNode, errors);
            postfixForm.emplace(unaryNode);
            tokenIter = std::prev(it);
            prevExpType = ExpressionTokenType::Operand;
            continue;
        }
        ExpressionTokenType expType = getExpressionTokenType(token);
        if (expType == ExpressionTokenType::Operand) {
            postfixForm.emplace(tokenIter);
        } else if (expType == ExpressionTokenType::OpeningBrace) {
            operations.emplace(tokenIter);
        } else if (expType == ExpressionTokenType::ClosingBrace) {
            bool foundBrace = false;
            while (!operations.empty()) {
                if (getExpressionTokenType(*operations.top()) != ExpressionTokenType::OpeningBrace) {
                    postfixForm.emplace(operations.top());
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
                operations.emplace(tokenIter);
            } else {
                while (!operations.empty() && getOperationPriority(*operations.top()) <= getOperationPriority(token)) {
                    postfixForm.emplace(operations.top());
                    operations.pop();
                }
                operations.emplace(tokenIter);
            }
        } else if (expType == ExpressionTokenType::RectBrace) {
            continue;
        } else {
            errors.push<ParserError>(token, "Unexpected token inside an expression");
        }
        prevExpType = expType;
    }
    while (!operations.empty()) {
        postfixForm.emplace(operations.top());
        operations.pop();
    }
    return postfixForm;
}

bool isElementaryType(TypeId typeId) {
    return typeId == IntType || typeId == FloatType || typeId == BoolType || typeId == NoneType;
}

void parseType(ParserContext &ctx) {
    ctx.node = ctx.pushChildNode(NodeType::TypeName);
    ctx.node->value = TypeRegistry::typeId(ctx.token());
    if (ctx.token().is(Keyword::List)) {
        const Token &leftBrace = (ctx.goNextToken(), ctx.token());
        const Token &varTypeList = (ctx.goNextToken(), ctx.token());
        auto typeId = TypeRegistry::typeId(varTypeList);
        const Token &rightBrace = (ctx.goNextToken(), ctx.token());
        if (!leftBrace.is(Operator::RectLeftBrace) || !isElementaryType(typeId) ||
            !rightBrace.is(Operator::RectRightBrace)) {
            ctx.pushError("Unexpected syntax for list declaration");
        }
        auto node = ctx.pushChildNode(NodeType::TypeName);
        node->value = typeId;
    }
    ctx.goParentNode();
}

void parseSimpleStatement(ParserContext &ctx) {
    assert(ctx.tokenIter->is(Keyword::Break) || ctx.tokenIter->is(Keyword::Continue) ||
           ctx.tokenIter->is(Keyword::Pass));
    ctx.goNextToken();
    if (!ctx.token().is(Special::EndOfExpression)) {
        ctx.pushError("Unexpected token in a break|continue|pass statement");
        ctx.goNextExpression();
    }
    ctx.goParentNode();
    ctx.goNextToken();
}

void parseBranchRoot(ParserContext &ctx) {
    while (ctx.nestingLevel > 0) {
        if (ctx.tokenIter == ctx.tokenEnd)
            return;
        while (ctx.token().is(Special::EndOfExpression) || ctx.token().is(Special::Colon)) {
            ctx.goNextToken();
            if (ctx.tokenIter == ctx.tokenEnd)
                return;
        }
        int currNestingLevel = 0;
        while (ctx.token().is(Special::Indentation)) {
            currNestingLevel++;
            ctx.goNextToken();
        }
        if (currNestingLevel > ctx.nestingLevel) {
            ctx.pushError("Unexpected indentation mismatch: " + std::to_string(ctx.nestingLevel) +
                          " indentation(s) expected, " + std::to_string(currNestingLevel) + " indentation(s) given");
        } else if (currNestingLevel < ctx.nestingLevel) {
            ctx.goParentNode();
            while (ctx.node->type != NodeType::BranchRoot) {
                if (!ctx.node->parent)
                    break;
                ctx.goParentNode();
            }
            ctx.nestingLevel--;
            std::advance(ctx.tokenIter, -currNestingLevel);
            return;
        }

        const Token &currToken = ctx.token();
        if (currToken.is(Keyword::If)) {
            ctx.node = ctx.pushChildNode(NodeType::IfStatement);
        } else if (currToken.is(Keyword::While)) {
            ctx.node = ctx.pushChildNode(NodeType::WhileStatement);
        } else if (currToken.is(Keyword::For)) {
            ctx.node = ctx.pushChildNode(NodeType::ForStatement);
        } else if (isVariableDeclaration(ctx.tokenIter, ctx.tokenEnd)) {
            ctx.node = ctx.pushChildNode(NodeType::VariableDeclaration);
        } else if (currToken.is(Keyword::Elif) || currToken.is(Keyword::Else)) {
            auto lastNode = ctx.node->children.back();
            if (lastNode->type == NodeType::IfStatement) {
                auto nodeType = currToken.is(Keyword::Elif) ? NodeType::ElifStatement : NodeType::ElseStatement;
                ctx.node = ParserContext::pushChildNode(lastNode, nodeType, currToken.ref);
            } else {
                ctx.pushError((currToken.is(Keyword::Elif) ? std::string("elif") : std::string("else")) +
                              " is not allowed here");
            }
        } else if (currToken.is(Keyword::Return)) {
            ctx.node = ctx.pushChildNode(NodeType::ReturnStatement);
        } else if (currToken.is(Keyword::Continue)) {
            ctx.node = ctx.pushChildNode(NodeType::ContinueStatement);
            parseSimpleStatement(ctx);
            continue;
        } else if (currToken.is(Keyword::Break)) {
            ctx.node = ctx.pushChildNode(NodeType::BreakStatement);
            parseSimpleStatement(ctx);
            continue;
        } else if (currToken.is(Keyword::Pass)) {
            ctx.node = ctx.pushChildNode(NodeType::PassStatement);
            parseSimpleStatement(ctx);
            continue;
        } else {
            ctx.node = ctx.pushChildNode(NodeType::Expression);
        }
        ctx.propagate();
    }
}

void parseElifStatement(ParserContext &ctx) {
    assert(ctx.tokenIter->is(Keyword::Elif));
    ctx.goNextToken();
    ctx.node = ctx.pushChildNode(NodeType::Expression);
    ctx.propagate();
    if (!ctx.token().is(Special::Colon)) {
        ctx.pushError("Colon expected here");
        ctx.goNextExpression();
    }
    ctx.node = ctx.pushChildNode(NodeType::BranchRoot);
    ctx.nestingLevel++;
    ctx.propagate();
}

void parseElseStatement(ParserContext &ctx) {
    assert(ctx.tokenIter->is(Keyword::Else));
    ctx.goNextToken();
    if (!ctx.token().is(Special::Colon)) {
        ctx.pushError("Colon expected here");
        ctx.goNextExpression();
    }
    ctx.node = ctx.pushChildNode(NodeType::BranchRoot);
    ctx.nestingLevel++;
    ctx.propagate();
}

void parseExpression(ParserContext &ctx) {
    auto it = ctx.tokenIter;
    while (!it->is(Special::Colon) && !it->is(Special::EndOfExpression))
        it++;
    const auto &tokenIterBegin = ctx.tokenIter;
    const auto &tokenIterEnd = it;
    std::stack<SubExpression> postfixForm = generatePostfixForm(tokenIterBegin, tokenIterEnd, ctx.errors);
    buildExpressionSubtree(postfixForm, ctx.node, ctx.errors);
    ctx.tokenIter = tokenIterEnd;
    ctx.goParentNode();
}

void parseFunctionArguments(ParserContext &ctx) {
    assert(ctx.token().is(Operator::LeftBrace));
    ctx.goNextToken();
    while (!ctx.token().is(Operator::RightBrace)) {
        const Token &argName = ctx.token();
        const Token &colon = (ctx.goNextToken(), ctx.token());
        const Token &argType = (ctx.goNextToken(), ctx.token());
        if (argName.type != TokenType::Identifier || !colon.is(Special::Colon) || !TypeRegistry::isTypename(argType)) {
            ctx.pushError("Function argument declaration is ill-formed");
            while (!ctx.token().is(Operator::RightBrace) && !ctx.token().is(Special::Colon))
                ctx.goNextToken();
            break;
        }
        ctx.node = ctx.pushChildNode(NodeType::FunctionArgument);
        parseType(ctx);
        auto argNameNode = ParserContext::pushChildNode(ctx.node, NodeType::VariableName, argName.ref);
        argNameNode->value = argName.id();
        ctx.goParentNode();
        ctx.goNextToken();
        if (ctx.token().is(Operator::Comma))
            ctx.goNextToken();
    }
    ctx.goParentNode();
    ctx.goNextToken();
}

void parseFunctionDefinition(ParserContext &ctx) {
    assert(ctx.tokenIter->is(Keyword::Definition));
    ctx.goNextToken();
    if (ctx.token().type != TokenType::Identifier) {
        ctx.pushError("Given token is not allowed here in function definition");
    }
    ctx.pushChildNode(NodeType::FunctionName)->value = ctx.token().id();
    ctx.goNextToken();
    if (!ctx.token().is(Operator::LeftBrace)) {
        ctx.pushError("Given token is not allowed here in function definition");
    }
    ctx.node = ctx.pushChildNode(NodeType::FunctionArguments);
    ctx.propagate();
    if (!ctx.token().is(Special::Arrow)) {
        ctx.pushError("Function return type is mandatory in its header");
    }
    ctx.goNextToken();
    if (!TypeRegistry::isTypename(ctx.token())) {
        ctx.pushError("Type name not found");
    }
    auto retTypeId = TypeRegistry::typeId(ctx.token());
    if (!isElementaryType(retTypeId))
        ctx.pushError("Function return type must be one of the following: int, float, bool, None");
    ctx.pushChildNode(NodeType::FunctionReturnType)->value = retTypeId;
    ctx.goNextToken();
    if (!ctx.token().is(Special::Colon)) {
        ctx.pushError("Colon expected at the end of function header");
    }
    ctx.goNextToken();
    ctx.node = ctx.pushChildNode(NodeType::BranchRoot);
    ctx.nestingLevel = 1;
    ctx.propagate();
}

void parseIfStatement(ParserContext &ctx) {
    assert(ctx.tokenIter->is(Keyword::If));
    ctx.goNextToken();
    ctx.node = ctx.pushChildNode(NodeType::Expression);
    ctx.propagate();
    if (!ctx.token().is(Special::Colon)) {
        ctx.pushError("Colon expected here");
        ctx.goNextExpression();
    }
    ctx.node = ctx.pushChildNode(NodeType::BranchRoot);
    ctx.nestingLevel++;
    ctx.propagate();
}

void parseProgramRoot(ParserContext &ctx) {
    while (ctx.tokenIter != ctx.tokenEnd) {
        if (ctx.token().is(Keyword::Definition)) {
            ctx.node = ctx.pushChildNode(NodeType::FunctionDefinition);
            ctx.propagate();
        } else {
            ctx.pushError("Function definition was expected");
            return;
        }
    }
}

void parseReturnStatement(ParserContext &ctx) {
    assert(ctx.tokenIter->is(Keyword::Return));
    ctx.goNextToken();
    if (ctx.token().is(Special::EndOfExpression)) {
        ctx.goParentNode();
        ctx.goNextToken();
        return;
    }
    const Token &currToken = ctx.token();
    if (currToken.type != TokenType::FloatingPointLiteral && currToken.type != TokenType::Identifier &&
        currToken.type != TokenType::IntegerLiteral && currToken.type != TokenType::StringLiteral &&
        !currToken.is(Operator::LeftBrace)) {
        ctx.pushError("Expression as function return value was expected");
        ctx.goNextExpression();
        return;
    }
    ctx.node = ctx.pushChildNode(NodeType::Expression);
    ctx.propagate();
    ctx.goParentNode();
}

void parseVariableDeclaration(ParserContext &ctx) {
    const Token &varName = ctx.token();
    ctx.goNextToken();
    const Token &varType = (ctx.goNextToken(), ctx.token());
    parseType(ctx);
    auto node = ctx.pushChildNode(NodeType::VariableName);
    node->value = varName.id();

    auto endOfDecl = std::next(ctx.tokenIter);
    if (endOfDecl->is(Special::EndOfExpression)) {
        // declaration without definition
        std::advance(ctx.tokenIter, 2);
        ctx.goParentNode();
    } else if (endOfDecl->is(Operator::Assign)) {
        // declaration with definition
        ctx.node = ctx.pushChildNode(NodeType::Expression);
        if (varType.is(Keyword::List)) {
            ctx.node = ctx.pushChildNode(NodeType::ListStatement);
        }
        std::advance(ctx.tokenIter, 2);
        ctx.propagate();
        ctx.goParentNode();
    } else {
        ctx.errors.push<ParserError>(*endOfDecl, "Definition expression or line break was expected");
    }
}

void parseWhileStatement(ParserContext &ctx) {
    assert(ctx.tokenIter->is(Keyword::While));
    ctx.goNextToken();
    ctx.node = ctx.pushChildNode(NodeType::Expression);
    ctx.propagate();
    if (!ctx.token().is(Special::Colon)) {
        ctx.pushError("Colon expected here");
        ctx.goNextExpression();
    }
    ctx.node = ctx.pushChildNode(NodeType::BranchRoot);
    ctx.nestingLevel++;
    ctx.propagate();
}

void parseForStatement(ParserContext &ctx) {
    assert(ctx.tokenIter->is(Keyword::For));
    ctx.goNextToken();
    auto forNode = ctx.node;
    auto it = ctx.tokenIter;
    auto forTargets = ParserContext::pushChildNode(forNode, NodeType::ForTargets, ctx.tokenIter->ref);
    while (!it->is(Keyword::In) && !it->is(Special::EndOfExpression)) {
        if (it->type == TokenType::Identifier) {
            auto targetNode = ParserContext::pushChildNode(forTargets, NodeType::VariableName, ctx.tokenIter->ref);
            targetNode->value = it->id();
            it++;
        } else if (it->is(Operator::Comma)) {
            it++;
        } else {
            ctx.pushError("Unexpected token in a for statement");
        }
    }
    ctx.tokenIter = it;
    ctx.goNextToken();
    ctx.node = ctx.pushChildNode(NodeType::ForIterable);
    ctx.node = ctx.pushChildNode(NodeType::Expression);
    ctx.propagate();
    ctx.goParentNode();
    if (!ctx.token().is(Special::Colon)) {
        ctx.pushError("Colon expected here");
        ctx.goNextExpression();
    }
    ctx.node = ctx.pushChildNode(NodeType::BranchRoot);
    ctx.nestingLevel++;
    ctx.propagate();
}

void parseListStatement(ParserContext &ctx) {
    assert(ctx.tokenIter->is(Operator::RectLeftBrace));
    while (!ctx.token().is(Operator::RectRightBrace)) {
        ctx.goNextToken();
        auto it = ctx.tokenIter;
        while (!it->is(Operator::Comma) && !it->is(Operator::RectRightBrace))
            it++;
        const auto &tokenIterBegin = ctx.tokenIter;
        const auto &tokenIterEnd = it;
        if (tokenIterEnd->is(Special::EndOfExpression)) {
            ctx.errors.push<ParserError>(*tokenIterEnd, "']' was expected");
        }
        ctx.node = ctx.pushChildNode(NodeType::Expression);
        std::stack<SubExpression> postfixForm = generatePostfixForm(tokenIterBegin, tokenIterEnd, ctx.errors);
        buildExpressionSubtree(postfixForm, ctx.node, ctx.errors);
        ctx.tokenIter = tokenIterEnd;
        ctx.goParentNode();
    }
    ctx.goNextToken();
    if (ctx.token().is(Special::EndOfExpression)) {
        ctx.goParentNode();
    } else if (ctx.token().is(Operator::Mult)) {
        ctx.goNextToken();
        ctx.node = ctx.pushChildNode(NodeType::ListDynamicSize);
        ctx.node = ctx.pushChildNode(NodeType::Expression);
        ctx.propagate();
        ctx.goParentNode();
        ctx.goParentNode();
    } else {
        ctx.pushError("Either end of line or '*' was expected");
    }
    ctx.goParentNode();
}

// clang-format off
#define SUBPARSER(NodeTypeVal) {NodeType::NodeTypeVal, parse##NodeTypeVal}

static std::unordered_map<NodeType, std::function<void(ParserContext &)>> subparsers = {
    SUBPARSER(BranchRoot),
    SUBPARSER(ElifStatement),
    SUBPARSER(ElseStatement),
    SUBPARSER(Expression),
    SUBPARSER(FunctionArguments),
    SUBPARSER(FunctionDefinition),
    SUBPARSER(IfStatement),
    SUBPARSER(ProgramRoot),
    SUBPARSER(ReturnStatement),
    SUBPARSER(VariableDeclaration),
    SUBPARSER(WhileStatement),
    SUBPARSER(ListStatement),
    SUBPARSER(ForStatement),
};
// clang-format on

} // namespace

SyntaxTree Parser::process(const TokenList &tokens) {
    SyntaxTree tree;
    tree.root = std::make_shared<Node>(NodeType::ProgramRoot);

    ParserContext ctx = {subparsers, tree.root, tokens.begin(), tokens.end(), 0};
    ctx.propagate();
    if (!ctx.errors.empty()) {
        throw ctx.errors;
    }

    return tree;
}
