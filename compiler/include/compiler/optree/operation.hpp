#pragma once

#include <algorithm>
#include <functional>
#include <list>
#include <memory>
#include <utility>
#include <vector>

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
    SpecId specId;
    Verifier verifier;

    std::vector<Value::Ptr> operands;
    std::vector<Value::Ptr> results;
    std::vector<Attribute> attributes;
    Body body;

    Operation(const Operation &) = delete;
    Operation(Operation &&) = default;
    ~Operation() = default;

    explicit Operation(Ptr parent = Ptr())
        : parent(parent), specId(nullptr), verifier([](const Operation *) { return true; }){};
    Operation(SpecId specId, const Verifier &verifier, Ptr parent = Ptr())
        : parent(parent), specId(specId), verifier(verifier){};

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
    AdaptorType as() const {
        if (is<AdaptorType>())
            return AdaptorType(this);
        return {};
    }

    template <typename AdaptorType>
    static Ptr make(Ptr parent = Ptr()) {
        return std::make_shared<Operation>(AdaptorType::getSpecId(), AdaptorType::verify, parent);
    }
};

} // namespace optree
