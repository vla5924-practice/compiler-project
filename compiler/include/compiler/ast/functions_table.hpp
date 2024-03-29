#pragma once

#include <map>
#include <string>
#include <vector>

#include "compiler/ast/types.hpp"

namespace ast {

struct Function {
    TypeId returnType;
    std::vector<TypeId> argumentsTypes;
    unsigned useCount = 0;

    Function() = default;
    Function(const Function &) = default;
    explicit Function(const TypeId type, const std::vector<TypeId> &args = {})
        : returnType(type), argumentsTypes(args){};
    Function(Function &&) = default;
    ~Function() = default;

    Function &operator=(const Function &) = default;
    Function &operator=(Function &&) = default;

    bool operator==(const Function &other) const {
        return (returnType == other.returnType) && (argumentsTypes == other.argumentsTypes);
    }

    bool operator!=(const Function &other) const {
        return !(*this == other);
    }
};

using FunctionsTable = std::map<std::string, Function>;

} // namespace ast
