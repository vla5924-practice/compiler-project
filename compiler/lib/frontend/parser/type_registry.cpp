#include "compiler/ast/types.hpp"
#include "lexer/token_types.hpp"
#include "parser/type_registry.hpp"

using ast::TypeId;
using namespace lexer;
using namespace parser;

std::map<std::string, TypeId> TypeRegistry::userDefinedTypes = {};

bool TypeRegistry::isTypename(const Token &token) {
    return token.is(Keyword::Int) || token.is(Keyword::Float) || token.is(Keyword::Bool) || token.is(Keyword::Str) ||
           token.is(Keyword::None) ||
           (token.type == TokenType::Identifier && userDefinedTypes.find(token.id()) != userDefinedTypes.end());
}

ast::TypeId TypeRegistry::typeId(const Token &token) {
    if (token.type == TokenType::Identifier) {
        const std::string &id = token.id();
        auto it = userDefinedTypes.find(id);
        return it != userDefinedTypes.end() ? it->second : ast::UnknownType;
    }
    if (token.is(Keyword::Int))
        return ast::IntType;
    if (token.is(Keyword::Float))
        return ast::FloatType;
    if (token.is(Keyword::Bool))
        return ast::BoolType;
    if (token.is(Keyword::Str))
        return ast::StrType;
    if (token.is(Keyword::List))
        return ast::ListType;
    if (token.is(Keyword::None))
        return ast::NoneType;
    return ast::UnknownType;
}
