#include "timer.h"

Timer::Timer() : is_running(false) {
}

void Timer::start() {
    start_time = std::chrono::high_resolution_clock::now();
    is_running = true;
}

void Timer::stop() {
    if (is_running) {
        end_time = std::chrono::high_resolution_clock::now();
        is_running = false;
    }
}

double Timer::getElapsedMs() const {
    if (is_running) {
        auto current_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time);
        return duration.count() / 1000.0;
    } else {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return duration.count() / 1000.0;
    }
}

double Timer::getElapsedUs() const {
    if (is_running) {
        auto current_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time);
        return static_cast<double>(duration.count());
    } else {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return static_cast<double>(duration.count());
    }
}

void Timer::reset() {
    is_running = false;
}