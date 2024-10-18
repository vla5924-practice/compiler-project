#pragma once

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <type_traits>
#include <vector>

namespace optree {

struct Type {
    using Ptr = std::shared_ptr<const Type>;
    using PtrVector = std::vector<Type::Ptr>;

    Type() = default;
    Type(const Type &) = default;
    Type(Type &&) = default;
    virtual ~Type() = default;

    Type &operator=(const Type &) = default;
    Type &operator=(Type &&) = default;

    template <typename DerivedType>
        requires std::derived_from<DerivedType, Type>
    bool is() const {
        return dynamic_cast<const std::remove_cvref_t<DerivedType> *>(this) != nullptr;
    }

    template <typename DerivedType>
        requires std::derived_from<DerivedType, Type>
    const std::remove_cvref_t<DerivedType> &as() const {
        return dynamic_cast<const std::remove_cvref_t<DerivedType> &>(*this);
    }

    operator bool() const {
        return !is<Type>();
    }

    virtual bool operator==(const Type &) const {
        return false;
    }

    virtual bool operator!=(const Type &other) const {
        return !(*this == other);
    }

    virtual unsigned bitWidth() const;
    virtual void dump(std::ostream &stream) const;

    template <typename ConcreteType, typename... Args>
    static auto make(Args... args) {
        return std::make_shared<const std::remove_cvref_t<ConcreteType>>(std::forward<Args>(args)...);
    }
};

struct NoneType : public Type {
    using Ptr = std::shared_ptr<const NoneType>;

    NoneType() = default;

    bool operator==(const Type &other) const override;
    using Type::operator!=;

    void dump(std::ostream &stream) const override;
};

struct IntegerType : public Type {
    using Ptr = std::shared_ptr<const IntegerType>;
    using NativeType = int64_t;

    const unsigned width;

    explicit IntegerType(unsigned width) : width(width){};

    bool operator==(const Type &other) const override;
    using Type::operator!=;

    unsigned bitWidth() const override;
    void dump(std::ostream &stream) const override;
};

struct BoolType : public IntegerType {
    using Ptr = std::shared_ptr<const BoolType>;
    using NativeType = bool;

    static constinit const unsigned intWidth = 8U;

    BoolType() : IntegerType(intWidth){};

    using IntegerType::operator==;
    using IntegerType::operator!=;
};

struct FloatType : public Type {
    using Ptr = std::shared_ptr<const FloatType>;
    using NativeType = double;

    const unsigned width;

    explicit FloatType(unsigned width) : width(width){};

    bool operator==(const Type &other) const override;
    using Type::operator!=;

    unsigned bitWidth() const override;
    void dump(std::ostream &stream) const override;
};

struct StrType : public Type {
    using Ptr = std::shared_ptr<const StrType>;
    using NativeType = std::string;

    const unsigned charWidth;

    explicit StrType(unsigned charWidth) : charWidth(charWidth){};

    bool operator==(const Type &other) const override;
    using Type::operator!=;

    void dump(std::ostream &stream) const override;
};

struct FunctionType : public Type {
    using Ptr = std::shared_ptr<const FunctionType>;

    const PtrVector arguments;
    const Type::Ptr result;

    FunctionType(const PtrVector &arguments, const Type::Ptr &result) : arguments(arguments), result(result){};
    explicit FunctionType(const Type::Ptr &result) : result(result){};

    bool operator==(const Type &other) const override;
    using Type::operator!=;

    void dump(std::ostream &stream) const override;
};

struct PointerType : public Type {
    using Ptr = std::shared_ptr<const PointerType>;

    const Type::Ptr pointee;

    PointerType(const Type::Ptr &pointee) : pointee(pointee){};

    bool operator==(const Type &other) const override;
    using Type::operator!=;

    void dump(std::ostream &stream) const override;
};

struct TupleType : public Type {
    using Ptr = std::shared_ptr<const TupleType>;

    const PtrVector members;

    TupleType(const PtrVector &members) : members(members){};

    bool operator==(const Type &other) const override;
    using Type::operator!=;

    void dump(std::ostream &stream) const override;
};

struct TypeStorage {
    TypeStorage() = delete;
    ~TypeStorage() = delete;

    static NoneType::Ptr noneType();
    static IntegerType::Ptr integerType(unsigned width = 64U);
    static BoolType::Ptr boolType();
    static FloatType::Ptr floatType(unsigned width = 64U);
    static StrType::Ptr strType(unsigned charWidth = 8U);
};

template <typename ConcreteType>
using NativeType = typename ConcreteType::NativeType;

using NativeInt = NativeType<IntegerType>;
using NativeBool = NativeType<BoolType>;
using NativeFloat = NativeType<FloatType>;
using NativeStr = NativeType<StrType>;

} // namespace optree
