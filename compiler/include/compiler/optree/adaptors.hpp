#pragma once

#include <concepts>
#include <string_view>

#include "compiler/optree/operation.hpp"
#include "compiler/optree/traits.hpp"

#define OPTREE_ADAPTOR_HELPER(BASE_ADAPTOR_CLASS, OPERATION_NAME, SPECID_MEMBER_NAME)                                  \
  public:                                                                                                              \
    using BASE_ADAPTOR_CLASS::BASE_ADAPTOR_CLASS;                                                                      \
    using BASE_ADAPTOR_CLASS::operator bool;                                                                           \
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

#define OPTREE_ADAPTOR_ATTRIBUTE(GET_NAME, SET_NAME, TYPE, NUMBER)                                                     \
    const TYPE &GET_NAME() const {                                                                                     \
        return op->attr(NUMBER).as<TYPE>();                                                                            \
    }                                                                                                                  \
    void SET_NAME(const TYPE &attr) {                                                                                  \
        op->attr(NUMBER).set(attr);                                                                                    \
    }

#define OPTREE_ADAPTOR_ATTRIBUTE_TYPE(GET_NAME, TYPE, NUMBER)                                                          \
    const TYPE &GET_NAME() const {                                                                                     \
        return op->attr(NUMBER).asType<TYPE>();                                                                        \
    }

namespace optree {

struct Adaptor {
    Operation::Ptr op;

    Adaptor(const Adaptor &) = default;
    Adaptor(Adaptor &&) = default;
    ~Adaptor() = default;

    Adaptor() : op(nullptr){};
    Adaptor(const Operation::Ptr &op) : op(op){};

    operator bool() const {
        return op.operator bool();
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

    template <typename AdaptorType>
        requires std::convertible_to<decltype(std::declval<AdaptorType>().name()), std::string>
    AdaptorType lookup(const std::string &name) const {
        for (const auto &childOp : op->body) {
            if (AdaptorType adapted = Operation::as<AdaptorType>(childOp)) {
                if (adapted.name() == name)
                    return adapted;
            }
        }
        return {};
    }

    static bool verify(const Operation *op);
};

struct FunctionOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Function", specId)

    void init(const std::string &name, Type::Ptr funcType) {
        op->addAttr(name);
        op->addAttr(funcType);
        for (auto argType : funcType->as<FunctionType>().arguments)
            op->addInward(argType);
    }

    OPTREE_ADAPTOR_ATTRIBUTE(name, setName, std::string, 0)
    OPTREE_ADAPTOR_ATTRIBUTE_TYPE(type, FunctionType, 1)

    static bool verify(const Operation *op);
};

struct FunctionCallOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "FunctionCall", specId)

    void init(const FunctionOp &callee, const std::vector<Value::Ptr> &arguments) {
        op->operands = arguments;
        op->results.emplace_back(Value::make(callee.type().result, op));
        op->addAttr(callee.name());
    }

    const std::string &name() const {
        return op->attr(0).as<std::string>();
    }

    void setName(const std::string &value) {
        op->attr(0).set(value);
    }

    Value::Ptr result() const {
        return op->result(0);
    }

    static bool verify(const Operation *op);
};

struct ReturnOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Return", specId)

    void init() {
    }

    static bool verify(const Operation *op);
};

struct AllocateOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Allocate", specId)

    void init(Type::Ptr type) {
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
    void init(Type::Ptr type, const VariantType &value) {
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

    void init(Type::Ptr resultType, Value::Ptr lhs, Value::Ptr rhs) {
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

    void init(ArithBinOpKind kind, Type::Ptr resultType, Value::Ptr lhs, Value::Ptr rhs) {
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
        BinaryOp::init(TypeStorage::integerType(), lhs, rhs);
        op->addAttr(kind);
    }

    LogicBinOpKind kind() const {
        return op->attr(0).as<LogicBinOpKind>();
    }

    static bool verify(const Operation *op);
};

struct UnaryOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Unary", specId)

    void init(Type::Ptr resultType, Value::Ptr value) {
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

    void init(ArithCastOpKind kind, Type::Ptr resultType, Value::Ptr value) {
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

    void init(LogicUnaryOpKind kind, Type::Ptr resultType, Value::Ptr value) {
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

    void init(Type::Ptr resultType, Value::Ptr src) {
        op->addOperand(src);
        op->results.emplace_back(Value::make(resultType, op));
    }

    void init(Value::Ptr src) {
        auto resultType = src->type->as<PointerType>().pointee;
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

struct ThenOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Then", specId)

    void init() {
    }

    static bool verify(const Operation *op);
};

struct ElseOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Else", specId)

    void init() {
    }

    static bool verify(const Operation *op);
};

struct IfOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "If", specId)

    void init(Value::Ptr cond, bool withElse = false) {
        op->addOperand(cond);
        op->addToBody(Operation::make<ThenOp>(op).op);
        if (withElse)
            op->addToBody(Operation::make<ElseOp>(op).op);
    }

    OPTREE_ADAPTOR_OPERAND(cond, setCond, 0);

    Operation::Ptr thenOp() const {
        return op->body.front();
    }

    Operation::Ptr elseOp() const {
        return op->body.size() == 2 ? op->body.back() : nullptr;
    }

    static bool verify(const Operation *op);
};

struct ConditionOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Condition", specId)

    void init() {
    }

    static bool verify(const Operation *op);
};

struct WhileOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "While", specId)

    void init() {
    }

    static bool verify(const Operation *op);
};

struct ForOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "For", specId)

    void init(Type::Ptr iteratorType, Value::Ptr start, Value::Ptr stop, Value::Ptr step) {
        op->addOperand(start);
        op->addOperand(stop);
        op->addOperand(step);
        op->addInward(iteratorType);
    }

    OPTREE_ADAPTOR_OPERAND(start, setStart, 0)
    OPTREE_ADAPTOR_OPERAND(stop, setStop, 1)
    OPTREE_ADAPTOR_OPERAND(step, setStep, 2)
    OPTREE_ADAPTOR_INWARD(iterator, 0)

    static bool verify(const Operation *op);
};

} // namespace optree
