#include "syntax_tree.hpp"

#include <sstream>

using namespace ast;

std::string SyntaxTree::dump() const {
    std::stringstream str;
    dump(str);
    return str.str();
}

void SyntaxTree::dump(std::ostream &stream) const {
    root->dump(stream);
}
