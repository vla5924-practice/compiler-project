#include <iostream>

#include "compiler/backend/optree/converter/converter.hpp"
#include "compiler/frontend/lexer/lexer.hpp"
#include "compiler/frontend/parser/parser.hpp"
#include "compiler/utils/stringvec.hpp"

using namespace lexer;
using namespace parser;
using namespace optree::converter;

int main() {
    StringVec source = {"def main() -> None:", "    x: float = 1 + 1.0"};
    auto token_list = Lexer::process(source);
    auto tree = Parser::process(token_list);
    tree.dump(std::cout);
    auto program = Converter::process(tree);
    program.root->dump(std::cout);
    return 0;
}
