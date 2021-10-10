#include "parser.hpp"

template <>
int Parser::parseLiteral(const Token &token) {
    return atoi(token.literal().c_str());
}

template <>
long Parser::parseLiteral(const Token &token) {
    return atol(token.literal().c_str());
}

template <>
double Parser::parseLiteral(const Token &token) {
    return atof(token.literal().c_str());
}

template <>
std::string_view Parser::parseLiteral(const Token &token) {
    return token.literal();
}

template <>
std::string Parser::parseLiteral(const Token &token) {
    return token.literal();
}
