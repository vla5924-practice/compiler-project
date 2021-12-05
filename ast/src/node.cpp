#include "node.hpp"

#include <sstream>

using namespace ast;

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

} // namespace

void Node::dump(std::ostream &stream, int depth) const {
    for (int i = 0; i < depth; i++)
        stream << "  ";
    // stream << "0x" << this << " ";
    switch (type) {
    case NodeType::BinaryOperation:
        stream << "BinaryOperation: " << binaryOperationToString(binOp()) << "\n";
        break;
    case NodeType::BranchRoot:
        stream << "BranchRoot\n";
        break;
    case NodeType::ElifStatement:
        stream << "ElifStatement\n";
        break;
    case NodeType::Expression:
        stream << "Expression\n";
        break;
    case NodeType::FloatingPointLiteralValue:
        stream << "FloatingPointLiteralValue: " << fpNum() << "\n";
        break;
    case NodeType::FunctionArgument:
        stream << "FunctionArgument\n";
        break;
    case NodeType::FunctionArguments:
        stream << "FunctionArguments\n";
        break;
    case NodeType::FunctionCall:
        stream << "FunctionCall\n";
        break;
    case NodeType::FunctionDefinition:
        stream << "FunctionDefinition\n";
        break;
    case NodeType::FunctionName:
        stream << "FunctionName: " << str() << "\n";
        break;
    case NodeType::FunctionReturnType:
        if (typeId() >= BuiltInTypesCount)
            stream << "FunctionReturnType: user-defined(" << typeId() << ")\n";
        else
            stream << "FunctionReturnType: " << typeIdToString(typeId()) << "\n";
        break;
    case NodeType::IfStatement:
        stream << "IfStatement\n";
        break;
    case NodeType::IntegerLiteralValue:
        stream << "IntegerLiteralValue: " << intNum() << "\n";
        break;
    case NodeType::ProgramRoot:
        stream << "ProgramRoot\n";
        break;
    case NodeType::ReturnStatement:
        stream << "ReturnStatement\n";
        break;
    case NodeType::StringLiteralValue:
        stream << "StringLiteralValue: " << str() << "\n";
        break;
    case NodeType::TypeConversion:
        stream << "TypeConversion\n";
        break;
    case NodeType::TypeName:
        if (typeId() >= BuiltInTypesCount)
            stream << "TypeName: user-defined(" << typeId() << ")\n";
        else
            stream << "TypeName: " << typeIdToString(typeId()) << "\n";
        break;
    case NodeType::UnaryOperation:
        stream << "UnaryOperation: " << unaryOperationToString(unOp()) << "\n";
        break;
    case NodeType::VariableDeclaration:
        stream << "VariableDeclaration\n";
        break;
    case NodeType::VariableName:
        stream << "VariableName: " << str() << "\n";
        break;
    case NodeType::WhileStatement:
        stream << "WhileStatement\n";
        break;
    default:
        stream << "Unknown\n";
    }
    for (auto child : children)
        child->dump(stream, depth + 1);
}

std::string Node::dump(int depth) const {
    std::stringstream str;
    dump(str);
    return str.str();
}
