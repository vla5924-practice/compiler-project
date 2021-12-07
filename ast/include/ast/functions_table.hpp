#pragma once

#include <map>
#include <string>
#include <vector>

#include "ast/types.hpp"

namespace ast {

struct Function {
    TypeId returnType;
    std::vector<TypeId> argumentsTypes;

    Function() = default;
    Function(const Function &) = default;
    Function(const TypeId type, const std::vector<TypeId> &args) : returnType(type), argumentsTypes(args){};
    Function(Function &&) = default;
    ~Function() = default;
};

using FunctionsTable = std::map<std::string, Function>;

} // namespace ast
