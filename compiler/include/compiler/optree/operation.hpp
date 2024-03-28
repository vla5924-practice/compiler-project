#pragma once

#include <algorithm>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include "compiler/utils/source_ref.hpp"

#include "compiler/optree/attribute.hpp"
#include "compiler/optree/types.hpp"
#include "compiler/optree/value.hpp"

namespace optree {

struct Operation {
    using Ptr = std::shared_ptr<Operation>;
    using Body = std::list<Ptr>;
    using Verifier = std::function<bool(const Operation *)>;
    using SpecId = void *;

    Ptr parent;
    Body::iterator position;
    SpecId specId;
    Verifier verifier;
    utils::SourceRef ref;
    std::string_view name;

    std::vector<Value::Ptr> operands;
    std::vector<Value::Ptr> results;
    std::vector<Value::Ptr> inwards;
    std::vector<Attribute> attributes;
    Body body;

    Operation(const Operation &) = delete;
    Operation(Operation &&) = default;
    ~Operation() = default;

    explicit Operation(Ptr parent = {}, Body::iterator position = {})
        : parent(parent), position(position), specId(nullptr), verifier([](const Operation *) { return true; }){};
    Operation(SpecId specId, const Verifier &verifier, std::string_view name = "Unknown", Ptr parent = Ptr(),
              Body::iterator position = {})
        : parent(parent), position(position), specId(specId), verifier(verifier), name(name){};

    const Value::Ptr &operand(size_t index) const {
        return operands[index];
    }

    Value::Ptr &operand(size_t index) {
        return operands[index];
    }

    const Value::Ptr &result(size_t index) const {
        return results[index];
    }

    Value::Ptr &result(size_t index) {
        return results[index];
    }

    const Value::Ptr &inward(size_t index) const {
        return inwards[index];
    }

    Value::Ptr &inward(size_t index) {
        return inwards[index];
    }

    const Attribute &attr(size_t index) const {
        return attributes[index];
    }

    Attribute &attr(size_t index) {
        return attributes[index];
    }

    template <typename VariantType>
    const VariantType &attr() const {
        for (const auto &a : attributes)
            if (a.is<VariantType>())
                return a.as<VariantType>();
        throw std::exception("There are no attributes with a given type");
    }

    operator bool() const {
        return specId;
    }

    size_t numOperands() const {
        return operands.size();
    }

    size_t numResults() const {
        return results.size();
    }

    size_t numInwards() const {
        return inwards.size();
    }

    size_t numAttrs() const {
        return attributes.size();
    }

    auto begin() {
        return body.begin();
    }

    auto end() {
        return body.end();
    }

    template <typename AdaptorType>
    bool is() const {
        return specId == AdaptorType::getSpecId();
    }

    template <typename AdaptorType>
    AdaptorType findParent() const {
        Ptr upperParent = parent;
        while (upperParent && !upperParent->is<AdaptorType>())
            upperParent = upperParent->parent;
        return AdaptorType(upperParent);
    }

    template <typename VariantType>
    Attribute &addAttr(const VariantType &value) {
        return attributes.emplace_back(value);
    }

    void addOperand(Value::Ptr value);
    void eraseOperand(size_t operandNumber);
    Value::Ptr addResult(Type::Ptr type);
    Value::Ptr addInward(Type::Ptr type);
    Body::iterator addToBody(Operation::Ptr op);
    void erase();

    bool verify() const;

    std::string dump() const;
    void dump(std::ostream &stream) const;

    template <typename AdaptorType>
    static AdaptorType make(Ptr parent = {}, Body::iterator position = {}) {
        auto op = std::make_shared<Operation>(AdaptorType::getSpecId(), AdaptorType::verify,
                                              AdaptorType::getOperationName(), parent, position);
        return {op};
    }

    template <typename AdaptorType>
    static AdaptorType as(const Operation::Ptr &op) {
        if (op->is<AdaptorType>())
            return AdaptorType(op);
        return {};
    }
};

} // namespace optree
