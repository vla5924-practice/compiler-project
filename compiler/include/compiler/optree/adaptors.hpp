#pragma once

#include "compiler/optree/operation.hpp"
#include "compiler/optree/traits.hpp"

#define OPTREE_ADAPTOR_HELPER(SPECID_MEMBER_NAME)                                                                      \
  public:                                                                                                              \
    using Adaptor::Adaptor;                                                                                            \
    static Operation::SpecId getSpecId() {                                                                             \
        return &SPECID_MEMBER_NAME;                                                                                    \
    }                                                                                                                  \
                                                                                                                       \
  private:                                                                                                             \
    static inline char SPECID_MEMBER_NAME;                                                                             \
                                                                                                                       \
  public:

namespace optree {

struct Adaptor {
    Operation *op;

    Adaptor(const Adaptor &) = default;
    Adaptor(Adaptor &&) = default;
    ~Adaptor() = default;

    Adaptor() : op(nullptr){};
    Adaptor(Operation *op) : op(op){};
    Adaptor(Operation::Ptr op) : op(op.get()){};

    operator bool() const {
        return op;
    }

    const utils::SourceRef &ref() const {
        return op->ref;
    }

    void setRef(const utils::SourceRef &ref) {
        op->ref = ref;
    }

    void init() {
    }

    static bool verify(const Operation *op) {
        return op != nullptr;
    }

    static Operation::SpecId getSpecId() {
        return nullptr;
    }
};

struct ModuleOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(specId)

    static bool verify(const Operation *op);
};

struct FunctionOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(specId)

    void init() {
        op->attributes.resize(2U);
    }

    const std::string &name() const {
        return op->attr(0).as<std::string>();
    }

    void setName(const std::string &value) {
        op->attr(0).set(value);
    }

    const FunctionType &type() const {
        return op->attr(1).as<Type>().as<FunctionType>();
    }

    void setType(const FunctionType &value) {
        op->attr(1).set(value);
    }

    static bool verify(const Operation *op);
};

struct ArithBinaryOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(specId)

    void init() {
        op->attributes.resize(1U);
        op->operands.resize(2U);
        op->results.resize(1U);
    }

    Value::Ptr lhs() const {
        return op->operand(0);
    }

    void setLhs(Value::Ptr value) {
        op->operand(0) = value;
    }

    Value::Ptr rhs() const {
        return op->operand(1);
    }

    void setRhs(Value::Ptr value) {
        op->operand(1) = value;
    }

    Value::Ptr result() const {
        return op->result(0);
    }

    ArithBinOpKind kind() const {
        return op->attr(0).as<ArithBinOpKind>();
    }

    static bool verify(const Operation *op);
};

} // namespace optree
