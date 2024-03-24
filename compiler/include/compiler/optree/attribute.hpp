#pragma once

#include <cassert>
#include <cstddef>
#include <string>
#include <variant>

#include "compiler/optree/definitions.hpp"
#include "compiler/optree/types.hpp"

namespace optree {

struct Attribute {
    std::variant<std::monostate, int64_t, double, bool, std::string, Type, ArithBinOpKind, LogicBinOpKind> storage;

    Attribute() = default;
    Attribute(const Attribute &) = default;
    Attribute(Attribute &&) = default;
    ~Attribute() = default;

    template <typename VariantType>
    explicit Attribute(const VariantType &value) : storage(value){};

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

    template <typename VariantType>
    void set(const VariantType &value) {
        storage = value;
    }

    template <typename VariantType>
        requires std::is_base_of_v<Type, VariantType>
    void set(const VariantType &value) {
        storage = dynamic_cast<const Type &>(value);
    }

    operator bool() const {
        return is<std::monostate>();
    }

    void clear() {
        storage = std::monostate();
    }
};

} // namespace optree
