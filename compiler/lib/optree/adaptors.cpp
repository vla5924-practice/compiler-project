#include "adaptors.hpp"

#include "compiler/optree/traits.hpp"

using namespace optree;
using namespace trait;

// The following definitions for the methods of the operation classes should be arranged in alphabetical order.

void AllocateOp::init(Type::Ptr type) {
    op->results.emplace_back(Value::make(type, op));
}

bool AllocateOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<0U>(op) && numResults<1U>(op);
}

void ArithBinaryOp::init(ArithBinOpKind kind, Type::Ptr resultType, Value::Ptr lhs, Value::Ptr rhs) {
    BinaryOp::init(resultType, lhs, rhs);
    op->addAttr(kind);
}

void ArithBinaryOp::init(ArithBinOpKind kind, Value::Ptr lhs, Value::Ptr rhs) {
    init(kind, lhs->type, lhs, rhs);
}

bool ArithBinaryOp::verify(const Operation *op) {
    return BinaryOp::verify(op) && oneAttrOfType<ArithBinOpKind>(op) && operandsAndResultsHaveSameType(op);
}

void ArithCastOp::init(ArithCastOpKind kind, Type::Ptr resultType, Value::Ptr value) {
    UnaryOp::init(resultType, value);
    op->addAttr(kind);
}

bool ArithCastOp::verify(const Operation *op) {
    return UnaryOp::verify(op) && oneAttrOfType<ArithCastOpKind>(op);
}

void BinaryOp::init(Type::Ptr resultType, Value::Ptr lhs, Value::Ptr rhs) {
    op->addResult(resultType);
    op->addOperand(lhs);
    op->addOperand(rhs);
}

bool BinaryOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<2U>(op) && numResults<1U>(op);
}

void ConditionOp::init() {
}

Value::Ptr ConditionOp::terminator() const {
    if (op->body.empty())
        return {};
    return op->body.back()->result(0);
}

bool ConditionOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<0U>(op) && numResults<0U>(op) && !op->body.empty() &&
           numResults<1U>(op->body.back().get());
}

void ConstantOp::init(Type::Ptr type, int64_t value) {
    op->results.emplace_back(Value::make(type, op));
    op->addAttr(value);
}

void ConstantOp::init(Type::Ptr type, double value) {
    op->results.emplace_back(Value::make(type, op));
    op->addAttr(value);
}

void ConstantOp::init(Type::Ptr type, const std::string &value) {
    op->results.emplace_back(Value::make(type, op));
    op->addAttr(value);
}

bool ConstantOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<0U>(op) && numResults<1U>(op) && numAttrs<1U>(op);
}

void ElseOp::init() {
}

bool ElseOp::verify(const Operation *op) {
    return Adaptor::verify(op);
}

void ForOp::init(Type::Ptr iteratorType, Value::Ptr start, Value::Ptr stop, Value::Ptr step) {
    op->addOperand(start);
    op->addOperand(stop);
    op->addOperand(step);
    op->addInward(iteratorType);
}

bool ForOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<3U>(op) && numResults<0U>(op);
}

void FunctionOp::init(const std::string &name, Type::Ptr funcType) {
    op->addAttr(name);
    op->addAttr(funcType);
    for (auto argType : funcType->as<FunctionType>().arguments)
        op->addInward(argType);
}

bool FunctionOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<0U>(op) && numResults<0U>(op) && op->attr(0).is<std::string>() &&
           op->attr(1).isType<FunctionType>();
}

void FunctionCallOp::init(const std::string &name, Type::Ptr resultType, const std::vector<Value::Ptr> &arguments) {
    op->operands = arguments;
    op->results.emplace_back(Value::make(resultType, op));
    op->addAttr(name);
}

void FunctionCallOp::init(const FunctionOp &callee, const std::vector<Value::Ptr> &arguments) {
    init(callee.name(), callee.type().result, arguments);
}

bool FunctionCallOp::verify(const Operation *op) {
    bool staticCheck = Adaptor::verify(op) && numResults<1U>(op) && op->attr(0).is<std::string>();
    if (!staticCheck)
        return false;
    if (auto moduleOp = op->findParent<ModuleOp>()) {
        if (auto funcOp = moduleOp.lookup<FunctionOp>(op->attr(0).as<std::string>())) {
            const auto &type = funcOp.type();
            return op->numOperands() == type.arguments.size() && op->result(0)->hasType(type.result);
        }
    }
    return false;
}

void IfOp::init(Value::Ptr cond, bool withElse) {
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

bool IfOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<1U>(op) && numResults<0U>(op);
}

void InputOp::init(Type::Ptr inputType) {
    op->addResult(inputType);
}

bool InputOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<0U>(op) && numResults<1U>(op);
}

void LoadOp::init(Type::Ptr resultType, Value::Ptr src) {
    op->addOperand(src);
    op->results.emplace_back(Value::make(resultType, op));
}

void LoadOp::init(Value::Ptr src) {
    auto resultType = src->type->as<PointerType>().pointee;
    init(resultType, src);
}

bool LoadOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<1U>(op) && numResults<1U>(op) &&
           op->operand(0)->canPointTo(op->result(0));
}

void LogicBinaryOp::init(LogicBinOpKind kind, Value::Ptr lhs, Value::Ptr rhs) {
    BinaryOp::init(TypeStorage::integerType(), lhs, rhs);
    op->addAttr(kind);
}

bool LogicBinaryOp::verify(const Operation *op) {
    return BinaryOp::verify(op) && oneAttrOfType<LogicBinOpKind>(op) && operandsHaveSameType(op) &&
           op->result(0)->type->is<IntegerType>();
}

void LogicUnaryOp::init(LogicUnaryOpKind kind, Type::Ptr resultType, Value::Ptr value) {
    UnaryOp::init(resultType, value);
    op->addAttr(kind);
}

bool LogicUnaryOp::verify(const Operation *op) {
    return UnaryOp::verify(op) && oneAttrOfType<LogicUnaryOpKind>(op) && operandsAndResultsHaveSameType(op) &&
           op->result(0)->type->is<IntegerType>();
}

void ModuleOp::init() {
}

bool ModuleOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<0U>(op) && numResults<0U>(op) && bodyContainsOnly<FunctionOp>(op);
}

void PrintOp::init(Value::Ptr valueToPrint) {
    op->addOperand(valueToPrint);
}

bool PrintOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<1U>(op) && numResults<0U>(op);
}

void ReturnOp::init() {
}

void ReturnOp::init(Value::Ptr value) {
    op->addOperand(value);
}

bool ReturnOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numResults<0U>(op) && op->numOperands() <= 1U;
}

void StoreOp::init(Value::Ptr dst, Value::Ptr valueToStore) {
    op->addOperand(dst);
    op->addOperand(valueToStore);
}

bool StoreOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<2U>(op) && numResults<0U>(op) &&
           op->operand(0)->canPointTo(op->result(1));
}

void ThenOp::init() {
}

bool ThenOp::verify(const Operation *op) {
    return Adaptor::verify(op);
}

void UnaryOp::init(Type::Ptr resultType, Value::Ptr value) {
    op->addResult(resultType);
    op->addOperand(value);
}

bool UnaryOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<1U>(op) && numResults<1U>(op);
}

void WhileOp::init() {
    op->addToBody(Operation::make<ConditionOp>(op).op);
}

ConditionOp WhileOp::conditionOp() const {
    return {op->body.front()};
}

bool WhileOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<0U>(op) && numResults<0U>(op) && !op->body.empty() &&
           op->body.front()->is<ConditionOp>();
}
