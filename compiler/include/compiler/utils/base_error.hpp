#pragma once

#include <sstream>
#include <stdexcept>
#include <string>

#include "compiler/utils/source_ref.hpp"

class BaseError : public std::exception {
    std::stringstream messageStr;

  public:
    BaseError(BaseError &&) = default;
    ~BaseError() = default;

    BaseError(const BaseError &other);
    explicit BaseError(const utils::SourceRef &ref, const std::string &message = {});
    explicit BaseError(const std::string &message = {});

    virtual const char *what() const noexcept;

    template <typename T>
    std::stringstream &operator<<(const T &value) {
        return messageStr << value;
    }
};
