#pragma once

#include <cstddef>
#include <list>
#include <memory>
#include <ostream>
#include <queue>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "compiler/utils/source_ref.hpp"

#include "compiler/optree/attribute.hpp"
#include "compiler/optree/types.hpp"
#include "compiler/optree/value.hpp"

namespace optree {

struct Operation {
    using Body = std::list<Operation *>;
    using SpecId = void *;

  private:
    static inline struct Storage {
        std::unordered_set<Operation *> data;
        std::queue<Operation *> outdated;

        Storage() = default;
        Storage(const Storage &) = delete;
        Storage(Storage &&) = delete;

        ~Storage() {
            cleanup();
            for (Operation *op : data)
                delete op;
        }

        void emplace(Operation *op) {
            cleanup();
            data.emplace(op);
        }

        void destroy(Operation *op) {
            outdated.push(op);
        }

        void cleanup() {
            while (!outdated.empty()) {
                Operation *op = outdated.front();
                delete op;
                outdated.pop();
                data.erase(op);
            }
        }
    } storage;

    SpecId specId;

    Operation(const Operation &) = delete;
    Operation(Operation &&) = default;
    ~Operation() = default;

    explicit Operation(Operation *parent = nullptr, const Body::iterator &position = {})
        : parent(parent), position(position), specId(nullptr){};
    Operation(SpecId specId, std::string_view name = "Unknown", Operation *parent = nullptr,
              const Body::iterator &position = {})
        : parent(parent), position(position), specId(specId), name(name){};

  public:
    Operation *parent;
    Body::iterator position;
    utils::SourceRef ref;
    std::string_view name;

    std::vector<Value::Ptr> operands;
    std::vector<Value::Ptr> results;
    std::vector<Value::Ptr> inwards;
    std::vector<Attribute> attributes;
    Body body;

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
    AdaptorType findParent() const {
        Operation *upperParent = parent;
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
    void eraseOperand(size_t operandNumber);
    Value::Ptr addResult(const Type::Ptr &type);
    Value::Ptr addInward(const Type::Ptr &type);
    void addToBody(Operation *op);
    void erase();

    std::string dump() const;
    void dump(std::ostream &stream) const;

    template <typename AdaptorType>
    static AdaptorType make(Operation *parent = nullptr, const Body::iterator &position = {}) {
        auto *op = new Operation(AdaptorType::getSpecId(), AdaptorType::getOperationName(), parent, position);
        storage.emplace(op);
        return op;
    }

    template <typename AdaptorType>
    static AdaptorType as(Operation *op) {
        if (op->is<AdaptorType>())
            return AdaptorType(op);
        return {};
    }
};

} // namespace optree
