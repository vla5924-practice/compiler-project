#pragma once

#include <list>
#include <string>

#include "utils/source_line.hpp"

namespace utils {

struct SourceFile {
    std::string filename;
    std::list<SourceLine> lines;

    SourceFile(const SourceFile &) = default;
    SourceFile(SourceFile &&) = default;
    ~SourceFile() = default;

    static SourceFile read(const std::string &filename);

  private:
    SourceFile() = default;
};

using SourceFilesList = std::list<SourceFile>;

} // namespace utils
