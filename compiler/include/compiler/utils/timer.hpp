#pragma once

#include <chrono>

namespace utils {

struct Timer {
    using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

    Timer() = default;
    Timer(const Timer &) = default;
    Timer(Timer &&) = default;
    ~Timer() = default;

    void start() {
        startPoint = std::chrono::steady_clock::now();
    }

    void stop() {
        stopPoint = std::chrono::steady_clock::now();
    }

    // Obtain elapsed time in milliseconds
    auto elapsed() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(stopPoint - startPoint).count();
    }

  private:
    TimePoint startPoint;
    TimePoint stopPoint;
};

} // namespace utils
