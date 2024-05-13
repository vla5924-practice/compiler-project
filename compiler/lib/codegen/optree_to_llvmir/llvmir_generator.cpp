#include "llvmir_generator.hpp"

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/definitions.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/optree/program.hpp"
#include "compiler/optree/types.hpp"
#include "compiler/optree/value.hpp"
#include "compiler/utils/helpers.hpp"

using namespace optree;
using namespace optree::llvmir_generator;

namespace {

namespace external {

constexpr std::string_view printf = "printf";
constexpr std::string_view scanf = "scanf";

} // namespace external

std::string_view getFormatSpecifier(const Type::Ptr &type) {
    if (type->is<IntegerType>())
        return "%lld";
    if (type->is<FloatType>())
        return "%lf";
    if (type->is<StrType>())
        return "%s";
    if (type->is<NoneType>())
        return "None";
    if (type->is<PointerType>())
        return "%p";
    COMPILER_UNREACHABLE("unexpected type for a format specifier");
}

} // namespace

LLVMIRGenerator::LLVMIRGenerator(const std::string &moduleName)
    : context(), builder(context), mod(moduleName, context), currentFunction(nullptr) {
}

llvm::Value *LLVMIRGenerator::findValue(const Value::Ptr &value) const {
    auto it = values.find(value.get());
    if (it == values.end())
        return nullptr;
    return it->second;
}

void LLVMIRGenerator::saveValue(const Value::Ptr &value, llvm::Value *llvmValue) {
    values[value.get()] = llvmValue;
}

llvm::Type *LLVMIRGenerator::convertType(const Type::Ptr &type) {
    if (type->is<NoneType>())
        return llvm::Type::getVoidTy(context);
    if (type->is<BoolType>())
        return llvm::Type::getInt1Ty(context);
    if (type->is<IntegerType>())
        return llvm::Type::getIntNTy(context, type->bitWidth());
    if (type->is<FloatType>())
        return llvm::Type::getDoubleTy(context);
    if (type->is<StrType>())
        return llvm::Type::getIntNPtrTy(context, type->as<StrType>().charWidth);
    if (type->is<PointerType>())
        return llvm::PointerType::getUnqual(context);
    COMPILER_UNREACHABLE("unexpected type");
}

llvm::BasicBlock *LLVMIRGenerator::createBlock() {
    return llvm::BasicBlock::Create(context, "bb", currentFunction);
}

llvm::Value *LLVMIRGenerator::normalizePredicate(const Value::Ptr &cond) {
    auto *pred = findValue(cond);
    if (!pred)
        return nullptr;
    if (pred->getType()->isIntegerTy(1))
        return pred;
    auto *i1Type = llvm::Type::getInt1Ty(context);
    auto *trunc = builder.CreateTrunc(pred, i1Type);
    return builder.CreateICmpEQ(trunc, llvm::ConstantInt::get(i1Type, 1U));
}

llvm::Value *LLVMIRGenerator::getGlobalString(const std::string &str) {
    auto it = globalStrings.find(str);
    if (it != globalStrings.end())
        return it->second;
    return globalStrings[str] = builder.CreateGlobalString(str);
}

llvm::FunctionCallee LLVMIRGenerator::getExternalFunction(std::string_view name) {
    auto it = externalFunctions.find(name);
    if (it != externalFunctions.end())
        return it->second;
    return externalFunctions[name] = loadExternalFunction(name);
}

llvm::FunctionCallee LLVMIRGenerator::loadExternalFunction(std::string_view name) {
    auto createVarArgFunction = [&](std::string_view name) {
        std::vector<llvm::Type *> arguments = {llvm::PointerType::getUnqual(context)};
        auto *llvmType = llvm::FunctionType::get(llvm::Type::getInt32Ty(context),
                                                 {llvm::PointerType::getUnqual(context)}, /*isVarArg*/ true);
        return mod.getOrInsertFunction(name, llvmType);
    };

    if (name == external::printf)
        return createVarArgFunction(name);
    if (name == external::scanf)
        return createVarArgFunction(name);
    COMPILER_UNREACHABLE("unexpected external function");
}

