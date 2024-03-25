#include "operation.hpp"

#include <ostream>
#include <sstream>
#include <unordered_map>

#include "compiler/utils/utils.hpp"

using namespace optree;

void Operation::addOperand(Value::Ptr value) {
    size_t operandNumber = operands.size();
    operands.emplace_back(value);
    value->uses.emplace_front(this, operandNumber);
}

void Operation::eraseOperand(size_t operandNumber) {
    auto &uses = operands[operandNumber]->uses;
    uses.remove_if([&](const Value::Use &use) { return use.user == this && use.operandNumber == operandNumber; });
    operands.erase(operands.begin() + operandNumber);
}

Value::Ptr Operation::addResult(Type::Ptr type) {
    return results.emplace_back(Value::make(type, this));
}

void Operation::addToBody(Operation::Ptr op) {
    body.emplace_back(op);
}

void Operation::erase() {
    for (const auto &result : results) {
        if (!result->uses.empty())
            throw std::logic_error("Operation cannot be erased since its results still have uses");
    }
    results.clear();
    if (!parent || position == Body::iterator())
        return;
    parent->body.erase(position);
}

bool Operation::verify() const {
    return verifier(this);
}

std::string Operation::dump() const {
    std::stringstream str;
    dump(str);
    return str.str();
}

static void dumpValue(const Value::Ptr &value, std::ostream &stream, int valueId) {
    stream << '#' << valueId << " : ";
    value->type->dump(stream);
}

static void dumpOperation(const Operation *op, std::ostream &stream, int depth,
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
    utils::interleaveComma(stream, op->results, [&](const Value::Ptr &result) {
        dumpValue(result, stream, nextValueId);
        valueIds[result.get()] = nextValueId++;
    });
    stream << ")\n";
    for (const auto &op : op->body)
        dumpOperation(op.get(), stream, depth + 1, valueIds, nextValueId);
}

void Operation::dump(std::ostream &stream) const {
    std::unordered_map<const Value *, int> valueIds;
    int valueId = 0;
    dumpOperation(this, stream, 0, valueIds, valueId);
}
