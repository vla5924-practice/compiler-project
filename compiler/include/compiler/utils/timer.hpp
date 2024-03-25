#pragma once

#include <chrono>


namespace utils {
    class Timer {
        using time_point = std::chrono::time_point<std::chrono::steady_clock>;
        using elapsed_time = std::chrono::duration<double>;

        time_point start_point;
        time_point end_point;
    public:
        void start() {
            start_point = std::chrono::steady_clock::now();
        }

        void end() {
            end_point = std::chrono::steady_clock::now();
        }

        double elapsed() {
            return std::chrono::duration_cast<std::chrono::milliseconds>(end_point - start_point).count();
        }
    };
}



