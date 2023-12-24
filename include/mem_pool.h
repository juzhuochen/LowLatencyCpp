#pragma once
#include "macros.h"
#include <cstddef>
#include <cstring>
#include <string>
#include <vector>
namespace Common {

template <typename T> class MemPool final {
private:
  struct ObjectBlock {
    T m_object;
    bool m_is_free = true;
  };
  std::vector<ObjectBlock> m_store;
  size_t m_next_free_index = 0;

public:
  explicit MemPool(std::size_t num_elems) : m_store(num_elems, {T(), true}) {
    ASSERT(reinterpret_cast<const ObjectBlock *>(&(m_store[0].m_object)) ==
               (m_store).data(),
           "T object should be the first member of ObjectBlock.");
  }

  template <typename... Args> T *allocate(Args... args) noexcept {
    auto obj_block = &(m_store[m_next_free_index]);
    ASSERT(obj_block->m_is_free, "Expected free OnjectBlock at index: " +
                                    std::to_string(m_next_free_index));
    T *ret = &(obj_block->m_object);
    ret = new (ret) T(args...); // placement new to construct the type T obj
    obj_block->m_is_free = false;
    updataNextFreeIndex();
    return ret;
  }
  auto deallocate(const T *elem) noexcept {
    const auto elem_idx =
        (reinterpret_cast<const ObjectBlock *>(elem) - (m_store).data());
    ASSERT(elem_idx >= 0 && static_cast<size_t>(elem_idx) < m_store.size(),
           "Element being deallocted does not belong to this memory pool.");
    ASSERT(!m_store[elem_idx].m_is_free,
           "Expected in-use ObjectBlock at index: " + std::to_string(elem_idx));
    m_store[elem_idx].m_is_free = true;
  }
  MemPool() = delete;
  MemPool(const MemPool &) = delete;
  MemPool(const MemPool &&) = delete;
  MemPool &operator=(const MemPool &) = delete;
  MemPool &operator=(const MemPool &&) = delete;

private:
  auto updataNextFreeIndex() noexcept {
    const auto initial_free_index = m_next_free_index;
    while (!m_store[m_next_free_index].m_is_free) {
      ++m_next_free_index;
      if (UNLIKELY(m_next_free_index == m_store.size())) {
        m_next_free_index = 0; // make the pool circular
      }
      if (UNLIKELY(initial_free_index == m_next_free_index)) {
        ASSERT(initial_free_index != m_next_free_index,
               "Memory pool out of space.");
      }
    }
  }
};

} // namespace Common