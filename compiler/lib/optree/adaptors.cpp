#include "adaptors.hpp"

#include <cstdint>
#include <string>
#include <vector>

#include "base_adaptor.hpp"
#include "definitions.hpp"
#include "operation.hpp"
#include "types.hpp"
#include "value.hpp"

using namespace optree;

// The following definitions for the methods of the operation classes should be arranged in alphabetical order.

void AllocateOp::init(const Type::Ptr &type) {
    op->results.emplace_back(Value::make(type, op));
}

void ArithBinaryOp::init(ArithBinOpKind kind, const Type::Ptr &resultType, const Value::Ptr &lhs,
                         const Value::Ptr &rhs) {
    BinaryOp::init(resultType, lhs, rhs);
    op->addAttr(kind);
}

void ArithBinaryOp::init(ArithBinOpKind kind, const Value::Ptr &lhs, const Value::Ptr &rhs) {
    init(kind, lhs->type, lhs, rhs);
}

void ArithCastOp::init(ArithCastOpKind kind, const Type::Ptr &resultType, const Value::Ptr &value) {
    UnaryOp::init(resultType, value);
    op->addAttr(kind);
}

void BinaryOp::init(const Type::Ptr &resultType, const Value::Ptr &lhs, const Value::Ptr &rhs) {
    op->addResult(resultType);
    op->addOperand(lhs);
    op->addOperand(rhs);
}

void ConditionOp::init() {
}

Value::Ptr ConditionOp::terminator() const {
    if (op->body.empty())
        return {};
    return op->body.back()->result(0);
}

void ConstantOp::init(const Type::Ptr &type, int64_t value) {
    op->results.emplace_back(Value::make(type, op));
    op->addAttr(value);
}

void ConstantOp::init(const Type::Ptr &type, double value) {
    op->results.emplace_back(Value::make(type, op));
    op->addAttr(value);
}

void ConstantOp::init(const Type::Ptr &type, const std::string &value) {
    op->results.emplace_back(Value::make(type, op));
    op->addAttr(value);
}

void ElseOp::init() {
}

void ForOp::init(const Type::Ptr &iteratorType, const Value::Ptr &start, const Value::Ptr &stop,
                 const Value::Ptr &step) {
    op->addOperand(start);
    op->addOperand(stop);
    op->addOperand(step);
    op->addInward(iteratorType);
}

void FunctionOp::init(const std::string &name, const Type::Ptr &funcType) {
    op->addAttr(name);
    op->addAttr(funcType);
    for (const auto &argType : funcType->as<FunctionType>().arguments)
        op->addInward(argType);
}

void FunctionCallOp::init(const std::string &name, const Type::Ptr &resultType,
                          const std::vector<Value::Ptr> &arguments) {
    op->operands = arguments;
    op->results.emplace_back(Value::make(resultType, op));
    op->addAttr(name);
}

void FunctionCallOp::init(const FunctionOp &callee, const std::vector<Value::Ptr> &arguments) {
    init(callee.name(), callee.type().result, arguments);
}

void IfOp::init(const Value::Ptr &cond, bool withElse) {
    op->addOperand(cond);
    op->addToBody(Operation::make<ThenOp>(op).op);
    if (withElse)
        op->addToBody(Operation::make<ElseOp>(op).op);
}

ThenOp IfOp::thenOp() const {
    return {op->body.front()};
}

ElseOp IfOp::elseOp() const {
    return {op->body.size() == 2 ? op->body.back() : nullptr};
}

void InputOp::init(const Type::Ptr &inputType) {
    op->addResult(inputType);
}

void LoadOp::init(const Type::Ptr &resultType, const Value::Ptr &src) {
    op->addOperand(src);
    op->results.emplace_back(Value::make(resultType, op));
}

void LoadOp::init(const Value::Ptr &src) {
    auto resultType = src->type->as<PointerType>().pointee;
    init(resultType, src);
}

void LogicBinaryOp::init(LogicBinOpKind kind, const Value::Ptr &lhs, const Value::Ptr &rhs) {
    BinaryOp::init(TypeStorage::integerType(), lhs, rhs);
    op->addAttr(kind);
}

void LogicUnaryOp::init(LogicUnaryOpKind kind, const Type::Ptr &resultType, const Value::Ptr &value) {
    UnaryOp::init(resultType, value);
    op->addAttr(kind);
}

void ModuleOp::init() {
}

void PrintOp::init(const Value::Ptr &valueToPrint) {
    op->addOperand(valueToPrint);
}

void ReturnOp::init() {
}

void ReturnOp::init(const Value::Ptr &value) {
    op->addOperand(value);
}

void StoreOp::init(const Value::Ptr &dst, const Value::Ptr &valueToStore) {
    op->addOperand(dst);
    op->addOperand(valueToStore);
}

void ThenOp::init() {
}

void UnaryOp::init(const Type::Ptr &resultType, const Value::Ptr &value) {
    op->addResult(resultType);
    op->addOperand(value);
}

void WhileOp::init() {
    op->addToBody(Operation::make<ConditionOp>(op).op);
}

ConditionOp WhileOp::conditionOp() const {
    return {op->body.front()};
}
