#pragma once
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <utility>

namespace logs {
template <typename... Args>
inline void print_log(const std::string &level, Args &&...args) {
    auto now = std::chrono::system_clock::now();
    auto tm  = std::chrono::system_clock::to_time_t(now);
    auto ltm = *std::localtime(&tm);
    std::cerr << "[" << std::put_time(&ltm, "%Y-%m-%d %H:%M:%S") << "] [" << level << "] ";
    using expander = int[];
    (void)expander{0, (std::cerr << std::forward<Args>(args), 0)...};
    std::cerr << '\n';
}

template <typename... Args>
inline void debug([[maybe_unused]] Args &&...args) {
#if defined(DEBUG) && DEBUG
    print_log("DEBUG", std::forward<Args>(args)...);
#endif
}

template <typename... Args>
inline void info(Args &&...args) {
    print_log("INFO", std::forward<Args>(args)...);
}

template <typename... Args>
inline void warning(Args &&...args) {
    print_log("WARNING", std::forward<Args>(args)...);
}

template <typename... Args>
inline void error(Args &&...args) {
    print_log("ERROR", std::forward<Args>(args)...);
}
} // namespace logs
