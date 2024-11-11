
#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace shilos {

using std::intptr_t;

struct memory_region {
  intptr_t baseaddr;
  intptr_t capacity;
  memory_region *prev;
};

class memory_stake {

private:
  // CAVEATS: this field must be updated only via assume_region()
  const memory_region *live_region_;

protected:
  void assume_region(const intptr_t baseaddr, const intptr_t capacity);

public:
  memory_stake() : live_region_(nullptr) {}

  ~memory_stake();

  const memory_region *live_region() { return live_region_; }
  inline intptr_t baseaddr() { return live_region_ ? live_region_->baseaddr : 0; }
  inline intptr_t capacity() { return live_region_ ? live_region_->capacity : 0; }
};

extern "C" {

//
const memory_region *_region_of(const void *ptr);

//
const memory_stake *_stake_of(const void *ptr);

//
inline intptr_t _stake_base_of(const void *const ptr) {
  const memory_region *region = _region_of(ptr);
  if (region)
    return region->baseaddr;
  return 0;
}

//
inline intptr_t _stake_offset_of(const void *const ptr) {
  const memory_region *region = _region_of(ptr);
  if (region)
    return reinterpret_cast<intptr_t>(ptr) - region->baseaddr;
  return reinterpret_cast<intptr_t>(ptr);
}

//
} // extern "C"

template <typename T> class relativ_ptr {
public:
  typedef T element_type;

private:
  intptr_t distance_;

  static intptr_t relativ_distance(const relativ_ptr<T> *rp, const T *tgt) noexcept {
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

} // namespace shilos