void LLVMIRGenerator::visit(const Operation::Ptr &op) {
    if (auto concreteOp = op->as<ModuleOp>())
        return visit(concreteOp);
    if (auto concreteOp = op->as<FunctionOp>())
        return visit(concreteOp);
    if (auto concreteOp = op->as<FunctionCallOp>())
        return visit(concreteOp);
    if (auto concreteOp = op->as<ReturnOp>())
        return visit(concreteOp);
    if (auto concreteOp = op->as<ConstantOp>())
        return visit(concreteOp);
    if (auto concreteOp = op->as<ArithBinaryOp>())
        return visit(concreteOp);
    if (auto concreteOp = op->as<LogicBinaryOp>())
        return visit(concreteOp);
    if (auto concreteOp = op->as<ArithCastOp>())
        return visit(concreteOp);
    if (auto concreteOp = op->as<LogicUnaryOp>())
        return visit(concreteOp);
    if (auto concreteOp = op->as<AllocateOp>())
        return visit(concreteOp);
    if (auto concreteOp = op->as<LoadOp>())
        return visit(concreteOp);
    if (auto concreteOp = op->as<StoreOp>())
        return visit(concreteOp);
    if (auto concreteOp = op->as<IfOp>())
        return visit(concreteOp);
    if (auto concreteOp = op->as<ThenOp>())
        return visit(concreteOp);
    if (auto concreteOp = op->as<ElseOp>())
        return visit(concreteOp);
    if (auto concreteOp = op->as<WhileOp>())
        return visit(concreteOp);
    if (auto concreteOp = op->as<ConditionOp>())
        return visit(concreteOp);
    if (auto concreteOp = op->as<ForOp>())
        return visit(concreteOp);
    if (auto concreteOp = op->as<InputOp>())
        return visit(concreteOp);
    if (auto concreteOp = op->as<PrintOp>())
        return visit(concreteOp);
    COMPILER_UNREACHABLE("unexpected operation");
}

void LLVMIRGenerator::visitBody(const Operation::Ptr &op) {
    for (const auto &inner : op->body)
        visit(inner);
}

void LLVMIRGenerator::visit(const ModuleOp &op) {
    for (const auto &inner : op->body)
        visit(inner);
}

void LLVMIRGenerator::visit(const FunctionOp &op) {
    const auto &funcType = op.type();
    std::vector<llvm::Type *> arguments;
    for (const auto &arg : funcType.arguments)
        arguments.push_back(convertType(arg));
    auto *llvmType = llvm::FunctionType::get(convertType(funcType.result), arguments, /*isVarArg*/ false);
    currentFunction = llvm::cast<llvm::Function>(mod.getOrInsertFunction(op.name(), llvmType).getCallee());
    for (size_t i = 0; i < op->numInwards(); i++)
        saveValue(op->inward(i), currentFunction->getArg(i));
    auto *bb = createBlock();
    builder.SetInsertPoint(bb);
    visitBody(op);
}

void LLVMIRGenerator::visit(const FunctionCallOp &op) {
    std::vector<llvm::Value *> arguments;
    for (const auto &arg : op->operands)
        arguments.push_back(findValue(arg));
    auto *inst = builder.CreateCall(mod.getFunction(op.name()), arguments);
    if (op->numResults() != 0)
        saveValue(op.result(), inst);
}

void LLVMIRGenerator::visit(const ReturnOp &op) {
    if (op->numResults() == 0)
        builder.CreateRetVoid();
    else
        builder.CreateRet(findValue(op.value()));
}

void LLVMIRGenerator::visit(const ConstantOp &op) {
    const auto &type = op.result()->type;
    auto result = [&](llvm::Value *v) -> void { saveValue(op.result(), v); };
    if (type->is<BoolType>())
        return result(llvm::ConstantInt::get(convertType(type), op.value().as<bool>()));
    if (type->is<IntegerType>()) {
        auto num = static_cast<int64_t>(op.value().as<NativeInt>());
        auto *value = llvm::ConstantInt::get(convertType(type), reinterpret_cast<uint64_t &>(num), /*IsSigned*/ true);
        return result(value);
    }
    if (type->is<FloatType>())
        return result(llvm::ConstantFP::get(convertType(type), static_cast<double>(op.value().as<NativeFloat>())));
    if (type->is<StrType>())
        return result(getGlobalString(op.value().as<NativeStr>()));
    COMPILER_UNREACHABLE("unexpected result type in ConstantOp");
}

