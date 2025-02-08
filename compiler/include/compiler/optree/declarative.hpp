#pragma once

#include <cstddef>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

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
        current = Operation::make<AdaptorType>().op;
        builder.insert(current);
        return *this;
    }

    template <typename AdaptorType, std::convertible_to<DeclarativeValue>... DeclarativeValues>
    DeclarativeModule &op(const DeclarativeValues &...operandValues) {
        op<AdaptorType>();
        return operands(operandValues...);
    }

    template <typename AdaptorType, typename... Args>
    DeclarativeModule &opInit(Args... args) {
        op<AdaptorType>();
        AdaptorType(current).init(std::forward<Args>(args)...);
        return *this;
    }

    DeclarativeModule &operand(const DeclarativeValue &operandValue);
    DeclarativeModule &operand(size_t index, const DeclarativeValue &operandValue);

    template <typename... DeclarativeValues>
    DeclarativeModule &operands(const DeclarativeValue &operandValue, const DeclarativeValues &...operandValues) {
        operand(operandValue);
        if constexpr (sizeof...(DeclarativeValues) != 0)
            operands(operandValues...);
        return *this;
    }

    template <typename T>
    DeclarativeModule &attr(const T &value) {
        current->addAttr(value);
        return *this;
    }

    template <typename T>
    DeclarativeModule &attr(size_t index, const T &value) {
        current->attr(index).set(value);
        return *this;
    }

    DeclarativeModule &result(const Type::Ptr &type);
    DeclarativeModule &inward(DeclarativeValue &inward, const Type::Ptr &type);
    DeclarativeModule &inward(DeclarativeValue &inward, size_t index);
    void withBody();
    void endBody();

    const Operation::Ptr &rootOp() const;
    const Operation::Ptr &childOp(size_t index = 0) const;
    Program makeProgram() const;

    std::string dump() const;
    void dump(std::ostream &stream) const;
};

} // namespace optree
