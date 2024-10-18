#include "semantizer/semantizer.hpp"

#include <optional>

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/optree/program.hpp"
#include "compiler/optree/helpers.hpp"
#include "compiler/utils/helpers.hpp"

#include "semantizer/semantizer_context.hpp"
#include "semantizer/traits.hpp"

#define VERIFY(ADAPTOR_CLASS_NAME, OP_NAME, CTX_NAME, VERIFIER_NAME)                                                   \
    template <>                                                                                                        \
    bool verify<ADAPTOR_CLASS_NAME>([[maybe_unused]] const ADAPTOR_CLASS_NAME &OP_NAME,                                \
                                    [[maybe_unused]] SemantizerContext &CTX_NAME,                                      \
                                    [[maybe_unused]] TraitVerifier &VERIFIER_NAME)

#define RETURN_ON_FAILURE(EXPR)                                                                                        \
    if (!(EXPR)) {                                                                                                     \
        return false;                                                                                                  \
    }

using namespace optree;
using namespace optree::semantizer;

namespace {

template <typename ValueRange, typename TypeRange>
bool valuesHaveTypes(ValueRange &&values, TypeRange &&types) {
    return std::equal(std::begin(types), std::end(types), std::begin(values), std::end(values),
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
    return verifier && verify(op.op->body, ctx);
}

VERIFY(FunctionOp, op, ctx, verifier) {
    verifier.verify<HasOperands>(0)
        .verify<HasResults>(0)
        .verify<HasAttributes>(2)
        .verify<HasNthAttrOfType<std::string>>(0)
        .verify<HasNthAttrOfType<FunctionType>>(1);
    RETURN_ON_FAILURE(verifier);
    ctx.functions.insert_or_assign(op.name(), op);
    const auto &argTypes = op.type().arguments;
    verifier.verify<HasInwards>(argTypes.size());
    if (!valuesHaveTypes(op.op->inwards, argTypes)) {
        ctx.pushOpError(op.op) << "must have inwards with types of arguments of provided function type";
        return false;
    }
    return verifier;
}

VERIFY(FunctionCallOp, op, ctx, verifier) {
    verifier.verify<HasOperands>(0)
        .verify<HasInwards>(0)
        .verify<HasAttributes>(1)
        .verify<HasNthAttrOfType<std::string>>(0);
    RETURN_ON_FAILURE(verifier);
    const auto &name = op.name();
    auto maybeFunc = ctx.findFunction(name);
    if (!maybeFunc) {
        ctx.pushOpError(op.op) << "has unknown callee name: " << name;
        return false;
    }
    const auto &funcType = maybeFunc->type();
    if (funcType.result->is<NoneType>())
        verifier.verify<HasResults>(0);
    else
        verifier.verify<HasResultOfType>(funcType.result);
    if (!valuesHaveTypes(op.op->operands, funcType.arguments)) {
        ctx.pushOpError(op.op) << "must have operands with types of arguments of provided function type";
        return false;
    }
    return verifier;
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
        ctx.pushOpError(op.op) << "must have result type one of int, float, bool, str";
        return false;
    }
    return verifier;
}

VERIFY(BinaryOp, op, ctx, verifier) {
    verifier.verify<HasOperands>(2).verify<HasResults>(1).verify<HasInwards>(0);
    return verifier;
}

VERIFY(ArithBinaryOp, op, ctx, verifier) {
    if (!verify(Operation::as<BinaryOp>(op.op), ctx, verifier))
        return false;
    TraitVerifier verifier(op.op, ctx);
    verifier.verify<HasAttributes>(1).verify<HasNthAttrOfType<ArithBinOpKind>>(0);
    RETURN_ON_FAILURE(verifier);
    if (auto maybeType = valuesHaveSameType(op.op->operands)) {
        const auto &type = maybeType.value();
        if (!op.result()->hasType(type)) {
            ctx.pushOpError(op.op) << "result must have type " << type;
            return false;
        }
    } else {
        ctx.pushOpError(op.op) << "operands must have same type ";
        return false;
    }
    return verifier;
}

bool verify(const Operation::Ptr &op, SemantizerContext &ctx) {
    TraitVerifier verifier(op, ctx);
    if (auto mop = Operation::as<ModuleOp>(op))
        return verify(mop, ctx, verifier);
    if (auto fop = Operation::as<FunctionOp>(op))
        return verify(fop, ctx, verifier);
    if (auto fop = Operation::as<FunctionCallOp>(op))
        return verify(fop, ctx, verifier);
    if (auto fop = Operation::as<ConstantOp>(op))
        return verify(fop, ctx, verifier);
    if (auto fop = Operation::as<ArithBinaryOp>(op))
        return verify(fop, ctx, verifier);
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
