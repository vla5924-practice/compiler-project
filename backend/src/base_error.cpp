#include "base_error.hpp"

BaseError::BaseError(size_t line_number, size_t column_number, const std::string &message) {
    what_str = "In line " + std::to_string(line_number) + " in column " + std::to_string(column_number) + " error:\n" +
               message;
}

BaseError::BaseError(const std::string &message) {
    what_str = "Error:\n" + message;
}

const char *BaseError::what() const noexcept {
    return what_str.c_str();
}
