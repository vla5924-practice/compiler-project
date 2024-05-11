#include "attribute.hpp"

#include <cstdint>
#include <ostream>
#include <string>
#include <variant>

#include "definitions.hpp"
#include "types.hpp"

using namespace optree;

void Attribute::dump(std::ostream &stream) const {
    if (is<std::monostate>()) {
        stream << "empty";
        return;
    }
    if (is<NativeInt>()) {
        stream << "int : " << as<NativeInt>();
        return;
    }
    if (is<NativeFloat>()) {
        stream << "float : " << as<NativeFloat>();
        return;
    }
    if (is<NativeBool>()) {
        stream << "bool : " << as<NativeBool>();
        return;
    }
    if (is<NativeStr>()) {
        stream << "str : " << as<NativeStr>();
        return;
    }
    if (is<Type::Ptr>()) {
        stream << "Type : ";
        as<Type::Ptr>()->dump(stream);
        return;
    }
    if (is<ArithBinOpKind>()) {
        stream << "ArithBinOpKind : " << static_cast<int>(as<ArithBinOpKind>());
        return;
    }
    if (is<ArithCastOpKind>()) {
        stream << "ArithCastOpKind : " << static_cast<int>(as<ArithCastOpKind>());
        return;
    }
    if (is<LogicBinOpKind>()) {
        stream << "LogicBinOpKind : " << static_cast<int>(as<LogicBinOpKind>());
        return;
    }
    if (is<LogicUnaryOpKind>()) {
        stream << "LogicUnaryOpKind : " << static_cast<int>(as<LogicUnaryOpKind>());
        return;
    }
}
