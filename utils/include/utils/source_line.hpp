#pragma once

#include <string>

namespace utils {

struct SourceLine {
    friend struct SourceFile; // forward declaration

    const SourceFile &file;
    size_t line_number;
    std::string text;

    SourceLine(const SourceFile &file_, size_t line_number_, const std::string &text_)
        : file(file_), line_number(line_number_), text(text_){};
};

} // namespace utils
