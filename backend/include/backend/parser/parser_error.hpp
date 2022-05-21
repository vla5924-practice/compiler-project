#pragma once

#include "base_error.hpp"
#include "lexer/token.hpp"

namespace parser {

class ParserError : public BaseError {
  public:
    ParserError(const lexer::Token &token, const std::string &message) : BaseError(token.ref, message){};
    ~ParserError() = default;
};

} // namespace parser
