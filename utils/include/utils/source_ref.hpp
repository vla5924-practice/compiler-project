#pragma once

#include "utils/source_file.hpp"

namespace utils {

struct SourceRef {
    const SourceLine &line;
    size_t column_number;

    SourceRef(const SourceLine &line_, size_t column_number_) : line(line_), column_number(column_number_){};
};

} // namespace utils
