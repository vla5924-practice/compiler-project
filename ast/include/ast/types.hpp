#pragma once

namespace ast {

using TypeId = size_t;

enum BuiltInTypes : TypeId {
    UnknownType = 0,
    IntType = 1,
    FloatType = 2,
    StrType = 3,
    NoneType = 4,
    BuiltInTypesCount,
};

} // namespace ast
