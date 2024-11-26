
#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <utility>

namespace shilos {

using std::intptr_t;
using std::size_t;

template <typename T> class intern_ptr;

class memory_stake {
  template <typename T> friend class intern_ptr;

protected:
  size_t capacity_;
  size_t root_offset_;
  size_t occupation_;

public:
  memory_stake(size_t capacity, size_t root_offset = 0, size_t occupation = sizeof(memory_stake))
      : capacity_(capacity), root_offset_(root_offset), occupation_(occupation) {}

  ~memory_stake() = default;
  memory_stake(const memory_stake &) = delete;            // no copying
  memory_stake(memory_stake &&) = delete;                 // no moving
  memory_stake &operator=(const memory_stake &) = delete; // no copying by assignment
  memory_stake &operator=(memory_stake &&) = delete;      // no moving by assignment

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

  template <typename T, typename... Args> intern_ptr<T> &&create(Args &&...args) {
    void *ptr = this->allocate(sizeof(T), alignof(T));
    if (!ptr)
      throw std::bad_alloc();
    new (ptr) T(std::forward<Args>(args)...);
    return std::move(intern_ptr<T>(this), reinterpret_cast<intptr_t>(ptr) - reinterpret_cast<intptr_t>(this));
  }
};

template <typename T> class domestic_ptr final {
  template <typename O> friend class intern_ptr;

public:
  typedef T target_type;

private:
  size_t offset_;

  domestic_ptr(size_t offset) : offset_(offset) {}

public:
  ~domestic_ptr() = default;
  domestic_ptr(const domestic_ptr<T> &) = delete;
  domestic_ptr(domestic_ptr<T> &&) = delete;
  domestic_ptr &operator=(const domestic_ptr<T> &) = delete;
  domestic_ptr &operator=(domestic_ptr<T> &&) = delete;

  explicit operator bool() const noexcept { return offset_ != 0; }
};

template <typename T> class intern_ptr final {
public:
  typedef T target_type;

private:
  memory_stake *stake_;
  size_t offset_;

  intern_ptr(memory_stake *stake, size_t offset) : stake_(stake), offset_(offset) {}

public:
  intern_ptr(memory_stake *stake) noexcept : stake_(stake), offset_(stake->root_offset_) {}

  ~intern_ptr() = default;
  intern_ptr(const intern_ptr<T> &) = default;
  intern_ptr(intern_ptr<T> &&) = default;
  intern_ptr &operator=(const intern_ptr<T> &) = default;
  intern_ptr &operator=(intern_ptr<T> &&) = default;

  template <typename F> //
  intern_ptr<F> &&get(domestic_ptr<F> T::*dpField) {
    return std::move(intern_ptr<F>(stake_, this->*dpField.offset_));
  }

  T *get() {
    if (offset_ == 0)
      return nullptr;
    return reinterpret_cast<T *>(reinterpret_cast<intptr_t>(stake_) + offset_);
  }

  const T *get() const {
    if (offset_ == 0)
      return nullptr;
    return reinterpret_cast<const T *>(reinterpret_cast<intptr_t>(stake_) + offset_);
  }

  T &operator*() { return *get(); }
  T *operator->() { return get(); }
  const T &operator*() const { return *get(); }
  const T *operator->() const { return get(); }

  explicit operator bool() const noexcept { return offset_ != 0; }

  bool operator==(const intern_ptr<T> &other) const { return other.stake_ == stake_ && other.offset_ == offset_; }

  bool operator!=(const intern_ptr<T> &other) const { return other.stake_ != stake_ || other.offset_ != offset_; }
};

} // namespace shilos
