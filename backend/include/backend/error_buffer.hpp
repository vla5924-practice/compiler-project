#pragma once

#include <list>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "base_error.hpp"

class ErrorBuffer : public std::exception {
    std::list<std::unique_ptr<BaseError>> buffer;

  public:
    ErrorBuffer() = default;
    ErrorBuffer(const ErrorBuffer &) = default;
    ErrorBuffer(ErrorBuffer &&) = default;
    ~ErrorBuffer() = default;

    template <typename ErrorT, typename... Args>
    void push(Args... args) {
        buffer.emplace_back(std::make_unique<ErrorT>(args...));
    }

    virtual const char *what() const noexcept {
        std::stringstream str;
        for (const auto &error : buffer) {
            str << error->what() << "\n";
        }
        return str.str().c_str();
    }
};