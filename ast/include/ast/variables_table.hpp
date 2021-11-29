#pragma once

#include <map>
#include <string>

#include "ast/types.hpp"

namespace ast {

using VariablesTable = std::map<std::string, TypeId>;

} // namespace ast
