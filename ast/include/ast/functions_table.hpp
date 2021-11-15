#pragma once

#include <map>
#include <string>
#include <vector>

#include "ast/types.hpp"

namespace ast {

struct Function {
    TypeId returnType;
    std::vector<TypeId> argumentsTypes;
};

using FunctionsTable = std::map<std::string, Function>;

} // namespace ast
