#include "ir_generator.hpp"

using namespace ast;
using namespace ir_generator;

IRGenerator::IRGenerator(const std::string &moduleName, bool emitDebugInfo)
    : context(), module(new llvm::Module(llvm::StringRef(moduleName), context)), builder(new llvm::IRBuilder(context)) {
}

void IRGenerator::process(const ast::SyntaxTree &tree) {
    std::vector<llvm::Type *> params;
    llvm::FunctionType *ft = llvm::FunctionType::get(llvm::Type::getVoidTy(context), params, false);
    llvm::Function *inz = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "INZ", this->module.get());
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(context, "", inz);
    builder->SetInsertPoint(BB);
    llvm::Constant *initValue;
    module->getOrInsertGlobal("XYZ", builder->getInt64Ty());
    llvm::GlobalVariable *var = module->getNamedGlobal("XYZ");
    builder->CreateStore(llvm::ConstantInt::get(context, llvm::APInt(64, 12345, true)), var);
}

llvm::Value *IRGenerator::visit(ast::Node::Ptr node) {
    switch (node->type) {
    case NodeType::BinaryOperation:
        return visitBinaryOperation(node.get());
    case NodeType::IntegerLiteralValue:
        return visitIntegerLiteralValue(node.get());
    case NodeType::FunctionDefinition:
        return visitFunctionDefinition(node.get());
    }
    return nullptr;
}

llvm::Value *IRGenerator::visitIntegerLiteralValue(ast::Node *node) {
    long value = node->intNum();
    return llvm::ConstantInt::get(context, llvm::APInt(64, value, true));
}

llvm::Value *IRGenerator::visitBinaryOperation(ast::Node *node) {
    llvm::Value *lhs = visit(node->children.front());
    llvm::Value *rhs = visit(node->children.back());
    BinaryOperation op = node->binOp();
    switch (op) {
    case BinaryOperation::Add:
        return builder->CreateAdd(lhs, rhs, "add");
    case BinaryOperation::Sub:
        return builder->CreateSub(lhs, rhs, "sub");
    case BinaryOperation::Mult:
        return builder->CreateMul(lhs, rhs, "mult");
    case BinaryOperation::Div:
        return builder->CreateFDiv(lhs, rhs, "div");
    }
    return nullptr;
}

llvm::Value *IRGenerator::visitFunctionDefinition(ast::Node *node) {
    llvm::Function *function = module->getFunction(node->id());
    if (!function)
        throw 1;

    llvm::BasicBlock *body = llvm::BasicBlock::Create(context, "entry", function);
    builder->SetInsertPoint(body);
    /* Emit the subroutine body, non-terminating instructions */
    llvm::Value *ret = llvm::ConstantInt::get(context, llvm::APInt(32, 0, true));
    builder->CreateRet(ret);
}

void IRGenerator::dump() {
    module->print(llvm::outs(), nullptr);
}
