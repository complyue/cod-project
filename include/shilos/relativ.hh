
#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>

namespace shilos {

using std::intptr_t;

template <typename T> class relativ_ptr {
private:
  intptr_t distance;

  static intptr_t compute_distance(const relativ_ptr<T> *base,
                                   const T *target) noexcept {
    if (!target)
      return 0;
    return reinterpret_cast<intptr_t>(target) -
           reinterpret_cast<intptr_t>(base);
  }

public:
  constexpr relativ_ptr() noexcept : distance(0) {}

  constexpr relativ_ptr(T *ptr) noexcept
      : distance(compute_distance(this, ptr)) {}

  constexpr relativ_ptr(const relativ_ptr &other) noexcept
      : distance(compute_distance(this, other.get())) {}

  constexpr relativ_ptr(relativ_ptr &&other) = delete;

  relativ_ptr &operator=(const relativ_ptr &other) noexcept {
    if (this != &other) {
      distance = compute_distance(this, other.get());
    }
    return *this;
  }

  relativ_ptr &operator=(relativ_ptr &&other) = delete;

  ~relativ_ptr() = default;

  relativ_ptr &operator=(T *ptr) noexcept {
    distance = compute_distance(this, ptr);
    return *this;
  }

  T *get() const noexcept {
    if (distance == 0)
      return nullptr;
    return reinterpret_cast<T *>(reinterpret_cast<const intptr_t>(this) +
                                 distance);
  }

  // welcome dereference from an lvalue of relative ptrs
  T &operator*() & noexcept { return *get(); }
  T *operator->() & noexcept { return get(); }

  // no dereference from an rvalue of relative ptrs
  T &operator*() && = delete;
  T *operator->() && = delete;

  explicit operator bool() const noexcept { return get() != nullptr; }

  bool operator==(const relativ_ptr &other) const noexcept {
    return get() == other.get();
  }

  bool operator!=(const relativ_ptr &other) const noexcept {
    return !(*this == other);
  }
};

} // namespace shilos
