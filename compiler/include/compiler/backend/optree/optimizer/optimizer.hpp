#pragma once

#include <cstddef>
#include <vector>

#include "compiler/optree/program.hpp"

#include "compiler/backend/optree/optimizer/transform.hpp"

namespace optree {
namespace optimizer {

class Optimizer {
    std::vector<BaseTransform::Ptr> transforms;
    size_t iterLimit;

  public:
    Optimizer();
    Optimizer(const Optimizer &) = default;
    Optimizer(Optimizer &&) = default;
    ~Optimizer() = default;

    void process(Program &program) const;
};

} // namespace optimizer
} // namespace optree
