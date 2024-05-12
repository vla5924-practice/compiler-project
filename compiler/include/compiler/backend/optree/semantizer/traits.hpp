#pragma once

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

    template <typename TraitType, typename... Args>
    TraitVerifier &verify(Args... args) {
        if (acc) {
            TraitType trait(op, ctx);
            acc &= trait.verify(std::forward<Args>(args)...);
        }
        return *this;
    }
};

class Trait {
  protected:
    const Operation::Ptr &op;
    SemantizerContext &ctx;

  public:
    Trait() = delete;
    Trait(const Trait &) = delete;
    Trait(Trait &&) = delete;
    ~Trait() = default;

    Trait(const Operation::Ptr &op, SemantizerContext &ctx) : op(op), ctx(ctx){};

    bool verify() {
        ctx.pushOpError(op) << "was not registered for verification: " << op->name;
        return false;
    }
};

struct HasOperands : Trait {
    using Trait::Trait;
    bool verify(size_t numOperands) {
        if (op->numOperands() != numOperands) {
            ctx.pushOpError(op) << "must have " << numOperands << " operands";
            return false;
        }
        return true;
    }
};

struct HasResults : Trait {
    using Trait::Trait;
    bool verify(size_t numResults) {
        if (op->numResults() == numResults)
            return true;
        ctx.pushOpError(op) << "must have " << numResults << " results";
        return false;
    }
};

struct HasResultOfType : Trait {
    using Trait::Trait;
    bool verify(const Type::Ptr &type) {
        if (op->numResults() == 1 && op->result(0)->hasType(type))
            return true;
        ctx.pushOpError(op) << "must have one result of " << type;
        return false;
    }
};

struct HasInwards : Trait {
    using Trait::Trait;
    bool verify(size_t numInwards) {
        if (op->numInwards() == numInwards)
            return true;
        ctx.pushOpError(op) << "must have " << numInwards << " inwards";
        return false;
    }
};

struct HasAttributes : Trait {
    using Trait::Trait;
    bool verify(size_t numAttrs) {
        if (op->numAttrs() == numAttrs)
            return true;
        ctx.pushOpError(op) << "must have " << numAttrs << " attributes";
        return false;
    }
};

template <typename T>
struct HasNthAttrOfType : Trait {
    using Trait::Trait;
    bool verify(size_t index) {
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
