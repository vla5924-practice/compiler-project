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

void visitNode(const Node::Ptr &node, ConverterContext &ctx);

void visitProgramRoot(const Node::Ptr &node, ConverterContext &ctx) {
    ctx.op = Operation::make<ModuleOp>();
    for (const auto &child : node->children)
        visitNode(child, ctx);
}

void visitFunctionDefinition(const Node::Ptr &node, ConverterContext &ctx) {
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

void visitBranchRoot(const Node::Ptr &node, ConverterContext &ctx) {
    for (const auto &child : node->children)
        visitNode(child, ctx);
}

void visitNode(const Node::Ptr &node, ConverterContext &ctx) {
    switch (node->type) {
    case NodeType::ProgramRoot:
        visitProgramRoot(node, ctx);
        return;
    case NodeType::FunctionDefinition:
        visitFunctionDefinition(node, ctx);
        return;
    case NodeType::BranchRoot:
        visitBranchRoot(node, ctx);
        return;
    }
}

Program Converter::process(const SyntaxTree &syntaxTree) {
    ConverterContext ctx;
    visitNode(syntaxTree.root, ctx);
    Program program;
    program.root = ctx.op;
    return program;
}
