#include "builder.hpp"

#include <iterator>

#include "operation.hpp"

using namespace optree;

Builder Builder::before(const Operation::Ptr &op) {
    return {op->parent, op->position};
}

Builder Builder::after(const Operation::Ptr &op) {
    return {op->parent, std::next(op->position)};
}

Builder Builder::atBodyBegin(const Operation::Ptr &op) {
    if (op->body.empty())
        return atBodyEnd(op);
    return {op, std::next(op->body.begin())};
}

Builder Builder::atBodyEnd(const Operation::Ptr &op) {
    return {op, op->body.end()};
}

void Builder::setInsertPointBefore(const Operation::Ptr &op) {
    currentOp = op->parent;
    insertPoint = op->position;
}

void Builder::setInsertPointAfter(const Operation::Ptr &op) {
    currentOp = op->parent;
    insertPoint = std::next(op->position);
}

void Builder::setInsertPointAtBodyBegin(const Operation::Ptr &op) {
    if (op->body.empty()) {
        setInsertPointAtBodyEnd(op);
        return;
    }
    currentOp = op;
    insertPoint = std::next(op->body.begin());
}

void Builder::setInsertPointAtBodyEnd(const Operation::Ptr &op) {
    currentOp = op;
    insertPoint = op->body.end();
}

void Builder::insert(const Operation::Ptr &op) {
    auto it = currentOp->body.insert(insertPoint, op);
    op->parent = currentOp;
    op->position = it;
}
