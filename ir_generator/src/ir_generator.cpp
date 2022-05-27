#include "ir_generator.hpp"

#include <array>
#include <cassert>
#include <iostream>

using namespace ast;
using namespace ir_generator;

namespace {

bool lhsRequiresPtr(BinaryOperation op) {
    switch (op) {
    case BinaryOperation::Assign:
    case BinaryOperation::FAssign:
        return true;
    }
    return false;
}

bool isLLVMPointer(llvm::Value *value) {
    return value->getType()->isPointerTy();
}

constexpr const char *const PRINT_FUNCTION_NAME = "print";
constexpr const char *const INPUT_FUNCTION_NAME = "input";
constexpr const char *const PRINTF_FUNCTION_NAME = "printf";
constexpr const char *const SCANF_FUNCTION_NAME = "scanf";

constexpr const char *const PLACEHOLDER_INT_NAME = ".placeholder.int";
constexpr const char *const PLACEHOLDER_FLOAT_NAME = ".placeholder.float";
constexpr const char *const PLACEHOLDER_STR_NAME = ".placeholder.str";
constexpr const char *const PLACEHOLDER_TRUE_NAME = ".placeholder.true";
constexpr const char *const PLACEHOLDER_FALSE_NAME = ".placeholder.false";
constexpr const char *const PLACEHOLDER_NONE_NAME = ".placeholder.none";
constexpr const char *const PLACEHOLDER_POINTER_NAME = ".placeholder.pointer";
constexpr const char *const PLACEHOLDER_NEWLINE_NAME = ".placeholder.newline";

const char *const placeholderNameByTypeId(TypeId id) {
    switch (id) {
    case BuiltInTypes::IntType:
        return PLACEHOLDER_INT_NAME;
    case BuiltInTypes::FloatType:
        return PLACEHOLDER_FLOAT_NAME;
    case BuiltInTypes::StrType:
        return PLACEHOLDER_STR_NAME;
    }
    return PLACEHOLDER_POINTER_NAME;
}

TypeId findVariableType(Node::Ptr node) {
    const std::string &name = node->str();
    auto currentNode = node->parent;
    while (currentNode) {
        if (currentNode->type == NodeType::BranchRoot) {
            auto &variables = currentNode->variables();
            auto it = variables.find(name);
            if (it != variables.end())
                return it->second.type;
        }
        currentNode = currentNode->parent;
    }
    return BuiltInTypes::NoneType;
}

TypeId detectExpressionType(Node::Ptr node) {
    switch (node->type) {
    case NodeType::Expression:
        return node->typeId();
    case NodeType::IntegerLiteralValue:
        return BuiltInTypes::IntType;
    case NodeType::FloatingPointLiteralValue:
        return BuiltInTypes::FloatType;
    case NodeType::StringLiteralValue:
        return BuiltInTypes::StrType;
    case NodeType::VariableName:
        return findVariableType(node);
    }
    return BuiltInTypes::NoneType;
}

} // namespace

static const std::unordered_map<std::string, std::string> placeholders = {
    {PLACEHOLDER_INT_NAME, "%d"},     {PLACEHOLDER_FLOAT_NAME, "%f"},    {PLACEHOLDER_STR_NAME, "%s"},
    {PLACEHOLDER_TRUE_NAME, "True"},  {PLACEHOLDER_FALSE_NAME, "False"}, {PLACEHOLDER_NONE_NAME, "None"},
    {PLACEHOLDER_POINTER_NAME, "%x"}, {PLACEHOLDER_NEWLINE_NAME, "\n"},
};

const std::unordered_map<std::string, IRGenerator::NodeVisitor> IRGenerator::builtInFunctions = {
    {PRINT_FUNCTION_NAME, &IRGenerator::visitPrintFunctionCall},
    {INPUT_FUNCTION_NAME, &IRGenerator::visitInputFunctionCall},
};

