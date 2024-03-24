#include "types.hpp"

using namespace optree;

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
    stream << "int(" << width << ")";
}

void StrType::dump(std::ostream &stream) const {
    stream << "str(" << charWidth << "xLEN)";
}

void FunctionType::dump(std::ostream &stream) const {
    stream << "func(";
    for (const auto &type : arguments) {
        type.dump(stream);
        stream << ' ';
    }
    stream << "-> ";
    result.dump(stream);
    stream << ')';
}

void PointerType::dump(std::ostream &stream) const {
    stream << "ptr(";
    pointee.dump(stream);
    stream << ')';
}
