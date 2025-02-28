#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>

#include "./uuid.hh"

namespace shilos {

using std::intptr_t;
using std::size_t;

template <typename RT>
concept ValidMemRegionRootType = requires {
  { RT::TYPE_UUID } -> std::same_as<const UUID &>;
};

template <typename VT, typename RT> class global_ptr;

//
// users should always update a regional_ptr<F> field of a record object of type T,
// via a global_ptr<T> to the outer record object
//
template <typename VT> class regional_ptr final {
  template <typename RT>
    requires ValidMemRegionRootType<RT>
  friend class memory_region;
  template <typename OT, typename RT> friend class global_ptr;

public:
  typedef VT target_type;

private:
  intptr_t offset_;

  regional_ptr(intptr_t offset) : offset_(offset) {}

public:
  regional_ptr() : offset_(0) {}

  template <typename RT>
  regional_ptr(global_ptr<VT, RT> gp)
      : offset_(reinterpret_cast<intptr_t>(gp.get()) - reinterpret_cast<intptr_t>(this)) {
#ifndef NDEBUG
    const intptr_t p_this = reinterpret_cast<intptr_t>(this);
    assert(p_this > reinterpret_cast<intptr_t>(gp.region()));
    assert(p_this < reinterpret_cast<intptr_t>(gp.region()) + gp.region()->capacity());
#endif
  }

  ~regional_ptr() = default;
  //
  // prohibit direct copying and assignment
  //
  // regional_ptr fields can only be updated via a global_ptr to its parent record
  //
  regional_ptr(const regional_ptr<VT> &) = delete;
  regional_ptr(regional_ptr<VT> &&) = delete;
  regional_ptr &operator=(const regional_ptr<VT> &) = delete;
  regional_ptr &operator=(regional_ptr<VT> &&) = delete;

  explicit operator bool() const noexcept { return offset_ != 0; }

  VT *get() {
    if (offset_ == 0)
      return nullptr;
    return reinterpret_cast<VT *>(reinterpret_cast<intptr_t>(this) + offset_);
  }

  const VT *get() const {
    if (offset_ == 0)
      return nullptr;
    return reinterpret_cast<const VT *>(reinterpret_cast<intptr_t>(this) + offset_);
  }

  VT &operator*() { return *get(); }
  VT *operator->() { return get(); }
  const VT &operator*() const { return *get(); }
  const VT *operator->() const { return get(); }

  std::strong_ordering operator<=>(const regional_ptr<VT> &other) const noexcept { return get() <=> other.get(); }
  bool operator==(const regional_ptr<VT> &other) const noexcept { return get() == other.get(); }
};

class regional_str final {
  template <typename RT>
    requires ValidMemRegionRootType<RT>
  friend class memory_region;

private:
  size_t length_;
  regional_ptr<std::byte> data_;

  regional_str(size_t length) : length_(length), data_() {}

public:
  size_t length() const { return length_; }
  std::byte *data() { return data_.get(); }
  const std::byte *data() const { return data_.get(); }

  ~regional_str() = default;
  regional_str(const regional_str &) = delete;
  regional_str(regional_str &&) = delete;
  regional_str &operator=(const regional_str &) = delete;
  regional_str &operator=(regional_str &&) = delete;

  operator std::string_view() const { return std::string_view(reinterpret_cast<const char *>(data()), length()); }

  std::strong_ordering operator<=>(const regional_str &other) const noexcept {
    if (auto cmp = length_ <=> other.length_; cmp != 0) {
      return cmp;
    }
    if (!data_ || !other.data_) {
      if (data_ == other.data_) {
        return std::strong_ordering::equal;
      }
      return !data_ ? std::strong_ordering::less : std::strong_ordering::greater;
    }
    const int result = std::memcmp(data_.get(), other.data_.get(), length_);
    if (result < 0)
      return std::strong_ordering::less;
    if (result > 0)
      return std::strong_ordering::greater;
    return std::strong_ordering::equal;
  }
  bool operator==(const regional_str &other) const noexcept {
    if (length_ != other.length_) {
      return false;
    }
    if (data_.get() == other.data_.get())
      return true;
    if (!data_ || !other.data_) {
      return false;
    }
    return 0 == std::memcmp(data_.get(), other.data_.get(), length_);
  }
};

inline std::ostream &operator<<(std::ostream &os, const regional_str &str) {
  os << static_cast<std::string_view>(str);
  return os;
}

template <typename RT>
  requires ValidMemRegionRootType<RT>
class DBMR;

template <typename RT>
  requires ValidMemRegionRootType<RT>
class memory_region {
  template <typename VT, typename RT1> friend class global_ptr;
  friend class DBMR<RT>;

public:
  template <typename... Args>
  static memory_region<RT> *alloc_region(const size_t payload_capacity, Args &&...args,
                                         std::allocator<std::byte> allocator = std::allocator<std::byte>()) {
    const size_t capacity = sizeof(memory_region) + payload_capacity;
    void *ptr = allocator.allocate(capacity);
    new (ptr) memory_region(capacity, std::forward<Args>(args)...);
    return reinterpret_cast<memory_region<RT> *>(ptr);
  }

protected:
  UUID rt_uuid_;
  size_t capacity_;
  size_t occupation_;
  size_t ro_offset_;

  template <typename... Args>
  memory_region(size_t capacity, Args &&...args)
      : rt_uuid_(RT::TYPE_UUID), capacity_(capacity), occupation_(sizeof(memory_region<RT>)) {
    void *ptr = allocate(sizeof(RT), alignof(RT));
    if (!ptr)
      throw std::bad_alloc();
    new (ptr) RT(this, std::forward<Args>(args)...);
    ro_offset_ = reinterpret_cast<intptr_t>(ptr) - reinterpret_cast<intptr_t>(this);
  }

public:
  ~memory_region() = default;
  // a region is a block of memory, meant to be referenced by ptr anyway, so no rvalue semantcis
  memory_region(const memory_region &) = delete;            // no copying
  memory_region(memory_region &&) = delete;                 // no moving
  memory_region &operator=(const memory_region &) = delete; // no copying by assignment
  memory_region &operator=(memory_region &&) = delete;      // no moving by assignment

