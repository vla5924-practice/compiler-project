#pragma once

#include <map>
#include <string>
#include <vector>

namespace ast {

struct Function {
    size_t returnType;
    std::vector<size_t> arguments;
};

using FunctionsTable = std::map<std::string, Function>;

} // namespace ast
