#pragma once

#include <string>

#include "compiler/optree/operation.hpp"
#include "compiler/utils/base_error.hpp"

namespace optree {
namespace semantizer {

class SemantizerError : public BaseError {
  public:
    using BaseError::BaseError;

    explicit SemantizerError(const Operation::Ptr &op, const std::string &message = {}) : BaseError(op->ref, message){};
    ~SemantizerError() override = default;
};

} // namespace semantizer
} // namespace optree
