#pragma once

#include <concepts>
#include <cstddef>
#include <ostream>
#include <string>
#include <type_traits>
#include <unordered_map>

#include "compiler/optree/attribute.hpp"
#include "compiler/optree/builder.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/optree/program.hpp"
#include "compiler/optree/types.hpp"
#include "compiler/optree/value.hpp"

namespace optree {

class DeclarativeModule;

class DeclarativeValue {
    friend class DeclarativeModule;

    Value::Ptr value;

  public:
    DeclarativeValue() = default;
    DeclarativeValue(const DeclarativeValue &) = default;
    DeclarativeValue(DeclarativeValue &&) = default;
    ~DeclarativeValue() = default;

    DeclarativeValue(const Value::Ptr &value) : value(value){};

    DeclarativeValue &operator=(const DeclarativeModule &mod);

    operator const Value::Ptr &() const {
        return value;
    }
};

class ValueStorage {
    std::unordered_map<int, DeclarativeValue> numKeyStorage;
    std::unordered_map<std::string_view, DeclarativeValue> strKeyStorage;

  protected:
    friend class DeclarativeModule;

    ValueStorage() = default;
    ValueStorage(const ValueStorage &) = default;
    ValueStorage(ValueStorage &&) = default;
    ~ValueStorage() = default;

  public:
    const DeclarativeValue &operator[](int key) const {
        return numKeyStorage.at(key);
    }

    DeclarativeValue &operator[](int key) {
        return numKeyStorage[key];
    }

    const DeclarativeValue &operator[](std::string_view key) const {
        return strKeyStorage.at(key);
    }

    DeclarativeValue &operator[](std::string_view key) {
        return strKeyStorage[key];
    }
};

class DeclarativeModule {
    Operation::Ptr root;
    Operation::Ptr current;
    Builder builder;
    ValueStorage valueStorage;

  protected:
    friend class DeclarativeValue;

    Value::Ptr currentResult(size_t index = 0) const {
        return current->result(index);
    }

  public:
    DeclarativeModule();
    DeclarativeModule(const DeclarativeModule &) = delete;
    DeclarativeModule(DeclarativeModule &&) = delete;
    ~DeclarativeModule() = default;

    const Type::Ptr tNone;
    const Type::Ptr tI64;
    const Type::Ptr tBool;
    const Type::Ptr tF64;
    const Type::Ptr tStr;
    Type::Ptr tPtr(const Type::Ptr &pointee) const;
    Type::Ptr tFunc(const Type::Ptr &result) const;
    Type::Ptr tFunc(Type::PtrVector &&arguments, const Type::Ptr &result) const;

    ValueStorage &values();

    template <typename AdaptorType>
    DeclarativeModule &op() {
        current = Operation::make<AdaptorType>();
        builder.insert(current);
        return *this;
    }

    template <typename AdaptorType, std::convertible_to<DeclarativeValue>... DeclarativeValues>
    DeclarativeModule &op(const DeclarativeValues &...operandValues) {
        op<AdaptorType>();
        return operands(operandValues...);
    }

    template <typename... DeclarativeValues>
    DeclarativeModule &operands(const DeclarativeValue &operand, const DeclarativeValues &...other) {
        current->addOperand(operand);
        if constexpr (sizeof...(DeclarativeValues) != 0)
            operands(other...);
        return *this;
    }

    template <typename T>
    DeclarativeModule &attr(const T &value) {
        if constexpr (std::is_same_v<std::remove_cvref_t<T>, bool>)
            current->addAttr(value);
        else if constexpr (std::is_same_v<std::remove_cvref_t<T>, const char *>)
            current->addAttr(std::string(value));
        else if constexpr (std::is_integral_v<T>)
            current->addAttr(static_cast<int64_t>(value));
        else if constexpr (std::is_floating_point_v<T>)
            current->addAttr(static_cast<double>(value));
        else
            current->addAttr(value);
        return *this;
    }

    DeclarativeModule &result(const Type::Ptr &type);
    DeclarativeModule &inward(DeclarativeValue &inward, const Type::Ptr &type);
    void withBody();
    void endBody();

    const Operation::Ptr &rootOp() const;
    Program makeProgram() const;

    std::string dump() const;
    void dump(std::ostream &stream) const;
};

} // namespace optree
