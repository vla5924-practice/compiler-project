#pragma once

#include <list>
#include <string>
#include <vector>

namespace utils {

struct Source;

struct SourceLine {
    const Source *const parent;
    size_t fileIndex;
    size_t number;
    std::string text;

    SourceLine(const Source *const parent_, size_t fileIndex_, size_t number_, std::string &&text_)
        : parent(parent_), fileIndex(fileIndex_), number(number_), text(std::move(text_)){};
    SourceLine(const Source *const parent_, size_t fileIndex_, size_t number_, const std::string &text_)
        : parent(parent_), fileIndex(fileIndex_), number(number_), text(text_){};
};

struct Source {
    std::list<SourceLine> lines;

    Source(const Source &) = default;
    Source(Source &&) = default;
    ~Source() = default;

    static Source fromFile(const std::string &filename);
    static Source fromStrings(const std::vector<std::string> &strings, const std::string &filename = "");

    void append(const std::string &filename);
    const std::string &filename(size_t index) const;

  private:
    std::vector<std::string> filenames;

    Source() = default;
};

struct SourceRef {
    const SourceLine &line;
    size_t columnNumber;

    SourceRef(const SourceLine &line_, size_t columnNumber_) : line(line_), columnNumber(columnNumber_){};
};

} // namespace utils
