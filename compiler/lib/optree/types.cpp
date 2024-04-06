#include "types.hpp"

#include <algorithm>
#include <ostream>
#include <unordered_map>

#include "compiler/utils/helpers.hpp"

#define TYPE_COMPARE_EARLY_RETURN(CLASS_NAME, OTHER_NAME)                                                              \
    if (this == &(OTHER_NAME))                                                                                         \
        return true;                                                                                                   \
    if (!(OTHER_NAME).is<CLASS_NAME>())                                                                                \
        return false;

using namespace optree;

bool sameTypes(const Type::Ptr &lhs, const Type::Ptr &rhs) {
    return *lhs == *rhs;
}

void dumpTypes(std::ostream &stream, const Type::PtrVector &types) {
    utils::interleaveComma(stream, types, [&](const Type::Ptr &type) { type->dump(stream); });
}

unsigned Type::bitWidth() const {
    return 0U;
}

void Type::dump(std::ostream &stream) const {
    stream << "<<NULL TYPE>>";
}

bool NoneType::operator==(const Type &other) const {
    return this == &other || other.is<NoneType>();
}

void NoneType::dump(std::ostream &stream) const {
    stream << "none";
}

bool IntegerType::operator==(const Type &other) const {
    TYPE_COMPARE_EARLY_RETURN(IntegerType, other)
    return width == other.as<IntegerType>().width;
}

unsigned IntegerType::bitWidth() const {
    return width;
}

void IntegerType::dump(std::ostream &stream) const {
    stream << "int(" << width << ")";
}

bool FloatType::operator==(const Type &other) const {
    TYPE_COMPARE_EARLY_RETURN(FloatType, other)
    return width == other.as<FloatType>().width;
}

unsigned FloatType::bitWidth() const {
    return width;
}

void FloatType::dump(std::ostream &stream) const {
    stream << "float(" << width << ")";
}

bool StrType::operator==(const Type &other) const {
    TYPE_COMPARE_EARLY_RETURN(StrType, other)
    return charWidth == other.as<StrType>().charWidth;
}

void StrType::dump(std::ostream &stream) const {
    stream << "str(" << charWidth << ")";
}

bool FunctionType::operator==(const Type &other) const {
    TYPE_COMPARE_EARLY_RETURN(FunctionType, other)
    const auto &otherFunc = other.as<FunctionType>();
    if (arguments.size() != otherFunc.arguments.size() || *result != *otherFunc.result)
        return false;
    return std::equal(arguments.begin(), arguments.end(), otherFunc.arguments.begin(), sameTypes);
}

void FunctionType::dump(std::ostream &stream) const {
    stream << "func((";
    dumpTypes(stream, arguments);
    stream << ") -> ";
    result->dump(stream);
    stream << ')';
}

bool PointerType::operator==(const Type &other) const {
    TYPE_COMPARE_EARLY_RETURN(PointerType, other)
    return *pointee == *other.as<PointerType>().pointee;
}

void PointerType::dump(std::ostream &stream) const {
    stream << "ptr(";
    pointee->dump(stream);
    stream << ')';
}

bool TupleType::operator==(const Type &other) const {
    TYPE_COMPARE_EARLY_RETURN(TupleType, other)
    const auto &otherTuple = other.as<TupleType>();
    if (members.size() != otherTuple.members.size())
        return false;
    return std::equal(members.begin(), members.end(), otherTuple.members.begin(), sameTypes);
}

void TupleType::dump(std::ostream &stream) const {
    stream << "tuple(";
    dumpTypes(stream, members);
    stream << ")";
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

BoolType::Ptr TypeStorage::boolType() {
    static BoolType::Ptr ptr = Type::make<BoolType>();
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
