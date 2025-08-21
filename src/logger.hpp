#pragma once
#include <iostream>
#include <mutex>
#include <string>
#include <chrono>
#include <iomanip>

// Simple thread-safe logger with timestamp
class Logger {
private:
    static std::mutex log_mutex; // Mutex for thread safety

    // Get current time as string
    static std::string timestamp() {
        using clock = std::chrono::system_clock;
        auto now = clock::now();
        auto in_time_t = clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::ostringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

public:
    // Log any number of arguments with timestamp
    template<typename... Args>
    static void log(Args&&... args) {
        std::scoped_lock lock(log_mutex);
        std::cout << "[" << timestamp() << "] ";
        (std::cout << ... << args) << '\n';
    }
};
