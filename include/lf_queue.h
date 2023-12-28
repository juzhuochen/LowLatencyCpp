// lock free queue
// SPSC type
#pragma once
#include "macros.h"
#include <atomic>
#include <cstddef>
#include <pthread.h>
#include <string>
#include <vector>

namespace Common {
template<typename T>
class LFQueue final {
  public:
    explicit LFQueue(std::size_t num_elem) : m_store(num_elem, T()) {}

    // write to queue
    auto getNextToWriteTo() noexcept { return &m_store[m_next_write_index]; }
    auto updateWriteIndex() noexcept {
        m_next_write_index = (m_next_write_index + 1) % m_store.size();
        m_num_elements++;
    }
    // read from queue
    auto getNextToread() const noexcept -> const T * {
        return (size() ?  &m_store[m_next_read_index]:nullptr);
    }
    auto updateReadIndex() {
        m_next_read_index = (m_next_read_index + 1) % m_store.size();
        ASSERT(
            m_num_elements != 0,
            "Read an invalid element in:" + std::to_string(pthread_self()));
        m_num_elements--;
    }
    auto size() const noexcept { return m_num_elements.load(); }

    LFQueue() = delete;
    LFQueue(const T &) = delete;
    LFQueue(const T &&) = delete;
    LFQueue &operator=(const LFQueue &) = delete;
    LFQueue &operator=(const LFQueue &&) = delete;

  private:
    std::vector<T> m_store;
    std::atomic<size_t> m_next_write_index = {0};
    std::atomic<size_t> m_next_read_index = {0};
    std::atomic<size_t> m_num_elements = {0};
};

} // namespace Common