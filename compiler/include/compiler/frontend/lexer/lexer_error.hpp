#pragma once

#include "compiler/utils/base_error.hpp"

namespace lexer {

class LexerError : public BaseError {
  public:
    LexerError(const utils::SourceRef &ref, const std::string &message) : BaseError(ref, message){};
    ~LexerError() = default;
};

} // namespace lexer
