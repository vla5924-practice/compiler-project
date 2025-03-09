#include <deque>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_set>

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/utils/helpers.hpp"
#include "compiler/utils/language.hpp"

#include "optimizer/opt_builder.hpp"
#include "optimizer/transform.hpp"

using namespace optree;
using namespace optree::optimizer;

namespace {

struct BooleanExpressionMinimization : public Transform<ModuleOp> {
    using Transform::Transform;

    std::string_view name() const override {
        return "BooleanExpressionMinimization";
    }

    void run(const Operation::Ptr &op, OptBuilder &builder) const override {
        
    }
};

} // namespace

namespace optree {
namespace optimizer {

BaseTransform::Ptr createBooleanExpressionMinimization() {
    return std::make_shared<BooleanExpressionMinimization>();
}

} // namespace optimizer
} // namespace optree
