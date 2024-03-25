#pragma once

#include <list>
#include <map>
#include <unordered_map>
#include <string>
#include <string_view>

#include "compiler/utils/error_buffer.hpp"
#include "compiler/utils/source_files.hpp"

#include "compiler/frontend/lexer/token.hpp"

namespace lexer {

class Lexer {
    static std::unordered_map<std::string_view, Keyword> keywords;
    static std::unordered_map<std::string_view, Operator> operators;

    static TokenList processString(const utils::SourceLine &source, ErrorBuffer &errors);

  public:
    Lexer() = delete;
    Lexer(const Lexer &) = delete;
    Lexer(Lexer &&) = delete;
    ~Lexer() = delete;

    static TokenList process(const utils::SourceFile &source);
};

} // namespace lexer
