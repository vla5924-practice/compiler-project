#include "lexer/token.hpp"

#include <sstream>

using namespace lexer;

namespace {

const char *const keywordToString(Keyword kw) {
    switch (kw) {
    case Keyword::And:
        return "and";
    case Keyword::Bool:
        return "bool";
    case Keyword::Break:
        return "break";
    case Keyword::Continue:
        return "continue";
    case Keyword::Definition:
        return "def";
    case Keyword::Elif:
        return "elif";
    case Keyword::Else:
        return "else";
    case Keyword::False:
        return "False";
    case Keyword::Float:
        return "float";
    case Keyword::For:
        return "for";
    case Keyword::If:
        return "if";
    case Keyword::Import:
        return "import";
    case Keyword::In:
        return "in";
    case Keyword::Int:
        return "int";
    case Keyword::None:
        return "None";
    case Keyword::Not:
        return "not";
    case Keyword::Or:
        return "or";
    case Keyword::Range:
        return "range";
    case Keyword::Return:
        return "return";
    case Keyword::Str:
        return "str";
    case Keyword::True:
        return "True";
    case Keyword::While:
        return "while";
    }
    return "";
}

const char *const operatorToString(Operator op) {
    switch (op) {
    case Operator::Add:
        return "+";
    case Operator::Assign:
        return "=";
    case Operator::Comma:
        return ",";
    case Operator::Div:
        return "/";
    case Operator::Dot:
        return ".";
    case Operator::Equal:
        return "==";
    case Operator::Greater:
        return ">";
    case Operator::GreaterEqual:
        return ">=";
    case Operator::LeftBrace:
        return "(";
    case Operator::Less:
        return "<";
    case Operator::LessEqual:
        return "<=";
    case Operator::Mod:
        return "%";
    case Operator::Mult:
        return "*";
    case Operator::NotEqual:
        return "!=";
    case Operator::RectLeftBrace:
        return "[";
    case Operator::RectRightBrace:
        return "]";
    case Operator::RightBrace:
        return ")";
    case Operator::Sub:
        return "-";
    }
    return "";
}

const char *const specialToString(Special spec) {
    switch (spec) {
    case Special::Arrow:
        return "Arrow";
    case Special::Colon:
        return "Colon";
    case Special::EndOfExpression:
        return "EndOfExpression";
    case Special::Indentation:
        return "Indentation";
    }
    return "";
}

} // namespace

std::string Token::dump() const {
    std::stringstream str;
    // str << "0x" << &token << " ";
    switch (type) {
    case TokenType::Identifier:
        str << "Identifier           : " << id();
        break;
    case TokenType::Keyword:
        str << "Keyword              : " << keywordToString(kw());
        break;
    case TokenType::Operator:
        str << "Operator             : " << operatorToString(op());
        break;
    case TokenType::Special:
        str << "Special              : " << specialToString(spec());
        break;
    case TokenType::FloatingPointLiteral:
        str << "FloatingPointLiteral : " << literal();
        break;
    case TokenType::IntegerLiteral:
        str << "IntegerLiteral       : " << literal();
        break;
    case TokenType::StringLiteral:
        str << "StringLiteral        : " << literal();
        break;
    }
    return str.str();
}
