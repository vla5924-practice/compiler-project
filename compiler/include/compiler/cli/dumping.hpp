#pragma once

#include <string>

#include "compiler/frontend/lexer/token.hpp"
#include "compiler/utils/stringvec.hpp"

namespace dumping {

std::string dump(const StringVec &strings);
std::string dump(const lexer::TokenList &tokens);

} // namespace dumping
