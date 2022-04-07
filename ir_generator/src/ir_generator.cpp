#include "ir_generator.hpp"

#include <cassert>

using namespace ast;
using namespace ir_generator;

namespace {

Node::Ptr &firstChild(Node *node) {
    return node->children.front();
}

Node::Ptr &secondChild(Node *node) {
    return *std::next(node->children.begin());
}

Node::Ptr &lastChild(Node *node) {
    return node->children.back();
}

bool lhsRequiresPtr(BinaryOperation op) {
    switch (op) {
    case BinaryOperation::Assign:
        return true;
    }
    return false;
}

constexpr const char *const PLACEHOLDER_INT_NAME = "__placeholder_int";
constexpr const char *const PLACEHOLDER_FLOAT_NAME = "__placeholder_float";
constexpr const char *const PLACEHOLDER_STR_NAME = "__placeholder_str";
constexpr const char *const PLACEHOLDER_TRUE_NAME = "__placeholder_true";
constexpr const char *const PLACEHOLDER_FALSE_NAME = "__placeholder_false";
constexpr const char *const PLACEHOLDER_NONE_NAME = "__placeholder_none";
constexpr const char *const PLACEHOLDER_POINTER_NAME = "__placeholder_pointer";

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
                return it->second;
        }
        currentNode = currentNode->parent;
    }
    return BuiltInTypes::NoneType;
}

} // namespace

static const std::unordered_map<std::string, std::string> placeholders = {
    {PLACEHOLDER_INT_NAME, "%d"},     {PLACEHOLDER_FLOAT_NAME, "%f"},    {PLACEHOLDER_STR_NAME, "%s"},
    {PLACEHOLDER_TRUE_NAME, "True"},  {PLACEHOLDER_FALSE_NAME, "False"}, {PLACEHOLDER_NONE_NAME, "None"},
    {PLACEHOLDER_POINTER_NAME, "%x"},
};

IRGenerator::IRGenerator(const std::string &moduleName, bool emitDebugInfo)
    : context(), module(new llvm::Module(llvm::StringRef(moduleName), context)), builder(new llvm::IRBuilder(context)),
      currentBlock(nullptr), currentFunction(nullptr) {
    {
        std::vector<llvm::Type *> arguments = {llvm::PointerType::get(llvm::Type::getInt8Ty(context), 0)};
        llvm::Function *printFunc =
            llvm::Function::Create(llvm::FunctionType::get(llvm::IntegerType::getInt32Ty(context), arguments,
                                                           true /* this is var arg func type*/),
                                   llvm::Function::ExternalLinkage, "printf", module.get());
        internalFunctions.insert_or_assign("printf", printFunc);
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

llvm::Value *IRGenerator::declareString(const std::string &str, std::string name) {
    if (name.empty()) {
        static size_t counter = 0;
        name = ".str" + std::to_string(counter++);
    }
    auto charType = llvm::IntegerType::get(context, 8);
    std::vector<llvm::Constant *> chars(str.length());
    for (size_t i = 0; i < str.size(); i++) {
        chars[i] = llvm::ConstantInt::get(charType, str[i]);
    }
    chars.push_back(llvm::ConstantInt::get(charType, 0));
    llvm::ArrayType *stringType = llvm::ArrayType::get(charType, chars.size());
    auto globalDeclaration = reinterpret_cast<llvm::GlobalVariable *>(module->getOrInsertGlobal(name, stringType));
    globalDeclaration->setInitializer(llvm::ConstantArray::get(stringType, chars));
    globalDeclaration->setConstant(true);
    globalDeclaration->setLinkage(llvm::GlobalValue::LinkageTypes::PrivateLinkage);
    globalDeclaration->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
    return llvm::ConstantExpr::getBitCast(globalDeclaration, charType->getPointerTo());
}

llvm::Value *IRGenerator::visitNode(Node::Ptr node) {
    assert(node && (node->type == NodeType::BinaryOperation || node->type == NodeType::Expression ||
                    node->type == NodeType::FloatingPointLiteralValue || node->type == NodeType::FunctionCall ||
                    node->type == NodeType::IntegerLiteralValue || node->type == NodeType::StringLiteralValue ||
                    node->type == NodeType::TypeConversion || node->type == NodeType::VariableName));

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
    case BinaryOperation::FMult:
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

llvm::Value *IRGenerator::visitFunctionCall(Node *node) {
    assert(node && node->type == NodeType::FunctionCall);

    const std::string &name = firstChild(node)->str();
    if (name == "print") {
        processPrintFunctionCall(node);
        return nullptr;
    }

    auto argsNode = lastChild(node);
    std::vector<llvm::Value *> arguments;
    for (const auto &currentNode : argsNode->children)
        arguments.push_back(visitExpression(currentNode.get()));
    return builder->CreateCall(functions[name], arguments);
}

llvm::Value *IRGenerator::visitIntegerLiteralValue(Node *node) {
    assert(node && node->type == NodeType::IntegerLiteralValue);

    long value = node->intNum();
    return llvm::ConstantInt::get(context, llvm::APInt(64, value, true));
}

llvm::Value *IRGenerator::visitStringLiteralValue(ast::Node *node) {
    assert(node && node->type == NodeType::StringLiteralValue);

    const std::string &value = node->str();
    return declareString(value);
}

llvm::Value *IRGenerator::visitTypeConversion(ast::Node *node) {
    assert(node && node->type == NodeType::TypeConversion);

    llvm::Value *base = visitNode(firstChild(node)); // fix
    TypeId typeId = lastChild(node)->typeId();
    llvm::Instruction *inst =
        llvm::CastInst::Create(typeId == BuiltInTypes::IntType ? llvm::Instruction::FPToSI : llvm::Instruction::SIToFP,
                               base, base->getType(), "", currentBlock);
    return inst->getOperand(0);
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

void IRGenerator::processPrintFunctionCall(Node *node) {
    assert(node && node->type == NodeType::FunctionCall && firstChild(node)->str() == "print");

    auto argsNode = lastChild(node).get();
    assert(argsNode->children.size() == 1); // print requires only one argument

    auto valueNode = firstChild(argsNode);
    TypeId typeId = BuiltInTypes::NoneType;
    switch (valueNode->type) {
    case NodeType::Expression:
        typeId = valueNode->typeId();
        break;
    case NodeType::IntegerLiteralValue:
        typeId = BuiltInTypes::IntType;
        break;
    case NodeType::FloatingPointLiteralValue:
        typeId = BuiltInTypes::FloatType;
        break;
    case NodeType::StringLiteralValue:
        typeId = BuiltInTypes::StrType;
        break;
    case NodeType::VariableName:
        typeId = findVariableType(valueNode);
        break;
    }
    auto placeholderName = placeholderNameByTypeId(typeId);
    std::vector<llvm::Value *> arguments = {module->getNamedGlobal(placeholderName)};
    if (placeholders.find(placeholderName)->second[0] != '%')
        arguments.push_back(visitNode(valueNode));
    builder->CreateCall(internalFunctions["printf"], arguments);
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
