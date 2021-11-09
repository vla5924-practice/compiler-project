#pragma once

#include <list>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "base_error.hpp"

class ErrorBuffer : public std::exception {
    std::list<std::shared_ptr<BaseError>> buffer;

  public:
    ErrorBuffer() = default;
    ErrorBuffer(const ErrorBuffer &) = default;
    ErrorBuffer(ErrorBuffer &&) = default;
    ~ErrorBuffer() = default;

    template <typename ErrorT, typename... Args>
    void push(Args... args) {
        buffer.emplace_back(std::make_shared<ErrorT>(args...));
    }

    std::string message() const {
        std::stringstream str;
        for (const auto &error : buffer) {
            str << error->what() << "\n";
        }
        return str.str();
    }

    bool empty() const {
        return buffer.empty();
    }
};
