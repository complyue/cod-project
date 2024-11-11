
#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
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

template <typename T> class intern_ptr {
public:
  typedef T element_type;

private:
  intptr_t offset_;

public:
  intern_ptr() noexcept : offset_(0) {}

  intern_ptr(T *ptr) : offset_(_stake_offset_of(static_cast<void *>(ptr))) {}

  intern_ptr(intern_ptr<T> &other) : offset_(other.offset_) { assert(_stake_base_of(this) == _stake_base_of(&other)); }

  intern_ptr(intern_ptr<T> &&other) = delete;

  intern_ptr &operator=(T *other) {
    assert(_stake_base_of(this) == _stake_base_of(other));
    offset_ = _stake_offset_of(other);
    return *this;
  }

  intern_ptr &operator=(intern_ptr<T> &other) {
    assert(_stake_base_of(this) == _stake_base_of(&other));
    offset_ = _stake_offset_of(&other);
    return *this;
  }

  intern_ptr &operator=(intern_ptr<T> &&other) = delete;

  ~intern_ptr() = default;

  // only works for lvalues
  T *get() & {
    if (offset_ == 0)
      return nullptr;
    return reinterpret_cast<T *>(reinterpret_cast<intptr_t>(this) + offset_);
  }

  // only works for lvalues
  const T *get() const & {
    if (offset_ == 0)
      return nullptr;
    return reinterpret_cast<const T *>(reinterpret_cast<intptr_t>(this) + offset_);
  }

  // welcome dereference from an lvalue of interned ptrs
  T &operator*() & { return *get(); }
  T *operator->() & { return get(); }
  const T &operator*() const & { return *get(); }
  const T *operator->() const & { return get(); }

  // forbid dereference from an rvalue of interned ptrs
  T &operator*() && = delete;
  T *operator->() && = delete;
  const T &operator*() const && = delete;
  const T *operator->() const && = delete;

  explicit operator bool() const noexcept { return offset_ != 0; }

  bool operator==(const intern_ptr<T> &other) const {
    return _stake_base_of(this) == _stake_base_of(&other) && offset_ == other.offset_;
  }

  bool operator!=(const intern_ptr<T> &other) const {
    return _stake_base_of(this) != _stake_base_of(&other) || offset_ != other.offset_;
  }
};

} // namespace shilos
