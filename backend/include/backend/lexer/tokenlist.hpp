#pragma once

#include <iostream>
#include <list>

#include "lexer/token.hpp"

namespace lexer {

using TokenList = std::list<Token>;

} // namespace lexer

std::ostream &operator<<(std::ostream &out, const lexer::TokenList &list);
