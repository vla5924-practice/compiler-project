#pragma once

#include <forward_list>
#include <memory>

#include "compiler/utils/source_ref.hpp"

#include "compiler/optree/types.hpp"

namespace optree {

class Operation;

struct Value {
    using Ptr = std::shared_ptr<Value>;

    struct Use {
        Operation *user;
        size_t operandNumber;

        Use() = delete;
        Use(const Use &) = delete;
        Use(Use &&) = default;
        ~Use() = default;

        Use(Operation *user, size_t operandNumber) : user(user), operandNumber(operandNumber){};
    };

    Type type;
    Operation *owner;
    std::forward_list<Use> uses;

    Value() = default;
    Value(const Value &) = delete;
    Value(Value &&) = default;
    ~Value() = default;

    Value(Type type, Operation *owner) : type(type), owner(owner){};
    Value(Type type, std::shared_ptr<Operation> owner) : Value(type, owner.get()){};

    operator bool() const {
        return type.operator bool();
    }

    const utils::SourceRef &ref() const;

    template <typename... Args>
    static Ptr make(Args... args) {
        return std::make_shared<Value>(std::forward<Args>(args)...);
    }
};

} // namespace optree
