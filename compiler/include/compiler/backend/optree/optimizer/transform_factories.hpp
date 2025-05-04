#pragma once

#include "compiler/backend/optree/optimizer/transform.hpp"

namespace optree {
namespace optimizer {

BaseTransform::Ptr createEraseUnusedFunctions();
BaseTransform::Ptr createEraseUnusedOps();
BaseTransform::Ptr createFoldConstants();
BaseTransform::Ptr createFoldControlFlowOps();
BaseTransform::Ptr createJoinConditionsBranches();
BaseTransform::Ptr createMinimizeBoolExpression();
BaseTransform::Ptr createSinkControlFlowOps();

} // namespace optimizer
} // namespace optree
