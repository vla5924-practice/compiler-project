#include "helpers.hpp"

#include <algorithm>

#include "compiler/utils/source_ref.hpp"

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/attribute.hpp"
#include "compiler/optree/base_adaptor.hpp"
#include "compiler/optree/builder.hpp"
#include "compiler/optree/definitions.hpp"
#include "compiler/optree/types.hpp"
#include "compiler/optree/value.hpp"

namespace optree {

Type::Ptr deduceTargetCastType(const Type::Ptr &outType, const Type::Ptr &inType, bool isAssignment) {
    if (isAssignment)
        return outType;
    if (inType == outType)
        return inType;
    bool fromInt = inType->is<IntegerType>();
    bool fromFloat = inType->is<FloatType>();
    bool toInt = outType->is<IntegerType>();
    bool toFloat = outType->is<FloatType>();
    bool isExt = inType->bitWidth() < outType->bitWidth();
    if (fromFloat && toInt)
        return inType;
    if (fromInt && toFloat)
        return outType;
    if (fromFloat && toFloat || fromInt && toInt)
        return isExt ? outType : inType;
    return {};
}

ArithCastOp insertNumericCastOp(const Type::Ptr &resultType, const Value::Ptr &value, Builder &builder,
                                utils::SourceRef &ref) {
    const auto &inType = value->type;
    if (inType == resultType)
        return {};
    bool fromInt = inType->is<IntegerType>();
    bool fromFloat = inType->is<FloatType>();
    bool toInt = resultType->is<IntegerType>();
    bool toFloat = resultType->is<FloatType>();
    bool isExt = inType->bitWidth() < resultType->bitWidth();
    auto kind = ArithCastOpKind::Unknown;
    if (fromInt && toInt)
        kind = isExt ? ArithCastOpKind::ExtI : ArithCastOpKind::TruncI;
    else if (fromFloat && toFloat)
        kind = isExt ? ArithCastOpKind::ExtF : ArithCastOpKind::TruncF;
    else if (fromInt && toFloat)
        kind = ArithCastOpKind::IntToFloat;
    else if (fromFloat && toInt)
        kind = ArithCastOpKind::FloatToInt;
    else
        return {};
    return builder.insert<ArithCastOp>(ref, kind, resultType, value);
}

bool similar(const Operation::Ptr &lhs, const Operation::Ptr &rhs, bool checkBody) {
    auto name = lhs->name == rhs->name;
    auto specId = name ? lhs->as<Adaptor>().getSpecId() == rhs->as<Adaptor>().getSpecId() : false;
    auto attrEqual = [](const Attribute &lhs, const Attribute &rhs) { return lhs == rhs; };
    bool attr = specId ? std::ranges::equal(lhs->attributes, rhs->attributes, attrEqual) : false;
    auto operandEqual = [&lhs, &rhs](const Value::Ptr &lhsValue, const Value::Ptr &rhsValue) {
        bool type = lhsValue->sameType(rhsValue);
        auto lhsOwner = lhsValue->owner.lock();
        auto rhsOwner = rhsValue->owner.lock();
        bool sameGlobalOwner = lhsOwner == rhsOwner;
        bool similarLocalOwner = false;
        if (!sameGlobalOwner && lhsOwner != lhs && rhsOwner != rhs) {
            similarLocalOwner = similar(lhsOwner, rhsOwner, false);
        }
        return type && (sameGlobalOwner || similarLocalOwner);
    };
    bool operands = attr ? std::ranges::equal(lhs->operands, rhs->operands, operandEqual) : false;
    auto valueEqual = [](const Value::Ptr &lhsValue, const Value::Ptr &rhsValue) {
        return lhsValue->sameType(rhsValue);
    };
    bool inwards = operands ? std::ranges::equal(lhs->inwards, rhs->inwards, valueEqual) : false;
    bool results = inwards ? std::ranges::equal(lhs->results, rhs->results, valueEqual) : false;
    bool body = true;
    if (checkBody) {
        body = std::ranges::equal(lhs->body, rhs->body, [](const Operation::Ptr &lhs, const Operation::Ptr &rhs) {
            return optree::similar(lhs, rhs, true);
        });
    }
    return name && attr && operands && inwards && results && body;
}

} // namespace optree
