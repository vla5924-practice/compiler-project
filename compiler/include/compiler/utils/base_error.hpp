#pragma once

#include <stdexcept>
#include <string>

#include "compiler/utils/source_ref.hpp"

class BaseError : public std::exception {
    std::string message;

  public:
    BaseError(const BaseError &) = default;
    BaseError(BaseError &&) = default;
    ~BaseError() = default;

    explicit BaseError(const utils::SourceRef &ref, const std::string &initMessage = {});
    explicit BaseError(const std::string &initMessage = {});

    virtual const char *what() const noexcept;

    template <typename T>
        requires std::same_as<decltype(message + std::declval<T>()), std::string>
    BaseError &operator<<(const T &value) {
        message += value;
        return *this;
    }

    template <typename T>
        requires std::same_as<decltype(message + std::to_string(std::declval<T>())), std::string>
    BaseError &operator<<(const T &value) {
        message += std::to_string(value);
        return *this;
    }

    template <typename T>
        requires std::same_as<decltype(std::declval<T>().dump()), std::string>
    BaseError &operator<<(const T &value) {
        message += value.dump();
        return *this;
    }

    template <typename T>
        requires std::same_as<decltype(std::declval<T>()->dump()), std::string>
    BaseError &operator<<(const T &value) {
        message += value->dump();
        return *this;
    }
};
