#pragma once

#include "compiler/backend/optree/optimizer/transform.hpp"

namespace optree {
namespace optimizer {

BaseTransform::Ptr createEraseUnusedOps();
BaseTransform::Ptr createFoldControlFlowOps();

} // namespace optimizer
} // namespace optree
