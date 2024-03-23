#pragma once

#include "compiler/optree/operation.hpp"
#include "compiler/optree/traits.hpp"

#define OPTREE_ADAPTOR_HELPER(SPECID_MEMBER_NAME)                                                                      \
  public:                                                                                                              \
    using Adaptor::Adaptor;                                                                                            \
    static Operation::SpecId getSpecId() { return &SPECID_MEMBER_NAME; }                                               \
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

    operator bool() const {
        return op;
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

    const std::string &name() const {
        return op->attr(0).as<std::string>();
    }

    FunctionType type() const {
        return op->attr(1).as<Type>().as<FunctionType>();
    }

    static bool verify(const Operation *op);
};

struct ArithBinaryOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(specId)

    Value::Ptr lhs() const {
        return op->operand(0);
    }

    Value::Ptr rhs() const {
        return op->operand(1);
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
