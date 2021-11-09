#pragma once

#include "base_error.hpp"

namespace lexer {

class LexerError : public BaseError {
  public:
    LexerError(size_t line_number, size_t column_number, const std::string &message)
        : BaseError(line_number, column_number, message){};
    ~LexerError() = default;
};

} // namespace lexer
