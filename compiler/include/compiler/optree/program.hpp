#pragma once

#include "compiler/optree/operation.hpp"

namespace optree {

struct Program {
    Operation::Ptr root;

    Program(const Program &) = default;
    Program(Program &&) = default;
    ~Program() = default;

    Program(const Operation::Ptr &root = {}) : root(root){};
};

} // namespace optree
