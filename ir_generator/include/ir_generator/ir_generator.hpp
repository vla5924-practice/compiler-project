#pragma once

#include <memory>
#include <string>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <ast/syntax_tree.hpp>

namespace ir_generator {

class IRGenerator {
    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;

    llvm::Value *visit(ast::Node::Ptr node);
    llvm::Value *visitIntegerLiteralValue(ast::Node *node);
    llvm::Value *visitBinaryOperation(ast::Node *node);
    llvm::Value *visitFunctionDefinition(ast::Node *node);

  public:
    explicit IRGenerator(const std::string &moduleName, bool emitDebugInfo = false);

    void process(const ast::SyntaxTree &tree);
    void writeToFile(const std::string &filename);
    void dump();
};

} // namespace ir_generator
