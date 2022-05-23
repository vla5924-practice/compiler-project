#pragma once

#include <list>
#include <map>
#include <string>
#include <string_view>

#include <utils/source_files.hpp>

#include "error_buffer.hpp"
#include "lexer/token.hpp"

namespace lexer {

class Lexer {
    static std::map<std::string_view, Keyword> keywords;
    static std::map<std::string_view, Operator> operators;

    static TokenList processString(const utils::SourceLine &source, ErrorBuffer &errors);

  public:
    Lexer() = delete;
    Lexer(const Lexer &) = delete;
    Lexer(Lexer &&) = delete;
    ~Lexer() = delete;

    static TokenList process(const utils::SourceFile &source);
};

} // namespace lexer