IRGenerator::IRGenerator(const std::string &moduleName, bool emitDebugInfo)
    : context(), module(new llvm::Module(llvm::StringRef(moduleName), context)), builder(new llvm::IRBuilder(context)),
      currentBlock(nullptr), currentFunction(nullptr) {
    std::array<const char *const, 2> varArgFunctions = {PRINTF_FUNCTION_NAME, SCANF_FUNCTION_NAME};
    for (auto funcName : varArgFunctions) {
        std::vector<llvm::Type *> arguments = {llvm::PointerType::get(llvm::Type::getInt8Ty(context), 0)};
        llvm::Function *function =
            llvm::Function::Create(llvm::FunctionType::get(llvm::IntegerType::getInt32Ty(context), arguments,
                                                           /* bool isVarArg = */ true),
                                   llvm::Function::ExternalLinkage, funcName, module.get());
        internalFunctions.insert_or_assign(funcName, function);
    }

    for (const auto &[name, value] : placeholders) {
        declareString(value, name);
    }
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

void IRGenerator::dump(llvm::raw_ostream &stream) {
    module->print(stream, nullptr);
}

void IRGenerator::dump(std::ostream &stream) {
    if (&stream == &std::cout) {
        dump(llvm::outs());
        return;
    }
    if (&stream == &std::cerr) {
        dump(llvm::errs());
        return;
    }
    assert(false && "Only std::cout and std::cerr stream objects are supported.");
}

std::string IRGenerator::dump() {
    std::string str;
    llvm::raw_string_ostream stream(str);
    dump(stream);
    return str;
}

llvm::Type *IRGenerator::createLLVMType(TypeId id) {
    switch (id) {
    case BuiltInTypes::BoolType:
        return llvm::Type::getInt1Ty(context);
    case BuiltInTypes::IntType:
        return llvm::Type::getInt64Ty(context);
    case BuiltInTypes::FloatType:
        return llvm::Type::getDoubleTy(context);
    case BuiltInTypes::StrType:
        return llvm::PointerType::get(llvm::Type::getInt8Ty(context), 0);
    case BuiltInTypes::NoneType:
        return llvm::Type::getVoidTy(context);
    }
    return llvm::Type::getInt64Ty(context);
}

void IRGenerator::initializeFunctions(const SyntaxTree &tree) {
    FunctionsTable knownFunctions = tree.functions;
    for (const auto &[funcName, function] : builtInFunctions) {
        knownFunctions.erase(funcName);
    }

    for (const auto &[funcName, function] : knownFunctions) {
        std::vector<llvm::Type *> arguments(function.argumentsTypes.size());
        for (size_t i = 0; i < arguments.size(); i++)
            arguments[i] = createLLVMType(function.argumentsTypes[i]);
        llvm::FunctionType *funcType = llvm::FunctionType::get(createLLVMType(function.returnType), arguments, false);
        llvm::Function *llvmFunc =
            llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, funcName, module.get());
        functions.insert_or_assign(funcName, llvmFunc);
    }
}

llvm::BasicBlock *IRGenerator::processBranchRoot(Node::Ptr node, bool createBlock) {
    assert(node && node->type == NodeType::BranchRoot);

    if (createBlock) {
        llvm::BasicBlock *nextBlock = llvm::BasicBlock::Create(context, "block");
        builder->CreateBr(nextBlock);
        nextBlock->insertInto(currentFunction);
        currentBlock = nextBlock;
    }
    localVariables.emplace_back();
    builder->SetInsertPoint(currentBlock);
    for (auto &n : node->children)
        processNode(n);
    localVariables.pop_back();
    return currentBlock;
}

llvm::Value *IRGenerator::declareString(const std::string &str, std::string name) {
    if (name.empty()) {
        static size_t counter = 0;
        name = ".str." + std::to_string(counter++);
    }
    return builder->CreateGlobalStringPtr(str, name, 0, module.get());
}

