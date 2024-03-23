#pragma once

#include <cassert>
#include <cstddef>
#include <string>
#include <variant>

#include "compiler/optree/definitions.hpp"

namespace optree {

struct Attribute {
    std::variant<int64_t, double, bool, std::string, ArithBinOpKind, LogicBinOpKind> storage;

    Attribute() = default;
    Attribute(const Attribute &) = default;
    Attribute(Attribute &&) = default;
    ~Attribute() = default;

    template <typename VariantType>
    explicit Attribute(const VariantType &value) : storage(value) {
    }

    template <typename VariantType>
    bool is() const noexcept {
        return std::holds_alternative<VariantType>(storage);
    }

    template <typename VariantType>
    const VariantType &as() const {
        return std::get<VariantType>(storage);
    }

    template <typename VariantType>
    VariantType &as() {
        return std::get<VariantType>(storage);
    }
};

} // namespace optree
