#pragma once

#include <gtest/gtest.h>

#include <string>
#include <utility>

#include "compiler/backend/optree/optimizer/optimizer.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/optree/types.hpp"
#include "compiler/optree/value.hpp"

class TestWithDummyValues : public ::testing::Test {
    optree::Operation::Ptr dummy;

  protected:
    static optree::Operation::Ptr makeOp() {
        return optree::Operation::make("Dummy");
    }

    optree::Value::Ptr makeValue(const optree::Type::Ptr &type) {
        return dummy->addResult(type);
    }

    optree::Value::Ptr makeValue() {
        return makeValue(optree::TypeStorage::noneType());
    }

    TestWithDummyValues() : dummy(makeOp()){};
    ~TestWithDummyValues() = default;
};
