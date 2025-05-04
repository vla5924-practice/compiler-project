#pragma once

#include "compiler/backend/optree/optimizer/transform.hpp"

namespace optree {
namespace optimizer {

BaseTransform::Ptr createControlFlowSinkOps();
BaseTransform::Ptr createEraseUnusedFunctions();
BaseTransform::Ptr createEraseUnusedOps();
BaseTransform::Ptr createFoldConstants();
BaseTransform::Ptr createFoldControlFlowOps();
BaseTransform::Ptr createJoinConditionsBranches();

} // namespace optimizer
} // namespace optree
