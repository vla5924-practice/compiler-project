#include "semantizer/semantizer.hpp"

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/optree/program.hpp"

#include "semantizer/semantizer_context.hpp"
#include "semantizer/traits.hpp"

#define VERIFY(ADAPTOR_CLASS_NAME)                                                                                     \
    template <>                                                                                                        \
    bool verify<ADAPTOR_CLASS_NAME>(const ADAPTOR_CLASS_NAME &op, SemantizerContext &ctx)

using namespace optree;
using namespace optree::semantizer;

template <typename ValueRange, typename TypeRange>
bool valuesHaveTypes(ValueRange &&values, TypeRange &&types) {
    return std::equal(std::begin(types), std::end(types), std::begin(values), std::end(values),
                      [](const Type::Ptr &type, const Value::Ptr &value) { return value->hasType(type); });
}

bool verify(const Operation::Ptr &op, SemantizerContext &ctx);

bool verify(const Operation::Body &body, SemantizerContext &ctx) {
    bool verified = true;
    for (const auto &op : body)
        verified &= verify(op, ctx);
    return verified;
}

template <typename AdaptorType>
bool verify(const AdaptorType &op, SemantizerContext &ctx) {
    std::terminate();
    return false;
}

VERIFY(ModuleOp) {
    TraitVerifier verifier(op.op, ctx);
    verifier.verify<HasOperands>(0).verify<HasResults>(0).verify<HasInwards>(0).verify<HasAttributes>(0);
    return verifier && verify(op.op->body, ctx);
}

VERIFY(FunctionOp) {
    TraitVerifier verifier(op.op, ctx);
    verifier.verify<HasOperands>(0)
        .verify<HasResults>(0)
        .verify<HasAttributes>(2)
        .verify<HasNthAttrOfType<std::string>>(0)
        .verify<HasNthAttrOfType<FunctionType>>(1);
    ctx.functions.insert_or_assign(op.name(), op);
    const auto &argTypes = op.type().arguments;
    verifier.verify<HasInwards>(argTypes.size());
    if (!valuesHaveTypes(op.op->inwards, argTypes)) {
        ctx.pushOpError(op.op) << "must have inwards with types of arguments of provided function type";
        return false;
    }
    return verifier;
}

VERIFY(FunctionCallOp) {
    TraitVerifier verifier(op.op, ctx);
    verifier.verify<HasOperands>(0)
        .verify<HasResults>(0)
        .verify<HasInwards>(0)
        .verify<HasAttributes>(1)
        .verify<HasNthAttrOfType<std::string>>(0);
    const auto &name = op.name();
    verifier.verify<HasInwards>(argTypes.size());
    if (!valuesHaveTypes(op.op->inwards, argTypes)) {
        ctx.pushOpError(op.op) << "must have inwards with types of arguments of provided function type";
        return false;
    }
    return verifier;
}

VERIFY(ConstantOp) {
    TraitVerifier verifier(op.op, ctx);
    verifier.verify<HasOperands>(0).verify<HasResults>(1).verify<HasAttributes>(1);
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

bool verify(const Operation::Ptr &op, SemantizerContext &ctx) {
    if (auto mop = Operation::as<ModuleOp>(op))
        return verify(mop, ctx);
    if (auto fop = Operation::as<FunctionOp>(op))
        return verify(fop, ctx);
    return false;
}

void Semantizer::process(const Program &program) {
    process(program.root);
}

void Semantizer::process(const Operation::Ptr &op) {
    SemantizerContext ctx;

    if (!ctx.errors.empty())
        throw ctx.errors;
}
