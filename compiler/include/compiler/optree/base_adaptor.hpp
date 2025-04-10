#pragma once

#include <string_view>

#include "compiler/optree/operation.hpp"
#include "compiler/utils/source_ref.hpp"

#define OPTREE_ADAPTOR_HELPER(BASE_ADAPTOR_CLASS, OPERATION_NAME)                                                      \
    using Base = BASE_ADAPTOR_CLASS;                                                                                   \
    using Base::Base;                                                                                                  \
    using Base::operator bool;                                                                                         \
    static std::string_view getOperationName() {                                                                       \
        return (OPERATION_NAME);                                                                                       \
    }                                                                                                                  \
    static Operation::SpecId getSpecId() {                                                                             \
        static char specId = 0;                                                                                        \
        return &specId;                                                                                                \
    }                                                                                                                  \
    static bool implementsSpecById(Operation::SpecId specId) {                                                         \
        return specId == getSpecId() || Base::implementsSpecById(specId);                                              \
    }

#define OPTREE_ADAPTOR_ATTRIBUTE(GET_NAME, SET_NAME, TYPE, NUMBER)                                                     \
    const TYPE &GET_NAME() const {                                                                                     \
        return op->attr(NUMBER).as<TYPE>();                                                                            \
    }                                                                                                                  \
    void SET_NAME(const TYPE &attr) {                                                                                  \
        op->attr(NUMBER).set(attr);                                                                                    \
    }

#define OPTREE_ADAPTOR_ATTRIBUTE_OPAQUE(GET_NAME, NUMBER)                                                              \
    const Attribute &GET_NAME() const {                                                                                \
        return op->attr(NUMBER);                                                                                       \
    }                                                                                                                  \
    Attribute &GET_NAME() {                                                                                            \
        return op->attr(NUMBER);                                                                                       \
    }

#define OPTREE_ADAPTOR_ATTRIBUTE_TYPE(GET_NAME, TYPE, NUMBER)                                                          \
    const TYPE &GET_NAME() const {                                                                                     \
        return op->attr(NUMBER).asType<TYPE>();                                                                        \
    }

#define OPTREE_ADAPTOR_OPERAND(GET_NAME, SET_NAME, NUMBER)                                                             \
    Value::Ptr GET_NAME() const {                                                                                      \
        return op->operand(NUMBER);                                                                                    \
    }                                                                                                                  \
    void SET_NAME(const Value::Ptr &value) {                                                                           \
        op->operand(NUMBER) = value;                                                                                   \
    }

#define OPTREE_ADAPTOR_RESULT(GET_NAME, NUMBER)                                                                        \
    Value::Ptr GET_NAME() const {                                                                                      \
        return op->result(NUMBER);                                                                                     \
    }

#define OPTREE_ADAPTOR_INWARD(GET_NAME, NUMBER)                                                                        \
    Value::Ptr GET_NAME() const {                                                                                      \
        return op->inward(NUMBER);                                                                                     \
    }

namespace optree {

struct Adaptor {
    Operation::Ptr op;

    Adaptor(const Adaptor &) = default;
    Adaptor(Adaptor &&) = default;
    ~Adaptor() = default;

    Adaptor() : op(nullptr){};
    Adaptor(const Operation::Ptr &op) : op(op){};

    Adaptor &operator=(const Adaptor &) = default;
    Adaptor &operator=(Adaptor &&) = default;

    operator bool() const {
        return op.operator bool();
    }

    operator const Operation::Ptr &() const {
        return op;
    }

    Operation *operator->() const {
        return op.get();
    }

    static std::string_view getOperationName() {
        return "Unknown";
    }

    static Operation::SpecId getSpecId() {
        return nullptr;
    }

    static bool implementsSpecById(Operation::SpecId) {
        return false;
    }
};

} // namespace optree
