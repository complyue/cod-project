
#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>

namespace shilos {

using std::intptr_t;
using std::size_t;

template <typename T> class global_ptr;

class memory_region {
  template <typename T> friend class global_ptr;

public:
  static memory_region *alloc_region(const size_t payload_capacity,
                                     std::allocator<std::byte> allocator = std::allocator<std::byte>()) {
    const size_t capacity = sizeof(memory_region) + payload_capacity;
    void *ptr = allocator.allocate(capacity);
    new (ptr) memory_region(capacity);
    return reinterpret_cast<memory_region *>(ptr);
  }

protected:
  size_t capacity_;
  size_t root_offset_;
  size_t occupation_;

  memory_region(size_t capacity, size_t root_offset = 0, size_t occupation = sizeof(memory_region))
      : capacity_(capacity), root_offset_(root_offset), occupation_(occupation) {}

public:
  ~memory_region() = default;
  memory_region(const memory_region &) = delete;            // no copying
  memory_region(memory_region &&) = delete;                 // no moving
  memory_region &operator=(const memory_region &) = delete; // no copying by assignment
  memory_region &operator=(memory_region &&) = delete;      // no moving by assignment

  size_t free_capacity() { return capacity_ - occupation_; }

  void *allocate(const size_t size, const size_t align) {
    // use current occupation mark as the allocated ptr, do proper alignment
    size_t free_spc = free_capacity();
    void *ptr = reinterpret_cast<void *>(reinterpret_cast<intptr_t>(this) + occupation_);
    if (!std::align(align, size, ptr, free_spc)) {
      throw std::bad_alloc();
    }
    // move the occupation mark
    occupation_ = reinterpret_cast<intptr_t>(ptr) + size - reinterpret_cast<intptr_t>(this);
    return ptr;
  }

  template <typename T, typename... Args> global_ptr<T> &&create(Args &&...args) {
    void *ptr = this->allocate(sizeof(T), alignof(T));
    if (!ptr)
      throw std::bad_alloc();
    new (ptr) T(std::forward<Args>(args)...);
    return std::move(global_ptr<T>(this, reinterpret_cast<intptr_t>(ptr) - reinterpret_cast<intptr_t>(this)));
  }
};

template <typename T> class regional_ptr final {
  template <typename O> friend class global_ptr;

public:
  typedef T target_type;

private:
  size_t offset_;

  regional_ptr(size_t offset) : offset_(offset) {}

public:
  ~regional_ptr() = default;
  regional_ptr(const regional_ptr<T> &) = delete;
  regional_ptr(regional_ptr<T> &&) = delete;
  regional_ptr &operator=(const regional_ptr<T> &) = delete;
  regional_ptr &operator=(regional_ptr<T> &&) = delete;

  explicit operator bool() const noexcept { return offset_ != 0; }
};

template <typename T> class global_ptr final {
public:
  typedef T target_type;

private:
  memory_region *region_;
  size_t offset_;

  global_ptr(memory_region *region, size_t offset) : region_(region), offset_(offset) {}

public:
  global_ptr(memory_region *region) noexcept : region_(region), offset_(region->root_offset_) {}

  ~global_ptr() = default;
  global_ptr(const global_ptr<T> &) = default;
  global_ptr(global_ptr<T> &&) = default;
  global_ptr &operator=(const global_ptr<T> &) = default;
  global_ptr &operator=(global_ptr<T> &&) = default;

  template <typename F> //
  const global_ptr<F> &set(regional_ptr<F> T::*ptrField, const global_ptr<F> &tgt) {
    if (tgt.region_ != region_) {
      throw std::logic_error("!?cross region ptr assignment?!");
    }
    this->*ptrField.offset_ = tgt.offset_;
    return tgt;
  }

  template <typename F> //
  global_ptr<F> &&get(regional_ptr<F> T::*ptrField) {
    return std::move(global_ptr<F>(region_, this->*ptrField.offset_));
  }

  T *get() {
    if (offset_ == 0)
      return nullptr;
    return reinterpret_cast<T *>(reinterpret_cast<intptr_t>(region_) + offset_);
  }

  const T *get() const {
    if (offset_ == 0)
      return nullptr;
    return reinterpret_cast<const T *>(reinterpret_cast<intptr_t>(region_) + offset_);
  }

  T &operator*() { return *get(); }
  T *operator->() { return get(); }
  const T &operator*() const { return *get(); }
  const T *operator->() const { return get(); }

  explicit operator bool() const noexcept { return offset_ != 0; }

  bool operator==(const global_ptr<T> &other) const { return other.region_ == region_ && other.offset_ == offset_; }

  bool operator!=(const global_ptr<T> &other) const { return other.region_ != region_ || other.offset_ != offset_; }
};

} // namespace shilos
