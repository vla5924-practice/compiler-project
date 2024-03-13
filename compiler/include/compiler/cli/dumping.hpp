#pragma once

#include <string>

#include "compiler/backend/lexer/token.hpp"
#include "compiler/backend/stringvec.hpp"

namespace dumping {

std::string dump(const StringVec &strings);
std::string dump(const lexer::TokenList &tokens);

} // namespace dumping
