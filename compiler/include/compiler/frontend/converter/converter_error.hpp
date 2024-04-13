#pragma once

#include <string>

#include "compiler/ast/node.hpp"
#include "compiler/utils/base_error.hpp"

namespace converter {

class ConverterError : public BaseError {
  public:
    ConverterError(const ast::Node::Ptr &node, const std::string &message) : BaseError(node->ref, message){};
    ~ConverterError() override = default;
};

} // namespace converter
