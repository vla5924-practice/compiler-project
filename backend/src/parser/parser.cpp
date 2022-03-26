#include "parser/parser.hpp"

#include <cassert>
#include <functional>
#include <unordered_map>

#include "parser/parser_context.hpp"
#include "parser/parser_error.hpp"
#include "parser/type_registry.hpp"

using namespace ast;
using namespace lexer;
using namespace parser;

namespace {

using TokenIterator = TokenList::const_iterator;
using SubExpression = std::variant<TokenIterator, ast::Node::Ptr>;

bool isVariableDeclaration(const TokenList::const_iterator &tokenIter, const TokenList::const_iterator &tokenEnd) {
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
                    currNode = ParserContext::unshiftChildNode(currNode, ast::NodeType::BinaryOperation);
                    currNode->value = getBinaryOperation(token);
                } else if (opType == OperationType::Unary) {
                    currNode = ParserContext::unshiftChildNode(currNode, ast::NodeType::UnaryOperation);
                } else {
                    errors.push<ParserError>(token,
                                             "Unknown operator found in expression, it must be either unary or binary");
                }
            } else if (expType == ExpressionTokenType::Operand) {
                if (token.type == TokenType::Identifier) {
                    ast::Node::Ptr node = ParserContext::unshiftChildNode(currNode, ast::NodeType::VariableName);
                    node->value = token.id();
                } else if (token.type == TokenType::IntegerLiteral) {
                    ast::Node::Ptr node = ParserContext::unshiftChildNode(currNode, ast::NodeType::IntegerLiteralValue);
                    node->value = std::atol(token.literal().c_str());
                } else if (token.type == TokenType::FloatingPointLiteral) {
                    ast::Node::Ptr node =
                        ParserContext::unshiftChildNode(currNode, ast::NodeType::FloatingPointLiteralValue);
                    node->value = std::stod(token.literal());
                } else if (token.type == TokenType::StringLiteral) {
                    ast::Node::Ptr node = ParserContext::unshiftChildNode(currNode, ast::NodeType::StringLiteralValue);
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
            auto node = ParserContext::pushChildNode(funcCallNode, ast::NodeType::FunctionName);
            node->value = token.id();
            auto argsBegin = std::next(tokenIter);
            auto it = argsBegin;
            while (!it->is(Operator::RightBrace))
                it++;
            auto argsEnd = it;
            if (std::distance(argsBegin, argsEnd) > 1) {
                auto argsNode = ParserContext::pushChildNode(funcCallNode, ast::NodeType::FunctionArguments);
                auto argBegin = std::next(argsBegin);
                for (auto argsIter = argBegin; argsIter != std::next(argsEnd); argsIter++) {
                    if (!argsIter->is(Operator::Comma) && argsIter != argsEnd)
                        continue;
                    const Token &token = *argsIter;
                    std::stack<SubExpression> argPostfixForm = generatePostfixForm(argBegin, argsIter, errors);
                    auto exprNode = ParserContext::pushChildNode(argsNode, ast::NodeType::Expression);
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

static void parseBranchRoot(ParserContext &ctx) {
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
            while (ctx.node->type != ast::NodeType::BranchRoot) {
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
            ctx.node = ctx.pushChildNode(ast::NodeType::IfStatement);
        } else if (currToken.is(Keyword::While)) {
            ctx.node = ctx.pushChildNode(ast::NodeType::WhileStatement);
        } else if (isVariableDeclaration(ctx.tokenIter, ctx.tokenEnd)) {
            ctx.node = ctx.pushChildNode(ast::NodeType::VariableDeclaration);
        } else if (currToken.is(Keyword::Elif) || currToken.is(Keyword::Else)) {
            auto lastNode = ctx.node->children.back();
            if (lastNode->type == ast::NodeType::IfStatement) {
                auto nodeType =
                    currToken.is(Keyword::Elif) ? ast::NodeType::ElifStatement : ast::NodeType::ElseStatement;
                ctx.node = ParserContext::pushChildNode(lastNode, nodeType);
            } else {
                ctx.pushError((currToken.is(Keyword::Elif) ? std::string("elif") : std::string("else")) +
                              " is not allowed here");
            }
        } else if (currToken.is(Keyword::Return)) {
            ctx.node = ctx.pushChildNode(ast::NodeType::ReturnStatement);
        } else {
            ctx.node = ctx.pushChildNode(ast::NodeType::Expression);
        }
        ctx.propagate();
    }
}

static void parseElifStatement(ParserContext &ctx) {
    assert(ctx.tokenIter->is(Keyword::Elif));
    ctx.goNextToken();
    ctx.node = ctx.pushChildNode(ast::NodeType::Expression);
    ctx.propagate();
    if (!ctx.token().is(Special::Colon)) {
        ctx.pushError("Colon expected here");
        ctx.goNextExpression();
    }
    ctx.node = ctx.pushChildNode(ast::NodeType::BranchRoot);
    ctx.nestingLevel++;
    ctx.propagate();
}

static void parseElseStatement(ParserContext &ctx) {
    assert(ctx.tokenIter->is(Keyword::Else));
    ctx.goNextToken();
    if (!ctx.token().is(Special::Colon)) {
        ctx.pushError("Colon expected here");
        ctx.goNextExpression();
    }
    ctx.node = ctx.pushChildNode(ast::NodeType::BranchRoot);
    ctx.nestingLevel++;
    ctx.propagate();
}

static void parseExpression(ParserContext &ctx) {
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

static void parseFunctionArguments(ParserContext &ctx) {
    assert(ctx.token().is(Operator::LeftBrace));
    ctx.goNextToken();
    while (!ctx.token().is(Operator::RightBrace)) {
        const Token &argName = *ctx.tokenIter;
        const Token &colon = *std::next(ctx.tokenIter);
        const Token &argType = *std::next(ctx.tokenIter, 2);
        if (argName.type != TokenType::Identifier || !colon.is(Special::Colon) || !TypeRegistry::isTypename(argType)) {
            ctx.pushError("Function argument declaration is ill-formed");
            while (!ctx.token().is(Operator::RightBrace) && !ctx.token().is(Special::Colon))
                ctx.goNextToken();
            break;
        }
        auto node = ctx.pushChildNode(ast::NodeType::FunctionArgument);
        auto argTypeNode = ParserContext::pushChildNode(node, ast::NodeType::TypeName);
        argTypeNode->value = TypeRegistry::typeId(argType);
        auto argNameNode = ParserContext::pushChildNode(node, ast::NodeType::VariableName);
        argNameNode->value = argName.id();

        const Token &last = *std::next(ctx.tokenIter, 3);
        if (last.is(Operator::Comma))
            std::advance(ctx.tokenIter, 4);
        else
            std::advance(ctx.tokenIter, 3);
    }
    ctx.goParentNode();
    ctx.goNextToken();
}

static void parseFunctionDefinition(ParserContext &ctx) {
    assert(ctx.tokenIter->is(Keyword::Definition));
    ctx.goNextToken();
    if (ctx.token().type != TokenType::Identifier) {
        ctx.pushError("Given token is not allowed here in function definition");
    }
    ctx.pushChildNode(ast::NodeType::FunctionName)->value = ctx.token().id();
    ctx.goNextToken();
    if (!ctx.token().is(Operator::LeftBrace)) {
        ctx.pushError("Given token is not allowed here in function definition");
    }
    ctx.node = ctx.pushChildNode(ast::NodeType::FunctionArguments);
    ctx.propagate();
    if (!ctx.token().is(Special::Arrow)) {
        ctx.pushError("Function return type is mandatory in its header");
    }
    ctx.goNextToken();
    if (!TypeRegistry::isTypename(ctx.token())) {
        ctx.pushError("Type name not found");
    }
    ctx.pushChildNode(ast::NodeType::FunctionReturnType)->value = TypeRegistry::typeId(ctx.token());
    ctx.goNextToken();
    if (!ctx.token().is(Special::Colon)) {
        ctx.pushError("Colon expected at the end of function header");
    }
    ctx.goNextToken();
    ctx.node = ctx.pushChildNode(ast::NodeType::BranchRoot);
    ctx.nestingLevel = 1;
    ctx.propagate();
}

static void parseIfStatement(ParserContext &ctx) {
    assert(ctx.tokenIter->is(Keyword::If));
    ctx.goNextToken();
    ctx.node = ctx.pushChildNode(ast::NodeType::Expression);
    ctx.propagate();
    if (!ctx.token().is(Special::Colon)) {
        ctx.pushError("Colon expected here");
        ctx.goNextExpression();
    }
    ctx.node = ctx.pushChildNode(ast::NodeType::BranchRoot);
    ctx.nestingLevel++;
    ctx.propagate();
}

static void parseProgramRoot(ParserContext &ctx) {
    while (ctx.tokenIter != ctx.tokenEnd) {
        if (ctx.token().is(Keyword::Definition)) {
            ctx.node = ctx.pushChildNode(ast::NodeType::FunctionDefinition);
            ctx.propagate();
        } else {
            ctx.pushError("Function definition was expected");
        }
    }
}

static void parseReturnStatement(ParserContext &ctx) {
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
    ctx.node = ctx.pushChildNode(ast::NodeType::Expression);
    ctx.propagate();
}

static void parseVariableDeclaration(ParserContext &ctx) {
    ctx.goNextToken();
    const Token &colon = ctx.token();
    const Token &varName = *std::prev(ctx.tokenIter);
    const Token &varType = *std::next(ctx.tokenIter);

    auto node = ctx.pushChildNode(ast::NodeType::TypeName);
    node->value = TypeRegistry::typeId(varType);
    node = ctx.pushChildNode(ast::NodeType::VariableName);
    node->value = varName.id();

    auto endOfDecl = std::next(ctx.tokenIter, 2);
    if (endOfDecl->is(Special::EndOfExpression)) {
        // declaration without definition
        std::advance(ctx.tokenIter, 3);
        ctx.goParentNode();
    } else if (endOfDecl->is(Operator::Assign)) {
        // declaration with definition
        ctx.node = ctx.pushChildNode(ast::NodeType::Expression);
        std::advance(ctx.tokenIter, 3);
        ctx.propagate();
        ctx.goParentNode();
    } else {
        ctx.errors.push<ParserError>(*endOfDecl, "Definition expression or line break was expected");
    }
}

static void parseWhileStatement(ParserContext &ctx) {
    assert(ctx.tokenIter->is(Keyword::While));
    ctx.goNextToken();
    ctx.node = ctx.pushChildNode(ast::NodeType::Expression);
    ctx.propagate();
    if (!ctx.token().is(Special::Colon)) {
        ctx.pushError("Colon expected here");
        ctx.goNextExpression();
    }
    ctx.node = ctx.pushChildNode(ast::NodeType::BranchRoot);
    ctx.nestingLevel++;
    ctx.propagate();
}

// clang-format off
#define SUBPARSER(NodeTypeVal) {ast::NodeType::NodeTypeVal, parse##NodeTypeVal}

static std::unordered_map<ast::NodeType, std::function<void(ParserContext &)>> subparsers = {
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
};
// clang-format on

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
