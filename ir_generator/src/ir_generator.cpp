#include "ir_generator.hpp"

#include <cassert>

using namespace ast;
using namespace ir_generator;

namespace {

bool lhsRequiresPtr(BinaryOperation op) {
    switch (op) {
    case BinaryOperation::Assign:
        return true;
    }
    return false;
}

} // namespace

IRGenerator::IRGenerator(const std::string &moduleName, bool emitDebugInfo)
    : context(), module(new llvm::Module(llvm::StringRef(moduleName), context)), builder(new llvm::IRBuilder(context)),
      currentBlock(nullptr), currentFunction(nullptr) {
}

void IRGenerator::process(const ast::SyntaxTree &tree) {
    initializeFunctions(tree);
    visit(tree.root);
}

void IRGenerator::writeToFile(const std::string &filename) {
    std::error_code error;
    llvm::raw_fd_ostream file(filename, error);
    module->print(file, nullptr);
    file.close();
}

void IRGenerator::dump() {
    module->print(llvm::outs(), nullptr);
}

void IRGenerator::initializeFunctions(const ast::SyntaxTree &tree) {
    for (const auto &[funcName, function] : tree.functions) {
        std::vector<llvm::Type *> arguments(function.argumentsTypes.size());
        for (size_t i = 0; i < arguments.size(); i++)
            arguments[i] = createLLVMType(function.argumentsTypes[i]);
        llvm::FunctionType *funcType = llvm::FunctionType::get(createLLVMType(function.returnType), arguments, false);
        llvm::Function *llvmFunc =
            llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, funcName, this->module.get());
        functions.insert_or_assign(funcName, llvmFunc);
    }
}

llvm::Type *IRGenerator::createLLVMType(ast::TypeId id) {
    using namespace ast;
    switch (id) {
    case IntType:
        return llvm::Type::getInt64Ty(context);
    case FloatType:
        return llvm::Type::getDoubleTy(context);
    case NoneType:
        return llvm::Type::getVoidTy(context);
    }
    return llvm::Type::getInt64Ty(context);
}

llvm::Value *IRGenerator::visit(ast::Node::Ptr node) {
    switch (node->type) {
    case NodeType::BinaryOperation:
        return visitBinaryOperation(node.get());
    case NodeType::IntegerLiteralValue:
        return visitIntegerLiteralValue(node.get());
    case NodeType::FloatingPointLiteralValue:
        return visitFloatingPointLiteralValue(node.get());
    case NodeType::FunctionDefinition:
        return visitFunctionDefinition(node.get());
    case NodeType::VariableDeclaration:
        return visitVariableDeclaration(node.get());
    case NodeType::IfStatement:
        return visitIfStatement(node.get());
    case NodeType::Expression:
        return visitExpression(node.get());
    case NodeType::ProgramRoot:
        return visitProgramRoot(node.get());
    case NodeType::VariableName:
        return visitVariableName(node.get());
    }
    return nullptr;
}

llvm::BasicBlock *IRGenerator::visitBranchRoot(ast::Node::Ptr node) {
    assert(node && node->type == NodeType::BranchRoot);

    if (node->type != NodeType::BranchRoot)
        throw -1; // sema error
    llvm::BasicBlock *block = llvm::BasicBlock::Create(context, "block", currentFunction);
    currentBlock = block;
    localVariables.emplace_back();
    builder->SetInsertPoint(block);
    for (auto &n : node->children)
        visit(n);
    localVariables.pop_back();
    return block;
}

