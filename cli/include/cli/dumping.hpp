#pragma once

#include <string>

#include <backend/lexer/tokenlist.hpp>
#include <backend/stringvec.hpp>

namespace dumping {

std::string dump(const StringVec &strings);
std::string dump(const lexer::TokenList &tokens);

} // namespace dumping