void IRGenerator::declareLocalVariable(TypeId type, std::string &name, llvm::Value *initialValue) {
    llvm::AllocaInst *inst = new llvm::AllocaInst(createLLVMType(type), 0, name, currentBlock);
    localVariables.back().insert_or_assign(name, inst);
    if (initialValue != nullptr) {
        if (isLLVMPointer(initialValue))
            initialValue = builder->CreateLoad(initialValue);
        builder->CreateStore(initialValue, inst);
    }
}

llvm::Constant *IRGenerator::getGlobalString(const std::string &name) {
    static llvm::Type *charPointerType = createLLVMType(BuiltInTypes::StrType);
    llvm::GlobalVariable *globalVariable = module->getNamedGlobal(name);
    return llvm::ConstantExpr::getBitCast(globalVariable, charPointerType);
}

llvm::Value *IRGenerator::visitNode(Node::Ptr node) {
    assert(node);
    assert(node->type == NodeType::BinaryOperation || node->type == NodeType::Expression ||
           node->type == NodeType::FloatingPointLiteralValue || node->type == NodeType::FunctionCall ||
           node->type == NodeType::IntegerLiteralValue || node->type == NodeType::StringLiteralValue ||
           node->type == NodeType::TypeConversion || node->type == NodeType::VariableName);

    Node *rawNode = node.get();
    switch (node->type) {
    case NodeType::BinaryOperation:
        return visitBinaryOperation(rawNode);
    case NodeType::Expression:
        return visitExpression(rawNode);
    case NodeType::FloatingPointLiteralValue:
        return visitFloatingPointLiteralValue(rawNode);
    case NodeType::FunctionCall:
        return visitFunctionCall(rawNode);
    case NodeType::IntegerLiteralValue:
        return visitIntegerLiteralValue(rawNode);
    case NodeType::StringLiteralValue:
        return visitStringLiteralValue(rawNode);
    case NodeType::TypeConversion:
        return visitTypeConversion(rawNode);
    case NodeType::VariableName:
        return visitVariableName(rawNode);
    }
    return nullptr;
}

llvm::Value *IRGenerator::visitBinaryOperation(Node *node) {
    assert(node && node->type == NodeType::BinaryOperation);

    Node::Ptr &lhsNode = node->firstChild();
    Node::Ptr &rhsNode = node->lastChild();
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
    case BinaryOperation::FMult:
        return builder->CreateFMul(lhs, rhs);
    case BinaryOperation::FDiv:
        return builder->CreateFDiv(lhs, rhs);
    case BinaryOperation::Assign:
    case BinaryOperation::FAssign:
        return builder->CreateStore(rhs, lhs);
    case BinaryOperation::Equal:
        return builder->CreateICmp(llvm::ICmpInst::ICMP_EQ, lhs, rhs);
    case BinaryOperation::FEqual:
        return builder->CreateFCmp(llvm::FCmpInst::FCMP_OEQ, lhs, rhs);
    case BinaryOperation::NotEqual:
        return builder->CreateICmp(llvm::ICmpInst::ICMP_NE, lhs, rhs);
    case BinaryOperation::FNotEqual:
        return builder->CreateFCmp(llvm::FCmpInst::FCMP_ONE, lhs, rhs);
    case BinaryOperation::Greater:
        return builder->CreateICmp(llvm::ICmpInst::ICMP_SGT, lhs, rhs);
    case BinaryOperation::FGreater:
        return builder->CreateFCmp(llvm::ICmpInst::FCMP_OGT, lhs, rhs);
    case BinaryOperation::GreaterEqual:
        return builder->CreateICmp(llvm::ICmpInst::ICMP_SGE, lhs, rhs);
    case BinaryOperation::FGreaterEqual:
        return builder->CreateFCmp(llvm::ICmpInst::FCMP_OGE, lhs, rhs);
    case BinaryOperation::Less:
        return builder->CreateICmp(llvm::ICmpInst::ICMP_SLT, lhs, rhs);
    case BinaryOperation::FLess:
        return builder->CreateFCmp(llvm::ICmpInst::FCMP_OLT, lhs, rhs);
    case BinaryOperation::LessEqual:
        return builder->CreateICmp(llvm::ICmpInst::ICMP_SLE, lhs, rhs);
    case BinaryOperation::FLessEqual:
        return builder->CreateFCmp(llvm::ICmpInst::FCMP_OLE, lhs, rhs);
    case BinaryOperation::And:
    case BinaryOperation::FAnd:
        return builder->CreateAnd(lhs, rhs);
    case BinaryOperation::Or:
    case BinaryOperation::FOr:
        return builder->CreateOr(lhs, rhs);
    }
    return nullptr;
}

