
#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>

#include "./uuid.hh"

namespace shilos {

using std::intptr_t;
using std::size_t;

template <typename VT, typename RT> class global_ptr;

//
// region-internal pointer fields should be declared as this type,
// for data structures meant to live in shilos
//
// users would always obtain the corresponding global_ptr<F> from a regional_ptr<F> field of a record object of type T,
// via a global_ptr<T> to the outer record object, and likely update a regional_ptr<F> field to point to other object
// via the global_ptr<T>
//
// actually, you can do virtually nothing with a regional_ptr<F> alone, and this is by design
//
template <typename VT> class regional_ptr final {
  template <typename OT, typename RT> friend class global_ptr;

public:
  typedef VT target_type;

private:
  size_t offset_;

  regional_ptr(size_t offset) : offset_(offset) {}

public:
  regional_ptr() : offset_(0) {}

  ~regional_ptr() = default;

  //
  // TODO: this overly restrictive?
  //
  // prohibit direct copying and assignment,
  // regional_ptr fields can only be updated via a global_ptr to its parent record:
  //
  regional_ptr(const regional_ptr<VT> &) = delete;
  regional_ptr(regional_ptr<VT> &&) = delete;
  regional_ptr &operator=(const regional_ptr<VT> &) = delete;
  regional_ptr &operator=(regional_ptr<VT> &&) = delete;

  explicit operator bool() const noexcept { return offset_ != 0; }
};

template <typename RT>
concept ValidMemRegionRootType = requires {
  { RT::TYPE_UUID } -> std::same_as<const UUID &>;
};

template <typename RT>
  requires ValidMemRegionRootType<RT>
class memory_region {
  template <typename VT, RT> friend class global_ptr;

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
    new (ptr) RT(std::forward<Args>(args)...);
    ro_offset_ = reinterpret_cast<intptr_t>(ptr) - reinterpret_cast<intptr_t>(this);
  }

public:
  ~memory_region() = default;
  // a region is a block of memory, meant to be referenced by ptr anyway, so no rvalue semantcis
  memory_region(const memory_region &) = delete;            // no copying
  memory_region(memory_region &&) = delete;                 // no moving
  memory_region &operator=(const memory_region &) = delete; // no copying by assignment
  memory_region &operator=(memory_region &&) = delete;      // no moving by assignment

  size_t capacity() { return capacity_; }
  size_t occupation() { return occupation_; }
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

  template <typename VT, typename... Args> global_ptr<VT, RT> create(Args &&...args) {
    void *ptr = this->allocate(sizeof(VT), alignof(VT));
    if (!ptr)
      throw std::bad_alloc();
    new (ptr) VT(std::forward<Args>(args)...);
    return global_ptr<VT, RT>( //
        this,                  //
        reinterpret_cast<intptr_t>(ptr) - reinterpret_cast<intptr_t>(this));
  }

  global_ptr<RT, RT> root() { //
    return std::move(global_ptr<RT, RT>(this, ro_offset_));
  }

  template <typename VT> //
  global_ptr<VT, RT> null() {
    return std::move(global_ptr<VT, RT>(this, 0));
  }
};

template <typename VT, typename RT> class global_ptr final {
public:
  typedef VT target_type;

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
    this->*ptrField.offset_ = tgt.offset_;
    return tgt;
  }

  template <typename F> //
  global_ptr<F, RT> get(regional_ptr<F> VT::*ptrField) {
    return global_ptr<F, RT>(region_, this->*ptrField.offset_);
  }

  template <typename F> //
  const global_ptr<F, RT> get(regional_ptr<F> VT::*ptrField) const {
    return global_ptr<F, RT>(region_, this->*ptrField.offset_);
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

  bool operator==(const global_ptr<VT, RT> &other) const {
    return other.region_ == region_ && other.offset_ == offset_;
  }

  bool operator!=(const global_ptr<VT, RT> &other) const {
    return other.region_ != region_ || other.offset_ != offset_;
  }
};

} // namespace shilos
