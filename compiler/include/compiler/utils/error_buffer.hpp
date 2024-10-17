#pragma once

#include <list>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "compiler/utils/base_error.hpp"

class ErrorBuffer : public std::exception {
    std::list<std::shared_ptr<BaseError>> buffer;

  public:
    ErrorBuffer() = default;
    ErrorBuffer(const ErrorBuffer &) = default;
    ErrorBuffer(ErrorBuffer &&) = default;
    ~ErrorBuffer() = default;

    template <std::derived_from<BaseError> ErrorT, typename... Args>
    std::shared_ptr<ErrorT> push(Args... args) {
        auto &error = buffer.emplace_back(std::make_shared<ErrorT>(std::forward<Args>(args)...));
        return std::dynamic_pointer_cast<ErrorT>(error);
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
