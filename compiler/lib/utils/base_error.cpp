#include "base_error.hpp"

#include <sstream>

using utils::SourceRef;

BaseError::BaseError(const BaseError &other) : BaseError() {
    messageStr << other.messageStr.view();
}

BaseError::BaseError(const SourceRef &ref, const std::string &message) {
    messageStr << "In line " << ref.line << " in column " << ref.column << " error:\n" << message;
}

BaseError::BaseError(const std::string &message) {
    messageStr << "Error:\n" + message;
}

const char *BaseError::what() const noexcept {
    return messageStr.view().data();
}
