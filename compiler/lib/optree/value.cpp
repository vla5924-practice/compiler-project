#include "value.hpp"

#include "compiler/optree/operation.hpp"

using namespace optree;

const utils::SourceRef &Value::ref() const {
    return owner->ref;
}
