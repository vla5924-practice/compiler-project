#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <variant>

#include "compiler/optree/definitions.hpp"
#include "compiler/optree/types.hpp"
#include "compiler/utils/helpers.hpp"

namespace optree {

struct Attribute {
    using Storage = std::variant<
        //
        std::monostate, NativeInt, NativeBool, NativeFloat, NativeStr, Type::Ptr, ArithBinOpKind, ArithCastOpKind,
        LogicBinOpKind, LogicUnaryOpKind
        //
        >;

    Storage storage;

    Attribute() = default;
    Attribute(const Attribute &) = default;
    Attribute(Attribute &&) = default;
    ~Attribute() = default;

    Attribute &operator=(const Attribute &) = default;
    Attribute &operator=(Attribute &&) = default;

    bool operator==(const Attribute &other) const;

    template <typename VariantType>
    explicit Attribute(const VariantType &value) {
        if constexpr (Attribute::canHold<VariantType>())
            set(value);
        else if constexpr (std::is_integral_v<VariantType>)
            set(static_cast<NativeInt>(value));
        else if constexpr (std::is_floating_point_v<VariantType>)
            set(static_cast<NativeFloat>(value));
        else if constexpr (std::is_constructible_v<std::string, VariantType>)
            set(std::string(value));
        else
            throw std::bad_variant_access();
    }

    template <typename VariantType>
    bool is() const noexcept {
        return std::holds_alternative<VariantType>(storage);
    }

    template <typename ConcreteType>
    bool isType() const {
        return is<Type::Ptr>() && as<Type::Ptr>()->is<ConcreteType>();
    }

    template <typename VariantType>
    const VariantType &as() const {
        return std::get<VariantType>(storage);
    }

    template <typename VariantType>
    VariantType &as() {
        return std::get<VariantType>(storage);
    }

    template <typename ConcreteType>
    const ConcreteType &asType() const {
        return as<Type::Ptr>()->as<ConcreteType>();
    }

    template <typename VariantType>
    void set(const VariantType &value) {
        storage = value;
    }

    template <typename VariantType>
        requires std::derived_from<VariantType, Type>
    void set(const std::shared_ptr<const VariantType> &value) {
        storage = std::dynamic_pointer_cast<const Type>(value);
    }

    operator bool() const {
        return is<std::monostate>();
    }

    void clear() {
        storage = std::monostate();
    }

    void dump(std::ostream &stream) const;

    template <typename T>
    static constexpr bool canHold() {
        return utils::canHoldAlternative<T, Storage>;
    }
};

} // namespace optree
