#include "base_error.hpp"

#include <sstream>

using utils::SourceRef;

BaseError::BaseError(const SourceRef &ref, const std::string &message) {
    std::stringstream str;
    str << "In line " << ref.line << " in column " << ref.column << " error:\n" << message;
    what_str = str.str();
}

BaseError::BaseError(const std::string &message) {
    what_str = "Error:\n" + message;
}

const char *BaseError::what() const noexcept {
    return what_str.c_str();
}
