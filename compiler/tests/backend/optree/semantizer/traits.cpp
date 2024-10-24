#include <deque>
#include <utility>

#include <gtest/gtest.h>

#include "compiler/backend/optree/semantizer/traits.hpp"
#include "compiler/optree/operation.hpp"

using namespace optree;
using namespace optree::semantizer;

namespace {

class TraitTest : public ::testing::Test {
    Operation::Ptr dummy;

  protected:
    Operation::Ptr op;
    SemantizerContext ctx;

    static Operation::Ptr makeOp() {
        return Operation::make("TestOp");
    }

    Value::Ptr makeValue(const Type::Ptr &type) {
        return dummy->addResult(type);
    }

    Value::Ptr makeValue() {
        return makeValue(TypeStorage::noneType());
    }

    template <typename Trait, typename... Args>
    bool verify(Args... args) {
        return Trait::verify(op, ctx, std::forward<Args>(args)...);
    }

  public:
    TraitTest() : dummy(makeOp()), op(makeOp()){};
    ~TraitTest() = default;
};

} // namespace

TEST_F(TraitTest, HasOperands_true) {
    op->addOperand(makeValue());
    op->addOperand(makeValue());
    ASSERT_TRUE(verify<HasOperands>(2));
}

TEST_F(TraitTest, HasOperands_false) {
    ASSERT_FALSE(verify<HasOperands>(7));
}
