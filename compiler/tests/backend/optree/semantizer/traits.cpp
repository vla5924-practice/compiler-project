#include <deque>
#include <utility>

#include <gtest/gtest.h>

#include "compiler/backend/optree/semantizer/traits.hpp"
#include "compiler/optree/operation.hpp"

#include "common.hpp"

using namespace optree;
using namespace optree::semantizer;

namespace {

class TraitTest : public TestWithDummyValues {
  protected:
    Operation::Ptr op;
    SemantizerContext ctx;

    template <typename Trait, typename... Args>
    bool verify(Args... args) {
        return Trait::verify(op, ctx, std::forward<Args>(args)...);
    }

  public:
    TraitTest() : TestWithDummyValues(), op(makeOp()){};
    ~TraitTest() = default;
};

} // namespace

TEST_F(TraitTest, has_operands_on_success) {
    op->addOperand(makeValue());
    op->addOperand(makeValue());
    ASSERT_TRUE(verify<HasOperands>(2));
}

TEST_F(TraitTest, has_operands_on_failure) {
    ASSERT_FALSE(verify<HasOperands>(7));
}