  const UUID &root_type_uuid() const { return rt_uuid_; }
  size_t capacity() const { return capacity_; }
  size_t occupation() const { return occupation_; }
  size_t free_capacity() const { return capacity_ - occupation_; }

  template <typename T> T *allocate(const size_t n = 1) {
    return reinterpret_cast<T *>(allocate(n * sizeof(T), alignof(T)));
  }

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

  template <typename VT, typename... Args> global_ptr<VT, RT> create(Args &&...args) {
    void *ptr = this->allocate<VT>();
    if (!ptr)
      throw std::bad_alloc();
    new (ptr) VT(std::forward<Args>(args)...);
    return global_ptr<VT, RT>( //
        this,                  //
        reinterpret_cast<intptr_t>(ptr) - reinterpret_cast<intptr_t>(this));
  }

  global_ptr<regional_str, RT> afford(std::string_view &str) {
    return afford(str.size(), reinterpret_cast<const std::byte *>(str.data()));
  }

  global_ptr<regional_str, RT> afford(const size_t length, const std::byte *data) {
    std::byte *p_data = this->allocate<std::byte>(length);
    if (!p_data)
      throw std::bad_alloc();
    std::memcpy(p_data, data, length);
    regional_str *p_str = this->allocate<regional_str>();
    if (!p_str)
      throw std::bad_alloc();
    new (p_str) regional_str(length);
    p_str->data_.offset_ = reinterpret_cast<intptr_t>(p_data) - //
                           reinterpret_cast<intptr_t>(&(p_str->data_.offset_));
    return global_ptr<regional_str, RT>(this,
                                        reinterpret_cast<intptr_t>(p_str) - //
                                            reinterpret_cast<intptr_t>(this));
  }

  global_ptr<RT, RT> root() { return global_ptr<RT, RT>(this, ro_offset_); }
  const global_ptr<RT, RT> root() const {
    return global_ptr<RT, RT>(const_cast<memory_region<RT> *>(this), ro_offset_);
  }

  template <typename VT> global_ptr<VT, RT> null() { return global_ptr<VT, RT>(this, 0); }
  template <typename VT> const global_ptr<VT, RT> null() const {
    return global_ptr<VT, RT>(const_cast<memory_region<RT> *>(this), 0);
  }
};

template <typename VT, typename RT> class global_ptr final {
  friend class memory_region<RT>;

public:
  typedef VT target_type;
  typedef RT root_type;

private:
  memory_region<RT> *region_;
  size_t offset_;

  global_ptr(memory_region<RT> *region, size_t offset) : region_(region), offset_(offset) {}

public:
  ~global_ptr() = default;
  // global_ptr can be freely used as both lvalue and rvalue
  global_ptr(const global_ptr<VT, RT> &) = default;
  global_ptr(global_ptr<VT, RT> &&) = default;
  global_ptr &operator=(const global_ptr<VT, RT> &) = default;
  global_ptr &operator=(global_ptr<VT, RT> &&) = default;

  template <typename F> //
  void clear(regional_ptr<F> VT::*ptrField) {
    this->*ptrField.offset_ = 0;
  }

  template <typename F> //
  const global_ptr<F, RT> &set(regional_ptr<F> VT::*ptrField, const global_ptr<F, RT> &tgt) {
    if (tgt.region_ != region_) {
      throw std::logic_error("!?cross region ptr assignment?!");
    }
    regional_ptr<F> &fp = this->*ptrField;
    VT *vp = tgt.get();
    if (!vp) {
      fp.offset_ = 0;
    } else {
      fp.offset_ = reinterpret_cast<intptr_t>(vp) - reinterpret_cast<intptr_t>(&fp);
    }
    return tgt;
  }

  template <typename F> //
  global_ptr<F, RT> get(regional_ptr<F> VT::*ptrField) {
    regional_ptr<F> &fp = this->*ptrField;
    VT *vp = fp.get();
    if (!vp) {
      return global_ptr<F, RT>(region_, 0);
    } else {
      return global_ptr<F, RT>(region_, reinterpret_cast<intptr_t>(vp) - reinterpret_cast<intptr_t>(region_));
    }
  }

  template <typename F> //
  const global_ptr<F, RT> get(regional_ptr<F> VT::*ptrField) const {
    const regional_ptr<F> &fp = this->*ptrField;
    const VT *vp = fp.get();
    if (!vp) {
      return global_ptr<F, RT>(region_, 0);
    } else {
      return global_ptr<F, RT>(region_, reinterpret_cast<intptr_t>(vp) - reinterpret_cast<intptr_t>(region_));
    }
  }

  VT *get() {
    if (offset_ == 0)
      return nullptr;
    return reinterpret_cast<VT *>(reinterpret_cast<intptr_t>(region_) + offset_);
  }

  const VT *get() const {
    if (offset_ == 0)
      return nullptr;
    return reinterpret_cast<const VT *>(reinterpret_cast<intptr_t>(region_) + offset_);
  }

  VT &operator*() { return *get(); }
  VT *operator->() { return get(); }
  const VT &operator*() const { return *get(); }
  const VT *operator->() const { return get(); }

  explicit operator bool() const noexcept { return offset_ != 0; }

  auto operator<=>(const global_ptr<VT, RT> &other) const = default;
  bool operator==(const global_ptr<VT, RT> &other) const = default;
};

} // namespace shilos
