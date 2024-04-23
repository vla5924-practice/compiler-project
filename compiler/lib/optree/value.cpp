#include "value.hpp"

#include "compiler/utils/source_ref.hpp"

#include "operation.hpp"

using namespace optree;

const utils::SourceRef &Value::ref() const {
    return owner.lock()->ref;
}

Value::Use::Use(const BackRef &user, size_t operandNumber) : user(user), operandNumber(operandNumber) {
}

Operation::Ptr Value::Use::lock() const noexcept {
    return user.lock();
}

bool Value::Use::userIs(const Operation *op) const noexcept {
    return lock().get() == op;
}
