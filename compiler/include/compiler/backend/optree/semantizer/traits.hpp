#pragma once

#include <algorithm>

#include "compiler/optree/operation.hpp"
#include "compiler/optree/types.hpp"

#include "compiler/backend/optree/semantizer/semantizer_context.hpp"

namespace optree {
namespace semantizer {

class TraitVerifier {
    const Operation::Ptr &op;
    SemantizerContext &ctx;
    bool acc;

  public:
    TraitVerifier() = delete;
    TraitVerifier(const TraitVerifier &) = delete;
    TraitVerifier(TraitVerifier &&) = delete;
    ~TraitVerifier() = default;

    TraitVerifier(const Operation::Ptr &op, SemantizerContext &ctx) : op(op), ctx(ctx), acc(true){};

    bool verified() const {
        return acc;
    }

    operator bool() const {
        return verified();
    }

    bool fail() {
        acc = false;
        return verified();
    }

    template <typename Trait, typename... Args>
        requires std::same_as<decltype(Trait::verify(op, ctx, std::declval<Args>()...)), bool>
    TraitVerifier &verify(Args... args) {
        if (acc)
            acc &= Trait::verify(op, ctx, std::forward<Args>(args)...);
        return *this;
    }
};

struct HasOperands {
    static bool verify(const Operation::Ptr &op, SemantizerContext &ctx, size_t numOperands) {
        if (op->numOperands() != numOperands) {
            ctx.pushOpError(op) << "must have " << numOperands << " operands";
            return false;
        }
        return true;
    }
};

struct HasOperandsOfType {
    static bool verify(const Operation::Ptr &op, SemantizerContext &ctx, size_t numOperands, const Type::Ptr &type) {
        if (op->numOperands() == numOperands &&
            std::all_of(op->operands.begin(), op->operands.end(),
                        [&](const Value::Ptr &operand) { return operand->hasType(type); }))
            return true;
        ctx.pushOpError(op) << "must have " << numOperands << " operands of " << type;
        return false;
    }
};

struct HasResults {
    static bool verify(const Operation::Ptr &op, SemantizerContext &ctx, size_t numResults) {
        if (op->numResults() == numResults)
            return true;
        ctx.pushOpError(op) << "must have " << numResults << " results";
        return false;
    }
};

struct HasResultOfType {
    static bool verify(const Operation::Ptr &op, SemantizerContext &ctx, const Type::Ptr &type) {
        if (op->numResults() == 1 && op->result(0)->hasType(type))
            return true;
        ctx.pushOpError(op) << "must have one result of " << type;
        return false;
    }
};

struct HasInwards {
    static bool verify(const Operation::Ptr &op, SemantizerContext &ctx, size_t numInwards) {
        if (op->numInwards() == numInwards)
            return true;
        ctx.pushOpError(op) << "must have " << numInwards << " inwards";
        return false;
    }
};

struct HasInwardsOfType {
    static bool verify(const Operation::Ptr &op, SemantizerContext &ctx, size_t numInwards, const Type::Ptr &type) {
        if (op->numInwards() == numInwards &&
            std::all_of(op->inwards.begin(), op->inwards.end(),
                        [&](const Value::Ptr &inward) { return inward->hasType(type); }))
            return true;
        ctx.pushOpError(op) << "must have " << numInwards << " inwards of " << type;
        return false;
    }
};

struct HasAttributes {
    static bool verify(const Operation::Ptr &op, SemantizerContext &ctx, size_t numAttrs) {
        if (op->numAttrs() == numAttrs)
            return true;
        ctx.pushOpError(op) << "must have " << numAttrs << " attributes";
        return false;
    }
};

template <typename T>
struct HasNthAttrOfType {
    static bool verify(const Operation::Ptr &op, SemantizerContext &ctx, size_t index) {
        if constexpr (std::is_base_of_v<Type, T>) {
            if (op->attr(index).isType<T>())
                return true;
        } else {
            if (op->attr(index).is<T>())
                return true;
        }
        ctx.pushOpError(op) << "must have attribute #" << index << " of other type";
        return false;
    }
};

} // namespace semantizer
} // namespace optree
