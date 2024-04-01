#pragma once

#include <cstdint>
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
    OPTREE_ADAPTOR_HELPER(Adaptor, "Module", specId)

    void init();

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

    void init(const std::string &name, const Type::Ptr &funcType);

    OPTREE_ADAPTOR_ATTRIBUTE(name, setName, std::string, 0)
    OPTREE_ADAPTOR_ATTRIBUTE_TYPE(type, FunctionType, 1)

    static bool verify(const Operation *op);
};

struct FunctionCallOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "FunctionCall", specId)

    void init(const std::string &name, const Type::Ptr &resultType, const std::vector<Value::Ptr> &arguments);
    void init(const FunctionOp &callee, const std::vector<Value::Ptr> &arguments);

    OPTREE_ADAPTOR_ATTRIBUTE(name, setName, std::string, 0)
    OPTREE_ADAPTOR_RESULT(result, 0)

    static bool verify(const Operation *op);
};

struct ReturnOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Return", specId)

    void init();
    void init(const Value::Ptr &value);

    OPTREE_ADAPTOR_OPERAND(value, setValue, 0)

    static bool verify(const Operation *op);
};

struct ConstantOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Constant", specId)

    void init(const Type::Ptr &type, int64_t value);
    void init(const Type::Ptr &type, double value);
    void init(const Type::Ptr &type, const std::string &value);

    OPTREE_ADAPTOR_ATTRIBUTE_OPAQUE(value, 0)
    OPTREE_ADAPTOR_RESULT(result, 0)

    static bool verify(const Operation *op);
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
    OPTREE_ADAPTOR_HELPER(Adaptor, "Binary", specId)

    void init(const Type::Ptr &resultType, const Value::Ptr &lhs, const Value::Ptr &rhs);

    OPTREE_ADAPTOR_OPERAND(lhs, setLhs, 0)
    OPTREE_ADAPTOR_OPERAND(rhs, setRhs, 1)
    OPTREE_ADAPTOR_RESULT(result, 0)

    static bool verify(const Operation *op);
};

struct ArithBinaryOp : BinaryOp {
    OPTREE_ADAPTOR_HELPER(BinaryOp, "ArithBinary", specId)

    void init(ArithBinOpKind kind, const Type::Ptr &resultType, const Value::Ptr &lhs, const Value::Ptr &rhs);
    void init(ArithBinOpKind kind, const Value::Ptr &lhs, const Value::Ptr &rhs);

    OPTREE_ADAPTOR_ATTRIBUTE(kind, setKind, ArithBinOpKind, 0)

    static bool verify(const Operation *op);
};

struct LogicBinaryOp : BinaryOp {
    OPTREE_ADAPTOR_HELPER(BinaryOp, "LogicBinary", specId)

    void init(LogicBinOpKind kind, const Value::Ptr &lhs, const Value::Ptr &rhs);

    OPTREE_ADAPTOR_ATTRIBUTE(kind, setKind, LogicBinOpKind, 0)

    static bool verify(const Operation *op);
};

struct UnaryOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Unary", specId)

    void init(const Type::Ptr &resultType, const Value::Ptr &value);

    OPTREE_ADAPTOR_OPERAND(value, setValue, 0)
    OPTREE_ADAPTOR_RESULT(result, 0)

    static bool verify(const Operation *op);
};

struct ArithCastOp : UnaryOp {
    OPTREE_ADAPTOR_HELPER(UnaryOp, "ArithCast", specId)

    void init(ArithCastOpKind kind, const Type::Ptr &resultType, const Value::Ptr &value);

    OPTREE_ADAPTOR_ATTRIBUTE(kind, setKind, ArithCastOpKind, 0)

    static bool verify(const Operation *op);
};

struct LogicUnaryOp : UnaryOp {
    OPTREE_ADAPTOR_HELPER(UnaryOp, "LogicUnary", specId)

    void init(LogicUnaryOpKind kind, const Type::Ptr &resultType, const Value::Ptr &value);

    OPTREE_ADAPTOR_ATTRIBUTE(kind, setKind, LogicUnaryOpKind, 0)

    static bool verify(const Operation *op);
};

// ----------------------------------------------------------------------------
// Memory access operations
// ----------------------------------------------------------------------------

struct AllocateOp;
struct LoadOp;
struct StoreOp;

struct AllocateOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Allocate", specId)

    void init(const Type::Ptr &type);

    OPTREE_ADAPTOR_RESULT(result, 0)

    static bool verify(const Operation *op);
};

struct LoadOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Load", specId)

    void init(const Type::Ptr &resultType, const Value::Ptr &src);
    void init(const Value::Ptr &src);

    OPTREE_ADAPTOR_OPERAND(src, setSrc, 0)
    OPTREE_ADAPTOR_RESULT(result, 0)

    static bool verify(const Operation *op);
};

struct StoreOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Store", specId)

    void init(const Value::Ptr &dst, const Value::Ptr &valueToStore);

    OPTREE_ADAPTOR_OPERAND(dst, setDst, 0)
    OPTREE_ADAPTOR_OPERAND(valueToStore, setValueToStore, 1)

    static bool verify(const Operation *op);
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
    OPTREE_ADAPTOR_HELPER(Adaptor, "If", specId)

    void init(const Value::Ptr &cond, bool withElse = false);

    OPTREE_ADAPTOR_OPERAND(cond, setCond, 0);

    ThenOp thenOp() const;
    ElseOp elseOp() const;

    static bool verify(const Operation *op);
};

struct ThenOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Then", specId)

    void init();

    static bool verify(const Operation *op);
};

struct ElseOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Else", specId)

    void init();

    static bool verify(const Operation *op);
};

struct WhileOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "While", specId)

    void init();

    ConditionOp conditionOp() const;

    static bool verify(const Operation *op);
};

struct ConditionOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Condition", specId)

    void init();

    Value::Ptr terminator() const;

    static bool verify(const Operation *op);
};

struct ForOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "For", specId)

    void init(const Type::Ptr &iteratorType, const Value::Ptr &start, const Value::Ptr &stop, const Value::Ptr &step);

    OPTREE_ADAPTOR_OPERAND(start, setStart, 0)
    OPTREE_ADAPTOR_OPERAND(stop, setStop, 1)
    OPTREE_ADAPTOR_OPERAND(step, setStep, 2)
    OPTREE_ADAPTOR_INWARD(iterator, 0)

    static bool verify(const Operation *op);
};

// ----------------------------------------------------------------------------
// Special operations
// ----------------------------------------------------------------------------

struct InputOp;
struct PrintOp;

struct InputOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Input", specId)

    void init(const Type::Ptr &inputType);

    OPTREE_ADAPTOR_RESULT(value, 0)

    static bool verify(const Operation *op);
};

struct PrintOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Print", specId)

    void init(const Value::Ptr &valueToPrint);

    OPTREE_ADAPTOR_OPERAND(value, setValue, 0)

    static bool verify(const Operation *op);
};

} // namespace optree
