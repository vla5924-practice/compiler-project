#pragma once

#include <deque>
#include <string>
#include <string_view>
#include <unordered_map>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/NoFolder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/optree/program.hpp"
#include "compiler/optree/types.hpp"
#include "compiler/optree/value.hpp"

namespace optree {
namespace llvmir_generator {

class LLVMIRGenerator {
    using IRBuilder = llvm::IRBuilder<llvm::NoFolder>;

    llvm::LLVMContext context;
    IRBuilder builder;
    llvm::Module mod;
    llvm::Function *currentFunction;
    std::unordered_map<const Value *, llvm::Value *> values;
    std::unordered_map<llvm::Value *, llvm::Type *> typedValues;
    std::unordered_map<std::string, llvm::Value *> globalStrings;
    std::unordered_map<std::string_view, llvm::FunctionCallee> externalFunctions;
    std::deque<llvm::BasicBlock *> basicBlocks;

    llvm::Value *findValue(const Value::Ptr &value) const;
    void saveValue(const Value::Ptr &value, llvm::Value *llvmValue);
    llvm::Type *convertType(const Type::Ptr &type);
    llvm::BasicBlock *createBlock();
    void eraseDeadBlocks();
    llvm::Value *normalizePredicate(const Value::Ptr &cond);
    llvm::Value *getGlobalString(const std::string &str);
    llvm::FunctionCallee getExternalFunction(std::string_view name);
    llvm::FunctionCallee loadExternalFunction(std::string_view name);

    void visit(const Operation::Ptr &op);
    void visitBody(const Operation::Ptr &op);

    void visit(const ModuleOp &op);
    void visit(const FunctionOp &op);
    void visit(const FunctionCallOp &op);
    void visit(const ReturnOp &op);
    void visit(const ConstantOp &op);

    void visit(const ArithBinaryOp &op);
    void visit(const LogicBinaryOp &op);
    void visit(const ArithCastOp &op);
    void visit(const ArithUnaryOp &op);
    void visit(const LogicUnaryOp &op);

    void visit(const AllocateOp &op);
    void visit(const LoadOp &op);
    void visit(const StoreOp &op);

    void visit(const IfOp &op);
    void visit(const ThenOp &op);
    void visit(const ElseOp &op);
    void visit(const WhileOp &op);
    void visit(const ConditionOp &op);
    void visit(const ForOp &op);

    void visit(const InputOp &op);
    void visit(const PrintOp &op);

  public:
    LLVMIRGenerator() = delete;
    LLVMIRGenerator(const LLVMIRGenerator &) = delete;
    LLVMIRGenerator(LLVMIRGenerator &&) = delete;
    ~LLVMIRGenerator() = default;

    explicit LLVMIRGenerator(const std::string &moduleName);

    void process(const Program &program);

    std::string dump() const;
    void dump(llvm::raw_ostream &stream) const;
    void dumpToFile(const std::string &filename) const;
};

} // namespace llvmir_generator
} // namespace optree
