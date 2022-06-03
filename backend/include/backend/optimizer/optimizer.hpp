#pragma once

#include <ast/syntax_tree.hpp>
#include <ast/types.hpp>

#include "backend/optimizer/optimizer_options.hpp"

namespace optimizer {

class Optimizer {
  public:
    Optimizer() = delete;
    Optimizer(const Optimizer &) = delete;
    Optimizer(Optimizer &&) = delete;
    ~Optimizer() = delete;

    static void process(ast::SyntaxTree &tree, const OptimizerOptions &options = OptimizerOptions::all());
};

} // namespace optimizer
