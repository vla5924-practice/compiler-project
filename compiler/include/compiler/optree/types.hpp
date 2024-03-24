#pragma once

#include <concepts>
#include <ostream>
#include <type_traits>
#include <vector>

#define OPTREE_TYPE_HELPER(CLASSID_MEMBER_NAME)                                                                        \
  protected:                                                                                                           \
    friend struct Type;                                                                                                \
    static TypeId getClassId() {                                                                                       \
        return &CLASSID_MEMBER_NAME;                                                                                   \
    }                                                                                                                  \
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
    virtual ~Type() = default;

    Type &operator=(const Type &) = default;
    Type &operator=(Type &&) = default;

    template <typename DerivedType>
        requires std::derived_from<DerivedType, Type>
    bool is() const {
        return id == DerivedType::getClassId();
    }

    template <typename DerivedType>
        requires std::derived_from<DerivedType, Type>
    const std::remove_cvref_t<DerivedType> &as() const {
        return *dynamic_cast<const DerivedType *>(this);
    }

    template <typename DerivedType>
        requires std::derived_from<DerivedType, Type>
    DerivedType &as() {
        return *dynamic_cast<DerivedType *>(this);
    }

    operator bool() const {
        return id != getClassId();
    }

    virtual void dump(std::ostream &stream) const;

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

    void dump(std::ostream &stream) const override;
};

struct IntegerType : public Type {
    OPTREE_TYPE_HELPER(classId)

    const unsigned width;

    using Type::Type;
    explicit IntegerType(unsigned width = 64U) : Type(&classId), width(width){};

    void dump(std::ostream &stream) const override;
};

struct FloatType : public Type {
    OPTREE_TYPE_HELPER(classId)

    const unsigned width;

    using Type::Type;
    explicit FloatType(unsigned width = 64U) : Type(&classId), width(width){};

    void dump(std::ostream &stream) const override;
};

struct StrType : public Type {
    OPTREE_TYPE_HELPER(classId)

    const unsigned charWidth;

    using Type::Type;
    explicit StrType(unsigned charWidth = 8U) : Type(&classId), charWidth(charWidth){};

    void dump(std::ostream &stream) const override;
};

struct FunctionType : public Type {
    OPTREE_TYPE_HELPER(classId)

    const std::vector<Type> arguments;
    const Type result;

    using Type::Type;

    FunctionType(const std::vector<Type> &arguments, const Type &result)
        : Type(&classId), arguments(arguments), result(result){};

    void dump(std::ostream &stream) const override;
};

struct PointerType : public Type {
    OPTREE_TYPE_HELPER(classId)

    const Type pointee;

    PointerType(const Type &pointee) : Type(&classId), pointee(pointee){};

    void dump(std::ostream &stream) const override;
};

} // namespace optree
