#pragma once

#include <stdexcept>
#include <string>

class BaseError : public std::exception {
    std::string what_str;

  public:
    BaseError(size_t line_number, size_t column_number, const std::string &message);
    BaseError(const std::string &message);
    ~BaseError() = default;

    virtual const char *what() const noexcept;
};
