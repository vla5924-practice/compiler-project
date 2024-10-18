#include <gtest/gtest.h>

#include "compiler/backend/optree/optimizer/optimizer.hpp"

using namespace optree;
using namespace optree::optimizer;

TEST(Optimizer, can_construct_optimizer) {
    ASSERT_NO_THROW(Optimizer opt);
}
