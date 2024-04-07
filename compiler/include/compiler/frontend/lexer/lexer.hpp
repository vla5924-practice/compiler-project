#pragma once

#include "compiler/utils/error_buffer.hpp"
#include "compiler/utils/source_files.hpp"

#include "compiler/frontend/lexer/token.hpp"

namespace lexer {

class Lexer {
    static TokenList processString(const utils::SourceLine &source, ErrorBuffer &errors);

  public:
    Lexer() = delete;
    Lexer(const Lexer &) = delete;
    Lexer(Lexer &&) = delete;
    ~Lexer() = delete;

    static TokenList process(const utils::SourceFile &source);
};

} // namespace lexer
