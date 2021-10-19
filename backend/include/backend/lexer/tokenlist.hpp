#pragma once

#include <iostream>
#include <list>

#include "lexer/token.hpp"

namespace lexer {

using TokenList = std::list<Token>;

std::ostream &operator<<(std::ostream &out, const TokenList &list);

} // namespace lexer
