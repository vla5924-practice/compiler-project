#include "ir_generator.hpp"

#include <cassert>

using namespace ast;
using namespace ir_generator;

namespace {

Node::Ptr firstChild(Node *node) {
    return node->children.front();
}

Node::Ptr secondChild(Node *node) {
    return *std::next(node->children.begin());
}

Node::Ptr lastChild(Node *node) {
    return node->children.back();
}

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

llvm::BasicBlock *IRGenerator::processBranchRoot(Node::Ptr node, bool createBlock) {
    assert(node && node->type == NodeType::BranchRoot);

    if (createBlock) {
        currentBlock = llvm::BasicBlock::Create(context, "block", currentFunction);
    }
    localVariables.emplace_back();
    builder->SetInsertPoint(currentBlock);
    for (auto &n : node->children)
        processNode(n);
    localVariables.pop_back();
    return currentBlock;
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

    Node::Ptr &lhsNode = firstChild(node);
    Node::Ptr &rhsNode = lastChild(node);
    llvm::Value *lhs = visitNode(lhsNode);
    llvm::Value *rhs = visitNode(rhsNode);

    BinaryOperation op = node->binOp();
    if (lhsNode->type == NodeType::VariableName && !lhsRequiresPtr(op))
        lhs = builder->CreateLoad(lhs);
    if (rhsNode->type == NodeType::VariableName)
        rhs = builder->CreateLoad(rhs);
    switch (op) {
    case BinaryOperation::Add:
        return builder->CreateAdd(lhs, rhs);
    case BinaryOperation::Sub:
        return builder->CreateSub(lhs, rhs);
    case BinaryOperation::Mult:
        return builder->CreateMul(lhs, rhs);
    case BinaryOperation::Div:
        return builder->CreateSDiv(lhs, rhs);
    case BinaryOperation::FAdd:
        return builder->CreateFAdd(lhs, rhs);
    case BinaryOperation::FSub:
        return builder->CreateFSub(lhs, rhs);
    case BinaryOperation::FMul:
        return builder->CreateFMul(lhs, rhs);
    case BinaryOperation::FDiv:
        return builder->CreateFDiv(lhs, rhs);
    case BinaryOperation::Assign:
        return builder->CreateStore(rhs, lhs);
    case BinaryOperation::Equal:
        return builder->CreateICmp(llvm::ICmpInst::ICMP_EQ, lhs, rhs);
    case BinaryOperation::NotEqual:
        return builder->CreateICmp(llvm::ICmpInst::ICMP_NE, lhs, rhs);
    case BinaryOperation::Greater:
        return builder->CreateICmp(llvm::ICmpInst::ICMP_SGT, lhs, rhs);
    case BinaryOperation::GreaterEqual:
        return builder->CreateICmp(llvm::ICmpInst::ICMP_SGE, lhs, rhs);
    case BinaryOperation::Less:
        return builder->CreateICmp(llvm::ICmpInst::ICMP_SLT, lhs, rhs);
    case BinaryOperation::LessEqual:
        return builder->CreateICmp(llvm::ICmpInst::ICMP_SLE, lhs, rhs);
    case BinaryOperation::And:
        return builder->CreateAnd(lhs, rhs);
    case BinaryOperation::Or:
        return builder->CreateOr(lhs, rhs);
    }
    return nullptr;
}

llvm::Value *IRGenerator::visitExpression(Node *node) {
    assert(node && node->type == NodeType::Expression);
    return visitNode(firstChild(node));
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
    visitNode(firstChild(node));
}

void IRGenerator::processFunctionDefinition(Node *node) {
    assert(node && node->type == NodeType::FunctionDefinition);

    std::string name = firstChild(node)->str();
    llvm::Function *function = module->getFunction(name);
    llvm::BasicBlock *body = llvm::BasicBlock::Create(context, name + "_entry", function);
    currentBlock = body;
    currentFunction = function;
    builder->SetInsertPoint(body);
    processBranchRoot(lastChild(node));
}

void IRGenerator::processIfStatement(Node *node) {
    assert(node && node->type == NodeType::IfStatement);

    llvm::BasicBlock *endBlock = llvm::BasicBlock::Create(context, "endif");
    llvm::BasicBlock *elseBlock = nullptr;

    bool hasAdditionals = node->children.size() > 2u;
    Node *lastElse = lastChild(node).get();

    if (hasAdditionals && lastElse->type == NodeType::ElseStatement) {
        elseBlock = llvm::BasicBlock::Create(context, "else");
    }

    auto tempElif = std::make_shared<Node>(NodeType::ElifStatement);
    tempElif->children.push_back(firstChild(node));
    tempElif->children.push_back(secondChild(node));
    node->children.insert(std::next(node->children.begin(), 2), tempElif);

    for (auto nodeIter = std::next(node->children.begin(), 2);
         nodeIter != node->children.end() && (*nodeIter)->type == NodeType::ElifStatement; nodeIter++) {
        llvm::BasicBlock *trueBlock = llvm::BasicBlock::Create(context, "iftrue", currentFunction);
        llvm::BasicBlock *falseBlock = nullptr;
        auto nextNode = std::next(nodeIter);
        bool lastNode = nextNode == node->children.end() || (*nextNode)->type == NodeType::ElseStatement;
        if (lastNode) {
            if (elseBlock) {
                falseBlock = elseBlock;
            } else {
                falseBlock = endBlock;
            }
        } else {
            falseBlock = llvm::BasicBlock::Create(context, "elseif");
        }
        Node *currentNode = nodeIter->get();
        llvm::Value *condition = visitNode(firstChild(currentNode));
        builder->CreateCondBr(condition, trueBlock, falseBlock);
        // builder->SetInsertPoint(trueBlock);
        currentBlock = trueBlock;
        processBranchRoot(lastChild(currentNode), false);
        builder->CreateBr(endBlock);
        builder->SetInsertPoint(falseBlock);
    }

    if (elseBlock) {
        // builder->SetInsertPoint(elseBlock);
        currentBlock = elseBlock;
        processBranchRoot(lastChild(lastElse), false);
        builder->CreateBr(endBlock);
    }
    endBlock->insertInto(currentFunction);
    builder->SetInsertPoint(endBlock);

    node->children.erase(std::next(node->children.begin(), 2));
}

void IRGenerator::processProgramRoot(Node *node) {
    assert(node && node->type == NodeType::ProgramRoot);

    for (auto &func : node->children)
        processNode(func);
}

void IRGenerator::processReturnStatement(Node *node) {
    assert(node && node->type == NodeType::ReturnStatement);

    llvm::Value *ret = visitNode(firstChild(node));
    builder->CreateRet(ret);
}

void IRGenerator::processVariableDeclaration(Node *node) {
    assert(node && node->type == NodeType::VariableDeclaration);

    auto nodeIter = node->children.begin();
    TypeId typeId = (*nodeIter)->typeId();
    nodeIter++;
    std::string name = (*nodeIter)->str();
    llvm::AllocaInst *inst = new llvm::AllocaInst(createLLVMType(typeId), 0, name, currentBlock);
    localVariables.back().insert_or_assign(name, inst);
    nodeIter++;
    if (nodeIter != node->children.end())
        builder->CreateStore(visitNode(*nodeIter), inst);
}
