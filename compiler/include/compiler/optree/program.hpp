#pragma once

#include "compiler/optree/operation.hpp"

namespace optree {

struct Program {
    Operation *root;

    Program() = default;
    Program(const Program &) = default;
    Program(Program &&) = default;
    ~Program();
};

} // namespace optree
