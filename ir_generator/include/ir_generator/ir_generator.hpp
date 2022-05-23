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
  public:
    using NodeVisitor = llvm::Value *(IRGenerator::*)(ast::Node *);

    explicit IRGenerator(const std::string &moduleName, bool emitDebugInfo = false);

    void process(const ast::SyntaxTree &tree);
    void writeToFile(const std::string &filename);
    void dump();

  private:
    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;

    llvm::BasicBlock *currentBlock;
    llvm::Function *currentFunction;
    std::list<std::unordered_map<std::string, llvm::AllocaInst *>> localVariables;
    std::unordered_map<std::string, llvm::Function *> functions;
    std::unordered_map<std::string, llvm::Function *> internalFunctions;

    static const std::unordered_map<std::string, NodeVisitor> builtInFunctions;

    llvm::Type *createLLVMType(ast::TypeId id);
    void initializeFunctions(const ast::SyntaxTree &tree);
    llvm::BasicBlock *processBranchRoot(ast::Node::Ptr node, bool createBlock = true);
    llvm::Value *declareString(const std::string &str, std::string name = "");
    void declareLocalVariable(ast::TypeId type, std::string &name, llvm::Value *initialValue = nullptr);
    llvm::Constant *getGlobalString(const std::string &name);

    llvm::Value *visitNode(ast::Node::Ptr node);
    llvm::Value *visitBinaryOperation(ast::Node *node);
    llvm::Value *visitExpression(ast::Node *node);
    llvm::Value *visitFloatingPointLiteralValue(ast::Node *node);
    llvm::Value *visitFunctionCall(ast::Node *node);
    llvm::Value *visitIntegerLiteralValue(ast::Node *node);
    llvm::Value *visitStringLiteralValue(ast::Node *node);
    llvm::Value *visitTypeConversion(ast::Node *node);
    llvm::Value *visitVariableName(ast::Node *node);

    llvm::Value *visitPrintFunctionCall(ast::Node *node);
    llvm::Value *visitInputFunctionCall(ast::Node *node);

    void processNode(ast::Node::Ptr node);
    void processExpression(ast::Node *node);
    void processFunctionDefinition(ast::Node *node);
    void processIfStatement(ast::Node *node);
    void processProgramRoot(ast::Node *node);
    void processReturnStatement(ast::Node *node);
    void processVariableDeclaration(ast::Node *node);
    void processWhileStatement(ast::Node *node);
};

} // namespace ir_generator
