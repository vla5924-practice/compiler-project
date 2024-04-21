#pragma once

#include "compiler/backend/optree/optimizer/transform.hpp"

namespace optree {
namespace optimizer {

BaseTransform::Ptr createEraseUnusedOps();

} // namespace optimizer
} // namespace optree
