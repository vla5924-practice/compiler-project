#pragma once

#include <unordered_set>

#include "token.hpp"

namespace parser {

class TypeRegistry {
    static std::unordered_set<std::string> userDefinedTypes;

  public:
    static bool isTypename(const Token &token);
};

} // namespace parser
