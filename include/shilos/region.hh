#pragma once

#include "./prelude.hh"

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace shilos {

using std::intptr_t;
using std::size_t;

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
      : offset_(!gp ? 0 : reinterpret_cast<intptr_t>(gp.get()) - reinterpret_cast<intptr_t>(this)) {
    // TODO: less severe checks here?
    assert(reinterpret_cast<intptr_t>(this) > reinterpret_cast<intptr_t>(&gp.region()) &&
           reinterpret_cast<intptr_t>(this) < reinterpret_cast<intptr_t>(&gp.region()) + gp.region().capacity());
  }

  template <typename RT> global_ptr<VT, RT> operator=(global_ptr<VT, RT> gp) {
    // TODO: less severe checks here?
    assert(reinterpret_cast<intptr_t>(this) > reinterpret_cast<intptr_t>(&gp.region()) &&
           reinterpret_cast<intptr_t>(this) < reinterpret_cast<intptr_t>(&gp.region()) + gp.region().capacity());
    if (!gp) {
      offset_ = 0;
    } else {
      offset_ = reinterpret_cast<intptr_t>(gp.get()) - reinterpret_cast<intptr_t>(this);
    }
    return gp;
  }

  //
  // support ctor & assignment from raw pointers, tho inherently unsafe
  //
  regional_ptr(VT *ptr) : offset_(!ptr ? 0 : reinterpret_cast<intptr_t>(ptr) - reinterpret_cast<intptr_t>(this)) {
    // TODO: this can be quite unsafe, log warning here?
  }
  VT *operator=(VT *ptr) {
    // TODO: this can be quite unsafe, log warning here?
    offset_ = !ptr ? 0 : reinterpret_cast<intptr_t>(ptr) - reinterpret_cast<intptr_t>(this);
    return ptr;
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

template <typename RT>
  requires ValidMemRegionRootType<RT>
class DBMR;

template <typename RT>
  requires ValidMemRegionRootType<RT>
class memory_region {
  template <typename VT, typename RT1> friend class global_ptr;
  friend class DBMR<RT>;

public:
  // General alloc_region for root types with additional constructor arguments
  template <typename... Args>
    requires std::constructible_from<RT, memory_region<RT> &, Args...>
  static memory_region<RT> *alloc_region(const size_t payload_capacity, Args &&...args) {
    return alloc_region_from(std::allocator<std::byte>(), payload_capacity, std::forward<Args>(args)...);
  }
  template <typename... Args>
    requires std::constructible_from<RT, memory_region<RT> &, Args...>
  static memory_region<RT> *alloc_region_from(std::allocator<std::byte> allocator, const size_t payload_capacity,
                                              Args &&...args) {
    const size_t capacity = sizeof(memory_region) + payload_capacity;
    void *ptr = allocator.allocate(capacity);
    return new (ptr) memory_region<RT>(capacity, std::forward<Args>(args)...);
  }

  // Simplified alloc_region for root types that only need memory_region& parameter
  static memory_region<RT> *alloc_region(const size_t payload_capacity)
    requires std::constructible_from<RT, memory_region<RT> &>
  {
    return alloc_region_from(std::allocator<std::byte>(), payload_capacity);
  }
  static memory_region<RT> *alloc_region_from(std::allocator<std::byte> allocator, const size_t payload_capacity)
    requires std::constructible_from<RT, memory_region<RT> &>
  {
    const size_t capacity = sizeof(memory_region) + payload_capacity;
    void *ptr = allocator.allocate(capacity);
    return new (ptr) memory_region<RT>(capacity);
  }

protected:
  UUID rt_uuid_;
  size_t capacity_;
  size_t occupation_;
  size_t ro_offset_;

  template <typename... Args>
    requires std::constructible_from<RT, memory_region<RT> &, Args...>
  memory_region(size_t capacity, Args &&...args)
      : rt_uuid_(RT::TYPE_UUID), capacity_(capacity), occupation_(sizeof(memory_region<RT>)) {
    RT *ptr = allocate<RT>();
    std::construct_at(ptr, *this, std::forward<Args>(args)...);
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

  template <typename VT> global_ptr<VT, RT> cast_ptr(const regional_ptr<VT> &rp) { return cast_ptr(rp.get()); }
  template <typename VT> global_ptr<VT, RT> cast_ptr(const VT *ptr) {
    assert(reinterpret_cast<intptr_t>(ptr) > reinterpret_cast<intptr_t>(this) &&
           reinterpret_cast<intptr_t>(ptr) < reinterpret_cast<intptr_t>(this) + capacity_);
    return global_ptr<VT, RT>(*this, reinterpret_cast<intptr_t>(ptr) - reinterpret_cast<intptr_t>(this));
  }

  template <typename VT> const global_ptr<VT, RT> cast_ptr(const regional_ptr<VT> &rp) const {
    return cast_ptr(rp.get());
  }
  template <typename VT> const global_ptr<VT, RT> cast_ptr(const VT *ptr) const {
    assert(reinterpret_cast<intptr_t>(ptr) > reinterpret_cast<intptr_t>(this) &&
           reinterpret_cast<intptr_t>(ptr) < reinterpret_cast<intptr_t>(this) + capacity_);
    // const_cast is safe here: global_ptr only stores a reference, doesn't modify the region
    return global_ptr<VT, RT>(const_cast<memory_region<RT> &>(*this),
                              reinterpret_cast<intptr_t>(ptr) - reinterpret_cast<intptr_t>(this));
  }

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

  template <typename VT, typename... Args>
    requires std::constructible_from<VT, Args...>
  void create_bits_to(regional_ptr<VT> &rp, Args &&...args) {
    assert(reinterpret_cast<intptr_t>(&rp) > reinterpret_cast<intptr_t>(this) &&
           reinterpret_cast<intptr_t>(&rp) < reinterpret_cast<intptr_t>(this) + capacity_);
    VT *ptr = this->allocate<VT>();
    std::construct_at(ptr, std::forward<Args>(args)...);
    rp.offset_ = reinterpret_cast<intptr_t>(ptr) - reinterpret_cast<intptr_t>(&rp);
  }
  template <typename VT, typename... Args>
    requires std::constructible_from<VT, memory_region<RT> &, Args...>
  void create_to(regional_ptr<VT> &rp, Args &&...args) {
    assert(reinterpret_cast<intptr_t>(&rp) > reinterpret_cast<intptr_t>(this) &&
           reinterpret_cast<intptr_t>(&rp) < reinterpret_cast<intptr_t>(this) + capacity_);
    VT *ptr = this->allocate<VT>();
    std::construct_at(ptr, *this, std::forward<Args>(args)...);
    rp.offset_ = reinterpret_cast<intptr_t>(ptr) - reinterpret_cast<intptr_t>(&rp);
  }

  template <typename VT, typename... Args>
    requires std::constructible_from<VT, Args...>
  global_ptr<VT, RT> create_bits(Args &&...args) {
    VT *ptr = this->allocate<VT>();
    std::construct_at(ptr, std::forward<Args>(args)...);
    return global_ptr<VT, RT>( //
        *this,                 //
        reinterpret_cast<intptr_t>(ptr) - reinterpret_cast<intptr_t>(this));
  }
  template <typename VT, typename... Args>
    requires std::constructible_from<VT, memory_region<RT> &, Args...>
  global_ptr<VT, RT> create(Args &&...args) {
    VT *ptr = this->allocate<VT>();
    std::construct_at(ptr, *this, std::forward<Args>(args)...);
    return global_ptr<VT, RT>( //
        *this,                 //
        reinterpret_cast<intptr_t>(ptr) - reinterpret_cast<intptr_t>(this));
  }

  global_ptr<RT, RT> root() { return global_ptr<RT, RT>(*this, ro_offset_); }
  const global_ptr<RT, RT> root() const {
    // const_cast is safe here: global_ptr only stores a reference, doesn't modify the region
    return global_ptr<RT, RT>(const_cast<memory_region<RT> &>(*this), ro_offset_);
  }

  template <typename VT> global_ptr<VT, RT> null() { return global_ptr<VT, RT>(*this, 0); }
  template <typename VT> const global_ptr<VT, RT> null() const {
    // const_cast is safe here: global_ptr only stores a reference, doesn't modify the region
    return global_ptr<VT, RT>(const_cast<memory_region<RT> &>(*this), 0);
  }
};

template <typename VT, typename RT> class global_ptr final {
  friend class memory_region<RT>;

public:
  typedef VT target_type;
  typedef RT root_type;

private:
  memory_region<RT> &region_;
  size_t offset_;

  global_ptr(memory_region<RT> &region, size_t offset) : region_(region), offset_(offset) {}

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
    if (&tgt.region_ != &region_) {
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
      return global_ptr<F, RT>(region_, reinterpret_cast<intptr_t>(vp) - reinterpret_cast<intptr_t>(&region_));
    }
  }

  template <typename F> //
  const global_ptr<F, RT> get(regional_ptr<F> VT::*ptrField) const {
    const regional_ptr<F> &fp = this->*ptrField;
    const VT *vp = fp.get();
    if (!vp) {
      // const_cast is safe here: global_ptr only stores a reference, doesn't modify the region
      return global_ptr<F, RT>(const_cast<memory_region<RT> &>(region_), 0);
    } else {
      // const_cast is safe here: global_ptr only stores a reference, doesn't modify the region
      return global_ptr<F, RT>(const_cast<memory_region<RT> &>(region_),
                               reinterpret_cast<intptr_t>(vp) - reinterpret_cast<intptr_t>(&region_));
    }
  }

  VT *get() {
    if (offset_ == 0)
      return nullptr;
    return reinterpret_cast<VT *>(reinterpret_cast<intptr_t>(&region_) + offset_);
  }

  const VT *get() const {
    if (offset_ == 0)
      return nullptr;
    return reinterpret_cast<const VT *>(reinterpret_cast<intptr_t>(&region_) + offset_);
  }

  VT &operator*() { return *get(); }
  VT *operator->() { return get(); }
  const VT &operator*() const { return *get(); }
  const VT *operator->() const { return get(); }

  explicit operator bool() const noexcept { return offset_ != 0; }

  // Access to the memory region
  memory_region<RT> &region() { return region_; }
  const memory_region<RT> &region() const { return region_; }

  auto operator<=>(const global_ptr<VT, RT> &other) const = default;
  bool operator==(const global_ptr<VT, RT> &other) const = default;

  // Comparison with nullptr
  bool operator==(std::nullptr_t) const noexcept { return offset_ == 0; }
  bool operator!=(std::nullptr_t) const noexcept { return offset_ != 0; }

  // Symmetric operators
  friend bool operator==(std::nullptr_t, const global_ptr<VT, RT> &ptr) noexcept { return ptr.offset_ == 0; }
  friend bool operator!=(std::nullptr_t, const global_ptr<VT, RT> &ptr) noexcept { return ptr.offset_ != 0; }
};

} // namespace shilos
