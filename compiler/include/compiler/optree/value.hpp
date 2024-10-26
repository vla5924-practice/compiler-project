#pragma once

#include <cstddef>
#include <forward_list>
#include <memory>

#include "compiler/utils/source_ref.hpp"

#include "compiler/optree/types.hpp"

namespace optree {

struct Operation;

struct Value {
    using Ptr = std::shared_ptr<Value>;
    using BackRef = std::weak_ptr<Operation>;

    struct Use {
        BackRef user;
        size_t operandNumber;

        Use() = delete;
        Use(const Use &) = delete;
        Use(Use &&) = default;
        ~Use() = default;

        Use(const BackRef &user, size_t operandNumber);

        decltype(user.lock()) lock() const noexcept;
        bool userIs(const Operation *op) const noexcept;
    };

    Type::Ptr type;
    BackRef owner;
    std::forward_list<Use> uses;

    Value() = default;
    Value(const Value &) = delete;
    Value(Value &&) = default;
    ~Value() = default;

    Value(const Type::Ptr &type, const BackRef &owner) : type(type), owner(owner){};

    operator bool() const {
        return type.operator bool() && !owner.expired();
    }

    bool hasType(const Type::Ptr &other) const {
        return *type == *other;
    }

    bool sameType(const Value::Ptr &other) const {
        return *type == *other->type;
    }

    bool canPointTo(const Value::Ptr &other) const {
        return type->is<PointerType>() && *type->as<PointerType>().pointee == *other->type;
    }

    const utils::SourceRef &ref() const;

    template <typename... Args>
    static Ptr make(Args... args) {
        return std::make_shared<Value>(std::forward<Args>(args)...);
    }
};

} // namespace optree
