#include "dumping.hpp"

#include <sstream>

using lexer::TokenList;

namespace dumping {

std::string dump(const StringVec &strings) {
    std::stringstream str;
    for (const auto &line : strings) {
        str << line << '\n';
    }
    return str.str();
}

std::string dump(const TokenList &tokens) {
    std::stringstream str;
    for (const auto &token : tokens) {
        str << token.dump() << '\n';
    }
    return str.str();
}

} // namespace dumping
