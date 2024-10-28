
#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>

namespace shilos {

using std::intptr_t;

template <typename _Tp> class relativ_ptr {
public:
  typedef _Tp element_type;

private:
  intptr_t distance;

  static intptr_t relativ_distance(const relativ_ptr<_Tp> *base,
                                   const _Tp *target) noexcept {
    if (!target)
      return 0;
    return reinterpret_cast<intptr_t>(target) -
           reinterpret_cast<intptr_t>(base);
  }

public:
  relativ_ptr() noexcept : distance(0) {}

  relativ_ptr(const _Tp *ptr) noexcept
      : distance(relativ_distance(this, ptr)) {}

  relativ_ptr(const relativ_ptr &other) noexcept
      : distance(relativ_distance(this, other.get())) {}

  relativ_ptr(relativ_ptr &&other) = delete;

  relativ_ptr &operator=(const _Tp *other) noexcept {
    if (this->get() != other) {
      distance = relativ_distance(this, other);
    }
    return *this;
  }

  relativ_ptr &operator=(const relativ_ptr &other) noexcept {
    if (this != &other) {
      distance = relativ_distance(this, other.get());
    }
    return *this;
  }

  relativ_ptr &operator=(relativ_ptr &&other) = delete;

  ~relativ_ptr() = default;

  relativ_ptr &operator=(_Tp *ptr) & noexcept {
    distance = relativ_distance(this, ptr);
    return *this;
  }

  // only works for lvalues
  _Tp *get() & noexcept {
    if (distance == 0)
      return nullptr;
    return reinterpret_cast<_Tp *>(reinterpret_cast<const intptr_t>(this) +
                                   distance);
  }

  // only works for lvalues
  const _Tp *get() const & noexcept {
    if (distance == 0)
      return nullptr;
    return reinterpret_cast<const _Tp *>(
        reinterpret_cast<const intptr_t>(this) + distance);
  }

  // welcome dereference from an lvalue of relative ptrs
  _Tp &operator*() & noexcept { return *get(); }
  _Tp *operator->() & noexcept { return get(); }
  const _Tp &operator*() const & noexcept { return *get(); }
  const _Tp *operator->() const & noexcept { return get(); }

  // forbid dereference from an rvalue of relative ptrs
  _Tp &operator*() && = delete;
  _Tp *operator->() && = delete;
  const _Tp &operator*() const && = delete;
  const _Tp *operator->() const && = delete;

  explicit operator bool() const noexcept { return get() != nullptr; }

  bool operator==(const relativ_ptr &other) const noexcept {
    return get() == other.get();
  }

  bool operator!=(const relativ_ptr &other) const noexcept {
    return !(*this == other);
  }
};

} // namespace shilos
