#include "lexer/tokenlist.hpp"

using namespace lexer;

std::ostream &operator<<(std::ostream &out, const TokenList &list) {
    for (const Token &token : list) {
        if (token.type == Token::Type::Keyword) {
            out << "KEYWORD" << std::endl;
        } else if (token.type == Token::Type::Operator) {
            out << "OPERATOR" << std::endl;
        } else if (token.type == Token::Type::Identifier) {
            out << "IDENTIFIER " << std::endl;
        } else if (token.type == Token::Type::IntegerLiteral) {
            out << "INTEGER" << std::endl;
        } else if (token.type == Token::Type::FloatingPointLiteral) {
            out << "FLOAT" << std::endl;
        } else if (token.type == Token::Type::StringLiteral) {
            out << "STRING" << std::endl;
        }
    }
    return out;
}
