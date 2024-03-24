#pragma once

#include <algorithm>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <stdexcept>
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

    std::vector<Value::Ptr> operands;
    std::vector<Value::Ptr> results;
    std::vector<Attribute> attributes;
    Body body;

    Operation(const Operation &) = delete;
    Operation(Operation &&) = default;
    ~Operation() = default;

    explicit Operation(Ptr parent = {}, Body::iterator position = {})
        : parent(parent), position(position), specId(nullptr), verifier([](const Operation *) { return true; }){};
    Operation(SpecId specId, const Verifier &verifier, Ptr parent = Ptr(), Body::iterator position = {})
        : parent(parent), position(position), specId(specId), verifier(verifier){};

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

    template <typename VariantType>
    Attribute &addAttr(const VariantType &value) {
        return attributes.emplace_back(value);
    }

    void addOperand(Value::Ptr value) {
        size_t operandNumber = operands.size();
        operands.emplace_back(value);
        value->uses.emplace_front(this, operandNumber);
    }

    void eraseOperand(size_t operandNumber) {
        auto &uses = operands[operandNumber]->uses;
        uses.remove_if([&](const Value::Use &use) { return use.user == this && use.operandNumber == operandNumber; });
        operands.erase(operands.begin() + operandNumber);
    }

    Value::Ptr addResult(const Type &type) {
        return results.emplace_back(Value::make(type, this));
    }

    void addToBody(Operation::Ptr op) {
        body.emplace_back(op);
    }

    bool verify() const {
        return verifier(this);
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
    const AdaptorType as() const {
        if (is<AdaptorType>())
            return AdaptorType(this);
        return {};
    }

    template <typename AdaptorType>
    AdaptorType as() {
        if (is<AdaptorType>())
            return AdaptorType(this);
        return {};
    }

    void erase() {
        for (const auto &result : results) {
            if (!result->uses.empty())
                throw std::logic_error("Operation cannot be erased since its results still have uses");
        }
        results.clear();
        if (!parent || position == Body::iterator())
            return;
        parent->body.erase(position);
    }

    template <typename AdaptorType>
    static Ptr make(Ptr parent = {}, Body::iterator position = {}) {
        return std::make_shared<Operation>(AdaptorType::getSpecId(), AdaptorType::verify, parent, position);
    }
};

} // namespace optree
