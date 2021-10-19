#include "parser/type_registry.hpp"

using namespace parser;

std::unordered_set<std::string> TypeRegistry::userDefinedTypes = {};

bool TypeRegistry::isTypename(const Token &token) {
    return token.is(Token::Keyword::Int) || token.is(Token::Keyword::Float) || token.is(Token::Keyword::String) ||
           token.is(Token::Keyword::None) ||
           (token.type == Token::Type::Identifier && userDefinedTypes.find(token.id()) != userDefinedTypes.end());
}
