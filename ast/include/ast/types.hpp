#pragma once

#include <cstddef>

namespace ast {

using TypeId = size_t;

enum BuiltInTypes : TypeId {
    UnknownType = 0,
    IntType = 1,
    FloatType = 2,
    StrType = 3,
    NoneType = 4,
    BoolType = 5,
    BuiltInTypesCount,
};

} // namespace ast
