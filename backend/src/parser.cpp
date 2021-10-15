#include "parser.hpp"

namespace {

ASTNode::Ptr pushChildNode(ASTNode::Ptr &node, const ASTNode::Type &nodeType) {
    node->children.push_back(std::make_shared<ASTNode>(ASTNode::Type::FunctionArguments, node));
    return node->children.back();
}

} // namespace

std::unordered_set<std::string> Parser::userDefinedTypes = {};

template <>
int Parser::parseLiteral(const Token &token) {
    return atoi(token.literal().c_str());
}

template <>
long Parser::parseLiteral(const Token &token) {
    return atol(token.literal().c_str());
}

template <>
double Parser::parseLiteral(const Token &token) {
    return atof(token.literal().c_str());
}

template <>
std::string_view Parser::parseLiteral(const Token &token) {
    return token.literal();
}

template <>
std::string Parser::parseLiteral(const Token &token) {
    return token.literal();
}

bool Parser::isTypename(const Token &token) {
    return token.is(Token::Keyword::Int) || token.is(Token::Keyword::Float) || token.is(Token::Keyword::String) ||
           token.is(Token::Keyword::None) ||
           (token.type == Token::Type::Identifier && userDefinedTypes.find(token.id()) != userDefinedTypes.end());
}

AST Parser::process(const TokenList &tokens) {
    using NodeType = ASTNode::Type;
    using TokenType = Token::Type;
    AST tree;
    tree.root = std::make_shared<ASTNode>(NodeType::ProgramRoot);
    ASTNode::Ptr currNode = tree.root;
    Token prevToken;
    bool functionBegin = false;
    bool functionArgumentsEnd = false;
    int nestingLevel = 0;
    for (const Token &currToken : tokens) {
        if (currNode->type == NodeType::ProgramRoot) {
            // here can be only function definition
            if (currToken.is(Token::Keyword::Definition)) {
                currNode = pushChildNode(currNode, NodeType::FunctionDefinition);
            } else {
                // semantic error
            }
            prevToken = currToken;
            continue;
        }
        if (currNode->type == NodeType::FunctionDefinition) {
            // in function definition
            if (currToken.type == TokenType::Identifier && prevToken.is(Token::Keyword::Definition)) {
                // save function name
                currNode->strLiteral = currToken.id();
            } else if (currToken.is(Token::Operator::LeftBrace) && prevToken.type == TokenType::Identifier) {
                // start analyzing arguments
                currNode = pushChildNode(currNode, NodeType::FunctionArguments);
            } else if (currToken.is(Token::Operator::Arrow) && functionArgumentsEnd) {
                // save return type on next step
            } else if (isTypename(currToken) && prevToken.is(Token::Operator::Arrow)) {
                pushChildNode(currNode, NodeType::FunctionReturnType);
                // TODO: really save typename!
            } else if (currToken.is(Token::Operator::Colon) && isTypename(prevToken)) {
                // end of function header
                currNode = pushChildNode(currNode, NodeType::FunctionBody);
                functionBegin = true;
            } else {
                // semantic error
            }
            prevToken = currToken;
            continue;
        }
        if (currNode->type == NodeType::FunctionArguments) {
            functionArgumentsEnd = false;
            // during reading function header, we met arguments list (possibly empty)
            if (currToken.is(Token::Operator::RightBrace)) {
                // end of arguments list, return back to function root
                currNode = currNode->parent;
                functionArgumentsEnd = true;
            } else {
                // semantic error
            }
            prevToken = currToken;
            continue;
        }
        if (currNode->type == NodeType::FunctionBody) {
            if (prevToken.is(Token::Special::Indentation)) {
                nestingLevel++;
            }
            if (currToken.is(Token::Special::Indentation)) {
                prevToken = currToken;
                continue;
            }
            if (nestingLevel > 1) {
                // syntax error: extra indentations
                continue;
            }
            if (currToken.is(Token::Keyword::If) && prevToken.is(Token::Special::EndOfExpression)) {
                // start of conditional expression
                currNode = pushChildNode(currNode, NodeType::IfExpression);
            } else if (currToken.is(Token::Keyword::While) && prevToken.is(Token::Special::EndOfExpression)) {
                // start of cycle expression
                currNode = pushChildNode(currNode, NodeType::WhileExpression);
            } else {
                // try to parse arithmetic/logic expression using postfix form
            }
            // TODO: add range-based for
            // TODO: add errors handling
            prevToken = currToken;
            continue;
        }
    }
    return tree;
}
