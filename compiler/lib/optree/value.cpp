#include "value.hpp"

#include "compiler/utils/source_ref.hpp"

#include "operation.hpp"

using namespace optree;

const utils::SourceRef &Value::ref() const {
    return owner->ref;
}
