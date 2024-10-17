#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "compiler/utils/base_error.hpp"
#include "compiler/utils/source_ref.hpp"

using namespace utils;

namespace {

class Dumpable {
    std::string data;

  public:
    explicit Dumpable(const std::string &data) : data(data) {
    }
    ~Dumpable() = default;

    std::string dump() const {
        return data;
    }
};

class TestError : public BaseError {
  public:
    TestError() = default;
    ~TestError() = default;
};

} // namespace

TEST(BaseError, can_construct_default) {
    ASSERT_NO_THROW(BaseError error);
}

TEST(BaseError, can_construct_from_message) {
    std::string message = "Hello world";
    BaseError error(message);
    ASSERT_STREQ(error.what(), "Error:\nHello world");
}

TEST(BaseError, can_construct_from_source_ref_and_message) {
    SourceRef ref(nullptr, 0, 0);
    std::string message = "Hello world";
    BaseError error(ref, message);
    ASSERT_STREQ(error.what(), "In line 0 in column 0 error:\nHello world");
}

TEST(BaseError, can_append_strings) {
    BaseError error;
    error << std::string("Hello world") << "123";
    ASSERT_STREQ(error.what(), "Error:\nHello world123");
}

TEST(BaseError, can_append_via_to_string) {
    BaseError error;
    error << -5 << 779U;
    ASSERT_STREQ(error.what(), "Error:\n-5779");
}

TEST(BaseError, can_append_via_dump) {
    BaseError error;
    error << Dumpable("Dumped");
    ASSERT_STREQ(error.what(), "Error:\nDumped");
}

TEST(BaseError, can_append_via_dump_ptr) {
    BaseError error;
    error << std::make_unique<Dumpable>("Dumped");
    ASSERT_STREQ(error.what(), "Error:\nDumped");
}

TEST(BaseError, can_append_on_derived_class) {
    TestError error;
    error << "TestError" << 3;
    ASSERT_STREQ(error.what(), "Error:\nTestError3");
}
