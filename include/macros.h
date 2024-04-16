#pragma once
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

// this standardized in cpp20
// #define LIKELY(x)   __builtin_expect(!!(x), 1)
// #define UNLIKELY(x) __builtin_expect(!!(x), 0)

inline auto ASSERT(bool cond, const std::string &msg) noexcept {
    if (!cond) [[unlikely]] {
        std::cerr << "ASSERT: " << msg << '\n';
        exit(EXIT_FAILURE);
    }
}

inline auto FATAL(const std::string &msg) noexcept {
    std::cerr << "FATAL:" << msg << '\n';
    exit(EXIT_FAILURE);
}