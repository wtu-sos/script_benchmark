#pragma once

#include <chrono>
#include <string>

class Timer {
public:
    Timer();
    void start();
    void stop();
    double getElapsedMs() const;
    double getElapsedUs() const;
    void reset();

private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point end_time;
    bool is_running;
};