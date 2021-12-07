#pragma once

#include <string>

#include <ast/syntax_tree.hpp>
#include <backend/lexer/tokenlist.hpp>
#include <backend/stringvec.hpp>

namespace serialization {

std::string serialize(const StringVec &strings);
std::string serialize(const lexer::TokenList &tokens);
std::string serialize(const ast::SyntaxTree &tree);

} // namespace serialization
