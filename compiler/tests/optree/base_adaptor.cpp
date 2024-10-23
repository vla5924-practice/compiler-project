#include <gtest/gtest.h>

#include "compiler/optree/base_adaptor.hpp"

using namespace optree;

namespace {

struct ColorOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Color")
};

struct RedColorOp : ColorOp {
    OPTREE_ADAPTOR_HELPER(ColorOp, "RedColor")
};

struct AnimalOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Animal")
};

struct CatOp : AnimalOp {
    OPTREE_ADAPTOR_HELPER(AnimalOp, "Cat")
};

} // namespace

TEST(BaseAdaptor, can_check_implements_own_spec) {
    ColorOp op1;
    RedColorOp op2;
    AnimalOp op3;
    CatOp op4;
    ASSERT_TRUE(op1.implementsSpecById(ColorOp::getSpecId()));
    ASSERT_TRUE(op2.implementsSpecById(RedColorOp::getSpecId()));
    ASSERT_TRUE(op3.implementsSpecById(AnimalOp::getSpecId()));
    ASSERT_TRUE(op4.implementsSpecById(CatOp::getSpecId()));
}

TEST(BaseAdaptor, can_check_not_implement_other_spec) {
    ColorOp op1;
    RedColorOp op2;
    AnimalOp op3;
    CatOp op4;
    ASSERT_FALSE(op1.implementsSpecById(RedColorOp::getSpecId()));
    ASSERT_FALSE(op2.implementsSpecById(AnimalOp::getSpecId()));
    ASSERT_FALSE(op3.implementsSpecById(CatOp::getSpecId()));
    ASSERT_FALSE(op4.implementsSpecById(ColorOp::getSpecId()));
}

TEST(BaseAdaptor, can_check_derived_implements_base_spec) {
    RedColorOp op1;
    CatOp op2;
    ASSERT_TRUE(op1.implementsSpecById(ColorOp::getSpecId()));
    ASSERT_TRUE(op2.implementsSpecById(AnimalOp::getSpecId()));
}
