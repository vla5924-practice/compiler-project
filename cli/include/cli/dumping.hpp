#pragma once

#include <string>

#include <ast/syntax_tree.hpp>
#include <backend/lexer/tokenlist.hpp>
#include <backend/stringvec.hpp>

namespace dumping {

std::string dump(const StringVec &strings);
std::string dump(const lexer::TokenList &tokens);
std::string dump(const ast::SyntaxTree &tree);

} // namespace dumping
