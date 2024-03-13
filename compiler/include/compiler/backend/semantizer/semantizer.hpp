#pragma once

#include "compiler/ast/syntax_tree.hpp"
#include "compiler/ast/types.hpp"

#include "compiler/backend/error_buffer.hpp"
#include "compiler/backend/semantizer/semantizer_context.hpp"
#include "compiler/backend/semantizer/semantizer_error.hpp"

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
