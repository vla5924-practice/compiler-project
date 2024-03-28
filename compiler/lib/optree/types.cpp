#include "types.hpp"

#include "compiler/utils/utils.hpp"

using namespace optree;

bool NoneType::operator==(const Type &other) const {
    return this == &other || other.is<NoneType>();
}

bool IntegerType::operator==(const Type &other) const {
    if (this == &other)
        return true;
    if (!other.is<IntegerType>())
        return false;
    return width == other.as<IntegerType>().width;
}

bool FloatType::operator==(const Type &other) const {
    if (this == &other)
        return true;
    if (!other.is<FloatType>())
        return false;
    return width == other.as<FloatType>().width;
}

bool StrType::operator==(const Type &other) const {
    if (this == &other)
        return true;
    if (!other.is<StrType>())
        return false;
    return charWidth == other.as<StrType>().charWidth;
}

bool FunctionType::operator==(const Type &other) const {
    if (this == &other)
        return true;
    if (!other.is<FunctionType>())
        return false;
    const FunctionType &otherFunc = other.as<FunctionType>();
    if (arguments.size() != otherFunc.arguments.size() || *result != *otherFunc.result)
        return false;
    return std::equal(arguments.begin(), arguments.end(), otherFunc.arguments.begin(),
                      [](const Type::Ptr &lhs, const Type::Ptr &rhs) { return *lhs == *rhs; });
}

bool PointerType::operator==(const Type &other) const {
    if (this == &other)
        return true;
    if (!other.is<PointerType>())
        return false;
    return *pointee == *other.as<PointerType>().pointee;
}

void Type::dump(std::ostream &stream) const {
    stream << "<<NULL TYPE>>";
}

void NoneType::dump(std::ostream &stream) const {
    stream << "None";
}

void IntegerType::dump(std::ostream &stream) const {
    stream << "int(" << width << ")";
}

void FloatType::dump(std::ostream &stream) const {
    stream << "float(" << width << ")";
}

void StrType::dump(std::ostream &stream) const {
    stream << "str(" << charWidth << "xLEN)";
}

void FunctionType::dump(std::ostream &stream) const {
    stream << "func((";
    utils::interleaveComma(stream, arguments, [&](const Type::Ptr &type) { type->dump(stream); });
    stream << ") -> ";
    result->dump(stream);
    stream << ')';
}

void PointerType::dump(std::ostream &stream) const {
    stream << "ptr(";
    pointee->dump(stream);
    stream << ')';
}

NoneType::Ptr TypeStorage::noneType() {
    static NoneType::Ptr ptr = Type::make<NoneType>();
    return ptr;
}

IntegerType::Ptr TypeStorage::integerType(unsigned width) {
    static std::unordered_map<unsigned, IntegerType::Ptr> storage;
    IntegerType::Ptr ptr;
    if (!storage.contains(width)) {
        ptr = Type::make<IntegerType>(width);
        storage[width] = ptr;
    } else {
        ptr = storage[width];
    }
    return ptr;
}

FloatType::Ptr TypeStorage::floatType(unsigned width) {
    static std::unordered_map<unsigned, FloatType::Ptr> storage;
    FloatType::Ptr ptr;
    if (!storage.contains(width)) {
        ptr = Type::make<FloatType>(width);
        storage[width] = ptr;
    } else {
        ptr = storage[width];
    }
    return ptr;
}

StrType::Ptr TypeStorage::strType(unsigned width) {
    static std::unordered_map<unsigned, StrType::Ptr> storage;
    StrType::Ptr ptr;
    if (!storage.contains(width)) {
        ptr = Type::make<StrType>(width);
        storage[width] = ptr;
    } else {
        ptr = storage[width];
    }
    return ptr;
}
