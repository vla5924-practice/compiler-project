#pragma once

#include "compiler/ast/node.hpp"
#include "compiler/utils/base_error.hpp"

namespace semantizer {

class SemantizerError : public BaseError {
  public:
    SemantizerError(const ast::Node &node, const std::string &message) : BaseError(node.ref, message){};
    SemantizerError(const std::string &message) : BaseError(message){};
    ~SemantizerError() = default;
};

} // namespace semantizer
