#include "adaptors.hpp"

#include "compiler/optree/traits.hpp"

using namespace optree;
using namespace trait;

bool ModuleOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<0U>(op) && numResults<0U>(op) && bodyContainsOnly<FunctionOp>(op);
}

bool FunctionOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<0U>(op) && numResults<0U>(op) && op->attr(0).is<std::string>() &&
           op->attr(1).is<Type>() && op->attr(1).as<Type>().is<FunctionType>();
}

bool ArithBinaryOp::verify(const Operation *op) {
    return Adaptor::verify(op) && numOperands<2U>(op) && numResults<2U>(op) && oneAttrOfType<ArithBinOpKind>(op);
}
