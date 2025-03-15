#pragma once

#include <deque>

#include "compiler/optree/operation.hpp"
#include "compiler/optree/program.hpp"

#include "compiler/backend/optree/optimizer/transform.hpp"

namespace optree {
namespace optimizer {

class Optimizer {
    std::deque<BaseTransform::Ptr> transforms;

  public:
    Optimizer() = default;
    Optimizer(const Optimizer &) = default;
    Optimizer(Optimizer &&) = default;
    ~Optimizer() = default;

    Optimizer &add(const BaseTransform::Ptr &transform);
    void process(const Operation::Ptr &op) const;
    void process(Program &program) const;
};

} // namespace optimizer
} // namespace optree
