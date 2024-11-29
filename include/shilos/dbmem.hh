
#pragma once

#include "./region.hh"

namespace shilos {

struct dbm_header {
  std::uint16_t magic;
  struct {
    std::int16_t major;
    std::int16_t minor;
  } version;
  std::int16_t flags;
  regional_ptr<std::byte> root; // subject to reinterpretation after sufficient type checks
};

class dbmem : public memory_region {
public:
  dbm_header *header() { return nullptr; }

  // allocate a memory block within this region's interesting address range
  void *allocate(size_t bytes, size_t align = 128);

  // all assignment (including clearing with nullptr) to (interned) ptrs inside this region
  // should go through this method, to facilitate correct reference counting
  template <typename T> T &assign_ptr(global_ptr<T> *holder, const T *addr) {
#ifndef NDEBUG
    {
      const auto holder_ptr = reinterpret_cast<intptr_t>(holder);
      if (this->capacity() <= 0)
        throw std::logic_error("!?new object in empty region?!");
      const auto base_ptr = reinterpret_cast<intptr_t>(this->header());
      const auto end_ptr = base_ptr + this->capacity();
      if (holder_ptr < base_ptr || holder_ptr >= end_ptr)
        throw std::out_of_range("!?holder out of region interests?!");
    }
#endif
    const T *curr = holder->get();
    if (curr != addr) {
      if (addr)
        increase_ref(reinterpret_cast<intptr_t>(addr));
      *holder = addr;
      if (curr)
        decrease_ref(reinterpret_cast<intptr_t>(curr));
    }
    return *holder;
  }

  template <typename T, typename... Args> T &new_held(global_ptr<T> *holder, Args &&...args) {
    T *addr = static_cast<T *>(this->allocate(sizeof(T), alignof(T)));
    new (addr) T(std::forward<Args>(args)...);
    assign_ptr(holder, addr);
    return *holder;
  }

  virtual ~dbmem() = default;

protected:
  // region-wide reference counting callbacks
  virtual void increase_ref(intptr_t ptr) {}
  virtual void decrease_ref(intptr_t ptr) {}
};

} // namespace shilos
