#pragma once

#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

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
    auto &operator<<(const T &value) {
        return messageStr << value;
    }

    template <typename T>
        requires std::same_as<decltype(std::declval<T>().dump(messageStr)), void>
    std::stringstream &operator<<(const T &value) {
        value.dump(messageStr);
        return messageStr;
    }

    template <typename T>
        requires std::same_as<decltype(std::declval<T>()->dump(messageStr)), void>
    std::stringstream &operator<<(const T &value) {
        value->dump(messageStr);
        return messageStr;
    }
};
