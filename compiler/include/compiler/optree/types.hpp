#pragma once

#include <type_traits>
#include <vector>

#define OPTREE_TYPE_HELPER(CLASSID_MEMBER_NAME)                                                                        \
  protected:                                                                                                           \
    friend struct Type;                                                                                                \
    static TypeId getClassId() { return &CLASSID_MEMBER_NAME; }                                                        \
                                                                                                                       \
  private:                                                                                                             \
    static inline char CLASSID_MEMBER_NAME = 0;                                                                        \
                                                                                                                       \
  public:

namespace optree {

struct Type {
    Type() : id(nullptr){};
    Type(const Type &) = default;
    Type(Type &&) = default;
    ~Type() = default;

    template <typename DerivedType>
    std::enable_if_t<std::is_base_of_v<Type, DerivedType>, bool> is() const {
        return id == DerivedType::getClassId();
    }

    template <typename DerivedType>
    DerivedType as() const {
        return DerivedType(*reinterpret_cast<const DerivedType *>(this));
    }

    operator bool() const {
        return id != getClassId();
    }

  protected:
    using TypeId = void *;

    TypeId id;

    Type(TypeId id) : id(id){};

    static TypeId getClassId() {
        return nullptr;
    }
};

struct NoneType : public Type {
    OPTREE_TYPE_HELPER(classId)

    using Type::Type;
    NoneType() : Type(&classId){};
};

struct IntegerType : public Type {
    OPTREE_TYPE_HELPER(classId)

    const unsigned width;

    using Type::Type;
    explicit IntegerType(unsigned width = 64U) : Type(&classId), width(width){};
};

struct FloatType : public Type {
    OPTREE_TYPE_HELPER(classId)

    const unsigned width;

    using Type::Type;
    explicit FloatType(unsigned width = 64U) : Type(&classId), width(width){};
};

struct StrType : public Type {
    OPTREE_TYPE_HELPER(classId)

    const unsigned charWidth;

    using Type::Type;
    explicit StrType(unsigned charWidth = 8U) : Type(&classId), charWidth(charWidth){};
};

struct FunctionType : public Type {
    OPTREE_TYPE_HELPER(classId)

    const std::vector<Type> arguments;
    const Type result;

    using Type::Type;

    FunctionType(const std::vector<Type> &arguments, const Type &result)
        : Type(&classId), arguments(arguments), result(result){};
};

} // namespace optree
