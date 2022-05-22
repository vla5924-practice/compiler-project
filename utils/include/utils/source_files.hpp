#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "utils/source_ref.hpp"

namespace utils {

struct SourceLine {
    std::string text;
    SourceRef ref;

    SourceLine() = default;
    SourceLine(const SourceLine &) = default;
    SourceLine(SourceLine &&) = default;
    ~SourceLine() = default;

    SourceLine(const std::string &text_) : text(text_), ref(){};
    SourceLine(const char *const text_) : text(text_), ref(){};
    SourceLine(const std::string &text_, const SourceRef &ref_) : text(text_), ref(ref_){};

    SourceRef makeRef(const std::string::const_iterator &iter) const {
        size_t column = std::distance(text.begin(), iter);
        return ref.inSameLine(column);
    }

    SourceLine &operator=(const SourceLine &) = default;
    SourceLine &operator=(SourceLine &&) = default;

    bool operator==(const SourceLine &other) const {
        return text == other.text;
    }

    bool operator!=(const SourceLine &other) const {
        return !(*this == other);
    }

    std::string::value_type &operator[](size_t i) {
        return text.operator[](i);
    }

    const std::string::value_type &operator[](size_t i) const {
        return text.operator[](i);
    }

    friend std::ostream &operator<<(std::ostream &stream, const SourceLine &line) {
        stream << line.text;
        return stream;
    }
};

using SourceFile = std::vector<SourceLine>;

SourceFile readFile(const std::string &path);

} // namespace utils
