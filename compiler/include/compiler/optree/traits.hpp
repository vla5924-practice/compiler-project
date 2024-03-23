#pragma once

#include <cstddef>

#include "compiler/optree/operation.hpp"

namespace optree {

namespace trait {

template <size_t N>
bool numOperands(const Operation *op) {
    return op->numOperands() == N;
}

template <size_t N>
bool numResults(const Operation *op) {
    return op->numResults() == N;
}

template <typename VariantType>
bool oneAttrOfType(const Operation *op) {
    return op->numAttrs() == 1U && op->attributes.front().is<VariantType>();
}

template <typename AdaptorType>
bool bodyContainsOnly(const Operation *op) {
    return std::all_of(op->body.begin(), op->body.end(),
                       [](const Operation::Ptr &op) { return op->is<AdaptorType>(); });
}

} // namespace trait

} // namespace optree
