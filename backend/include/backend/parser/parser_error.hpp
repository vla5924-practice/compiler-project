#pragma once

#include "base_error.hpp"

namespace parser {

class ParserError : public BaseError {
  public:
    ParserError(size_t line_number, size_t column_number, const std::string &message)
        : BaseError(line_number, column_number, message){};
    ~ParserError() = default;
};

} // namespace parser
