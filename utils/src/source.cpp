#include "source.hpp"

#include <fstream>

using namespace utils;

Source Source::fromFile(const std::string &filename) {
    Source source;
    source.append(filename);
    return source;
}

Source Source::fromStrings(const std::vector<std::string> &strings, const std::string &filename) {
    Source source;
    source.filenames.push_back(filename);
    size_t lineNumber = 1u;
    for (const std::string &str : strings)
        source.lines.emplace_back(&source, 0, lineNumber++, str);
    return source;
}

void Source::append(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open())
        return;

    filenames.push_back(filename);
    size_t fileIndex = filenames.size() - 1u;

    std::string str;
    size_t lineNumber = 1u;
    while (std::getline(file, str))
        lines.emplace_back(this, fileIndex, lineNumber++, str);
}

const std::string &Source::filename(size_t index) const {
    return filenames[index];
}
