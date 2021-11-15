#pragma once

#include <map>
#include <unordered_set>

#include <ast/types.hpp>

#include "lexer/token.hpp"

namespace parser {

class TypeRegistry {
  private:
    static std::map<std::string, ast::TypeId> userDefinedTypes;

  public:
    static bool isTypename(const lexer::Token &token);
    static ast::TypeId typeId(const lexer::Token &token);
};

} // namespace parser
