#pragma once

#include <cstddef>

#include "compiler/optree/operation.hpp"
#include "compiler/optree/types.hpp"
#include "compiler/optree/value.hpp"

#include "compiler/backend/optree/semantizer/semantizer_context.hpp"

#define DECLARE_SEMANTIZER_TRAIT(TRAIT_CLASS_NAME, ...)                                                                \
    struct TRAIT_CLASS_NAME {                                                                                          \
        static bool verify(const Operation::Ptr &, SemantizerContext &, __VA_ARGS__);                                  \
    }

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

DECLARE_SEMANTIZER_TRAIT(HasOperands, size_t numOperands);
DECLARE_SEMANTIZER_TRAIT(HasOperandsOfType, size_t numOperands, const Type::Ptr &type);
DECLARE_SEMANTIZER_TRAIT(HasResults, size_t numResults);
DECLARE_SEMANTIZER_TRAIT(HasResultOfType, const Type::Ptr &type);
DECLARE_SEMANTIZER_TRAIT(HasInwards, size_t numInwards);
DECLARE_SEMANTIZER_TRAIT(HasInwardsOfType, size_t numInwards, const Type::Ptr &type);
DECLARE_SEMANTIZER_TRAIT(HasAttributes, size_t numAttrs);
template <typename T>
DECLARE_SEMANTIZER_TRAIT(HasNthAttrOfType, size_t index);

template <typename T>
bool HasNthAttrOfType<T>::verify(const Operation::Ptr &op, SemantizerContext &ctx, size_t index) {
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

} // namespace semantizer
} // namespace optree
