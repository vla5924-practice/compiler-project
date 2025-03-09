#include "semantizer/semantizer.hpp"

#include <cstddef>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/definitions.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/optree/program.hpp"
#include "compiler/optree/types.hpp"
#include "compiler/optree/value.hpp"
#include "compiler/utils/debug.hpp"

#include "semantizer/semantizer_context.hpp"
#include "semantizer/traits.hpp"

#define VERIFY(ADAPTOR_CLASS_NAME, OP_NAME, CTX_NAME, VERIFIER_NAME)                                                   \
    template <>                                                                                                        \
    bool verify<ADAPTOR_CLASS_NAME>([[maybe_unused]] const ADAPTOR_CLASS_NAME &(OP_NAME),                              \
                                    [[maybe_unused]] SemantizerContext &(CTX_NAME),                                    \
                                    [[maybe_unused]] TraitVerifier &(VERIFIER_NAME))

#define RETURN_ON_FAILURE(EXPR)                                                                                        \
    if (!(EXPR)) {                                                                                                     \
        return false;                                                                                                  \
    }

using namespace optree;
using namespace optree::semantizer;

namespace {

template <typename ValueRange, typename TypeRange>
bool valuesHaveTypes(ValueRange &&values, TypeRange &&types) {
    return (std::empty(values) && std::empty(types)) ||
           std::equal(std::begin(types), std::end(types), std::begin(values), std::end(values),
                      [](const Type::Ptr &type, const Value::Ptr &value) { return value->hasType(type); });
}

template <typename ValueRange>
std::optional<Type::Ptr> valuesHaveSameType(ValueRange &&values) {
    if (std::empty(values))
        return {};
    const auto &type = (*std::begin(values))->type;
    for (const auto &value : values)
        if (!value->hasType(type))
            return {};
    return {type};
}

bool verify(const Operation::Ptr &op, SemantizerContext &ctx);

bool verify(const Operation::Body &body, SemantizerContext &ctx) {
    bool verified = true;
    for (const auto &op : body)
        verified &= verify(op, ctx);
    return verified;
}

template <typename AdaptorType>
bool verify(const AdaptorType &, SemantizerContext &, TraitVerifier &) {
    COMPILER_UNREACHABLE("unexpected call to non-concrete verify function");
}

VERIFY(ModuleOp, op, ctx, verifier) {
    verifier.verify<HasOperands>(0).verify<HasResults>(0).verify<HasInwards>(0).verify<HasAttributes>(0);
    return verifier && verify(op->body, ctx);
}

VERIFY(FunctionOp, op, ctx, verifier) {
    verifier.verify<HasOperands>(0)
        .verify<HasResults>(0)
        .verify<HasAttributes>(2)
        .verify<HasNthAttrOfType<std::string>>(0)
        .verify<HasNthAttrOfType<FunctionType>>(1);
    RETURN_ON_FAILURE(verifier);
    ctx.functions[op.name()] = op;
    const auto &argTypes = op.type().arguments;
    verifier.verify<HasInwards>(argTypes.size());
    if (!valuesHaveTypes(op->inwards, argTypes)) {
        ctx.pushOpError(op) << "must have inwards with types of arguments of provided function type";
        return false;
    }
    return verifier && verify(op->body, ctx);
}

VERIFY(FunctionCallOp, op, ctx, verifier) {
    verifier.verify<HasInwards>(0).verify<HasAttributes>(1).verify<HasNthAttrOfType<std::string>>(0);
    RETURN_ON_FAILURE(verifier);
    const auto &name = op.name();
    auto maybeFunc = ctx.findFunction(name);
    if (!maybeFunc) {
        ctx.pushOpError(op) << "has unknown callee name: " << name;
        return false;
    }
    const auto &funcType = maybeFunc->type();
    verifier.verify<HasResultOfType>(funcType.result);
    if (!valuesHaveTypes(op->operands, funcType.arguments)) {
        ctx.pushOpError(op) << "must have operands with types of arguments of provided function type";
        return false;
    }
    return verifier;
}

VERIFY(ReturnOp, op, ctx, verifier) {
    auto parent = op->findParent<FunctionOp>();
    if (!parent) {
        ctx.pushOpError(op) << "must live within function body";
        return false;
    }
    size_t numOperands = parent.type().result->is<NoneType>() ? 0U : 1U;
    verifier.verify<HasOperands>(numOperands).verify<HasResults>(0).verify<HasInwards>(0).verify<HasAttributes>(0);
    RETURN_ON_FAILURE(verifier);
    if (numOperands > 0 && !op.value()->hasType(parent.type().result)) {
        ctx.pushOpError(op) << "must have operands with result types of parent function type";
        return false;
    }
    return true;
}

VERIFY(ConstantOp, op, ctx, verifier) {
    verifier.verify<HasOperands>(0).verify<HasResults>(1).verify<HasAttributes>(1);
    RETURN_ON_FAILURE(verifier);
    const auto &attr = op.value();
    const auto &type = op.result()->type;
    if (type->is<IntegerType>())
        verifier.verify<HasNthAttrOfType<NativeInt>>(0);
    else if (type->is<BoolType>())
        verifier.verify<HasNthAttrOfType<NativeBool>>(0);
    else if (type->is<FloatType>())
        verifier.verify<HasNthAttrOfType<NativeFloat>>(0);
    else if (type->is<StrType>())
        verifier.verify<HasNthAttrOfType<NativeStr>>(0);
    else {
        ctx.pushOpError(op) << "must have result type one of int, float, bool, str";
        return false;
    }
    return verifier;
}

VERIFY(BinaryOp, op, ctx, verifier) {
    verifier.verify<HasOperands>(2).verify<HasResults>(1).verify<HasInwards>(0);
    return verifier;
}

VERIFY(ArithBinaryOp, op, ctx, verifier) {
    if (!verify(op->as<BinaryOp>(), ctx, verifier))
        return false;
    verifier.verify<HasAttributes>(1).verify<HasNthAttrOfType<ArithBinOpKind>>(0);
    RETURN_ON_FAILURE(verifier);
    if (auto maybeType = valuesHaveSameType(op->operands)) {
        const auto &type = maybeType.value();
        if (!op.result()->hasType(type)) {
            ctx.pushOpError(op) << "result must have type " << type;
            return false;
        }
    } else {
        ctx.pushOpError(op) << "operands must have same type";
        return false;
    }
    return verifier;
}

VERIFY(LogicBinaryOp, op, ctx, verifier) {
    if (!verify(op->as<BinaryOp>(), ctx, verifier))
        return false;
    verifier.verify<HasAttributes>(1).verify<HasNthAttrOfType<LogicBinOpKind>>(0).verify<HasResultOfType>(
        TypeStorage::boolType());
    RETURN_ON_FAILURE(verifier);
    if (!valuesHaveSameType(op->operands)) {
        ctx.pushOpError(op) << "operands must have same type";
        return false;
    }
    return verifier;
}

VERIFY(UnaryOp, op, ctx, verifier) {
    verifier.verify<HasOperands>(1).verify<HasResults>(1).verify<HasInwards>(0);
    return verifier;
}

VERIFY(ArithCastOp, op, ctx, verifier) {
    if (!verify(op->as<UnaryOp>(), ctx, verifier))
        return false;
    verifier.verify<HasAttributes>(1).verify<HasNthAttrOfType<ArithCastOpKind>>(0);
    RETURN_ON_FAILURE(verifier);
    const auto &inType = op.value()->type;
    const auto &outType = op.result()->type;
    bool valid = true;
    std::string_view message;
    switch (op.kind()) {
    case ArithCastOpKind::IntToFloat:
        valid = inType->is<IntegerType>() && outType->is<FloatType>();
        message = "must have int operand and float result";
        break;
    case ArithCastOpKind::FloatToInt:
        valid = inType->is<FloatType>() && outType->is<IntegerType>();
        message = "must have float operand and int result";
        break;
    case ArithCastOpKind::ExtI:
        valid = inType->is<IntegerType>() && outType->is<IntegerType>() && outType->bitWidth() > inType->bitWidth();
        message = "must have int operand and int result with greater bitwidth";
        break;
    case ArithCastOpKind::TruncI:
        valid = inType->is<IntegerType>() && outType->is<IntegerType>() && outType->bitWidth() < inType->bitWidth();
        message = "must have int operand and int result with less bitwidth";
        break;
    case ArithCastOpKind::ExtF:
        valid = inType->is<FloatType>() && outType->is<FloatType>() && outType->bitWidth() > inType->bitWidth();
        message = "must have float operand and float result with greater bitwidth";
        break;
    case ArithCastOpKind::TruncF:
        valid = inType->is<FloatType>() && outType->is<FloatType>() && outType->bitWidth() < inType->bitWidth();
        message = "must have float operand and float result with less bitwidth";
        break;
    default:
        ctx.pushOpError(op) << "has unknown cast kind";
        return false;
    };
    if (valid)
        return true;
    ctx.pushOpError(op) << message << ", but got " << inType << " operand and " << outType << " result";
    return false;
}

VERIFY(LogicUnaryOp, op, ctx, verifier) {
    if (!verify(op->as<UnaryOp>(), ctx, verifier))
        return false;
    verifier.verify<HasOperandsOfType>(1, TypeStorage::boolType())
        .verify<HasAttributes>(1)
        .verify<HasNthAttrOfType<LogicUnaryOpKind>>(0);
    return verifier;
}

VERIFY(AllocateOp, op, ctx, verifier) {
    verifier.verify<HasOperands>(0).verify<HasResults>(1).verify<HasInwards>(0).verify<HasAttributes>(0);
    RETURN_ON_FAILURE(verifier);
    const auto &type = op.result()->type;
    if (type->is<PointerType>())
        return true;
    ctx.pushOpError(op) << "must have pointer result";
    return false;
}

VERIFY(LoadOp, op, ctx, verifier) {
    verifier.verify<HasOperands>(1).verify<HasResults>(1).verify<HasInwards>(0).verify<HasAttributes>(0);
    RETURN_ON_FAILURE(verifier);
    if (op.src()->canPointTo(op.result()))
        return true;
    ctx.pushOpError(op) << "must have source operand type as pointer to result type";
    return false;
}

VERIFY(StoreOp, op, ctx, verifier) {
    verifier.verify<HasOperands>(2).verify<HasResults>(0).verify<HasInwards>(0).verify<HasAttributes>(0);
    RETURN_ON_FAILURE(verifier);
    if (op.dst()->canPointTo(op.valueToStore()))
        return true;
    ctx.pushOpError(op) << "must have destination operand type as pointer to value to store operand type";
    return false;
}

VERIFY(IfOp, op, ctx, verifier) {
    verifier.verify<HasOperandsOfType>(1, TypeStorage::boolType())
        .verify<HasResults>(0)
        .verify<HasInwards>(0)
        .verify<HasAttributes>(0);
    RETURN_ON_FAILURE(verifier);
    if (op->body.size() >= 1 && op->body.front()->is<ThenOp>()) {
        if (op->body.size() == 1)
            return verify(op.thenOp(), ctx);
        if (op->body.size() == 2 && op->body.back()->is<ElseOp>())
            return verify(op.thenOp(), ctx) && verify(op.elseOp(), ctx);
    }
    ctx.pushOpError(op) << "must have one operation (ThenOp) or two operations (ThenOp, ElseOp) within body";
    return false;
}

VERIFY(ThenOp, op, ctx, verifier) {
    verifier.verify<HasOperands>(0).verify<HasResults>(0).verify<HasInwards>(0).verify<HasAttributes>(0);
    RETURN_ON_FAILURE(verifier);
    if (op->parent) {
        if (!op->parent->is<IfOp>() || op->parent->body.front() != op.op) {
            ctx.pushOpError(op) << "must be first operation within body of parent IfOp";
            return false;
        }
    }
    return verify(op->body, ctx);
}

VERIFY(ElseOp, op, ctx, verifier) {
    verifier.verify<HasOperands>(0).verify<HasResults>(0).verify<HasInwards>(0).verify<HasAttributes>(0);
    RETURN_ON_FAILURE(verifier);
    if (op->parent) {
        if (!op->parent->is<IfOp>() || op->parent->body.back() != op.op) {
            ctx.pushOpError(op) << "must be last operation within body of parent IfOp";
            return false;
        }
    }
    return verify(op->body, ctx);
}

VERIFY(WhileOp, op, ctx, verifier) {
    verifier.verify<HasOperands>(0).verify<HasResults>(0).verify<HasInwards>(0).verify<HasAttributes>(0);
    RETURN_ON_FAILURE(verifier);
    if (op->body.size() >= 1 && op->body.front()->is<ConditionOp>())
        return verify(op->body, ctx);
    ctx.pushOpError(op) << "must have one operation (ConditionOp) within body";
    return false;
}

VERIFY(ConditionOp, op, ctx, verifier) {
    verifier.verify<HasOperands>(0).verify<HasResults>(0).verify<HasInwards>(0).verify<HasAttributes>(0);
    RETURN_ON_FAILURE(verifier);
    if (op->parent && (!op->parent->is<WhileOp>() || op->parent->body.front() != op.op)) {
        ctx.pushOpError(op) << "must have first operation within parent WhileOp";
        return false;
    }
    if (op->body.size() < 1) {
        ctx.pushOpError(op) << "must have at least one operation within body";
        return false;
    }
    const auto &lastOp = op->body.back();
    if (lastOp->numResults() != 1 || !lastOp->result(0)->type->is<BoolType>()) {
        ctx.pushOpError(op) << "must have operation with one bool result as last within body";
        return false;
    }
    return true;
}

VERIFY(ForOp, op, ctx, verifier) {
    verifier.verify<HasOperandsOfType>(3, TypeStorage::integerType())
        .verify<HasResults>(0)
        .verify<HasInwardsOfType>(1, TypeStorage::integerType())
        .verify<HasAttributes>(0);
    RETURN_ON_FAILURE(verifier);
    return verify(op->body, ctx);
}

VERIFY(InputOp, op, ctx, verifier) {
    verifier.verify<HasOperands>(1).verify<HasResults>(0).verify<HasInwards>(0).verify<HasAttributes>(0);
    RETURN_ON_FAILURE(verifier);
    if (op.dst()->type->is<PointerType>())
        return true;
    ctx.pushOpError(op) << "must have one pointer operand";
    return false;
}

VERIFY(PrintOp, op, ctx, verifier) {
    verifier.verify<HasResults>(0).verify<HasInwards>(0).verify<HasAttributes>(0);
    return verifier;
}

bool verify(const Operation::Ptr &op, SemantizerContext &ctx) {
    TraitVerifier verifier(op, ctx);
    if (auto concreteOp = op->as<ModuleOp>())
        return verify(concreteOp, ctx, verifier);
    if (auto concreteOp = op->as<FunctionOp>())
        return verify(concreteOp, ctx, verifier);
    if (auto concreteOp = op->as<FunctionCallOp>())
        return verify(concreteOp, ctx, verifier);
    if (auto concreteOp = op->as<ReturnOp>())
        return verify(concreteOp, ctx, verifier);
    if (auto concreteOp = op->as<ConstantOp>())
        return verify(concreteOp, ctx, verifier);
    if (auto concreteOp = op->as<ArithBinaryOp>())
        return verify(concreteOp, ctx, verifier);
    if (auto concreteOp = op->as<LogicBinaryOp>())
        return verify(concreteOp, ctx, verifier);
    if (auto concreteOp = op->as<ArithCastOp>())
        return verify(concreteOp, ctx, verifier);
    if (auto concreteOp = op->as<LogicUnaryOp>())
        return verify(concreteOp, ctx, verifier);
    if (auto concreteOp = op->as<AllocateOp>())
        return verify(concreteOp, ctx, verifier);
    if (auto concreteOp = op->as<LoadOp>())
        return verify(concreteOp, ctx, verifier);
    if (auto concreteOp = op->as<StoreOp>())
        return verify(concreteOp, ctx, verifier);
    if (auto concreteOp = op->as<IfOp>())
        return verify(concreteOp, ctx, verifier);
    if (auto concreteOp = op->as<ThenOp>())
        return verify(concreteOp, ctx, verifier);
    if (auto concreteOp = op->as<ElseOp>())
        return verify(concreteOp, ctx, verifier);
    if (auto concreteOp = op->as<WhileOp>())
        return verify(concreteOp, ctx, verifier);
    if (auto concreteOp = op->as<ConditionOp>())
        return verify(concreteOp, ctx, verifier);
    if (auto concreteOp = op->as<ForOp>())
        return verify(concreteOp, ctx, verifier);
    if (auto concreteOp = op->as<InputOp>())
        return verify(concreteOp, ctx, verifier);
    if (auto concreteOp = op->as<PrintOp>())
        return verify(concreteOp, ctx, verifier);
    ctx.pushOpError(op) << "is not registered for verification";
    return false;
}

} // namespace

void Semantizer::process(const Program &program) {
    process(program.root);
}

void Semantizer::process(const Operation::Ptr &op) {
    SemantizerContext ctx;
    verify(op, ctx);
    if (!ctx.errors.empty())
        throw ctx.errors;
}
