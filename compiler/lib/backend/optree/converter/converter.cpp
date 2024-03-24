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
    auto [op, constOp] = ctx.addToBody<ConstantOp>(IntegerType(), node->intNum());
    return constOp.result();
}

Value::Ptr visitFloatingPointLiteralValue(const Node::Ptr &node, ConverterContext &ctx) {
    auto [op, constOp] = ctx.addToBody<ConstantOp>(FloatType(), node->fpNum());
    return constOp.result();
}

Value::Ptr visitStringLiteralValue(const Node::Ptr &node, ConverterContext &ctx) {
    auto [op, constOp] = ctx.addToBody<ConstantOp>(StrType(), node->str());
    return constOp.result();
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
