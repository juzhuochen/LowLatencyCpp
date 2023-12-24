// lock free queue
// SPSC type
#pragma once
#include "macros.h"
#include <atomic>

#include <cstddef>
#include <iostream>
#include <pthread.h>
#include <string>
#include <vector>

namespace Common {
template <typename T> class LFQueue final {
public:
  LFQueue(std::size_t num_elem) : num_elements_(num_elem) {
    store_(num_elem, T());
  }
  
  // write to queue
  auto getNextToWriteTo() noexcept { return &store_[next_write_index_]; }
  auto updateWriteIndex() noexcept {
    next_write_index_ = (next_write_index_ + 1) % store_.size();
    num_elements_++;
  }
  // read from queue
  auto getNextToread() const noexcept -> const T * {
    return (next_read_index_ == next_write_index_) ? nullptr
                                                   : &store_[next_read_index_];
  }
  auto updateReadIndex() {
    next_read_index_ = (next_read_index_ + 1) % store_.size();
    ASSERT(num_elements_ != 0,
           "Read an invalid element in:" + std::to_string(pthread_self()));
    num_elements_--;
  }
  auto size() const noexcept { return num_elements_.load(); }
  LFQueue() = delete;
  LFQueue(const T &) = delete;
  LFQueue(const T &&) = delete;
  LFQueue &operator=(const LFQueue &) = delete;
  LFQueue &operator=(const LFQueue &&) = delete;

private:
  std::vector<T> store_;
  std::atomic<size_t> next_write_index_ = {0};
  std::atomic<size_t> next_read_index_ = {0};
  std::atomic<size_t> num_elements_ = {0};
};

} // namespace Common