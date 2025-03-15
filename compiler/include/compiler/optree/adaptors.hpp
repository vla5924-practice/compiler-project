#pragma once

#include <string>
#include <vector>

#include "compiler/optree/base_adaptor.hpp"
#include "compiler/optree/definitions.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/optree/types.hpp"
#include "compiler/optree/value.hpp"

namespace optree {

// ----------------------------------------------------------------------------
// Fundamental operations
// ----------------------------------------------------------------------------

struct ModuleOp;
struct FunctionOp;
struct FunctionCallOp;
struct ReturnOp;
struct ConstantOp;

struct ModuleOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Module")

    void init();

    template <typename AdaptorType>
        requires std::convertible_to<decltype(std::declval<AdaptorType>().name()), std::string>
    AdaptorType lookup(const std::string &name) const {
        for (const auto &childOp : op->body) {
            if (AdaptorType adapted = childOp->as<AdaptorType>()) {
                if (adapted.name() == name)
                    return adapted;
            }
        }
        return {};
    }
};

struct FunctionOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Function")

    void init(const std::string &name, const Type::Ptr &funcType);

    OPTREE_ADAPTOR_ATTRIBUTE(name, setName, std::string, 0)
    OPTREE_ADAPTOR_ATTRIBUTE_TYPE(type, FunctionType, 1)
};

struct FunctionCallOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "FunctionCall")

    void init(const std::string &name, const Type::Ptr &resultType, const std::vector<Value::Ptr> &arguments = {});
    void init(const FunctionOp &callee, const std::vector<Value::Ptr> &arguments = {});

    OPTREE_ADAPTOR_ATTRIBUTE(name, setName, std::string, 0)
    OPTREE_ADAPTOR_RESULT(result, 0)
};

struct ReturnOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Return")

    void init();
    void init(const Value::Ptr &value);

    OPTREE_ADAPTOR_OPERAND(value, setValue, 0)
};

struct ConstantOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Constant")

    template <typename T>
    void init(const Type::Ptr &type, const T &value) {
        op->results.emplace_back(Value::make(type, op));
        op->addAttr(value);
    }

    OPTREE_ADAPTOR_ATTRIBUTE_OPAQUE(value, 0)
    OPTREE_ADAPTOR_RESULT(result, 0)
};

// ----------------------------------------------------------------------------
// Computation operations
// ----------------------------------------------------------------------------

struct BinaryOp;
struct ArithBinaryOp;
struct LogicBinaryOp;
struct UnaryOp;
struct ArithCastOp;
struct LogicUnaryOp;

struct BinaryOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Binary")

    void init(const Type::Ptr &resultType, const Value::Ptr &lhs, const Value::Ptr &rhs);

    OPTREE_ADAPTOR_OPERAND(lhs, setLhs, 0)
    OPTREE_ADAPTOR_OPERAND(rhs, setRhs, 1)
    OPTREE_ADAPTOR_RESULT(result, 0)
};

struct ArithBinaryOp : BinaryOp {
    OPTREE_ADAPTOR_HELPER(BinaryOp, "ArithBinary")

    void init(ArithBinOpKind kind, const Type::Ptr &resultType, const Value::Ptr &lhs, const Value::Ptr &rhs);
    void init(ArithBinOpKind kind, const Value::Ptr &lhs, const Value::Ptr &rhs);

    OPTREE_ADAPTOR_ATTRIBUTE(kind, setKind, ArithBinOpKind, 0)
};

struct LogicBinaryOp : BinaryOp {
    OPTREE_ADAPTOR_HELPER(BinaryOp, "LogicBinary")

    void init(LogicBinOpKind kind, const Value::Ptr &lhs, const Value::Ptr &rhs);

    OPTREE_ADAPTOR_ATTRIBUTE(kind, setKind, LogicBinOpKind, 0)
};

