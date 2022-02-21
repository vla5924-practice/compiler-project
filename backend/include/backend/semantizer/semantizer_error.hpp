#pragma once

#include "ast/node.hpp"
#include "base_error.hpp"

namespace semantizer {

class SemantizerError : public BaseError {
  public:
    SemantizerError(const ast::Node &node, const std::string &message) : BaseError(1, 1, message){};
    ~SemantizerError() = default;
};

} // namespace semantizer
