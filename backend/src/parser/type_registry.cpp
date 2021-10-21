#include "parser/type_registry.hpp"

using namespace parser;

std::map<std::string, size_t> TypeRegistry::userDefinedTypes = {};

bool TypeRegistry::isTypename(const Token &token) {
    return token.is(Token::Keyword::Int) || token.is(Token::Keyword::Float) || token.is(Token::Keyword::String) ||
           token.is(Token::Keyword::None) ||
           (token.type == Token::Type::Identifier && userDefinedTypes.find(token.id()) != userDefinedTypes.end());
}

size_t TypeRegistry::typeId(const Token &token) {
    if (token.type == Token::Type::Identifier) {
        const std::string &id = token.id();
        auto it = userDefinedTypes.find(id);
        return it != userDefinedTypes.end() ? it->second : BuiltInTypes::UnknownType;
    }
    if (token.is(Token::Keyword::Int))
        return BuiltInTypes::IntType;
    if (token.is(Token::Keyword::Float))
        return BuiltInTypes::FloatType;
    if (token.is(Token::Keyword::String))
        return BuiltInTypes::StrType;
    if (token.is(Token::Keyword::None))
        return BuiltInTypes::NoneType;
    return BuiltInTypes::UnknownType;
}
