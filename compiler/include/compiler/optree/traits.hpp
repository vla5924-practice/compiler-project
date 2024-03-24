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

template <size_t N>
bool numAttrs(const Operation *op) {
    return op->numAttrs() == N;
}

template <typename VariantType>
bool oneAttrOfType(const Operation *op) {
    return numAttrs<1U>(op) && op->attributes.front().is<VariantType>();
}

template <typename AdaptorType>
bool bodyContainsOnly(const Operation *op) {
    return std::all_of(op->body.begin(), op->body.end(),
                       [](const Operation::Ptr &op) { return op->is<AdaptorType>(); });
}

bool operandsHaveSameType(const Operation *op) {
    if (op->operands.empty())
        return true;
    const Type &type = op->operand(0)->type;
    return std::all_of(op->operands.begin(), op->operands.end(),
                       [&type](const Value::Ptr &value) { return value->type == type; });
}

bool operandsAndResultsHaveSameType(const Operation *op) {
    if (!operandsHaveSameType(op))
        return false;
    const Type &type = op->operand(0)->type;
    return std::all_of(op->results.begin(), op->results.end(),
                       [&type](const Value::Ptr &value) { return value->type == type; });
}

} // namespace trait

} // namespace optree