llvm::Value *IRGenerator::visitExpression(Node *node) {
    assert(node && node->type == NodeType::Expression);
    return visitNode(node->firstChild());
}

llvm::Value *IRGenerator::visitFloatingPointLiteralValue(Node *node) {
    assert(node && node->type == NodeType::FloatingPointLiteralValue);

    double value = node->fpNum();
    return llvm::ConstantFP::get(context, llvm::APFloat(value));
}

llvm::Value *IRGenerator::visitFunctionCall(Node *node) {
    assert(node && node->type == NodeType::FunctionCall);

    const std::string &name = node->firstChild()->str();
    auto visitorIter = builtInFunctions.find(name);
    if (visitorIter != builtInFunctions.end()) {
        NodeVisitor visitor = visitorIter->second;
        return (this->*visitor)(node);
    }

    auto argsNode = node->lastChild();
    std::vector<llvm::Value *> arguments;
    for (const auto &currentNode : argsNode->children) {
        llvm::Value *arg = visitExpression(currentNode.get());
        if (isLLVMPointer(arg))
            arg = builder->CreateLoad(arg);
        arguments.push_back(arg);
    }
    return builder->CreateCall(functions[name], arguments);
}

llvm::Value *IRGenerator::visitIntegerLiteralValue(Node *node) {
    assert(node && node->type == NodeType::IntegerLiteralValue);

    long value = node->intNum();
    return llvm::ConstantInt::get(context, llvm::APInt(64, value, true));
}

llvm::Value *IRGenerator::visitStringLiteralValue(Node *node) {
    assert(node && node->type == NodeType::StringLiteralValue);

    const std::string &value = node->str();
    return declareString(value);
}

