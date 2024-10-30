
#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace shilos {

using std::intptr_t;
using std::ptrdiff_t;

template <typename T> class relativ_ptr {
public:
  typedef T element_type;

private:
  ptrdiff_t distance_;

  static ptrdiff_t relativ_distance(const relativ_ptr<T> *rp, const T *tgt) noexcept {
    if (!tgt)
      return 0;
    return reinterpret_cast<intptr_t>(tgt) - reinterpret_cast<intptr_t>(rp);
  }

public:
  relativ_ptr() noexcept : distance_(0) {}

  relativ_ptr(const T *ptr) noexcept : distance_(relativ_distance(this, ptr)) {}

  relativ_ptr(const relativ_ptr &other) noexcept : distance_(relativ_distance(this, other.get())) {}

  relativ_ptr(relativ_ptr &&other) = delete;

  relativ_ptr &operator=(const T *other) noexcept {
    if (this->get() != other) {
      distance_ = relativ_distance(this, other);
    }
    return *this;
  }

  relativ_ptr &operator=(const relativ_ptr &other) noexcept {
    if (this != &other) {
      distance_ = relativ_distance(this, other.get());
    }
    return *this;
  }

  relativ_ptr &operator=(relativ_ptr &&other) = delete;

  ~relativ_ptr() = default;

  relativ_ptr &operator=(T *ptr) & noexcept {
    distance_ = relativ_distance(this, ptr);
    return *this;
  }

  // only works for lvalues
  T *get() & noexcept {
    if (distance_ == 0)
      return nullptr;
    return reinterpret_cast<T *>(reinterpret_cast<intptr_t>(this) + distance_);
  }

  // only works for lvalues
  const T *get() const & noexcept {
    if (distance_ == 0)
      return nullptr;
    return reinterpret_cast<const T *>(reinterpret_cast<intptr_t>(this) + distance_);
  }

  // welcome dereference from an lvalue of relative ptrs
  T &operator*() & noexcept { return *get(); }
  T *operator->() & noexcept { return get(); }
  const T &operator*() const & noexcept { return *get(); }
  const T *operator->() const & noexcept { return get(); }

  // forbid dereference from an rvalue of relative ptrs
  T &operator*() && = delete;
  T *operator->() && = delete;
  const T &operator*() const && = delete;
  const T *operator->() const && = delete;

  explicit operator bool() const noexcept { return get() != nullptr; }

  bool operator==(const relativ_ptr &other) const noexcept { return get() == other.get(); }

  bool operator!=(const relativ_ptr &other) const noexcept { return !(*this == other); }
};

template <typename RT> class stake_header {
  std::uint16_t magic;
  struct {
    std::int16_t major;
    std::int16_t minor;
  } version;
  std::int16_t flags;
  relativ_ptr<RT> root;
};

template <typename RT> class memory_stake {
public:
  virtual ~memory_stake(){};

  virtual stake_header<RT> *header() = 0;
  virtual ptrdiff_t capacity() { return 0; }

  // allocate a memory block within this stake's interesting address range
  virtual void *allocate(size_t bytes, size_t align = 128) = 0;

  // all assignment (including clearing with nullptr) to (relative) ptrs inside this stake
  // should go through this method, to facilitate correct reference counting
  template <typename T> T &assign_ptr(relativ_ptr<T> *holder, const T *addr) {
#ifndef NDEBUG
    {
      const auto holder_ptr = reinterpret_cast<intptr_t>(holder);
      if (this->capacity() <= 0)
        throw std::logic_error("!?new object in empty stake?!");
      const auto base_ptr = reinterpret_cast<intptr_t>(this->header());
      const auto end_ptr = base_ptr + this->capacity();
      if (holder_ptr < base_ptr || holder_ptr >= end_ptr)
        throw std::out_of_range("!?holder out of stake interests?!");
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

  template <typename T, typename... Args> T &new_held(relativ_ptr<T> *holder, Args &&...args) {
    T *addr = static_cast<T *>(this->allocate(sizeof(T), alignof(T)));
    new (addr) T(std::forward<Args>(args)...);
    assign_ptr(holder, addr);
    return *holder;
  }

protected:
  // stake-wide reference counting callbacks
  virtual void increase_ref(intptr_t ptr) {}
  virtual void decrease_ref(intptr_t ptr) {}
};

} // namespace shilos
