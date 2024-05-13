#include "converter/converter.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "compiler/ast/node_type.hpp"
#include "compiler/ast/syntax_tree.hpp"
#include "compiler/ast/types.hpp"
#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/definitions.hpp"
#include "compiler/optree/helpers.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/optree/program.hpp"
#include "compiler/optree/types.hpp"
#include "compiler/optree/value.hpp"
#include "compiler/utils/helpers.hpp"
#include "compiler/utils/language.hpp"
#include "compiler/utils/source_ref.hpp"

#include "converter/converter_context.hpp"

using namespace optree;
using namespace converter;

using ast::Node;
using ast::NodeType;
using ast::SyntaxTree;

namespace language = utils::language;

namespace {

Type::Ptr convertType(ast::TypeId typeId) {
    switch (typeId) {
    case ast::IntType:
        return TypeStorage::integerType();
    case ast::FloatType:
        return TypeStorage::floatType();
    case ast::StrType:
        return TypeStorage::strType();
    case ast::NoneType:
        return TypeStorage::noneType();
    case ast::BoolType:
        return TypeStorage::boolType();
    default:
        return {};
    }
}

std::string_view prettyTypeName(const Type::Ptr &type) {
    if (type->is<NoneType>())
        return "None";
    if (type->is<IntegerType>())
        return "int";
    if (type->is<BoolType>())
        return "bool";
    if (type->is<FloatType>())
        return "float";
    if (type->is<StrType>())
        return "str";
    if (type->is<FunctionType>())
        return "<internal-function-type>";
    if (type->is<PointerType>())
        return prettyTypeName(type->as<PointerType>().pointee);
    return "<undefined-type>";
}

template <const Node::Ptr &(Node::*childAccessor)(void) const>
bool isSomewhereInAssignment(const Node::Ptr &node) {
    const auto &parent = node->parent;
    return node->type == ast::NodeType::VariableName && parent->type == ast::NodeType::BinaryOperation &&
           (parent->binOp() == ast::BinaryOperation::Assign || parent->binOp() == ast::BinaryOperation::FAssign) &&
           (parent.get()->*childAccessor)() == node;
}

bool isLhsInAssignment(const Node::Ptr &node) {
    return isSomewhereInAssignment<&Node::firstChild>(node);
}

bool isRhsInAssignment(const Node::Ptr &node) {
    return isSomewhereInAssignment<&Node::lastChild>(node);
}

bool isFunctionCallInputNode(const Node::Ptr &node) {
    return node->type == NodeType::FunctionCall && node->firstChild()->str() == language::funcInput;
}

bool isAssignment(ast::BinaryOperation binOp) {
    return binOp == ast::BinaryOperation::Assign || binOp == ast::BinaryOperation::FAssign;
}

void createInputOp(const Node::Ptr &varNameNode, const utils::SourceRef &inputRef, ConverterContext &ctx) {
    auto var = ctx.findVariable(varNameNode->str());
    if (!var.value) {
        ctx.pushError(varNameNode, "variable was not declared in this scope: " + varNameNode->str());
        return;
    }
    if (!var.needsLoad) {
        ctx.pushError(varNameNode, "variable cannot be modified: " + varNameNode->str());
        return;
    }
    ctx.insert<InputOp>(inputRef, var.value);
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
    ctx.goInto(moduleOp);
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
    ctx.goInto(funcOp);
    ctx.enterScope();
    for (size_t i = 0; i < argNames.size(); i++)
        ctx.saveVariable(argNames[i], funcOp->inward(i), false);
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
    auto &nameNode = node->secondChild();
    const auto &name = nameNode->str();
    if (ctx.wouldBeRedeclaration(name)) {
        ctx.pushError(node, "variable is already declared:" + name);
        return;
    }
    auto type = convertType(node->firstChild()->typeId());
    auto allocOp = ctx.insert<AllocateOp>(node->ref, Type::make<PointerType>(type));
    ctx.saveVariable(name, allocOp.result(), true);
    if (node->children.size() == 3U) {
        auto defNode = node->lastChild();
        if (defNode->type == NodeType::Expression && isFunctionCallInputNode(defNode->firstChild())) {
            createInputOp(nameNode, defNode->ref, ctx);
            return;
        }
        auto defValue = visitNode(defNode, ctx);
        const auto &defType = defValue->type;
        if (*type != *defType) {
            if (auto castOp = insertNumericCastOp(type, defValue, ctx.builder, defNode->ref))
                defValue = castOp.result();
        }
        ctx.insert<StoreOp>(defNode->ref, allocOp.result(), defValue);
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
    ctx.goInto(whileOp.conditionOp());
    processNode(node->firstChild(), ctx);
    ctx.goParent();
    processNode(node->lastChild(), ctx);
    ctx.goParent();
}

void processIfStatement(const Node::Ptr &node, ConverterContext &ctx) {
    auto cond = visitNode(node->firstChild(), ctx);
    bool withElse = node->children.size() > 2;
    auto ifOp = ctx.insert<IfOp>(node->ref, cond, withElse);
    ctx.goInto(ifOp.thenOp());
    processNode(node->secondChild(), ctx);
    ctx.goParent();
    int depth = 0;
    auto elEnd = node->children.end();
    for (auto it = std::next(node->children.begin(), 2); it != elEnd; ++it) {
        depth++;
        ctx.goInto(ifOp.elseOp());
        const auto &elNode = *it;
        if (elNode->type == NodeType::ElseStatement) {
            processNode(elNode->firstChild(), ctx);
        } else if (elNode->type == NodeType::ElifStatement) {
            auto cond = visitNode(elNode->firstChild(), ctx);
            bool withElse = std::next(it) != elEnd;
            ifOp = ctx.insert<IfOp>(elNode->ref, cond, withElse);
            depth++;
            ctx.goInto(ifOp.thenOp());
            processNode(elNode->lastChild(), ctx);
            ctx.goParent();
        } else {
            COMPILER_UNREACHABLE("Unexpected NodeType inside IfStatement");
        }
    }
    while (depth-- > 0)
        ctx.goParent();
    ctx.goParent();
}

Value::Ptr visitExpression(const Node::Ptr &node, ConverterContext &ctx) {
    return visitNode(node->firstChild(), ctx);
}

Value::Ptr visitIntegerLiteralValue(const Node::Ptr &node, ConverterContext &ctx) {
    auto value = static_cast<NativeInt>(node->intNum());
    return ctx.insert<ConstantOp>(node->ref, TypeStorage::integerType(), value).result();
}

Value::Ptr visitBooleanLiteralValue(const Node::Ptr &node, ConverterContext &ctx) {
    auto value = static_cast<NativeBool>(node->boolean());
    return ctx.insert<ConstantOp>(node->ref, TypeStorage::boolType(), value).result();
}

Value::Ptr visitFloatingPointLiteralValue(const Node::Ptr &node, ConverterContext &ctx) {
    auto value = static_cast<NativeFloat>(node->fpNum());
    return ctx.insert<ConstantOp>(node->ref, TypeStorage::floatType(), value).result();
}

Value::Ptr visitStringLiteralValue(const Node::Ptr &node, ConverterContext &ctx) {
    auto value = static_cast<NativeStr>(node->str());
    return ctx.insert<ConstantOp>(node->ref, TypeStorage::strType(), value).result();
}

Value::Ptr visitBinaryOperation(const Node::Ptr &node, ConverterContext &ctx) {
    auto &lhsNode = node->firstChild();
    auto &rhsNode = node->secondChild();
    auto binOp = node->binOp();
    if (node->parent->type == NodeType::Expression && lhsNode->type == NodeType::VariableName && isAssignment(binOp) &&
        isFunctionCallInputNode(rhsNode)) {
        createInputOp(lhsNode, rhsNode->ref, ctx);
        return {};
    }
    auto lhs = visitNode(lhsNode, ctx);
    auto rhs = visitNode(rhsNode, ctx);
    Type::Ptr lhsType = lhs->type;
    const Type::Ptr &rhsType = rhs->type;
    auto typeError = [](const Type::Ptr &type) {
        std::stringstream error;
        error << "unexpected expression type: " << prettyTypeName(type) << ", supported types are: int, bool, float";
        return error.str();
    };
    if (isAssignment(binOp)) {
        if (lhsType->is<PointerType>())
            lhsType = lhsType->as<PointerType>().pointee;
        else
            ctx.pushError(node, "left-handed operand of an assignment expression must be a variable name");
    }
    if (!utils::isAny<IntegerType, FloatType>(lhsType)) {
        ctx.pushError(node, typeError(lhsType));
        throw ctx.errors;
    }
    if (!utils::isAny<IntegerType, FloatType>(rhsType)) {
        ctx.pushError(node, typeError(rhsType));
        throw ctx.errors;
    }
    if (*lhsType != *rhsType) {
        auto needsType = deduceTargetCastType(lhsType, rhsType, isAssignment(binOp));
        if (auto castOp = insertNumericCastOp(needsType, lhs, ctx.builder, lhsNode->ref))
            lhs = castOp.result();
        if (auto castOp = insertNumericCastOp(needsType, rhs, ctx.builder, rhsNode->ref))
            rhs = castOp.result();
    }
    auto makeArithBinaryOp = [&](ArithBinOpKind kindI, ArithBinOpKind kindF) {
        auto kind = lhs->type->is<IntegerType>() ? kindI : kindF;
        return ctx.insert<ArithBinaryOp>(node->ref, kind, lhs, rhs).result();
    };
    auto makeLogicBinaryOp = [&](LogicBinOpKind kindI, LogicBinOpKind kindF) {
        auto kind = lhs->type->is<IntegerType>() ? kindI : kindF;
        return ctx.insert<LogicBinaryOp>(node->ref, kind, lhs, rhs).result();
    };
    switch (binOp) {
    case ast::BinaryOperation::Add:
    case ast::BinaryOperation::FAdd:
        return makeArithBinaryOp(ArithBinOpKind::AddI, ArithBinOpKind::AddF);
    case ast::BinaryOperation::Sub:
    case ast::BinaryOperation::FSub:
        return makeArithBinaryOp(ArithBinOpKind::SubI, ArithBinOpKind::SubF);
    case ast::BinaryOperation::Mult:
    case ast::BinaryOperation::FMult:
        return makeArithBinaryOp(ArithBinOpKind::MulI, ArithBinOpKind::MulF);
    case ast::BinaryOperation::Div:
    case ast::BinaryOperation::FDiv:
        return makeArithBinaryOp(ArithBinOpKind::DivI, ArithBinOpKind::DivF);
    case ast::BinaryOperation::Equal:
    case ast::BinaryOperation::FEqual:
        return makeLogicBinaryOp(LogicBinOpKind::Equal, LogicBinOpKind::Equal);
    case ast::BinaryOperation::NotEqual:
    case ast::BinaryOperation::FNotEqual:
        return makeLogicBinaryOp(LogicBinOpKind::NotEqual, LogicBinOpKind::NotEqual);
    case ast::BinaryOperation::Less:
    case ast::BinaryOperation::FLess:
        return makeLogicBinaryOp(LogicBinOpKind::LessI, LogicBinOpKind::LessF);
    case ast::BinaryOperation::Greater:
    case ast::BinaryOperation::FGreater:
        return makeLogicBinaryOp(LogicBinOpKind::GreaterI, LogicBinOpKind::GreaterF);
    case ast::BinaryOperation::LessEqual:
    case ast::BinaryOperation::FLessEqual:
        return makeLogicBinaryOp(LogicBinOpKind::LessEqualI, LogicBinOpKind::LessEqualF);
    case ast::BinaryOperation::GreaterEqual:
    case ast::BinaryOperation::FGreaterEqual:
        return makeLogicBinaryOp(LogicBinOpKind::GreaterEqualI, LogicBinOpKind::GreaterEqualF);
    case ast::BinaryOperation::And:
        return makeLogicBinaryOp(LogicBinOpKind::AndI, LogicBinOpKind::Unknown);
    case ast::BinaryOperation::Or:
        return makeLogicBinaryOp(LogicBinOpKind::OrI, LogicBinOpKind::Unknown);
    case ast::BinaryOperation::Assign:
    case ast::BinaryOperation::FAssign:
        ctx.insert<StoreOp>(node->ref, lhs, rhs);
        return rhs;
    default:
        COMPILER_UNREACHABLE("Unexpected ast::BinaryOperation value in visitBinaryOperation");
    }
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
    if (name == language::funcPrint) {
        if (node->parent->type != NodeType::Expression) {
            ctx.pushError(node, "print() statement cannot be within an expression context");
            throw ctx.errors;
        }
        std::vector<Value::Ptr> arguments;
        for (auto &argNode : node->lastChild()->children)
            arguments.emplace_back(visitNode(argNode, ctx));
        ctx.insert<PrintOp>(node->ref, arguments);
        return {};
    }
    if (name == language::funcInput) {
        ctx.pushError(node, "input() statement must be a right-handed operand of an isolated assignment expression");
        throw ctx.errors;
    }
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
    case NodeType::IfStatement:
        processIfStatement(node, ctx);
        return;
    default:
        COMPILER_UNREACHABLE("Unexpected ast::NodeType value in processNode");
    }
}

Value::Ptr visitNode(const Node::Ptr &node, ConverterContext &ctx) {
    switch (node->type) {
    case NodeType::Expression:
        return visitExpression(node, ctx);
    case NodeType::IntegerLiteralValue:
        return visitIntegerLiteralValue(node, ctx);
    case NodeType::BooleanLiteralValue:
        return visitBooleanLiteralValue(node, ctx);
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
    default:
        COMPILER_UNREACHABLE("Unexpected ast::NodeType value in visitNode");
    }
}

} // namespace

Program Converter::process(const SyntaxTree &syntaxTree) {
    ConverterContext ctx;
    processNode(syntaxTree.root, ctx);
    if (!ctx.errors.empty())
        throw ctx.errors;
    return {ctx.op};
}
