#include "lexer/tokenlist.hpp"

using namespace lexer;

std::ostream &operator<<(std::ostream &out, const TokenList &list) {
    for (const Token &token : list) {
        if (token.type == TokenType::Keyword) {
            out << "KEYWORD" << std::endl;
        } else if (token.type == TokenType::Operator) {
            out << "OPERATOR" << std::endl;
        } else if (token.type == TokenType::Identifier) {
            out << "IDENTIFIER " << std::endl;
        } else if (token.type == TokenType::IntegerLiteral) {
            out << "INTEGER" << std::endl;
        } else if (token.type == TokenType::FloatingPointLiteral) {
            out << "FLOAT" << std::endl;
        } else if (token.type == TokenType::StringLiteral) {
            out << "STRING" << std::endl;
        } else if (token.type == TokenType::Special) {
            out << "SPECIAL" << std::endl;
        }
    }
    return out;
}
