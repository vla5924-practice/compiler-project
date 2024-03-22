#pragma once

#include <memory>
#include <string>

namespace utils {

struct SourceRef {
    std::weak_ptr<const std::string> filename;
    size_t line;
    size_t column;

    SourceRef() = default;
    SourceRef(const SourceRef &) = default;
    SourceRef(SourceRef &&) = default;
    ~SourceRef() = default;

    SourceRef(std::shared_ptr<const std::string> filename_, size_t line_, size_t column_)
        : filename(filename_), line(line_), column(column_){};

    SourceRef inSameLine(size_t column_) const {
        SourceRef other(*this);
        other.column = column_;
        return other;
    }

    SourceRef inSameFile(size_t line_, size_t column_) const {
        SourceRef other = inSameLine(column_);
        other.line = line_;
        return other;
    }

    SourceRef &operator=(const SourceRef &) = default;
    SourceRef &operator=(SourceRef &&) = default;
};

} // namespace utils