struct UnaryOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Unary")

    void init(const Type::Ptr &resultType, const Value::Ptr &value);

    OPTREE_ADAPTOR_OPERAND(value, setValue, 0)
    OPTREE_ADAPTOR_RESULT(result, 0)
};

struct ArithCastOp : UnaryOp {
    OPTREE_ADAPTOR_HELPER(UnaryOp, "ArithCast")

    void init(ArithCastOpKind kind, const Type::Ptr &resultType, const Value::Ptr &value);

    OPTREE_ADAPTOR_ATTRIBUTE(kind, setKind, ArithCastOpKind, 0)
};

struct LogicUnaryOp : UnaryOp {
    OPTREE_ADAPTOR_HELPER(UnaryOp, "LogicUnary")

    void init(LogicUnaryOpKind kind, const Value::Ptr &value);

    OPTREE_ADAPTOR_ATTRIBUTE(kind, setKind, LogicUnaryOpKind, 0)
};

// ----------------------------------------------------------------------------
// Memory access operations
// ----------------------------------------------------------------------------

struct AllocateOp;
struct LoadOp;
struct StoreOp;

struct AllocateOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Allocate")

    void init(const Type::Ptr &type, const Value::Ptr &dynamicSize = {});

    OPTREE_ADAPTOR_RESULT(result, 0)

    // dynamicSize is an optional operand
    Value::Ptr dynamicSize() const;
    void setDynamicSize(const Value::Ptr &value);
};

struct LoadOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Load")

    void init(const Type::Ptr &resultType, const Value::Ptr &src, const Value::Ptr &offset = {});
    void init(const Value::Ptr &src, const Value::Ptr &offset = {});

    OPTREE_ADAPTOR_OPERAND(src, setSrc, 0)
    OPTREE_ADAPTOR_RESULT(result, 0)

    // offset is an optional operand
    Value::Ptr offset() const;
    void setOffset(const Value::Ptr &value);
};

struct StoreOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Store")

    void init(const Value::Ptr &dst, const Value::Ptr &valueToStore, const Value::Ptr &offset = {});

    OPTREE_ADAPTOR_OPERAND(dst, setDst, 0)
    OPTREE_ADAPTOR_OPERAND(valueToStore, setValueToStore, 1)

    // offset is an optional operand
    Value::Ptr offset() const;
    void setOffset(const Value::Ptr &value);
};

// ----------------------------------------------------------------------------
// Control flow operations
// ----------------------------------------------------------------------------

struct IfOp;
struct ThenOp;
struct ElseOp;
struct WhileOp;
struct ConditionOp;
struct ForOp;

struct IfOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "If")

    void init(const Value::Ptr &cond, bool withElse = false);

    OPTREE_ADAPTOR_OPERAND(cond, setCond, 0);

    ThenOp thenOp() const;
    ElseOp elseOp() const;
};

struct ThenOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Then")

    void init();
};

struct ElseOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Else")

    void init();
};

struct WhileOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "While")

    void init();

    ConditionOp conditionOp() const;
};

struct ConditionOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Condition")

    void init();

    Value::Ptr terminator() const;
};

struct ForOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "For")

    void init(const Type::Ptr &iteratorType, const Value::Ptr &start, const Value::Ptr &stop, const Value::Ptr &step);

    OPTREE_ADAPTOR_OPERAND(start, setStart, 0)
    OPTREE_ADAPTOR_OPERAND(stop, setStop, 1)
    OPTREE_ADAPTOR_OPERAND(step, setStep, 2)
    OPTREE_ADAPTOR_INWARD(iterator, 0)
};

// ----------------------------------------------------------------------------
// Special operations
// ----------------------------------------------------------------------------

struct InputOp;
struct PrintOp;

struct InputOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Input")

    void init(const Value::Ptr &dst);

    OPTREE_ADAPTOR_OPERAND(dst, setDst, 0)
};

struct PrintOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Print")

    void init(const Value::Ptr &valueToPrint);
    void init(const std::vector<Value::Ptr> &valuesToPrint);
};

} // namespace optree
