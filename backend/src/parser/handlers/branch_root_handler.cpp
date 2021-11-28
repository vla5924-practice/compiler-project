#include "parser/handlers/branch_root_handler.hpp"

#include "lexer/token_types.hpp"
#include "parser/parser_error.hpp"
#include "parser/register_handler.hpp"
#include "parser/type_registry.hpp"

using namespace lexer;
using namespace parser;

namespace {

bool isVariableDeclaration(const TokenList::const_iterator &tokenIter, const TokenList::const_iterator &tokenEnd) {
    if (tokenIter == tokenEnd || std::next(tokenIter) == tokenEnd || std::next(tokenIter, 2) == tokenEnd)
        return false;

    const Token &varName = *tokenIter;
    const Token &colon = *std::next(tokenIter);
    const Token &varType = *std::next(tokenIter, 2);
    return varName.type == TokenType::Identifier && colon.is(Special::Colon) || TypeRegistry::isTypename(varType);
}

} // namespace

void BranchRootHandler::run(ParserState &state) {
    while (state.token().is(Special::EndOfExpression) || state.token().is(Special::Colon)) {
        state.goNextToken();
        if (state.tokenIter == state.tokenEnd)
            return;
    }
    if (state.token().is(Special::Indentation)) {
        int currNestingLevel = 0;
        while (state.token().is(Special::Indentation)) {
            currNestingLevel++;
            state.goNextToken();
        }
        if (nestingLevel - currNestingLevel == 1) {
            nestingLevel--;
            state.node = state.node->parent;
            if (state.token().is(Keyword::Else) || state.token().is(Keyword::Elif))
                waitForNesting = true;
            return;
        } else if (waitForNesting && currNestingLevel - nestingLevel == 1) {
            nestingLevel++;
            waitForNesting = false;
        } else if (nestingLevel == currNestingLevel) {
            // it's ok
        } else {
            state.errors.push<ParserError>(state.token(),
                                           "Unexpected indentation mismatch: " + std::to_string(nestingLevel) +
                                               " indentation(s) expected, " + std::to_string(currNestingLevel) +
                                               " indentation(s) given");
        }
    }

    const Token &currToken = state.token();
    const Token &prevToken = *std::prev(state.tokenIter);

    if (currToken.is(Keyword::If)) {
        waitForNesting = true;
        wasInIfStatement++;
        state.node = state.pushChildNode(ast::NodeType::IfStatement);
        state.goNextToken();
        return;
    }
    if (currToken.is(Keyword::While)) {
        waitForNesting = true;
        state.node = state.pushChildNode(ast::NodeType::WhileStatement);
        state.goNextToken();
        return;
    }
    if (isVariableDeclaration(state.tokenIter, state.tokenEnd)) {
        state.node = state.pushChildNode(ast::NodeType::VariableDeclaration);
        state.goNextToken();
        return;
    }
    if (currToken.is(Keyword::Elif) || currToken.is(Keyword::Else)) {
        if (wasInIfStatement) {
            wasInIfStatement--;
            state.node = state.node->parent;
        } else {
            state.errors.push<ParserError>(currToken,
                                           (currToken.is(Keyword::Elif) ? std::string("elif") : std::string("else")) +
                                               " is not allowed here");
        }
        return;
    }
    state.node = state.pushChildNode(ast::NodeType::Expression);
}

void BranchRootHandler::reset() {
    nestingLevel = 1;
    waitForNesting = false;
    wasInIfStatement = 0;
}

REGISTER_PARSING_HANDLER(BranchRootHandler, ast::NodeType::BranchRoot);
