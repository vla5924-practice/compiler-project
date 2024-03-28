#pragma once

#include "compiler/ast/syntax_tree.hpp"
#include "compiler/optree/program.hpp"

namespace converter {

class Converter {
  public:
    Converter() = delete;
    Converter(const Converter &) = delete;
    Converter(Converter &&) = delete;
    ~Converter() = delete;

    static optree::Program process(const ast::SyntaxTree &syntaxTree);
};

} // namespace converter
