#include "converter/converter.hpp"

#include <cassert>

#include "compiler/optree/adaptors.hpp"

#include "converter/converter_context.hpp"
#include "converter/converter_error.hpp"

#if __has_builtin(__builtin_unreachable)
#define UNREACHABLE(MSG)                                                                                               \
    assert(false && (MSG));                                                                                            \
    __builtin_unreachable()
#else
#define UNREACHABLE(msg) assert(false && (MSG))
#endif

using namespace optree;
using namespace converter;

using ast::Node;
using ast::NodeType;
using ast::SyntaxTree;

Type::Ptr convertType(ast::TypeId typeId) {
    switch (typeId) {
    case ast::NoneType:
        return TypeStorage::noneType();
    case ast::IntType:
        return TypeStorage::integerType();
    case ast::FloatType:
        return TypeStorage::floatType();
    case ast::StrType:
        return TypeStorage::strType();
    }
    return {};
}

bool isLhsInAssignment(const Node::Ptr &node) {
    const auto &parent = node->parent;
    return node->type == ast::NodeType::VariableName && parent->type == ast::NodeType::BinaryOperation &&
           (parent->binOp() == ast::BinaryOperation::Assign || parent->binOp() == ast::BinaryOperation::FAssign) &&
           parent->firstChild() == node;
}

void processNode(const Node::Ptr &node, ConverterContext &ctx);
Value::Ptr visitNode(const Node::Ptr &node, ConverterContext &ctx);

void processProgramRoot(const Node::Ptr &node, ConverterContext &ctx) {
    for (const auto &child : node->children) {
        const auto &name = child->firstChild()->str();
        const auto &typeNode = *std::next(child->children.begin(), 2);
        ctx.functions[name] = convertType(typeNode->typeId());
    }
    auto moduleOp = Operation::make<ModuleOp>();
    ctx.goInto(moduleOp.op);
    for (const auto &child : node->children)
        processNode(child, ctx);
}

void processFunctionDefinition(const Node::Ptr &node, ConverterContext &ctx) {
    auto it = node->children.begin();
    const std::string &name = (*it)->str();
    ++it;
    Type::PtrVector arguments;
    std::vector<std::string> argNames;
    for (const auto &argNode : (*it)->children) {
        arguments.push_back(convertType(argNode->firstChild()->typeId()));
        argNames.push_back(argNode->lastChild()->str());
    }
    ++it;
    auto funcType = Type::make<FunctionType>(arguments, convertType((*it)->typeId()));
    auto funcOp = ctx.insert<FunctionOp>(node->ref, name, funcType);
    ctx.goInto(funcOp.op);
    ctx.enterScope();
    for (size_t i = 0; i < argNames.size(); i++)
        ctx.saveVariable(argNames[i], funcOp.op->inward(i), false);
    ++it;
    processNode(*it, ctx);
    ctx.exitScope();
    ctx.goParent();
}

void processBranchRoot(const Node::Ptr &node, ConverterContext &ctx) {
    ctx.enterScope();
    for (const auto &child : node->children)
        processNode(child, ctx);
    ctx.exitScope();
}

void processVariableDeclaration(const Node::Ptr &node, ConverterContext &ctx) {
    const auto &name = node->secondChild()->str();
    if (ctx.wouldBeRedeclaration(name)) {
        ctx.pushError(node, "variable is already declared:" + name);
        return;
    }
    auto type = convertType(node->firstChild()->typeId());
    auto allocOp = ctx.insert<AllocateOp>(node->ref, Type::make<PointerType>(type));
    ctx.saveVariable(name, allocOp.result(), true);
    if (node->children.size() == 3U) {
        auto intitializer = node->lastChild();
        auto definition = visitNode(intitializer, ctx);
        ctx.insert<StoreOp>(intitializer->ref, allocOp.result(), definition);
    }
}

void processReturnStatement(const Node::Ptr &node, ConverterContext &ctx) {
    if (node->children.empty())
        ctx.insert<ReturnOp>(node->ref);
    else
        ctx.insert<ReturnOp>(node->ref, visitNode(node->firstChild(), ctx));
}

