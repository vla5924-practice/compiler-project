#pragma once

#include <string_view>

#include "compiler/optree/operation.hpp"
#include "compiler/utils/source_ref.hpp"

#define OPTREE_ADAPTOR_HELPER(BASE_ADAPTOR_CLASS, OPERATION_NAME)                                                      \
    using BASE_ADAPTOR_CLASS::BASE_ADAPTOR_CLASS;                                                                      \
    using BASE_ADAPTOR_CLASS::operator bool;                                                                           \
    static std::string_view getOperationName() {                                                                       \
        return (OPERATION_NAME);                                                                                       \
    }                                                                                                                  \
    static Operation::SpecId getSpecId() {                                                                             \
        static char specId = 0;                                                                                        \
        return &specId;                                                                                                \
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

    operator Operation::Ptr() const {
        return op;
    }

    Operation *operator->() const {
        return op.get();
    }

    const utils::SourceRef &ref() const {
        return op->ref;
    }

    void setRef(const utils::SourceRef &ref) {
        op->ref = ref;
    }

    static std::string_view getOperationName() {
        return "Unknown";
    }

    static Operation::SpecId getSpecId() {
        return nullptr;
    }
};

} // namespace optree
