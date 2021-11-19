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

    llvm::Type *createLLVMType(ast::TypeId id);
    void initializeFunctions(const ast::SyntaxTree &tree);
    llvm::BasicBlock *processBranchRoot(ast::Node::Ptr node);

    llvm::Value *visitNode(ast::Node::Ptr node);
    llvm::Value *visitBinaryOperation(ast::Node *node);
    llvm::Value *visitExpression(ast::Node *node);
    llvm::Value *visitFloatingPointLiteralValue(ast::Node *node);
    llvm::Value *visitIntegerLiteralValue(ast::Node *node);
    llvm::Value *visitVariableName(ast::Node *node);

    void processNode(ast::Node::Ptr node);
    void processExpression(ast::Node *node);
    void processFunctionDefinition(ast::Node *node);
    void processIfStatement(ast::Node *node);
    void processProgramRoot(ast::Node *node);
    void processReturnStatement(ast::Node *node);
    void processVariableDeclaration(ast::Node *node);

  public:
    explicit IRGenerator(const std::string &moduleName, bool emitDebugInfo = false);

    void process(const ast::SyntaxTree &tree);
    void writeToFile(const std::string &filename);
    void dump();
};

} // namespace ir_generator
