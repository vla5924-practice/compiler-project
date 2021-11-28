#pragma once

#include "base_error.hpp"
#include "lexer/token.hpp"

namespace parser {

class ParserError : public BaseError {
  public:
    ParserError(size_t line_number, size_t column_number, const std::string &message)
        : BaseError(line_number, column_number, message){};
    ParserError(const lexer::Token &token, const std::string &message) : BaseError(token.line, token.column, message){};
    ~ParserError() = default;
};

} // namespace parser
