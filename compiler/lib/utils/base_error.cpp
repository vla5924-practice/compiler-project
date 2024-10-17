#include "base_error.hpp"

#include <sstream>
#include <string>

using utils::SourceRef;

BaseError::BaseError(const SourceRef &ref, const std::string &initMessage) {
    std::stringstream str;
    str << "In line " << ref.line << " in column " << ref.column << " error:\n" << initMessage;
    message = str.str();
}

BaseError::BaseError(const std::string &initMessage) {
    message = "Error:\n" + initMessage;
}

const char *BaseError::what() const noexcept {
    return message.data();
}
