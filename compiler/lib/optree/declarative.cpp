#include "declarative.hpp"

#include <cstddef>
#include <ostream>
#include <string>

#include "adaptors.hpp"
#include "builder.hpp"
#include "operation.hpp"
#include "program.hpp"
#include "types.hpp"

using namespace optree;

DeclarativeValue &DeclarativeValue::operator=(const DeclarativeModule &mod) {
    value = mod.currentResult();
    return *this;
}

DeclarativeModule::DeclarativeModule()
    : root(Operation::make<ModuleOp>()), current(root), builder(Builder::atBodyBegin(root)),
      tNone(TypeStorage::noneType()), tI64(TypeStorage::integerType(64U)), tBool(TypeStorage::boolType()),
      tF64(TypeStorage::floatType(64U)), tStr(TypeStorage::strType(8U)) {
}

Type::Ptr DeclarativeModule::tPtr(const Type::Ptr &pointee) const {
    return Type::make<PointerType>(pointee);
}

Type::Ptr DeclarativeModule::tFunc(const Type::Ptr &result) const {
    return Type::make<FunctionType>(result);
}

Type::Ptr DeclarativeModule::tFunc(Type::PtrVector &&arguments, const Type::Ptr &result) const {
    return Type::make<FunctionType>(arguments, result);
}

ValueStorage &DeclarativeModule::values() {
    return valueStorage;
}

DeclarativeModule &DeclarativeModule::operand(const DeclarativeValue &operandValue) {
    current->addOperand(operandValue);
    return *this;
}

DeclarativeModule &DeclarativeModule::operand(size_t index, const DeclarativeValue &operandValue) {
    current->operand(index) = operandValue;
    return *this;
}

DeclarativeModule &DeclarativeModule::result(const Type::Ptr &type) {
    current->addResult(type);
    return *this;
}

DeclarativeModule &DeclarativeModule::inward(DeclarativeValue &inward, const Type::Ptr &type) {
    inward.value = current->addInward(type);
    return *this;
}

DeclarativeModule &DeclarativeModule::inward(DeclarativeValue &inward, size_t index) {
    inward.value = current->inward(index);
    return *this;
}

void DeclarativeModule::withBody() {
    builder.setInsertPointAtBodyBegin(current);
}

void DeclarativeModule::endBody() {
    current = current->parent;
    builder.setInsertPointAfter(current);
}

const Operation::Ptr &DeclarativeModule::rootOp() const {
    return root;
}

Program DeclarativeModule::makeProgram() const {
    return {root};
}

std::string DeclarativeModule::dump() const {
    return root->dump();
}

void DeclarativeModule::dump(std::ostream &stream) const {
    root->dump(stream);
}
