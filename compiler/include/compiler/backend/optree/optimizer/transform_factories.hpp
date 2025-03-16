#pragma once

#include "compiler/backend/optree/optimizer/transform.hpp"

namespace optree {
namespace optimizer {

BaseTransform::Ptr createEraseUnusedOps();
BaseTransform::Ptr createEraseUnusedFunctions();
BaseTransform::Ptr createFoldConstants();
BaseTransform::Ptr createBooleanExpressionMinimization();

} // namespace optimizer
} // namespace optree