void processWhileStatement(const Node::Ptr &node, ConverterContext &ctx) {
    auto whileOp = ctx.insert<WhileOp>(node->ref);
    ctx.enterScope();
    ctx.goInto(whileOp.conditionOp().op);
    processNode(node->firstChild(), ctx);
    ctx.goParent();
    ctx.exitScope();
    ctx.enterScope();
    processNode(node->lastChild(), ctx);
    ctx.goParent();
    ctx.exitScope();
}

Value::Ptr visitExpression(const Node::Ptr &node, ConverterContext &ctx) {
    return visitNode(node->firstChild(), ctx);
}

Value::Ptr visitIntegerLiteralValue(const Node::Ptr &node, ConverterContext &ctx) {
    return ctx.insert<ConstantOp>(node->ref, TypeStorage::integerType(), node->intNum()).result();
}

Value::Ptr visitFloatingPointLiteralValue(const Node::Ptr &node, ConverterContext &ctx) {
    return ctx.insert<ConstantOp>(node->ref, TypeStorage::floatType(), node->fpNum()).result();
}

Value::Ptr visitStringLiteralValue(const Node::Ptr &node, ConverterContext &ctx) {
    return ctx.insert<ConstantOp>(node->ref, TypeStorage::strType(), node->str()).result();
}

Value::Ptr visitBinaryOperation(const Node::Ptr &node, ConverterContext &ctx) {
    auto lhs = visitNode(node->firstChild(), ctx);
    auto rhs = visitNode(node->lastChild(), ctx);
    const Type::Ptr &lhsType = lhs->type;
    const Type::Ptr &rhsType = rhs->type;
    auto binOp = node->binOp();
    if (lhsType != rhsType) {
        if (lhsType->is<IntegerType>() && rhsType->is<FloatType>()) {
            if (binOp == ast::BinaryOperation::Assign || binOp == ast::BinaryOperation::FAssign)
                rhs = ctx.insert<ArithCastOp>(ArithCastOpKind::FloatToInt, lhsType, rhs).result();
            else
                lhs = ctx.insert<ArithCastOp>(ArithCastOpKind::IntToFloat, rhsType, lhs).result();
        } else if (lhsType->is<FloatType>() && rhsType->is<IntegerType>()) {
            rhs = ctx.insert<ArithCastOp>(ArithCastOpKind::IntToFloat, lhsType, rhs).result();
        }
    }
    auto makeArithBinaryOp = [&](ArithBinOpKind kind) {
        return ctx.insert<ArithBinaryOp>(node->ref, kind, lhs, rhs).result();
    };
    auto makeLogicBinaryOp = [&](LogicBinOpKind kind) {
        return ctx.insert<LogicBinaryOp>(node->ref, kind, lhs, rhs).result();
    };
    switch (binOp) {
    case ast::BinaryOperation::Add:
        return makeArithBinaryOp(ArithBinOpKind::AddI);
    case ast::BinaryOperation::Sub:
        return makeArithBinaryOp(ArithBinOpKind::SubI);
    case ast::BinaryOperation::Mult:
        return makeArithBinaryOp(ArithBinOpKind::MulI);
    case ast::BinaryOperation::Div:
        return makeArithBinaryOp(ArithBinOpKind::DivI);
    case ast::BinaryOperation::FAdd:
        return makeArithBinaryOp(ArithBinOpKind::AddF);
    case ast::BinaryOperation::FSub:
        return makeArithBinaryOp(ArithBinOpKind::SubF);
    case ast::BinaryOperation::FMult:
        return makeArithBinaryOp(ArithBinOpKind::MulF);
    case ast::BinaryOperation::FDiv:
        return makeArithBinaryOp(ArithBinOpKind::DivF);
    case ast::BinaryOperation::Equal:
        return makeLogicBinaryOp(LogicBinOpKind::Equal);
    case ast::BinaryOperation::NotEqual:
        return makeLogicBinaryOp(LogicBinOpKind::NotEqual);
    case ast::BinaryOperation::Less:
        return makeLogicBinaryOp(LogicBinOpKind::LessI);
    case ast::BinaryOperation::Greater:
        return makeLogicBinaryOp(LogicBinOpKind::GreaterI);
    case ast::BinaryOperation::LessEqual:
        return makeLogicBinaryOp(LogicBinOpKind::LessEqualI);
    case ast::BinaryOperation::GreaterEqual:
        return makeLogicBinaryOp(LogicBinOpKind::GreaterEqualI);
    case ast::BinaryOperation::FLess:
        return makeLogicBinaryOp(LogicBinOpKind::LessF);
    case ast::BinaryOperation::FGreater:
        return makeLogicBinaryOp(LogicBinOpKind::GreaterF);
    case ast::BinaryOperation::FLessEqual:
        return makeLogicBinaryOp(LogicBinOpKind::LessEqualF);
    case ast::BinaryOperation::FGreaterEqual:
        return makeLogicBinaryOp(LogicBinOpKind::GreaterEqualF);
    case ast::BinaryOperation::And:
        return makeLogicBinaryOp(LogicBinOpKind::AndI);
    case ast::BinaryOperation::Or:
        return makeLogicBinaryOp(LogicBinOpKind::OrI);
    case ast::BinaryOperation::Assign:
    case ast::BinaryOperation::FAssign:
        ctx.insert<StoreOp>(node->ref, lhs, rhs);
        return rhs;
    }
    UNREACHABLE("Unexpected ast::BinaryOperation value in visitBinaryOperation");
}

