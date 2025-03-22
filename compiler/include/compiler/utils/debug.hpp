#pragma once

// Suppress security warning regarding some I/O functions on MSVC
#pragma warning(disable : 4996)

#include <cstdlib>
#include <iostream>
#include <string>

#include "compiler/utils/platform.hpp"

#if defined(ENABLE_COMPILER_DEBUG) || !defined(NDEBUG) || defined(DEBUG)
#define COMPILER_DEBUG(STATEMENT) STATEMENT
#else
#define COMPILER_DEBUG(...)
#endif

// NOLINTNEXTLINE(bugprone-macro-parentheses)
#define COMPILER_DEBUG_PRINT(SUBEXPR) COMPILER_DEBUG(::utils::DebugPrinter::get() << SUBEXPR)

#if defined(COMPILER_TOOLCHAIN_MSVC)
#define COMPILER_UNREACHABLE(MESSAGE)                                                                                  \
    do {                                                                                                               \
        COMPILER_DEBUG_PRINT("UNREACHABLE: " << (MESSAGE) << '\n');                                                    \
        __assume(false);                                                                                               \
    } while (0)
#elif defined(COMPILER_TOOLCHAIN_GCC_COMPATIBLE)
#define COMPILER_UNREACHABLE(MESSAGE)                                                                                  \
    do {                                                                                                               \
        COMPILER_DEBUG_PRINT("UNREACHABLE: " << (MESSAGE) << '\n');                                                    \
        __builtin_unreachable();                                                                                       \
    } while (0)
#else
#define COMPILER_UNREACHABLE(MESSAGE) COMPILER_DEBUG_PRINT("UNREACHABLE: " << (MESSAGE) << '\n')
#endif

namespace utils {

class DebugPrinter {
    bool printerEnabled;

    DebugPrinter() : printerEnabled(false) {
        if (const char *valuePtr = std::getenv("COMPILER_DEBUG")) {
            std::string env(valuePtr);
            printerEnabled = (env == "1");
        }
    }
    DebugPrinter(const DebugPrinter &) = delete;
    DebugPrinter(DebugPrinter &&) = delete;
    ~DebugPrinter() = default;

  public:
    bool enabled() const {
        return printerEnabled;
    }

    void setEnabled(bool newValue) {
        printerEnabled = newValue;
    }

    template <typename T>
    DebugPrinter &operator<<(const T &object) {
        if (printerEnabled)
            std::cerr << object;
        return *this;
    }

    static DebugPrinter &get() {
        static DebugPrinter instance;
        return instance;
    }
};

} // namespace utils
