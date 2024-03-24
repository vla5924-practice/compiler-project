#pragma once

#include <string_view>

#include "compiler/optree/operation.hpp"
#include "compiler/optree/traits.hpp"

#define OPTREE_ADAPTOR_HELPER(BASE_ADAPTOR_CLASS, OPERATION_NAME, SPECID_MEMBER_NAME)                                  \
  public:                                                                                                              \
    using BASE_ADAPTOR_CLASS::BASE_ADAPTOR_CLASS;                                                                      \
    static std::string_view getOperationName() {                                                                       \
        return (OPERATION_NAME);                                                                                       \
    }                                                                                                                  \
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

    static std::string_view getOperationName() {
        return "Unknown";
    }

    static Operation::SpecId getSpecId() {
        return nullptr;
    }
};

struct ModuleOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Module", specId)

    static bool verify(const Operation *op);
};

struct FunctionOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Function", specId)

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

struct AllocateOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Allocate", specId)

    void init(const Type &type) {
        op->results.emplace_back(Value::make(type, op));
    }

    Value::Ptr result() const {
        return op->result(0);
    }

    static bool verify(const Operation *op);
};

struct ConstantOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Constant", specId)

    template <typename VariantType>
    void init(const Type &type = {}, const VariantType &value = {}) {
        op->results.emplace_back(Value::make(type, op));
        op->addAttr(value);
    }

    const Attribute &value() {
        return op->attr(0);
    }

    Value::Ptr result() const {
        return op->result(0);
    }

    static bool verify(const Operation *op);
};

struct BinaryOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Binary", specId)

    void init(const Type &resultType, Value::Ptr lhs, Value::Ptr rhs) {
        op->addResult(resultType);
        op->addOperand(lhs);
        op->addOperand(rhs);
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

    static bool verify(const Operation *op);
};

struct ArithBinaryOp : BinaryOp {
    OPTREE_ADAPTOR_HELPER(BinaryOp, "ArithBinary", specId)

    void init(ArithBinOpKind kind, const Type &resultType, Value::Ptr lhs, Value::Ptr rhs) {
        BinaryOp::init(resultType, lhs, rhs);
        op->addAttr(kind);
    }

    void init(ArithBinOpKind kind, Value::Ptr lhs, Value::Ptr rhs) {
        init(kind, lhs->type, lhs, rhs);
    }

    ArithBinOpKind kind() const {
        return op->attr(0).as<ArithBinOpKind>();
    }

    static bool verify(const Operation *op);
};

struct LogicBinaryOp : BinaryOp {
    OPTREE_ADAPTOR_HELPER(BinaryOp, "LogicBinary", specId)

    void init(LogicBinOpKind kind, Value::Ptr lhs, Value::Ptr rhs) {
        BinaryOp::init(IntegerType(), lhs, rhs);
        op->addAttr(kind);
    }

    LogicBinOpKind kind() const {
        return op->attr(0).as<LogicBinOpKind>();
    }

    static bool verify(const Operation *op);
};

struct UnaryOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Unary", specId)

    void init(const Type &resultType, Value::Ptr value) {
        op->addResult(resultType);
        op->addOperand(value);
    }

    Value::Ptr value() const {
        return op->operand(0);
    }

    void setValue(Value::Ptr value) {
        op->operand(0) = value;
    }

    Value::Ptr result() const {
        return op->result(0);
    }

    static bool verify(const Operation *op);
};

struct ArithCastOp : UnaryOp {
    OPTREE_ADAPTOR_HELPER(UnaryOp, "ArithCast", specId)

    void init(ArithCastOpKind kind, const Type &resultType, Value::Ptr value) {
        UnaryOp::init(resultType, value);
        op->addAttr(kind);
    }

    ArithCastOpKind kind() const {
        return op->attr(0).as<ArithCastOpKind>();
    }

    static bool verify(const Operation *op);
};

struct LogicUnaryOp : UnaryOp {
    OPTREE_ADAPTOR_HELPER(UnaryOp, "LogicUnary", specId)

    void init(LogicUnaryOpKind kind, const Type &resultType, Value::Ptr value) {
        UnaryOp::init(resultType, value);
        op->addAttr(kind);
    }

    LogicUnaryOpKind kind() const {
        return op->attr(0).as<LogicUnaryOpKind>();
    }

    static bool verify(const Operation *op);
};

struct LoadOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Load", specId)

    void init(const Type &resultType, Value::Ptr src) {
        op->addOperand(src);
        op->results.emplace_back(Value::make(resultType, op));
    }

    void init(Value::Ptr src) {
        auto resultType = src->type.as<PointerType>().pointee;
        init(resultType, src);
    }

    Value::Ptr src() const {
        return op->operand(0);
    }

    Value::Ptr result() const {
        return op->result(0);
    }

    static bool verify(const Operation *op);
};

struct StoreOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Store", specId)

    void init(Value::Ptr dst, Value::Ptr valueToStore) {
        op->addOperand(dst);
        op->addOperand(valueToStore);
    }

    Value::Ptr dst() const {
        return op->operand(0);
    }

    Value::Ptr valueToStore() const {
        return op->operand(1);
    }

    static bool verify(const Operation *op);
};

} // namespace optree
