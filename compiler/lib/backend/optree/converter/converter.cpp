#include "converter/converter.hpp"

#include "compiler/optree/adaptors.hpp"

#include "converter/converter_context.hpp"

using namespace optree;
using namespace converter;

using ast::Node;
using ast::NodeType;
using ast::SyntaxTree;

Type convertType(ast::TypeId typeId) {
    switch (typeId) {
    case ast::NoneType:
        return NoneType();
    case ast::IntType:
        return IntegerType(64U);
    case ast::FloatType:
        return FloatType(64U);
    case ast::StrType:
        return StrType(8U);
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
    ctx.op = Operation::make<ModuleOp>();
    for (const auto &child : node->children)
        processNode(child, ctx);
}

void processFunctionDefinition(const Node::Ptr &node, ConverterContext &ctx) {
    auto [op, funcOp] = ctx.addToBody<FunctionOp>();
    auto it = node->children.begin();
    funcOp.setName((*it)->str());
    ++it;
    std::vector<Type> arguments;
    for (const auto &typeNode : (*it)->children)
        arguments.push_back(convertType(typeNode->typeId()));
    ++it;
    funcOp.setType(FunctionType(arguments, convertType((*it)->typeId())));
    ctx.op = op;
    ++it;
    visitNode(*it, ctx);
    ctx.goParent();
}

void processBranchRoot(const Node::Ptr &node, ConverterContext &ctx) {
    ctx.enterScope();
    for (const auto &child : node->children)
        visitNode(child, ctx);
    ctx.exitScope();
}

void processVariableDeclaration(const Node::Ptr &node, ConverterContext &ctx) {
    auto type = convertType(node->firstChild()->typeId());
    auto [op, allocOp] = ctx.addToBody<AllocateOp>(PointerType(type));
    const auto &name = node->secondChild()->str();
    ctx.saveVariable(name, allocOp.result());
    if (node->children.size() == 3U)
        visitNode(node->lastChild(), ctx);
}

Value::Ptr visitExpression(const Node::Ptr &node, ConverterContext &ctx) {
    return visitNode(node->firstChild(), ctx);
}

Value::Ptr visitIntegerLiteralValue(const Node::Ptr &node, ConverterContext &ctx) {
    return ctx.addToBodyWrap<ConstantOp>(IntegerType(), node->intNum()).result();
}

Value::Ptr visitFloatingPointLiteralValue(const Node::Ptr &node, ConverterContext &ctx) {
    return ctx.addToBodyWrap<ConstantOp>(FloatType(), node->fpNum()).result();
}

Value::Ptr visitStringLiteralValue(const Node::Ptr &node, ConverterContext &ctx) {
    return ctx.addToBodyWrap<ConstantOp>(StrType(), node->str()).result();
}

Value::Ptr visitBinaryOperation(const Node::Ptr &node, ConverterContext &ctx) {
    auto lhs = visitNode(node->firstChild(), ctx);
    auto rhs = visitNode(node->lastChild(), ctx);
    const Type &lhsType = lhs->type;
    const Type &rhsType = rhs->type;
    auto binOp = node->binOp();
    if (lhsType != rhsType) {
        if (lhsType.is<IntegerType>() && rhsType.is<FloatType>()) {
            if (binOp == ast::BinaryOperation::Assign || binOp == ast::BinaryOperation::FAssign)
                rhs = ctx.addToBodyWrap<ArithCastOp>(ArithCastOpKind::FloatToInt, lhsType, rhs).result();
            else
                lhs = ctx.addToBodyWrap<ArithCastOp>(ArithCastOpKind::IntToFloat, rhsType, lhs).result();
        } else if (lhsType.is<FloatType>() && rhsType.is<IntegerType>()) {
            rhs = ctx.addToBodyWrap<ArithCastOp>(ArithCastOpKind::IntToFloat, lhsType, rhs).result();
        }
    }
    auto makeArithBinaryOp = [&](ArithBinOpKind kind) {
        return ctx.addToBodyWrap<ArithBinaryOp>(kind, lhs, rhs).result();
    };
    auto makeLogicBinaryOp = [&](LogicBinOpKind kind) {
        return ctx.addToBodyWrap<LogicBinaryOp>(kind, lhs, rhs).result();
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
        ctx.addToBody<StoreOp>(lhs, rhs);
        return rhs;
    }
    return {};
}

Value::Ptr visitVariableName(const Node::Ptr &node, ConverterContext &ctx) {
    auto value = ctx.findVariable(node->str());
    // TODO: error if var == nullptr
    if (isLhsInAssignment(node))
        return value;
    return ctx.addToBodyWrap<LoadOp>(value).result();
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
    }
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
    }
    return {};
}

Program Converter::process(const SyntaxTree &syntaxTree) {
    ConverterContext ctx;
    visitNode(syntaxTree.root, ctx);
    Program program;
    program.root = ctx.op;
    return program;
}
