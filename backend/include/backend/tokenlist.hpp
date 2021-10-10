#include <iostream>
#include <list>

#include "token.hpp"

using TokenList = std::list<Token>;

std::ostream &operator<<(std::ostream &out, const TokenList &list);
