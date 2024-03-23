#pragma once

#include "compiler/ast/syntax_tree.hpp"
#include "compiler/optree/program.hpp"

namespace optree {

namespace converter {

class Converter {
  public:
    Converter() = delete;
    Converter(const Converter &) = delete;
    Converter(Converter &&) = delete;
    ~Converter() = delete;

    static Program process(const ast::SyntaxTree &syntaxTree);
};

} // namespace converter

} // namespace optree
