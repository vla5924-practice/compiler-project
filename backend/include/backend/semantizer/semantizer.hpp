#pragma once

#include "error_buffer.hpp"
#include "semantizer_error.hpp"
#include <ast/syntax_tree.hpp>
#include <ast/types.hpp>

namespace semantizer {

class Semantizer {
  public:
    Semantizer() = delete;
    Semantizer(const Semantizer &) = delete;
    Semantizer(Semantizer &&) = delete;
    ~Semantizer() = delete;

    static void process(ast::SyntaxTree &tree);
};

} // namespace semantizer
