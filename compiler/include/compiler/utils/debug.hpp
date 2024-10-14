#pragma once

// Suppress security warning regarding some I/O functions on MSVC
#pragma warning(disable : 4996)

#include <cstdlib>
#include <iostream>
#include <string>

#if defined(NDEBUG) && !defined(DEBUG)
#define COMPILER_DEBUG(STATEMENT)
#else
#define COMPILER_DEBUG(STATEMENT) STATEMENT
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
