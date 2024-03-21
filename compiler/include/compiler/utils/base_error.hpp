#pragma once

#include <stdexcept>
#include <string>

#include "compiler/utils/source_ref.hpp"

class BaseError : public std::exception {
    std::string what_str;

  public:
    BaseError(const utils::SourceRef &ref, const std::string &message);
    BaseError(const std::string &message);
    ~BaseError() = default;

    virtual const char *what() const noexcept;
};
