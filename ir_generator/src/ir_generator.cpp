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

void IRGenerator::process(const SyntaxTree &tree) {
    initializeFunctions(tree);
    processNode(tree.root);
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

llvm::Type *IRGenerator::createLLVMType(TypeId id) {
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

void IRGenerator::initializeFunctions(const SyntaxTree &tree) {
    for (const auto &[funcName, function] : tree.functions) {
        std::vector<llvm::Type *> arguments(function.argumentsTypes.size());
        for (size_t i = 0; i < arguments.size(); i++)
            arguments[i] = createLLVMType(function.argumentsTypes[i]);
        llvm::FunctionType *funcType = llvm::FunctionType::get(createLLVMType(function.returnType), arguments, false);
        llvm::Function *llvmFunc =
            llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, funcName, module.get());
        functions.insert_or_assign(funcName, llvmFunc);
    }
    // built-in print function
    {
        llvm::Function *printFunc = module->getFunction("printf");
        if (printFunc == nullptr)
            printFunc = llvm::Function::Create(
                llvm::FunctionType::get(llvm::IntegerType::getInt32Ty(context),
                                        llvm::PointerType::get(llvm::Type::getInt8Ty(context), 0),
                                        true /* this is var arg func type*/),
                llvm::Function::ExternalLinkage, "printf", module.get());
        functions.insert_or_assign("print", printFunc);
    }
}

llvm::BasicBlock *IRGenerator::processBranchRoot(Node::Ptr node) {
    assert(node && node->type == NodeType::BranchRoot);

    llvm::BasicBlock *block = llvm::BasicBlock::Create(context, "block", currentFunction);
    currentBlock = block;
    localVariables.emplace_back();
    builder->SetInsertPoint(block);
    for (auto &n : node->children)
        processNode(n);
    localVariables.pop_back();
    return block;
}

llvm::Value *IRGenerator::visitNode(Node::Ptr node) {
    assert(node && (node->type == NodeType::BinaryOperation || node->type == NodeType::Expression ||
                    node->type == NodeType::FloatingPointLiteralValue || node->type == NodeType::IntegerLiteralValue ||
                    node->type == NodeType::VariableName));

    switch (node->type) {
    case NodeType::BinaryOperation:
        return visitBinaryOperation(node.get());
    case NodeType::Expression:
        return visitExpression(node.get());
    case NodeType::FloatingPointLiteralValue:
        return visitFloatingPointLiteralValue(node.get());
    case NodeType::IntegerLiteralValue:
        return visitIntegerLiteralValue(node.get());
    case NodeType::VariableName:
        return visitVariableName(node.get());
    }
    return nullptr;
}

llvm::Value *IRGenerator::visitBinaryOperation(Node *node) {
    assert(node && node->type == NodeType::BinaryOperation);

    Node::Ptr &lhsNode = node->children.front();
    Node::Ptr &rhsNode = node->children.back();
    llvm::Value *lhs = visitNode(lhsNode);
    llvm::Value *rhs = visitNode(rhsNode);

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

llvm::Value *IRGenerator::visitExpression(Node *node) {
    assert(node && node->type == NodeType::Expression);
    return visitNode(node->children.front());
}

llvm::Value *IRGenerator::visitFloatingPointLiteralValue(Node *node) {
    assert(node && node->type == NodeType::FloatingPointLiteralValue);

    double value = node->fpNum();
    return llvm::ConstantFP::get(context, llvm::APFloat(value));
}

llvm::Value *IRGenerator::visitIntegerLiteralValue(Node *node) {
    assert(node && node->type == NodeType::IntegerLiteralValue);

    long value = node->intNum();
    return llvm::ConstantInt::get(context, llvm::APInt(64, value, true));
}

llvm::Value *IRGenerator::visitVariableName(Node *node) {
    assert(node && node->type == NodeType::VariableName);

    for (const auto &layer : localVariables) {
        auto it = layer.find(node->str());
        if (it != layer.end()) {
            return it->second;
        }
    }
    return nullptr;
}

void IRGenerator::processNode(Node::Ptr node) {
    assert(node && (node->type == NodeType::Expression || node->type == NodeType::FunctionDefinition ||
                    node->type == NodeType::IfStatement || node->type == NodeType::ProgramRoot ||
                    node->type == NodeType::VariableDeclaration));

    switch (node->type) {
    case NodeType::Expression:
        visitExpression(node.get());
        return;
    case NodeType::FunctionDefinition:
        processFunctionDefinition(node.get());
        return;
    case NodeType::IfStatement:
        processIfStatement(node.get());
        return;
    case NodeType::ProgramRoot:
        processProgramRoot(node.get());
        return;
    case NodeType::VariableDeclaration:
        processVariableDeclaration(node.get());
        return;
    }
}

void IRGenerator::processExpression(Node *node) {
    assert(node && node->type == NodeType::Expression);
    visitNode(node->children.front());
}

void IRGenerator::processFunctionDefinition(Node *node) {
    assert(node && node->type == NodeType::FunctionDefinition);

    std::string name = node->children.front()->str();
    llvm::Function *function = module->getFunction(name);
    llvm::BasicBlock *body = llvm::BasicBlock::Create(context, name + "_entry", function);
    currentBlock = body;
    currentFunction = function;
    builder->SetInsertPoint(body);
    processBranchRoot(node->children.back());
}

void IRGenerator::processIfStatement(Node *node) {
    assert(node && node->type == NodeType::IfStatement);

    llvm::Value *condition = visitNode(node->children.front());
    llvm::BasicBlock *trueBlock = processBranchRoot(*std::next(node->children.begin()));
    llvm::BasicBlock *falseBlock = nullptr;
    if (node->children.size() == 3u) { // has elif/else block
        falseBlock = processBranchRoot(node->children.back());
    } else {
        falseBlock = llvm::BasicBlock::Create(context, "else", currentFunction);
    }
    builder->CreateCondBr(condition, trueBlock, falseBlock);
}

void IRGenerator::processProgramRoot(Node *node) {
    assert(node && node->type == NodeType::ProgramRoot);

    for (auto &func : node->children)
        processNode(func);
}

void IRGenerator::processReturnStatement(Node *node) {
    assert(node && node->type == NodeType::ReturnStatement);

    llvm::Value *ret = visitNode(node->children.front());
    builder->CreateRet(ret);
}

void IRGenerator::processVariableDeclaration(Node *node) {
    assert(node && node->type == NodeType::VariableDeclaration);

    TypeId typeId = node->children.front()->typeId();
    std::string name = std::next(node->children.begin())->get()->str();
    llvm::AllocaInst *inst = new llvm::AllocaInst(createLLVMType(typeId), 0, name, currentBlock);
    localVariables.back().insert_or_assign(name, inst);
    if (node->children.size() == 3u) // with definition
        builder->CreateStore(visitNode(node->children.back()), inst);
}