void LLVMIRGenerator::visit(const ArithBinaryOp &op) {
    auto result = [&](llvm::Value *v) -> void { saveValue(op.result(), v); };
    auto *lhs = findValue(op.lhs());
    auto *rhs = findValue(op.rhs());
    switch (op.kind()) {
    case ArithBinOpKind::AddI:
        return result(builder.CreateAdd(lhs, rhs));
    case ArithBinOpKind::SubI:
        return result(builder.CreateSub(lhs, rhs));
    case ArithBinOpKind::MulI:
        return result(builder.CreateMul(lhs, rhs));
    case ArithBinOpKind::DivI:
        return result(builder.CreateSDiv(lhs, rhs));
    case ArithBinOpKind::AddF:
        return result(builder.CreateFAdd(lhs, rhs));
    case ArithBinOpKind::SubF:
        return result(builder.CreateFSub(lhs, rhs));
    case ArithBinOpKind::MulF:
        return result(builder.CreateFMul(lhs, rhs));
    case ArithBinOpKind::DivF:
        return result(builder.CreateFDiv(lhs, rhs));
    default:
        COMPILER_UNREACHABLE("unexpected kind in ArithBinaryOp");
    }
}

void LLVMIRGenerator::visit(const LogicBinaryOp &op) {
    const auto &type = op.lhs()->type;
    auto result = [&](llvm::Value *v) -> void { saveValue(op.result(), v); };
    auto *lhs = findValue(op.lhs());
    auto *rhs = findValue(op.rhs());
    switch (op.kind()) {
    case LogicBinOpKind::Equal:
        if (type->is<IntegerType>())
            return result(builder.CreateICmpEQ(lhs, rhs));
        return result(builder.CreateFCmpOEQ(lhs, rhs));
    case LogicBinOpKind::NotEqual:
        if (type->is<IntegerType>())
            return result(builder.CreateICmpNE(lhs, rhs));
        return result(builder.CreateFCmpONE(lhs, rhs));
    case LogicBinOpKind::AndI:
        return result(builder.CreateLogicalAnd(lhs, rhs));
    case LogicBinOpKind::OrI:
        return result(builder.CreateLogicalOr(lhs, rhs));
    case LogicBinOpKind::LessI:
        return result(builder.CreateICmpSLT(lhs, rhs));
    case LogicBinOpKind::GreaterI:
        return result(builder.CreateICmpSGT(lhs, rhs));
    case LogicBinOpKind::LessEqualI:
        return result(builder.CreateICmpSLE(lhs, rhs));
    case LogicBinOpKind::GreaterEqualI:
        return result(builder.CreateICmpSGE(lhs, rhs));
    case LogicBinOpKind::LessF:
        return result(builder.CreateFCmpOLT(lhs, rhs));
    case LogicBinOpKind::GreaterF:
        return result(builder.CreateFCmpOGT(lhs, rhs));
    case LogicBinOpKind::LessEqualF:
        return result(builder.CreateFCmpOLE(lhs, rhs));
    case LogicBinOpKind::GreaterEqualF:
        return result(builder.CreateFCmpOGE(lhs, rhs));
    default:
        COMPILER_UNREACHABLE("unexpected kind in LogicBinaryOp");
    }
}

void LLVMIRGenerator::visit(const ArithCastOp &op) {
    auto result = [&](llvm::Value *v) -> void { saveValue(op.result(), v); };
    auto *operand = findValue(op.value());
    auto *destType = convertType(op.result()->type);
    switch (op.kind()) {
    case ArithCastOpKind::IntToFloat:
        return result(builder.CreateSIToFP(operand, destType));
    case ArithCastOpKind::FloatToInt:
        return result(builder.CreateFPToSI(operand, destType));
    case ArithCastOpKind::ExtI:
        return result(builder.CreateSExt(operand, destType));
    case ArithCastOpKind::TruncI:
        return result(builder.CreateTrunc(operand, destType));
    case ArithCastOpKind::ExtF:
        return result(builder.CreateFPExt(operand, destType));
    case ArithCastOpKind::TruncF:
        return result(builder.CreateFPTrunc(operand, destType));
    default:
        COMPILER_UNREACHABLE("unexpected kind in ArithCastOp");
    }
}

void LLVMIRGenerator::visit(const LogicUnaryOp &op) {
    auto result = [&](llvm::Value *v) -> void { saveValue(op.result(), v); };
    auto *operand = findValue(op.value());
    switch (op.kind()) {
    case LogicUnaryOpKind::Not:
        return result(builder.CreateNot(operand));
    default:
        COMPILER_UNREACHABLE("unexpected kind in LogicUnaryOp");
    }
}

void LLVMIRGenerator::visit(const AllocateOp &op) {
    const auto &pointee = op.result()->type->as<PointerType>().pointee;
    auto *llvmType = convertType(pointee);
    auto *inst = builder.CreateAlloca(llvmType);
    saveValue(op.result(), inst);
    typedValues[inst] = llvmType;
}

