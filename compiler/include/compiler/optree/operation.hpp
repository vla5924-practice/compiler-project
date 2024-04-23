#pragma once

#include <cstddef>
#include <functional>
#include <list>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "compiler/utils/source_ref.hpp"

#include "compiler/optree/attribute.hpp"
#include "compiler/optree/types.hpp"
#include "compiler/optree/value.hpp"

namespace optree {

struct Operation : public std::enable_shared_from_this<Operation> {
    using Ptr = std::shared_ptr<Operation>;
    using Body = std::list<Ptr>;
    using SpecId = void *;

  private:
    using ValueMapping = std::unordered_map<Value::Ptr, Value::Ptr>;

    SpecId specId;

    explicit Operation(const Ptr &parent = nullptr, const Body::iterator &position = {})
        : parent(parent), position(position), specId(nullptr){};
    explicit Operation(SpecId specId, std::string_view name = "Unknown", const Ptr &parent = nullptr,
                       const Body::iterator &position = {})
        : specId(specId), parent(parent), position(position), ref(), name(name){};

    void addUse(const Value::Ptr &value, size_t operandNumber);
    void removeUse(const Value::Ptr &value, size_t operandNumber);
    void updateUse(const Value::Ptr &value, size_t operandNumber, const std::function<void(Value::Use &)> &actor);

    Ptr cloneImpl(const ValueMapping &operandsMap = {});
    Ptr cloneWithoutBodyImpl(const ValueMapping &operandsMap, ValueMapping &outputsMap);

    static SpecId getUnknownSpecId();

  public:
    Ptr parent;
    Body::iterator position;
    utils::SourceRef ref;
    std::string_view name;

    std::vector<Value::Ptr> operands;
    std::vector<Value::Ptr> results;
    std::vector<Value::Ptr> inwards;
    std::vector<Attribute> attributes;
    Body body;

    Operation(const Operation &) = delete;
    Operation(Operation &&) = delete;
    ~Operation() = default;

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
        throw std::logic_error("There are no attributes with a given type");
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
    AdaptorType as() {
        if (is<AdaptorType>())
            return AdaptorType(shared_from_this());
        return {};
    }

    template <typename AdaptorType>
    AdaptorType findParent() const {
        Ptr upperParent = parent;
        while (upperParent && !upperParent->is<AdaptorType>()) {
            if (upperParent->is<AdaptorType>())
                return {upperParent};
            upperParent = upperParent->parent;
        }
        return {};
    }

    template <typename VariantType>
    Attribute &addAttr(const VariantType &value) {
        return attributes.emplace_back(value);
    }

    void addOperand(const Value::Ptr &value);
    void insertOperand(size_t operandNumber, const Value::Ptr &value);
    void setOperand(size_t operandNumber, const Value::Ptr &value);
    void eraseOperand(size_t operandNumber);

    Value::Ptr addResult(const Type::Ptr &type);
    Value::Ptr addInward(const Type::Ptr &type);

    void addToBody(const Ptr &op);
    void erase();

    Ptr clone();
    Ptr cloneWithoutBody();

    std::string dump() const;
    void dump(std::ostream &stream) const;
    bool isUnknown() const;

    template <typename AdaptorType>
    static AdaptorType make(const Ptr &parent = {}, const Body::iterator &position = {}) {
        auto *op = new Operation(AdaptorType::getSpecId(), AdaptorType::getOperationName(), parent, position);
        return {Operation::Ptr(op)};
    }

    static Ptr make(std::string_view name, const Ptr &parent = {}, const Body::iterator &position = {});
};

} // namespace optree
