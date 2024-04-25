#pragma once

#include <string>

#include "compiler/optree/operation.hpp"
#include "compiler/utils/error_buffer.hpp"

#include "compiler/backend/optree/semantizer/semantizer_error.hpp"

namespace optree {
namespace semantizer {

struct SemantizerContext {
    ErrorBuffer errors;

    void pushError(const Operation::Ptr &op, const std::string &message) {
        errors.push<SemantizerError>(op, message);
    }
};

} // namespace semantizer
} // namespace optree
