#pragma once

#include <map>
#include <string>

#include "ast/types.hpp"

namespace ast {

struct Variable {
    TypeId type;
    struct {
        bool modified = false;
    } attributes;

    bool operator==(const Variable &other) const {
        return type == other.type;
    };
};

using VariablesTable = std::map<std::string, Variable>;

} // namespace ast