Value::Ptr visitVariableName(const Node::Ptr &node, ConverterContext &ctx) {
    auto var = ctx.findVariable(node->str());
    if (!var.value) {
        ctx.pushError(node, "variable was not declared in this scope: " + node->str());
        throw ctx.errors;
    }
    if (isLhsInAssignment(node) || !var.needsLoad)
        return var.value;
    return ctx.insert<LoadOp>(node->ref, var.value).result();
}

Value::Ptr visitFunctionCall(const Node::Ptr &node, ConverterContext &ctx) {
    const std::string &name = node->firstChild()->str();
    if (!ctx.functions.contains(name)) {
        ctx.pushError(node, "call to undefined function: " + name);
        throw ctx.errors;
    }
    std::vector<Value::Ptr> arguments;
    for (auto &argNode : node->lastChild()->children) {
        arguments.emplace_back(visitNode(argNode, ctx));
    }
    return ctx.insert<FunctionCallOp>(node->ref, name, ctx.functions[name], arguments).result();
}

void processNode(const Node::Ptr &node, ConverterContext &ctx) {
    switch (node->type) {
    case NodeType::ProgramRoot:
        processProgramRoot(node, ctx);
        return;
    case NodeType::FunctionDefinition:
        processFunctionDefinition(node, ctx);
        return;
    case NodeType::BranchRoot:
        processBranchRoot(node, ctx);
        return;
    case NodeType::VariableDeclaration:
        processVariableDeclaration(node, ctx);
        return;
    case NodeType::Expression:
        visitExpression(node, ctx);
        return;
    case NodeType::ReturnStatement:
        processReturnStatement(node, ctx);
        return;
    case NodeType::WhileStatement:
        processWhileStatement(node, ctx);
        return;
    }
    UNREACHABLE("Unexpected ast::NodeType value in processNode");
}

Value::Ptr visitNode(const Node::Ptr &node, ConverterContext &ctx) {
    switch (node->type) {
    case NodeType::Expression:
        return visitExpression(node, ctx);
    case NodeType::IntegerLiteralValue:
        return visitIntegerLiteralValue(node, ctx);
    case NodeType::FloatingPointLiteralValue:
        return visitFloatingPointLiteralValue(node, ctx);
    case NodeType::StringLiteralValue:
        return visitStringLiteralValue(node, ctx);
    case NodeType::BinaryOperation:
        return visitBinaryOperation(node, ctx);
    case NodeType::VariableName:
        return visitVariableName(node, ctx);
    case NodeType::FunctionCall:
        return visitFunctionCall(node, ctx);
    }
    UNREACHABLE("Unexpected ast::NodeType value in visitNode");
}

Program Converter::process(const SyntaxTree &syntaxTree) {
    ConverterContext ctx;
    processNode(syntaxTree.root, ctx);
    if (!ctx.errors.empty())
        throw ctx.errors;
    Program program;
    program.root = ctx.op;
    return program;
}
