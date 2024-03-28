#include "adaptors.hpp"

#include "compiler/optree/traits.hpp"

using namespace optree;
using namespace trait;

bool ModuleOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<0U>(op) && numResults<0U>(op) && bodyContainsOnly<FunctionOp>(op);
}

bool FunctionOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<0U>(op) && numResults<0U>(op) && op->attr(0).is<std::string>() &&
           op->attr(1).isType<FunctionType>();
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

bool ReturnOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numResults<0U>(op) && op->numOperands() <= 1U;
}

bool AllocateOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<0U>(op) && numResults<1U>(op);
}

bool ConstantOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<0U>(op) && numResults<1U>(op) && numAttrs<1U>(op);
}

bool BinaryOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<2U>(op) && numResults<1U>(op);
}

bool ArithBinaryOp::verify(const Operation *op) {
    return BinaryOp::verify(op) && oneAttrOfType<ArithBinOpKind>(op) && operandsAndResultsHaveSameType(op);
}

bool LogicBinaryOp::verify(const Operation *op) {
    return BinaryOp::verify(op) && oneAttrOfType<LogicBinOpKind>(op) && operandsHaveSameType(op) &&
           op->result(0)->type->is<IntegerType>();
}

bool UnaryOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<1U>(op) && numResults<1U>(op);
}

bool ArithCastOp::verify(const Operation *op) {
    return UnaryOp::verify(op) && oneAttrOfType<ArithCastOpKind>(op);
}

bool LogicUnaryOp::verify(const Operation *op) {
    return UnaryOp::verify(op) && oneAttrOfType<LogicUnaryOpKind>(op) && operandsAndResultsHaveSameType(op) &&
           op->result(0)->type->is<IntegerType>();
}

bool LoadOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<1U>(op) && numResults<1U>(op) &&
           op->operand(0)->canPointTo(op->result(0));
}

bool StoreOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<2U>(op) && numResults<0U>(op) &&
           op->operand(0)->canPointTo(op->result(1));
}

bool ConditionOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<0U>(op) && numResults<0U>(op) && !op->body.empty() &&
           numResults<1U>(op->body.back().get());
}

bool WhileOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<0U>(op) && numResults<0U>(op) && !op->body.empty() &&
           op->body.front()->is<ConditionOp>();
}
