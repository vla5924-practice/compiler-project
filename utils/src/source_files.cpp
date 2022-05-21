#include "source_files.hpp"

#include <forward_list>
#include <fstream>
#include <memory>
#include <stdexcept>

using namespace utils;

SourceFile readFile(const std::string &path) {
    std::ifstream stream(path);
    if (!stream.is_open())
        throw std::runtime_error("Unable to read file " + path);

    static std::forward_list<std::shared_ptr<std::string>> filenames;
    auto filename = filenames.emplace_front(std::make_shared<std::string>(path));
    size_t line = 1u;
    constexpr size_t column = 1u;

    std::string text;
    SourceFile file;
    while (std::getline(stream, text)) {
        file.emplace_back(text, SourceRef(filename, line++, column));
    }

    return file;
}
