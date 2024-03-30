#pragma once

#include <list>

#include "compiler/optree/operation.hpp"

namespace optree {

struct Program {
    Operation::Ptr root;

    Program() = default;
    Program(const Program &) = default;
    Program(Program &&) = default;
    ~Program() = default;
};

} // namespace optree
