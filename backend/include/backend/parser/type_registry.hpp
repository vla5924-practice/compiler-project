#pragma once

#include <map>
#include <unordered_set>

#include "token.hpp"

namespace parser {

class TypeRegistry {
  public:
    enum BuiltInTypes : size_t {
        UnknownType = 0,
        IntType = 1,
        FloatType = 2,
        StrType = 3,
        NoneType = 4,
        BuiltInTypesCount,
    };

  private:
    static std::map<std::string, size_t> userDefinedTypes;

  public:
    static bool isTypename(const Token &token);
    static size_t typeId(const Token &token);
};

} // namespace parser
