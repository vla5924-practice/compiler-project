#include <gtest/gtest.h>

#include "compiler/optree/types.hpp"

using namespace optree;

TEST(TypeStorage, can_obtain_none_type) {
    ASSERT_NO_THROW(TypeStorage::noneType());
}

TEST(TypeStorage, can_obtain_valid_none_type) {
    ASSERT_TRUE(TypeStorage::noneType()->is<NoneType>());
}

TEST(TypeStorage, can_obtain_predictable_none_type) {
    auto expected = Type::make<NoneType>();
    ASSERT_EQ(*expected, *TypeStorage::noneType());
}

TEST(TypeStorage, can_obtain_integer_type) {
    ASSERT_NO_THROW(TypeStorage::integerType(64U));
}

TEST(TypeStorage, can_obtain_valid_integer_type) {
    ASSERT_TRUE(TypeStorage::integerType(64U)->is<IntegerType>());
}

TEST(TypeStorage, can_obtain_predictable_integer_type) {
    auto expected = Type::make<IntegerType>(123U);
    auto actual = TypeStorage::integerType(123U);
    ASSERT_EQ(*expected, *actual);
    ASSERT_EQ(expected->width, actual->width);
}

TEST(TypeStorage, can_obtain_float_type) {
    ASSERT_NO_THROW(TypeStorage::floatType(64U));
}

TEST(TypeStorage, can_obtain_valid_float_type) {
    ASSERT_TRUE(TypeStorage::floatType(64U)->is<FloatType>());
}

TEST(TypeStorage, can_obtain_predictable_float_type) {
    auto expected = Type::make<FloatType>(123U);
    auto actual = TypeStorage::floatType(123U);
    ASSERT_EQ(*expected, *actual);
    ASSERT_EQ(expected->width, actual->width);
}