llvm::Value *IRGenerator::visitTypeConversion(Node *node) {
    assert(node && node->type == NodeType::TypeConversion);

    Node::Ptr &base = node->lastChild();
    llvm::Value *operand = nullptr;
    if (base->type == NodeType::VariableName)
        operand = builder->CreateLoad(visitVariableName(base.get()));
    else
        operand = visitNode(base);
    TypeId srcType = detectExpressionType(base);
    TypeId dstType = node->firstChild()->typeId();

    if (srcType == dstType)
        return operand;
    if (srcType == BuiltInTypes::IntType && dstType == BuiltInTypes::FloatType)
        return new llvm::SIToFPInst(operand, createLLVMType(dstType), "typeconv", currentBlock);
    if (srcType == BuiltInTypes::FloatType && dstType == BuiltInTypes::IntType)
        return new llvm::FPToSIInst(operand, createLLVMType(dstType), "typeconv", currentBlock);
    return nullptr;
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

llvm::Value *IRGenerator::visitPrintFunctionCall(Node *node) {
    assert(node && node->type == NodeType::FunctionCall && node->firstChild()->str() == PRINT_FUNCTION_NAME);

    auto argsNode = node->lastChild();
    assert(argsNode->children.size() == 1u); // print requires only one argument

    auto valueNode = argsNode->firstChild();
    auto placeholderName = placeholderNameByTypeId(detectExpressionType(valueNode));
    std::vector<llvm::Value *> arguments = {getGlobalString(placeholderName)};
    if (placeholders.find(placeholderName)->second[0] == '%') {
        llvm::Value *arg = visitNode(valueNode);
        if (isLLVMPointer(arg) && arg->getType() != createLLVMType(BuiltInTypes::StrType))
            arg = builder->CreateLoad(arg);
        arguments.push_back(arg);
    }
    llvm::Function *function = internalFunctions[PRINTF_FUNCTION_NAME];
    builder->CreateCall(function, arguments);
    builder->CreateCall(function, {getGlobalString(PLACEHOLDER_NEWLINE_NAME)});

    return nullptr;
}

llvm::Value *IRGenerator::visitInputFunctionCall(Node *node) {
    assert(node && node->type == NodeType::FunctionCall && node->firstChild()->str() == INPUT_FUNCTION_NAME);

    TypeId returnType = node->lastChild()->typeId();
    llvm::AllocaInst *temporary = new llvm::AllocaInst(createLLVMType(returnType), 0, "input", currentBlock);
    std::vector<llvm::Value *> arguments = {getGlobalString(placeholderNameByTypeId(returnType)), temporary};
    builder->CreateCall(internalFunctions[SCANF_FUNCTION_NAME], arguments);
    return builder->CreateLoad(temporary);
}

void IRGenerator::processNode(Node::Ptr node) {
    assert(node);
    assert(node->type == NodeType::Expression || node->type == NodeType::FunctionDefinition ||
           node->type == NodeType::IfStatement || node->type == NodeType::ProgramRoot ||
           node->type == NodeType::ReturnStatement || node->type == NodeType::VariableDeclaration ||
           node->type == NodeType::WhileStatement);

    Node *rawNode = node.get();
    switch (node->type) {
    case NodeType::Expression:
        processExpression(rawNode);
        return;
    case NodeType::FunctionDefinition:
        processFunctionDefinition(rawNode);
        return;
    case NodeType::IfStatement:
        processIfStatement(rawNode);
        return;
    case NodeType::ProgramRoot:
        processProgramRoot(rawNode);
        return;
    case NodeType::ReturnStatement:
        processReturnStatement(rawNode);
        return;
    case NodeType::VariableDeclaration:
        processVariableDeclaration(rawNode);
        return;
    case NodeType::WhileStatement:
        processWhileStatement(rawNode);
        return;
    }
}

void IRGenerator::processExpression(Node *node) {
    assert(node && node->type == NodeType::Expression);
    visitNode(node->firstChild());
}

void IRGenerator::processFunctionDefinition(Node *node) {
    assert(node && node->type == NodeType::FunctionDefinition);

    std::string name = node->firstChild()->str();
    llvm::Function *function = module->getFunction(name);
    llvm::BasicBlock *body = llvm::BasicBlock::Create(context, name + "_entry", function);
    currentBlock = body;
    currentFunction = function;
    builder->SetInsertPoint(body);

    Node::Ptr &argsNode = node->secondChild();
    unsigned argIndex = 0u;
    localVariables.emplace_back();
    for (auto &argNode : argsNode->children) {
        assert(argNode->type == NodeType::FunctionArgument);

        auto nodeIter = argNode->children.begin();
        TypeId typeId = (*nodeIter)->typeId();
        nodeIter++;
        std::string name = (*nodeIter)->str();
        declareLocalVariable(typeId, name, function->getArg(argIndex++));
    }
    processBranchRoot(node->lastChild());
    localVariables.pop_back();

    if (function->getReturnType()->isVoidTy()) {
        builder->CreateRetVoid();
    }
}

void IRGenerator::processIfStatement(Node *node) {
    assert(node && node->type == NodeType::IfStatement);

    llvm::BasicBlock *endBlock = llvm::BasicBlock::Create(context, "endif");
    llvm::BasicBlock *elseBlock = nullptr;

    bool hasAdditionals = node->children.size() > 2u;
    auto lastElse = node->lastChild();

    if (hasAdditionals && lastElse->type == NodeType::ElseStatement) {
        elseBlock = llvm::BasicBlock::Create(context, "elsebody");
    }

    auto tempElif = std::make_shared<Node>(NodeType::ElifStatement);
    tempElif->children.push_back(node->firstChild());
    tempElif->children.push_back(node->secondChild());
    node->children.insert(std::next(node->children.begin(), 2), tempElif);

    llvm::BasicBlock *condBlock = nullptr;
    llvm::BasicBlock *nextBlock = nullptr;

    for (auto nodeIter = std::next(node->children.begin(), 2);
         nodeIter != node->children.end() && (*nodeIter)->type == NodeType::ElifStatement; nodeIter++) {
        if (nextBlock == nullptr) {
            condBlock = llvm::BasicBlock::Create(context, "ifcond");
            builder->CreateBr(condBlock);
            condBlock->insertInto(currentFunction);
        } else {
            condBlock = nextBlock;
            nextBlock->insertInto(currentFunction);
        }
        llvm::BasicBlock *trueBlock = llvm::BasicBlock::Create(context, "ifbody", currentFunction);
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
            falseBlock = llvm::BasicBlock::Create(context, "ifcond");
            nextBlock = falseBlock;
        }
        builder->SetInsertPoint(condBlock);
        Node *currentNode = nodeIter->get();
        llvm::Value *condition = visitNode(currentNode->firstChild());
        builder->CreateCondBr(condition, trueBlock, falseBlock);
        currentBlock = trueBlock;
        processBranchRoot(currentNode->lastChild(), false);
        builder->CreateBr(endBlock);
        builder->SetInsertPoint(falseBlock);
    }

    if (elseBlock) {
        elseBlock->insertInto(currentFunction);
        currentBlock = elseBlock;
        processBranchRoot(lastElse->lastChild(), false);
        builder->CreateBr(endBlock);
    }
    endBlock->insertInto(currentFunction);
    builder->SetInsertPoint(endBlock);
    currentBlock = endBlock;

    node->children.erase(std::next(node->children.begin(), 2));
}

