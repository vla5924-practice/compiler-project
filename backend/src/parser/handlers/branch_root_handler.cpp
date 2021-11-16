#include "parser/handlers/branch_root_handler.hpp"

#include "lexer/token_types.hpp"
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
    int currNestingLevel = 0;
    while (state.token().is(Special::EndOfExpression)) {
        state.goNextToken();
        if (state.tokenIter == state.tokenEnd)
            return;
    }
    while (state.token().is(Special::Indentation)) {
        currNestingLevel++;
        state.goNextToken();
    }
    if (nestingLevel - currNestingLevel == 1) {
        state.node = state.node->parent;
        state.goNextToken();
        return;
    } else if (currNestingLevel != nestingLevel) {
        // syntax error
    }

    const Token &currToken = state.token();
    const Token &prevToken = *std::prev(state.tokenIter);

    if (currToken.is(Keyword::If)) {
        state.node = state.pushChildNode(ast::NodeType::IfStatement);
    } else if (currToken.is(Keyword::While)) {
        state.node = state.pushChildNode(ast::NodeType::WhileStatement);
    } else if (isVariableDeclaration(state.tokenIter, state.tokenEnd)) {
        state.node = state.pushChildNode(ast::NodeType::VariableDeclaration);
    } else if (currToken.is(Keyword::Elif) || currToken.is(Keyword::Else)) {
        // syntax error
    } else {
        state.node = state.pushChildNode(ast::NodeType::Expression);
    }
    // TODO: add range-based for
    // TODO: add errors handling
    state.goNextToken();
}

void BranchRootHandler::reset() {
    nestingLevel = 0;
}

REGISTER_PARSING_HANDLER(BranchRootHandler, ast::NodeType::BranchRoot);
