#pragma once

#include <gtest/gtest.h>

#include <string>
#include <utility>

#include "compiler/backend/optree/optimizer/optimizer.hpp"
#include "compiler/optree/declarative.hpp"

class TransformTestBase : public ::testing::Test {
    optree::DeclarativeModule actual;
    optree::DeclarativeModule expected;
    std::string expectedStr;

    virtual void setupOptimizer(optree::optimizer::Optimizer &opt) const {
    }

  protected:
    std::pair<optree::DeclarativeModule &, optree::ValueStorage &> getActual() {
        return {actual, actual.values()};
    }

    std::pair<optree::DeclarativeModule &, optree::ValueStorage &> getExpected() {
        return {expected, expected.values()};
    }

    void saveActualAsExpected() {
        expectedStr = actual.dump();
    }

    void runOptimizer() {
        optree::optimizer::Optimizer opt;
        setupOptimizer(opt);
        optree::Program program = actual.makeProgram();
        opt.process(program);
    }

    auto assertSameOpTree() {
        if (expectedStr.empty())
            ASSERT_EQ(expected.dump(), actual.dump());
        else
            ASSERT_EQ(expectedStr, actual.dump());
    }

  public:
    TransformTestBase() = default;
    virtual ~TransformTestBase() = default;
};