void LLVMIRGenerator::visit(const LoadOp &op) {
    auto *ptr = findValue(op.src());
    saveValue(op.result(), builder.CreateLoad(typedValues[ptr], ptr));
}

void LLVMIRGenerator::visit(const StoreOp &op) {
    builder.CreateStore(findValue(op.valueToStore()), findValue(op.dst()));
}

void LLVMIRGenerator::visit(const IfOp &op) {
    auto *prevBlock = builder.GetInsertBlock();
    auto *thenBlock = createBlock();
    builder.SetInsertPoint(thenBlock);
    visit(op.thenOp());
    auto *elseBlock = createBlock();
    auto *nextBlock = elseBlock;
    if (auto elseOp = op.elseOp()) {
        builder.SetInsertPoint(elseBlock);
        visit(elseOp);
        nextBlock = createBlock();
        builder.CreateBr(nextBlock);
    }
    builder.SetInsertPoint(thenBlock);
    builder.CreateBr(nextBlock);
    builder.SetInsertPoint(prevBlock);
    builder.CreateCondBr(normalizePredicate(op.cond()), thenBlock, elseBlock);
    builder.SetInsertPoint(nextBlock);
}

void LLVMIRGenerator::visit(const ThenOp &op) {
    visitBody(op);
}

void LLVMIRGenerator::visit(const ElseOp &op) {
    visitBody(op);
}

void LLVMIRGenerator::visit(const WhileOp &op) {
    auto *condBlock = createBlock();
    builder.CreateBr(condBlock);
    builder.SetInsertPoint(condBlock);
    auto condOp = op.conditionOp();
    visit(condOp);
    auto *thenBlock = createBlock();
    auto *nextBlock = createBlock();
    builder.CreateCondBr(normalizePredicate(condOp.terminator()), thenBlock, nextBlock);
    builder.SetInsertPoint(thenBlock);
    for (auto it = std::next(op->body.begin()); it != op->body.end(); ++it)
        visit(*it);
    builder.CreateBr(condBlock);
    builder.SetInsertPoint(nextBlock);
}

void LLVMIRGenerator::visit(const ConditionOp &op) {
    visitBody(op);
}

void LLVMIRGenerator::visit(const ForOp &op) {
    auto *llvmType = convertType(op.start()->type);
    auto *allocaI = builder.CreateAlloca(llvmType);
    builder.CreateStore(findValue(op.start()), allocaI);
    auto *condBlock = createBlock();
    builder.CreateBr(condBlock);
    builder.SetInsertPoint(condBlock);
    auto *loadedI = builder.CreateLoad(llvmType, allocaI);
    auto *cond = builder.CreateICmpSLT(loadedI, findValue(op.stop()));
    auto *thenBlock = createBlock();
    auto *nextBlock = createBlock();
    builder.CreateCondBr(cond, thenBlock, nextBlock);
    builder.SetInsertPoint(thenBlock);
    visitBody(op);
    auto *nextI = builder.CreateAdd(loadedI, findValue(op.step()));
    builder.CreateStore(nextI, allocaI);
    builder.CreateBr(condBlock);
    builder.SetInsertPoint(nextBlock);
}

void LLVMIRGenerator::visit(const InputOp &op) {
    std::string format(getFormatSpecifier(op.dst()->type->as<PointerType>().pointee));
    builder.CreateCall(getExternalFunction(external::scanf), {getGlobalString(format), findValue(op.dst())});
}

void LLVMIRGenerator::visit(const PrintOp &op) {
    std::vector<llvm::Value *> arguments;
    arguments.reserve(op->numOperands() + 1U);
    arguments.push_back(nullptr);
    std::string format;
    format.reserve(4U * op->numOperands());
    for (const auto &operand : op->operands) {
        arguments.push_back(findValue(operand));
        format += getFormatSpecifier(operand->type);
    }
    arguments.front() = getGlobalString(format);
    builder.CreateCall(getExternalFunction(external::printf), arguments);
}

void LLVMIRGenerator::process(const Program &program) {
    visit(program.root);
}

std::string LLVMIRGenerator::dump() const {
    std::string str;
    llvm::raw_string_ostream os(str);
    dump(os);
    return str;
}

void LLVMIRGenerator::dump(llvm::raw_ostream &stream) const {
    mod.print(stream, nullptr);
}

void LLVMIRGenerator::dumpToFile(const std::string &filename) const {
    std::error_code error;
    llvm::raw_fd_ostream os(filename, error);
    dump(os);
    os.close();
}