void IRGenerator::processProgramRoot(Node *node) {
    assert(node && node->type == NodeType::ProgramRoot);

    for (auto &func : node->children)
        processNode(func);
}

void IRGenerator::processReturnStatement(Node *node) {
    assert(node && node->type == NodeType::ReturnStatement);

    llvm::Value *ret = visitNode(node->firstChild());
    if (isLLVMPointer(ret))
        ret = builder->CreateLoad(ret);
    builder->CreateRet(ret);
}

void IRGenerator::processVariableDeclaration(Node *node) {
    assert(node && node->type == NodeType::VariableDeclaration);

    auto nodeIter = node->children.begin();
    TypeId typeId = (*nodeIter)->typeId();
    nodeIter++;
    std::string name = (*nodeIter)->str();
    nodeIter++;
    llvm::Value *initialValue = nodeIter != node->children.end() ? visitNode(*nodeIter) : nullptr;
    declareLocalVariable(typeId, name, initialValue);
}

void IRGenerator::processWhileStatement(Node *node) {
    assert(node && node->type == NodeType::WhileStatement);

    llvm::BasicBlock *condBlock = llvm::BasicBlock::Create(context, "whilecond");
    llvm::BasicBlock *beginBlock = llvm::BasicBlock::Create(context, "whilebody");
    llvm::BasicBlock *endBlock = llvm::BasicBlock::Create(context, "endwhile");

    builder->CreateBr(condBlock);
    condBlock->insertInto(currentFunction);
    builder->SetInsertPoint(condBlock);
    llvm::Value *condition = visitNode(node->firstChild());
    builder->CreateCondBr(condition, beginBlock, endBlock);
    currentBlock = beginBlock;
    beginBlock->insertInto(currentFunction);
    processBranchRoot(node->lastChild(), false);
    builder->CreateBr(condBlock);
    endBlock->insertInto(currentFunction);
    builder->SetInsertPoint(endBlock);
    currentBlock = endBlock;
}
