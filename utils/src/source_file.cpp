#include "source_file.hpp"

#include <fstream>

using namespace utils;

SourceFile SourceFile::read(const std::string &filename) {
    std::ifstream file(filename);
    std::string str;
    SourceFile source;
    size_t line_number = 1u;
    while (std::getline(file, str)) {
        source.lines.emplace_back(source, line_number++, str);
    }
    return source;
}
