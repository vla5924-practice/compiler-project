#include "builder.hpp"

#include <iterator>

#include "operation.hpp"

using namespace optree;

Builder Builder::before(Operation *op) {
    return {op->parent, op->position};
}

Builder Builder::after(Operation *op) {
    return {op->parent, std::next(op->position)};
}

Builder Builder::atBodyBegin(Operation *op) {
    if (op->body.empty())
        return atBodyEnd(op);
    return {op, std::next(op->body.begin())};
}

Builder Builder::atBodyEnd(Operation *op) {
    return {op, op->body.end()};
}

void Builder::setInsertPointBefore(Operation *op) {
    currentOp = op->parent;
    insertPoint = op->position;
}

void Builder::setInsertPointAfter(Operation *op) {
    currentOp = op->parent;
    insertPoint = std::next(op->position);
}

void Builder::setInsertPointAtBodyBegin(Operation *op) {
    if (op->body.empty()) {
        setInsertPointAtBodyEnd(op);
        return;
    }
    currentOp = op;
    insertPoint = std::next(op->body.begin());
}

void Builder::setInsertPointAtBodyEnd(Operation *op) {
    currentOp = op;
    insertPoint = op->body.end();
}

void Builder::insert(Operation *op) {
    auto it = currentOp->body.insert(insertPoint, op);
    op->parent = currentOp;
    op->position = it;
}
