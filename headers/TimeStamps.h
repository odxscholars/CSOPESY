#ifndef TIMESTAMPS_H
#define TIMESTAMPS_H

#include <string>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>

inline std::string formatTimestamp(const std::chrono::system_clock::time_point &time)
{
    std::time_t time_c = std::chrono::system_clock::to_time_t(time);
    std::tm ltm{};
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&ltm, &time_c); // Use localtime_s on Windows
#else
    ltm = *std::localtime(&time_c); // Standard approach on other systems
#endif

    std::ostringstream ss;
    ss << std::put_time(&ltm, "%m/%d/2024 %H:%M:%S");
    return ss.str();
}

#endif