llvm::Value *IRGenerator::visitBinaryOperation(ast::Node *node) {
    assert(node && node->type == NodeType::BinaryOperation);

    ast::Node::Ptr &lhsNode = node->children.front();
    ast::Node::Ptr &rhsNode = node->children.back();
    llvm::Value *lhs = visit(lhsNode);
    llvm::Value *rhs = visit(rhsNode);

    BinaryOperation op = node->binOp();
    if (lhsNode->type == NodeType::VariableName && !lhsRequiresPtr(op))
        lhs = builder->CreateLoad(lhs);
    if (rhsNode->type == NodeType::VariableName)
        rhs = builder->CreateLoad(rhs);
    switch (op) {
    case BinaryOperation::Add:
        return builder->CreateAdd(lhs, rhs, "addtmp");
    case BinaryOperation::Sub:
        return builder->CreateSub(lhs, rhs, "subtmp");
    case BinaryOperation::Mult:
        return builder->CreateMul(lhs, rhs, "multmp");
    case BinaryOperation::Div:
        return builder->CreateSDiv(lhs, rhs, "sdivtmp");
    case BinaryOperation::FAdd:
        return builder->CreateFAdd(lhs, rhs, "faddtmp");
    case BinaryOperation::FSub:
        return builder->CreateFSub(lhs, rhs, "fsubtmp");
    case BinaryOperation::FMul:
        return builder->CreateFMul(lhs, rhs, "fmultmp");
    case BinaryOperation::FDiv:
        return builder->CreateFDiv(lhs, rhs, "fdivtmp");
    case BinaryOperation::Assign:
        return builder->CreateStore(rhs, lhs);
    case BinaryOperation::Equal:
        return builder->CreateICmp(llvm::ICmpInst::ICMP_EQ, lhs, rhs, "eqtmp");
    }
    return nullptr;
}

llvm::Value *IRGenerator::visitExpression(ast::Node *node) {
    assert(node && node->type == NodeType::Expression);
    return visit(node->children.front());
}

llvm::Value *IRGenerator::visitFloatingPointLiteralValue(ast::Node *node) {
    assert(node && node->type == NodeType::FloatingPointLiteralValue);

    double value = node->fpNum();
    return llvm::ConstantFP::get(context, llvm::APFloat(value));
}

llvm::Value *IRGenerator::visitFunctionDefinition(ast::Node *node) {
    assert(node && node->type == NodeType::FunctionDefinition);

    std::string name = node->children.front()->str();
    llvm::Function *function = module->getFunction(name);
    llvm::BasicBlock *body = llvm::BasicBlock::Create(context, name + "_entry", function);
    currentBlock = body;
    currentFunction = function;
    builder->SetInsertPoint(body);
    visitBranchRoot(node->children.back());
    return body;
}

llvm::Value *IRGenerator::visitIfStatement(ast::Node *node) {
    assert(node && node->type == NodeType::IfStatement);

    llvm::Value *condition = visit(node->children.front());
    llvm::BasicBlock *trueBlock = visitBranchRoot(*std::next(node->children.begin()));
    llvm::BasicBlock *falseBlock = nullptr;
    if (node->children.size() == 3u) { // has elif/else block
        falseBlock = visitBranchRoot(node->children.back());
    } else {
        falseBlock = llvm::BasicBlock::Create(context, "else", currentFunction);
    }
    return builder->CreateCondBr(condition, trueBlock, falseBlock);
}

llvm::Value *IRGenerator::visitIntegerLiteralValue(ast::Node *node) {
    assert(node && node->type == NodeType::IntegerLiteralValue);

    long value = node->intNum();
    return llvm::ConstantInt::get(context, llvm::APInt(64, value, true));
}

llvm::Value *IRGenerator::visitProgramRoot(ast::Node *node) {
    assert(node && node->type == NodeType::ProgramRoot);

    for (auto &func : node->children) {
        visit(func);
    }
    return nullptr;
}

llvm::Value *IRGenerator::visitReturnStatement(ast::Node *node) {
    assert(node && node->type == NodeType::ReturnStatement);

    llvm::Value *ret = visit(node->children.front());
    return builder->CreateRet(ret);
}

llvm::Value *IRGenerator::visitVariableDeclaration(ast::Node *node) {
    assert(node && node->type == NodeType::VariableDeclaration);

    ast::TypeId typeId = node->children.front()->typeId();
    std::string name = std::next(node->children.begin())->get()->str();
    llvm::AllocaInst *inst = new llvm::AllocaInst(createLLVMType(typeId), 0, name, currentBlock);
    localVariables.back().insert_or_assign(name, inst);
    if (node->children.size() == 3u) { // with definition
        return builder->CreateStore(visit(node->children.back()), inst);
    } else {
        return inst;
    }
}

llvm::Value *IRGenerator::visitVariableName(ast::Node *node) {
    assert(node && node->type == NodeType::VariableName);

    for (const auto &layer : localVariables) {
        auto it = layer.find(node->str());
        if (it != layer.end()) {
            return it->second;
        }
    }
    return nullptr;
}
