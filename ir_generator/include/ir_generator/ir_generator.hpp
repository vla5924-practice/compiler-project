#pragma once

#include <memory>
#include <string>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <ast/syntax_tree.hpp>

namespace ir_generator {

class IRGenerator {
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    llvm::LLVMContext context;

  public:
    explicit IRGenerator(const std::string &moduleName, bool emitDebugInfo = false);

    void process(const ast::SyntaxTree &tree);
    void writeToFile(const std::string &filename);
};

} // namespace ir_generator
