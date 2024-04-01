#pragma once

#include <algorithm>
#include <cstddef>

#include "compiler/optree/operation.hpp"
#include "compiler/optree/types.hpp"
#include "compiler/optree/value.hpp"

namespace optree {

namespace trait {

template <size_t N>
inline bool numOperands(const Operation *op) {
    return op->numOperands() == N;
}

template <size_t N>
inline bool numResults(const Operation *op) {
    return op->numResults() == N;
}

template <size_t N>
inline bool numInwards(const Operation *op) {
    return op->numInwards() == N;
}

template <size_t N>
inline bool numAttrs(const Operation *op) {
    return op->numAttrs() == N;
}

template <typename VariantType>
inline bool oneAttrOfType(const Operation *op) {
    return numAttrs<1U>(op) && op->attributes.front().is<VariantType>();
}

template <typename AdaptorType>
inline bool bodyContainsOnly(const Operation *op) {
    return std::all_of(op->body.begin(), op->body.end(),
                       [](const Operation::Ptr &op) { return op->is<AdaptorType>(); });
}

inline bool operandsHaveSameType(const Operation *op) {
    if (op->operands.empty())
        return true;
    const Type::Ptr &type = op->operand(0)->type;
    return std::all_of(op->operands.begin(), op->operands.end(),
                       [&type](const Value::Ptr &value) { return value->hasType(type); });
}

inline bool operandsAndResultsHaveSameType(const Operation *op) {
    if (!operandsHaveSameType(op))
        return false;
    const Type::Ptr &type = op->operand(0)->type;
    return std::all_of(op->results.begin(), op->results.end(),
                       [&type](const Value::Ptr &value) { return value->hasType(type); });
}

} // namespace trait

} // namespace optree
