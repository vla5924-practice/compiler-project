#include "parser/type_registry.hpp"

using namespace lexer;
using namespace parser;

std::map<std::string, size_t> TypeRegistry::userDefinedTypes = {};

bool TypeRegistry::isTypename(const Token &token) {
    return token.is(Keyword::Int) || token.is(Keyword::Float) || token.is(Keyword::Str) || token.is(Keyword::None) ||
           (token.type == TokenType::Identifier && userDefinedTypes.find(token.id()) != userDefinedTypes.end());
}

size_t TypeRegistry::typeId(const Token &token) {
    if (token.type == TokenType::Identifier) {
        const std::string &id = token.id();
        auto it = userDefinedTypes.find(id);
        return it != userDefinedTypes.end() ? it->second : BuiltInTypes::UnknownType;
    }
    if (token.is(Keyword::Int))
        return BuiltInTypes::IntType;
    if (token.is(Keyword::Float))
        return BuiltInTypes::FloatType;
    if (token.is(Keyword::Str))
        return BuiltInTypes::StrType;
    if (token.is(Keyword::None))
        return BuiltInTypes::NoneType;
    return BuiltInTypes::UnknownType;
}
