#pragma once

#include "compiler/optree/operation.hpp"
#include "compiler/optree/program.hpp"

namespace optree {
namespace semantizer {

class Semantizer {
  public:
    Semantizer() = delete;
    Semantizer(const Semantizer &) = delete;
    Semantizer(Semantizer &&) = delete;
    ~Semantizer() = delete;

    static void process(const Program &program);
    static void process(const Operation::Ptr &op);
};

} // namespace semantizer
} // namespace optree
