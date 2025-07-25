#pragma once

#include "./prelude.hh"

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace shilos {

using std::intptr_t;
using std::size_t;

// Concept to validate that a type is suitable for use as a regional type
template <typename T>
concept RegionalType = std::is_trivially_destructible_v<T> || std::is_destructible_v<T>;

// Concept to validate that a type can be constructed with memory_region& as first parameter
template <typename T, typename RT>
concept RegionalConstructibleWithRegion = requires {
  requires ValidMemRegionRootType<RT>;
  requires std::constructible_from<T, memory_region<RT> &>;
};

// Concept to validate that a type can be constructed without memory_region&
template <typename T>
concept RegionalConstructibleBits = std::constructible_from<T>;

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
  static memory_region<RT> *alloc_region(const size_t payload_capacity, Args &&...args) noexcept(false) {
    return alloc_region_with(std::allocator<std::byte>(), payload_capacity, std::forward<Args>(args)...);
  }
  template <typename... Args>
    requires std::constructible_from<RT, memory_region<RT> &, Args...>
  static memory_region<RT> *alloc_region_with(std::allocator<std::byte> allocator, const size_t payload_capacity,
                                              Args &&...args) noexcept(false) {
    const size_t capacity = sizeof(memory_region) + payload_capacity;
    void *ptr = allocator.allocate(capacity);
    return new (ptr) memory_region<RT>(capacity, std::forward<Args>(args)...);
  }

  // Simplified alloc_region for root types that only need memory_region& parameter
  static memory_region<RT> *alloc_region(const size_t payload_capacity)
    requires std::constructible_from<RT, memory_region<RT> &>
  {
    return alloc_region_with(std::allocator<std::byte>(), payload_capacity);
  }
  static memory_region<RT> *alloc_region_with(std::allocator<std::byte> allocator, const size_t payload_capacity)
    requires std::constructible_from<RT, memory_region<RT> &>
  {
    const size_t capacity = sizeof(memory_region) + payload_capacity;
    void *ptr = allocator.allocate(capacity);
    return new (ptr) memory_region<RT>(capacity);
  }

  // Manual deallocation methods
  static void free_region(memory_region<RT> *region) noexcept {
    if (!region)
      return;
    std::allocator<std::byte> allocator;
    free_region_with(allocator, region);
  }

  template <typename Allocator> static void free_region_with(Allocator allocator, memory_region<RT> *region) noexcept {
    if (!region)
      return;
    size_t capacity = region->capacity_;
    // Skip destructor call - memory_region doesn't need cleanup, just deallocate the memory
    allocator.deallocate(static_cast<std::byte *>(static_cast<void *>(region)), capacity);
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
  ~memory_region() {
    // This destructor should NEVER be called directly via delete!
    // Use auto_region<RT> for RAII or memory_region<RT>::free_region() for manual management.
    // Direct delete on allocator-allocated memory is undefined behavior.
    assert(false && "memory_region destructor called directly - use auto_region or free_region instead");
  }
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

  template <typename T> T *allocate(const size_t n = 1) noexcept(false) {
    static_assert(RegionalType<T>, "Type must be a valid regional type");
    return reinterpret_cast<T *>(allocate(n * sizeof(T), alignof(T)));
  }

  void *allocate(const size_t size, const size_t align) noexcept(false) {
    // use current occupation mark as the allocated ptr, do proper alignment
    size_t free_spc = free_capacity();
    void *ptr = reinterpret_cast<void *>(reinterpret_cast<intptr_t>(this) + occupation_);

    if (!std::align(align, size, ptr, free_spc)) {
      throw std::bad_alloc();
    }

    // Calculate the actual aligned address and update occupation
    // The new occupation is the aligned address + size, relative to region start
    occupation_ = reinterpret_cast<intptr_t>(ptr) + size - reinterpret_cast<intptr_t>(this);
    return ptr;
  }

  template <typename VT, typename... Args>
    requires RegionalConstructibleBits<VT> && std::constructible_from<VT, Args...>
  void create_bits_to(regional_ptr<VT> &rp, Args &&...args) noexcept(false) {
    static_assert(RegionalType<VT>, "Type must be a valid regional type");
    assert(reinterpret_cast<intptr_t>(&rp) > reinterpret_cast<intptr_t>(this) &&
           reinterpret_cast<intptr_t>(&rp) < reinterpret_cast<intptr_t>(this) + capacity_);
    VT *ptr = this->allocate<VT>();
    std::construct_at(ptr, std::forward<Args>(args)...);
    rp.offset_ = reinterpret_cast<intptr_t>(ptr) - reinterpret_cast<intptr_t>(&rp);
  }
  template <typename VT, typename... Args>
    requires RegionalConstructibleWithRegion<VT, RT> && std::constructible_from<VT, memory_region<RT> &, Args...>
  void create_to(regional_ptr<VT> &rp, Args &&...args) noexcept(false) {
    static_assert(RegionalType<VT>, "Type must be a valid regional type");
    assert(reinterpret_cast<intptr_t>(&rp) > reinterpret_cast<intptr_t>(this) &&
           reinterpret_cast<intptr_t>(&rp) < reinterpret_cast<intptr_t>(this) + capacity_);
    VT *ptr = this->allocate<VT>();
    std::construct_at(ptr, *this, std::forward<Args>(args)...);
    rp.offset_ = reinterpret_cast<intptr_t>(ptr) - reinterpret_cast<intptr_t>(&rp);
  }

  template <typename VT, typename... Args>
    requires RegionalConstructibleBits<VT> && std::constructible_from<VT, Args...>
  global_ptr<VT, RT> create_bits(Args &&...args) noexcept(false) {
    static_assert(RegionalType<VT>, "Type must be a valid regional type");
    VT *ptr = this->allocate<VT>();
    std::construct_at(ptr, std::forward<Args>(args)...);
    return global_ptr<VT, RT>( //
        *this,                 //
        reinterpret_cast<intptr_t>(ptr) - reinterpret_cast<intptr_t>(this));
  }
  template <typename VT, typename... Args>
    requires RegionalConstructibleWithRegion<VT, RT> && std::constructible_from<VT, memory_region<RT> &, Args...>
  global_ptr<VT, RT> create(Args &&...args) noexcept(false) {
    static_assert(RegionalType<VT>, "Type must be a valid regional type");
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

  template <typename T>
    requires yaml::YamlConvertible<T, RT>
  global_ptr<T, RT> create_from_yaml(const yaml::Node &node) noexcept(false) {
    static_assert(RegionalType<T>, "Type must be a valid regional type");
    auto raw_ptr = this->template allocate<T>();
    from_yaml<T>(*this, node, raw_ptr);
    return this->cast_ptr(raw_ptr);
  }

  // Allocate an object of type T from YAML and assign it to an existing regional_ptr.
  template <typename T>
    requires yaml::YamlConvertible<T, RT>
  void create_from_yaml_at(const yaml::Node &node, regional_ptr<T> &to_ptr) noexcept(false) {
    static_assert(RegionalType<T>, "Type must be a valid regional type");
    auto ptr = this->template allocate<T>();
    from_yaml<T>(*this, node, ptr);
    to_ptr = ptr;
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
      throw std::invalid_argument("Cross-region pointer assignment is not allowed. "
                                  "Attempted to assign a pointer from a different memory region. "
                                  "Both the source and target pointers must belong to the same memory region.");
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

template <typename RT>
  requires ValidMemRegionRootType<RT>
class auto_region final {
private:
  memory_region<RT> *region_;
  std::function<void(memory_region<RT> *)> deleter_;

public:
  // Constructor with default allocator
  template <typename... Args>
    requires std::constructible_from<RT, memory_region<RT> &, Args...>
  explicit auto_region(const size_t payload_capacity, Args &&...args) {
    region_ = memory_region<RT>::alloc_region(payload_capacity, std::forward<Args>(args)...);
    deleter_ = [](memory_region<RT> *r) {
      if (r) {
        std::allocator<std::byte> allocator;
        // Skip destructor call - memory_region doesn't need cleanup, just deallocate
        allocator.deallocate(static_cast<std::byte *>(static_cast<void *>(r)), r->capacity());
      }
    };
  }

  // Simplified constructor for root types that only need memory_region& parameter
  explicit auto_region(const size_t payload_capacity)
    requires std::constructible_from<RT, memory_region<RT> &>
  {
    region_ = memory_region<RT>::alloc_region(payload_capacity);
    deleter_ = [](memory_region<RT> *r) {
      if (r) {
        std::allocator<std::byte> allocator;
        // Skip destructor call - memory_region doesn't need cleanup, just deallocate
        allocator.deallocate(static_cast<std::byte *>(static_cast<void *>(r)), r->capacity());
      }
    };
  }

  // Constructor with custom allocator
  template <typename Allocator, typename... Args>
    requires std::constructible_from<RT, memory_region<RT> &, Args...>
  auto_region(Allocator allocator, const size_t payload_capacity, Args &&...args) {
    region_ = memory_region<RT>::alloc_region_with(allocator, payload_capacity, std::forward<Args>(args)...);
    size_t capacity = region_->capacity();
    deleter_ = [allocator, capacity](memory_region<RT> *r) mutable {
      if (r) {
        // Skip destructor call - memory_region doesn't need cleanup, just deallocate
        allocator.deallocate(static_cast<std::byte *>(static_cast<void *>(r)), capacity);
      }
    };
  }

  // Simplified constructor for custom allocator with root types that only need memory_region& parameter
  template <typename Allocator>
  auto_region(Allocator allocator, const size_t payload_capacity)
    requires std::constructible_from<RT, memory_region<RT> &>
  {
    region_ = memory_region<RT>::alloc_region_with(allocator, payload_capacity);
    size_t capacity = region_->capacity();
    deleter_ = [allocator, capacity](memory_region<RT> *r) mutable {
      if (r) {
        // Skip destructor call - memory_region doesn't need cleanup, just deallocate
        allocator.deallocate(static_cast<std::byte *>(static_cast<void *>(r)), capacity);
      }
    };
  }

  // Destructor
  ~auto_region() {
    if (deleter_) {
      deleter_(region_);
    }
  }

  // Movable but not copyable
  auto_region(const auto_region &) = delete;
  auto_region &operator=(const auto_region &) = delete;

  auto_region(auto_region &&other) noexcept : region_(other.region_), deleter_(std::move(other.deleter_)) {
    other.region_ = nullptr;
    other.deleter_ = nullptr;
  }

  auto_region &operator=(auto_region &&other) noexcept {
    if (this != &other) {
      // Clean up current resource
      if (deleter_) {
        deleter_(region_);
      }

      // Take ownership of other's resource
      region_ = other.region_;
      deleter_ = std::move(other.deleter_);

      // Clear other
      other.region_ = nullptr;
      other.deleter_ = nullptr;
    }
    return *this;
  }

  // Access operators
  memory_region<RT> &operator*() {
    assert(region_ != nullptr);
    return *region_;
  }

  const memory_region<RT> &operator*() const {
    assert(region_ != nullptr);
    return *region_;
  }

  memory_region<RT> *operator->() {
    assert(region_ != nullptr);
    return region_;
  }

  const memory_region<RT> *operator->() const {
    assert(region_ != nullptr);
    return region_;
  }

  // Check if valid
  explicit operator bool() const noexcept { return region_ != nullptr; }

  // Get raw pointer (use with caution)
  memory_region<RT> *get() { return region_; }
  const memory_region<RT> *get() const { return region_; }
};

} // namespace shilos
