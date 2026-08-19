#ifndef _ALLOCATORS_H
#define _ALLOCATORS_H

#include "pch.h"

class FrameArena {
private:
  uint8_t *m_buffer = nullptr;
  size_t m_capacity = 0;
  size_t m_offset = 0;

public:
  explicit FrameArena(size_t bytes) : m_capacity(bytes) {
    m_buffer = new uint8_t[bytes];
  }

  ~FrameArena() { delete[] m_buffer; }

  // Disable copies
  FrameArena(const FrameArena &) = delete;
  FrameArena &operator=(const FrameArena &) = delete;

  template <typename T, typename... Args>
  T *New(Args &&...args) {
    // Enforce natural alignment for the type
    size_t alignment = alignof(T);
    auto currentAddr = reinterpret_cast<size_t>(m_buffer + m_offset);
    size_t padding = (alignment - (currentAddr % alignment)) % alignment;

    if (m_offset + padding + sizeof(T) > m_capacity) {
      throw std::bad_alloc(); // Arena exhausted
    }

    m_offset += padding;
    T *ptr = reinterpret_cast<T *>(m_buffer + m_offset);
    m_offset += sizeof(T);

    // Construct object in-place using Placement New
    return ::new (static_cast<void *>(ptr)) T(std::forward<Args>(args)...);
  }

  // Resetting the entire arena is O(1)—just move the offset back to zero!
  void Reset() noexcept { m_offset = 0; }
};

#endif
