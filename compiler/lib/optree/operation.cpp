#include "operation.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "compiler/utils/helpers.hpp"

#include "attribute.hpp"
#include "types.hpp"
#include "value.hpp"

using namespace optree;

void Operation::addUse(const Value::Ptr &value, size_t operandNumber) {
    value->uses.emplace_front(weak_from_this(), operandNumber);
}

void Operation::removeUse(const Value::Ptr &value, size_t operandNumber) {
    value->uses.remove_if(
        [&](const Value::Use &use) { return use.userIs(this) && use.operandNumber == operandNumber; });
}

void Operation::updateUse(const Value::Ptr &value, size_t operandNumber,
                          const std::function<void(Value::Use &)> &actor) {
    for (auto &use : value->uses)
        if (use.userIs(this) && use.operandNumber == operandNumber)
            actor(use);
}

Operation::Ptr Operation::cloneImpl(const ValueMapping &operandsMap) {
    ValueMapping outputsMap;
    auto newOp = cloneWithoutBodyImpl(operandsMap, outputsMap);
    for (const auto &nestedOp : body) {
        newOp->addToBody(nestedOp->cloneImpl(outputsMap));
    }
    for (const auto &inward : inwards)
        outputsMap.erase(inward);
    return newOp;
}

Operation::Ptr Operation::cloneWithoutBodyImpl(const ValueMapping &operandsMap, ValueMapping &outputsMap) {
    auto newOp = Operation::Ptr(new Operation(specId, name));
    auto producer = [&](const Value::Ptr &value) { return Value::make(value->type, newOp); };
    newOp->ref = ref;
    std::vector<Value::Ptr> newOperands;
    std::transform(operands.begin(), operands.end(), std::back_inserter(newOperands), [&](const Value::Ptr &value) {
        if (operandsMap.contains(value))
            return operandsMap.at(value);
        return value;
    });
    for (const auto &operand : newOperands)
        newOp->addOperand(operand);
    std::transform(results.begin(), results.end(), std::back_inserter(newOp->results), producer);
    for (size_t i = 0; i < results.size(); i++)
        outputsMap[result(i)] = newOp->result(i);
    std::transform(inwards.begin(), inwards.end(), std::back_inserter(newOp->inwards), producer);
    for (size_t i = 0; i < inwards.size(); i++)
        outputsMap[inward(i)] = newOp->inward(i);
    newOp->attributes = attributes;
    return newOp;
}

Operation::SpecId Operation::getUnknownSpecId() {
    static char unknownSpec;
    return &unknownSpec;
}

void Operation::addOperand(const Value::Ptr &value) {
    addUse(value, operands.size());
    operands.emplace_back(value);
}

void Operation::insertOperand(size_t operandNumber, const Value::Ptr &value) {
    auto operandIt = operands.emplace(operands.begin() + operandNumber, value);
    addUse(value, operandNumber);
    for (auto it = std::next(operandIt); it != operands.end(); ++it) {
        updateUse(*it, operandNumber++, [](Value::Use &use) { use.operandNumber++; });
    }
}

void Operation::setOperand(size_t operandNumber, const Value::Ptr &value) {
    auto &operand = operands[operandNumber];
    removeUse(operand, operandNumber);
    operand = value;
    addUse(value, operandNumber);
}

void Operation::eraseOperand(size_t operandNumber) {
    removeUse(operands[operandNumber], operandNumber);
    auto operandIt = operands.erase(operands.begin() + operandNumber);
    for (auto it = operandIt; it != operands.end(); ++it) {
        updateUse(*it, ++operandNumber, [](Value::Use &use) { use.operandNumber--; });
    }
}

Value::Ptr Operation::addResult(const Type::Ptr &type) {
    return results.emplace_back(Value::make(type, weak_from_this()));
}

Value::Ptr Operation::addInward(const Type::Ptr &type) {
    return inwards.emplace_back(Value::make(type, weak_from_this()));
}

void Operation::addToBody(const Operation::Ptr &op) {
    op->position = body.emplace(body.end(), op);
    op->parent = shared_from_this();
}

void Operation::erase() {
    while (!body.empty()) {
        body.back()->erase();
    }
    for (const auto &result : results) {
        if (!result->uses.empty())
            throw std::logic_error("Operation cannot be erased since its results still have uses");
    }
    results.clear();
    for (const auto &inward : inwards) {
        if (!inward->uses.empty())
            throw std::logic_error("Operation cannot be erased since its inwards still have uses");
    }
    inwards.clear();
    for (size_t i = 0; i < operands.size(); i++) {
        removeUse(operands[i], i);
    }
    operands.clear();
    attributes.clear();
    if (!parent)
        return;
    parent->body.erase(position);
}

Operation::Ptr Operation::clone() {
    return cloneImpl();
}

Operation::Ptr Operation::cloneWithoutBody() {
    ValueMapping inwardsMap;
    return cloneWithoutBodyImpl({}, inwardsMap);
}

std::string Operation::dump() const {
    std::stringstream str;
    dump(str);
    return str.str();
}

namespace {

void dumpValue(const Value::Ptr &value, std::ostream &stream, int valueId) {
    stream << '#' << valueId << " : ";
    value->type->dump(stream);
}

void dumpOperation(const Operation *op, std::ostream &stream, int depth,
                   std::unordered_map<const Value *, int> &valueIds, int &nextValueId) {
    for (int i = 0; i < depth; i++)
        stream << "  ";
    stream << op->name;
    if (!op->attributes.empty()) {
        stream << " {";
        utils::interleaveComma(stream, op->attributes, [&](const Attribute &attr) { attr.dump(stream); });
        stream << "}";
    }
    stream << " (";
    utils::interleaveComma(stream, op->operands,
                           [&](const Value::Ptr &operand) { dumpValue(operand, stream, valueIds[operand.get()]); });
    stream << ") -> (";
    auto printOwnValue = [&](const Value::Ptr &value) {
        dumpValue(value, stream, nextValueId);
        valueIds[value.get()] = nextValueId++;
    };
    utils::interleaveComma(stream, op->results, printOwnValue);
    stream << ")";
    if (!op->inwards.empty()) {
        stream << " [";
        utils::interleaveComma(stream, op->inwards, printOwnValue);
        stream << "]";
    }
    stream << "\n";
    for (const auto &op : op->body)
        dumpOperation(op.get(), stream, depth + 1, valueIds, nextValueId);
}

} // namespace

void Operation::dump(std::ostream &stream) const {
    std::unordered_map<const Value *, int> valueIds;
    int valueId = 0;
    dumpOperation(this, stream, 0, valueIds, valueId);
}

bool Operation::isUnknown() const {
    return specId == getUnknownSpecId();
}

Operation::Ptr Operation::make(std::string_view name, const Ptr &parent, const Body::iterator &position) {
    return Ptr(new Operation(getUnknownSpecId(), name, parent, position));
}
