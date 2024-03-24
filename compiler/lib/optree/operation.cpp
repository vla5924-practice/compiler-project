#include "operation.hpp"

#include <ostream>
#include <sstream>

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

Value::Ptr Operation::addResult(const Type &type) {
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

std::string Operation::dump(int depth) const {
    std::stringstream str;
    dump(str, depth);
    return str.str();
}

void Operation::dump(std::ostream &stream, int depth) const {
    for (int i = 0; i < depth; i++)
        stream << "  ";
    stream << name << " ( ";
    for (const auto &operand : operands) {
        stream << operand.get() << ':';
        operand->type.dump(stream);
        stream << ' ';
    }
    stream << ") -> ";
    for (const auto &result : results) {
        stream << result.get() << ':';
        result->type.dump(stream);
        stream << ' ';
    }
    stream << '\n';
    for (const auto &op : body)
        op->dump(stream, depth + 1);
}
