#pragma once

#include <list>
#include <memory>
#include <string>
#include <unordered_map>

#pragma warning(push)
#pragma warning(disable : 4624)
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#pragma warning(pop)

#include <ast/syntax_tree.hpp>

namespace ir_generator {

class IRGenerator {
    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;

    llvm::BasicBlock *currentBlock;
    llvm::Function *currentFunction;
    std::list<std::unordered_map<std::string, llvm::AllocaInst *>> localVariables;
    std::unordered_map<std::string, llvm::Function *> functions;

    llvm::Value *visit(ast::Node::Ptr node);
    llvm::Value *visitIntegerLiteralValue(ast::Node *node);
    llvm::Value *visitFloatingPointLiteralValue(ast::Node *node);
    llvm::Value *visitBinaryOperation(ast::Node *node);
    llvm::Value *visitFunctionDefinition(ast::Node *node);
    llvm::Value *visitVariableDeclaration(ast::Node *node);
    llvm::Value *visitIfStatement(ast::Node *node);
    llvm::Value *visitVariableName(ast::Node *node);
    llvm::Value *visitExpression(ast::Node *node);
    llvm::Value *visitReturnStatement(ast::Node *node);
    llvm::Value *visitProgramRoot(ast::Node *node);

    llvm::BasicBlock *visitBranchRoot(ast::Node::Ptr node);

    void initializeFunctions(const ast::SyntaxTree &tree);
    llvm::Type *createLLVMType(ast::TypeId id);

  public:
    explicit IRGenerator(const std::string &moduleName, bool emitDebugInfo = false);

    void process(const ast::SyntaxTree &tree);
    void writeToFile(const std::string &filename);
    void dump();
};

} // namespace ir_generator
