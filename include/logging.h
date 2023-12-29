#pragma once

#include "lf_queue.h"
#include "macros.h"
#include "thread_utils.h"
#include <atomic>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <thread>
#include <utility>
namespace Common {
constexpr std::size_t LOG_QUEUE_SIZE = 8 * 1024 * 1024;
enum class LogType : int8_t {

    CHAR = 0,
    INTEGER = 1,
    LONG_INTEGER = 2,
    LONG_LONG_INTEGER = 3,
    UNSIGNED_INTEGER = 4,
    UNSIGNED_LONG_INTEGER = 5,
    UNSIGNED_LONG_LONG_INTEGER = 6,
    FLOAT = 7,
    DOUBLE = 8,
};

struct LogElement {
    LogType m_type = LogType::CHAR;
    union {
        char c;
        int i;
        long l;
        long long ll;
        unsigned u;
        unsigned long ul;
        unsigned long long ull;
        float f;
        double d;

    } m_union;
};
class Logger final {
  public:
    auto flushQueue() noexcept {
        while (m_running) {
            for (const auto *next = m_queue.getNextToread();
                 m_queue.size() && next; next = m_queue.getNextToread()) {
                switch (next->m_type) {
                    case LogType::CHAR:
                        m_file << next->m_union.c;
                        break;
                    case LogType::INTEGER:
                        m_file << next->m_union.i;
                        break;
                    case LogType::LONG_INTEGER:
                        m_file << next->m_union.l;
                        break;
                    case LogType::LONG_LONG_INTEGER:
                        m_file << next->m_union.ll;
                        break;
                    case LogType::UNSIGNED_INTEGER:
                        m_file << next->m_union.u;
                        break;
                    case LogType::UNSIGNED_LONG_INTEGER:
                        m_file << next->m_union.ul;
                        break;
                    case LogType::UNSIGNED_LONG_LONG_INTEGER:
                        m_file << next->m_union.ull;
                        break;
                    case LogType::FLOAT:
                        m_file << next->m_union.f;
                        break;
                    case LogType::DOUBLE:
                        m_file << next->m_union.d;
                        break;
                }
                m_queue.updateReadIndex();
                next = m_queue.getNextToread();
            }
            using namespace std::literals::chrono_literals;
            std::this_thread::sleep_for(1ms);
        }
    }
    explicit Logger(std::string file_name)
        : m_file_name(std::move(file_name)), m_queue(LOG_QUEUE_SIZE) {
        m_file.open(file_name);
        ASSERT(m_file.is_open(), "Could not open log file" + file_name);
        m_logger_thread = creatAndStartThread(
            -1, "Common/Logger", [this]() { flushQueue(); });
        ASSERT(m_logger_thread != nullptr, "Failed to start Logger thread.");
    }
    ~Logger() {
        std::cerr << "Flushing and closing logger for " << m_file_name
                  << std::endl;
        while (m_queue.size()) {
            using namespace std::literals::chrono_literals;
            std::this_thread::sleep_for(1s);
        }
        m_running = false;
        m_logger_thread->join();
        m_file.close();
    }
    auto pushValue(const LogElement &log_elem) noexcept {
        *(m_queue.getNextToWriteTo()) = log_elem;
        m_queue.updateWriteIndex();
    }
    auto pushValue(const char val) noexcept {
        pushValue(LogElement{LogType::CHAR, {.c = val}});
    }

    auto pushValue(const int val) noexcept {
        pushValue(LogElement{LogType::INTEGER, {.i = val}});
    }
    auto pushValue(const long val) noexcept {
        pushValue(LogElement{LogType::LONG_INTEGER, {.l = val}});
    }
    auto pushValue(const long long val) noexcept {
        pushValue(LogElement{LogType::LONG_LONG_INTEGER, {.ll = val}});
    }
    auto pushValue(const unsigned int val) noexcept {
        pushValue(LogElement{LogType::UNSIGNED_INTEGER, {.u = val}});
    }
    auto pushValue(const unsigned long val) noexcept {
        pushValue(LogElement{LogType::UNSIGNED_LONG_INTEGER, {.ul = val}});
    }
    auto pushValue(const unsigned long long val) noexcept {
        pushValue(
            LogElement{LogType::UNSIGNED_LONG_LONG_INTEGER, {.ull = val}});
    }
    auto pushValue(const float val) noexcept {
        pushValue(LogElement{LogType::FLOAT, {.f = val}});
    }
    auto pushValue(const double val) noexcept {
        pushValue(LogElement{LogType::DOUBLE, {.d = val}});
    }
    auto pushValue(const char *val) noexcept {
        while (*val) {
            pushValue(*val);
            ++val;
        }
    }
    auto pushValue(const std::string &val) noexcept { pushValue(val.c_str()); }

    template<typename T, typename... Args>
    auto log(const char *str, const T &val, Args... args) noexcept {
        while (*str) {
            if (*str == '%') {
                if (UNLIKELY(*(str + 1) == '%')) {
                    ++str;
                } else {
                    pushValue(val);
                    log(str + 1, args...);
                    return;
                }
            }
            pushValue(*str++);
        }
        FATAL("extra  args privoided to log");
    }
    auto log(const char *str) noexcept {
        while (*str) {
            if (*str == '%') {
                if (UNLIKELY(*(str + 1) == '%')) {
                    ++str;
                } else {
                    FATAL("Missing args to log()");
                }
            }
            pushValue(*str++);
        }
    }
    
    Logger() = delete;
    Logger(const Logger &) = delete;
    Logger(const Logger &&) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger &operator=(const Logger &&) = delete;

  private:
    const std::string m_file_name;
    std::ofstream m_file;
    LFQueue<LogElement> m_queue;
    std::atomic<bool> m_running = {true};
    std::thread *m_logger_thread = {nullptr};
};
} // namespace Common