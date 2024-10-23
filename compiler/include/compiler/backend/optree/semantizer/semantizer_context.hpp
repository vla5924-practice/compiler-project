#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/utils/error_buffer.hpp"

#include "compiler/backend/optree/semantizer/semantizer_error.hpp"

namespace optree {
namespace semantizer {

struct SemantizerContext {
    ErrorBuffer errors;
    std::unordered_map<std::string, FunctionOp> functions;

    std::optional<FunctionOp> findFunction(const std::string &name) const {
        auto it = functions.find(name);
        if (it == functions.end())
            return {};
        return {it->second};
    }

    SemantizerError &pushError(const Operation::Ptr &op, const std::string &message = {}) {
        return *errors.push<SemantizerError>(op, message);
    }

    SemantizerError &pushOpError(const Operation::Ptr &op, const std::string &message = {}) {
        return *errors.push<SemantizerError>(op, std::string(op->name) + " operation " + message);
    }
};

} // namespace semantizer
} // namespace optree
