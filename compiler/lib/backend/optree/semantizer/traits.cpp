#include "semantizer/traits.hpp"

#include <algorithm>
#include <cstddef>

#include "compiler/optree/operation.hpp"
#include "compiler/optree/types.hpp"
#include "compiler/optree/value.hpp"

#include "semantizer/semantizer_context.hpp"

using namespace optree;
using namespace optree::semantizer;

bool HasOperands::verify(const Operation::Ptr &op, SemantizerContext &ctx, size_t numOperands) {
    if (op->numOperands() != numOperands) {
        ctx.pushOpError(op) << "must have " << numOperands << " operands";
        return false;
    }
    return true;
}

bool HasOperandsOfType::verify(const Operation::Ptr &op, SemantizerContext &ctx, size_t numOperands,
                               const Type::Ptr &type) {
    if (op->numOperands() == numOperands &&
        std::all_of(op->operands.begin(), op->operands.end(),
                    [&](const Value::Ptr &operand) { return operand->hasType(type); }))
        return true;
    ctx.pushOpError(op) << "must have " << numOperands << " operands of " << type;
    return false;
}

bool HasResults::verify(const Operation::Ptr &op, SemantizerContext &ctx, size_t numResults) {
    if (op->numResults() == numResults)
        return true;
    ctx.pushOpError(op) << "must have " << numResults << " results";
    return false;
}

bool HasResultOfType::verify(const Operation::Ptr &op, SemantizerContext &ctx, const Type::Ptr &type) {
    if (op->numResults() == 1 && op->result(0)->hasType(type))
        return true;
    ctx.pushOpError(op) << "must have one result of " << type;
    return false;
}

bool HasInwards::verify(const Operation::Ptr &op, SemantizerContext &ctx, size_t numInwards) {
    if (op->numInwards() == numInwards)
        return true;
    ctx.pushOpError(op) << "must have " << numInwards << " inwards";
    return false;
}

bool HasInwardsOfType::verify(const Operation::Ptr &op, SemantizerContext &ctx, size_t numInwards,
                              const Type::Ptr &type) {
    if (op->numInwards() == numInwards && std::all_of(op->inwards.begin(), op->inwards.end(),
                                                      [&](const Value::Ptr &inward) { return inward->hasType(type); }))
        return true;
    ctx.pushOpError(op) << "must have " << numInwards << " inwards of " << type;
    return false;
}

bool HasAttributes::verify(const Operation::Ptr &op, SemantizerContext &ctx, size_t numAttrs) {
    if (op->numAttrs() == numAttrs)
        return true;
    ctx.pushOpError(op) << "must have " << numAttrs << " attributes";
    return false;
}
