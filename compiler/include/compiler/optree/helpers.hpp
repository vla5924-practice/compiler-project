#pragma once

#include "compiler/utils/source_ref.hpp"

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/builder.hpp"
#include "compiler/optree/types.hpp"
#include "compiler/optree/value.hpp"

namespace optree {

Type::Ptr deduceTargetCastType(const Type::Ptr &outType, const Type::Ptr &inType, bool isAssignment = false);

ArithCastOp insertNumericCastOp(const Type::Ptr &resultType, const Value::Ptr &value, Builder &builder,
                                utils::SourceRef &ref);
                                
template <typename AdaptorType>
AdaptorType getValueOwnerAs(const Value::Ptr &value) {
    if (value->owner.expired())
        return {};
    return value->owner.lock()->as<AdaptorType>();
}

} // namespace optree
