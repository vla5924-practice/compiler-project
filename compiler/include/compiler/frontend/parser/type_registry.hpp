#pragma once

#include "compiler/ast/types.hpp"

#include "compiler/frontend/lexer/token.hpp"

namespace parser {

class TypeRegistry {
  public:
    static bool isTypename(const lexer::Token &token);
    static ast::TypeId typeId(const lexer::Token &token);
};

} // namespace parser
