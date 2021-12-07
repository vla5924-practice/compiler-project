#include "serialization.hpp"

#include <sstream>

using namespace ast;
using namespace lexer;

namespace {

const char *const binaryOperationToString(BinaryOperation binOp) {
    switch (binOp) {
    case BinaryOperation::Add:
        return "Add";
    case BinaryOperation::And:
        return "And";
    case BinaryOperation::Assign:
        return "Assign";
    case BinaryOperation::Div:
        return "Div";
    case BinaryOperation::Equal:
        return "Equal";
    case BinaryOperation::FAdd:
        return "FAdd";
    case BinaryOperation::FDiv:
        return "FDiv";
    case BinaryOperation::FMul:
        return "FMul";
    case BinaryOperation::FSub:
        return "FSub";
    case BinaryOperation::Greater:
        return "Greater";
    case BinaryOperation::GreaterEqual:
        return "GreaterEqual";
    case BinaryOperation::Less:
        return "Less";
    case BinaryOperation::LessEqual:
        return "LessEqual";
    case BinaryOperation::Mult:
        return "Mult";
    case BinaryOperation::NotEqual:
        return "NotEqual";
    case BinaryOperation::Or:
        return "Or";
    case BinaryOperation::Sub:
        return "Sub";
    case BinaryOperation::Unknown:
        return "Unknown";
    }
    return "";
}

const char *const unaryOperationToString(UnaryOperation unOp) {
    switch (unOp) {
    case UnaryOperation::Not:
        return "Not";
    case UnaryOperation::Unknown:
        return "Unknown";
    }
    return "";
}

const char *const typeIdToString(TypeId typeId) {
    switch (typeId) {
    case IntType:
        return "IntType";
    case FloatType:
        return "FloatType";
    case StrType:
        return "StrType";
    case NoneType:
        return "NoneType";
    }
    return "";
}

void serializeNode(Node *node, std::stringstream &str, int depth) {
    for (int i = 0; i < depth; i++)
        str << "  ";
    // str << "0x" << node << " ";
    switch (node->type) {
    case NodeType::BinaryOperation:
        str << "BinaryOperation: " << binaryOperationToString(node->binOp()) << "\n";
        break;
    case NodeType::BranchRoot:
        str << "BranchRoot\n";
        break;
    case NodeType::ElifStatement:
        str << "ElifStatement\n";
        break;
    case NodeType::Expression:
        str << "Expression\n";
        break;
    case NodeType::FloatingPointLiteralValue:
        str << "FloatingPointLiteralValue: " << node->fpNum() << "\n";
        break;
    case NodeType::FunctionArgument:
        str << "FunctionArgument\n";
        break;
    case NodeType::FunctionArguments:
        str << "FunctionArguments\n";
        break;
    case NodeType::FunctionDefinition:
        str << "FunctionDefinition\n";
        break;
    case NodeType::FunctionName:
        str << "FunctionName: " << node->str() << "\n";
        break;
    case NodeType::FunctionReturnType:
        if (node->typeId() >= BuiltInTypesCount)
            str << "FunctionReturnType: user-defined(" << node->typeId() << ")\n";
        else
            str << "FunctionReturnType: " << typeIdToString(node->typeId()) << "\n";
        break;
    case NodeType::IfStatement:
        str << "IfStatement\n";
        break;
    case NodeType::IntegerLiteralValue:
        str << "IntegerLiteralValue: " << node->intNum() << "\n";
        break;
    case NodeType::ProgramRoot:
        str << "ProgramRoot\n";
        break;
    case NodeType::ReturnStatement:
        str << "ReturnStatement\n";
        break;
    case NodeType::StringLiteralValue:
        str << "StringLiteralValue: " << node->str() << "\n";
        break;
    case NodeType::TypeConversion:
        str << "TypeConversion\n";
        break;
    case NodeType::TypeName:
        if (node->typeId() >= BuiltInTypesCount)
            str << "TypeName: user-defined(" << node->typeId() << ")\n";
        else
            str << "TypeName: " << typeIdToString(node->typeId()) << "\n";
        break;
    case NodeType::UnaryOperation:
        str << "UnaryOperation: " << unaryOperationToString(node->unOp()) << "\n";
        break;
    case NodeType::VariableDeclaration:
        str << "VariableDeclaration\n";
        break;
    case NodeType::VariableName:
        str << "VariableName: " << node->str() << "\n";
        break;
    case NodeType::WhileStatement:
        str << "WhileStatement\n";
        break;
    default:
        str << "Unknown\n";
    }
    for (auto child : node->children)
        serializeNode(child.get(), str, depth + 1);
}

} // namespace

namespace serialization {

std::string serialize(const StringVec &strings) {
    std::stringstream str;
    for (const auto &line : strings) {
        str << line << '\n';
    }
    return str.str();
}

std::string serialize(const TokenList &tokens) {
    std::stringstream str;
    for (const auto &token : tokens) {
        str << token.dump() << '\n';
    }
    return str.str();
}

std::string serialize(const SyntaxTree &tree) {
    std::stringstream str;
    serializeNode(tree.root.get(), str, 0);
    return str.str();
}

} // namespace serialization
