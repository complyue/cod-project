
#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <new>
#include <stdexcept>

namespace shilos {

using std::intptr_t;
using std::size_t;

class memory_stake;

struct memory_region {
  intptr_t baseaddr;
  intptr_t capacity;
  memory_region *prev;
  memory_stake *stake;
};

class memory_stake {

private:
  // CAVEATS: this field must be updated only via assume_region()
  const memory_region *live_region_;

protected:
  // register the specified region as new live_region_ of this stake, to the tls stake registry
  void assume_region(const intptr_t baseaddr, const intptr_t capacity);

public:
  memory_stake() : live_region_(nullptr) {}

  // unregister from the tls stake registry, wrt live and historic regions ever registered
  ~memory_stake();

  // copying is prohibited
  memory_stake(const memory_stake &) = delete;
  memory_stake &operator=(const memory_stake &) = delete;

  // moving is allowed (welcomed)
  memory_stake(memory_stake &&other);
  memory_stake &operator=(memory_stake &&other);

  const memory_region *live_region() { return live_region_; }
  inline intptr_t baseaddr() { return live_region_ ? live_region_->baseaddr : 0; }
  inline intptr_t capacity() { return live_region_ ? live_region_->capacity : 0; }
};

extern "C" {

//
const memory_region *_region_of(const void *ptr);

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
  void validate() {
    const memory_region *const region = _region_of(this);
    if (!region)
      throw std::logic_error("!?relativ_ptr out-lives its stake?!");

    assert(region->baseaddr <= reinterpret_cast<intptr_t>(this));
    if (reinterpret_cast<intptr_t>(this) >= region->baseaddr + region->capacity)
      throw std::logic_error("!?relativ_ptr self out-of stake capacity, or lack of canonicalization?!");

    if (distance_) {
      T *const tgt = get();
      assert(region->baseaddr <= reinterpret_cast<intptr_t>(tgt));
      if (reinterpret_cast<intptr_t>(tgt) >= region->baseaddr + region->capacity)
        throw std::logic_error("!?relativ_ptr target out-of stake capacity, or lack of canonicalization?!");
    }
  }

  relativ_ptr<T> *canonicalize() {
    return const_cast<relativ_ptr<T> *>(static_cast<const relativ_ptr<T> *>(this)->canonicalize());
  }

  const relativ_ptr<T> *canonicalize() const {
    const memory_region *const region = _region_of(this);
    assert(region);
    if (region == region->stake->live_region()) [[likely]] {
      return this;
    }
    const intptr_t self_offset = reinterpret_cast<intptr_t>(this) - region->baseaddr;
    return reinterpret_cast<relativ_ptr<T> *>(region->stake->live_region()->baseaddr + self_offset);
  }

  relativ_ptr() noexcept : distance_(0) {}

  relativ_ptr(T *ptr) noexcept : distance_(relativ_distance(this, ptr)) {
    // TODO: this ctor should always be called inplace within some region, how to ensure that?
    assert(_region_of(this) == _region_of(ptr));
  }

  relativ_ptr(const relativ_ptr &other) noexcept : distance_(relativ_distance(this, other.get())) {
    assert(_region_of(this) == _region_of(&other));
  }

  relativ_ptr(relativ_ptr &&other) = delete;

  relativ_ptr &operator=(const relativ_ptr &other) & noexcept {
    if (this != &other) {
      distance_ = relativ_distance(this, other.get());
    }
    return *this;
  }

  relativ_ptr &operator=(relativ_ptr &&other) = delete;

  ~relativ_ptr() = default;

  relativ_ptr &operator=(T *other) & noexcept {
    assert(_region_of(this) == _region_of(other));
    if (this->get() != other) {
      distance_ = relativ_distance(this, other);
    }
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

class live_stake : public memory_stake {
protected:
  virtual void *allocate(size_t size, size_t align) = 0;

public:
  virtual ~live_stake() = default;

  template <typename T, typename... Args> T &intern(Args &&...args) {
    void *ptr = this->allocate(sizeof(T), alignof(T));
    if (!ptr)
      throw std::bad_alloc();
    return *new (ptr) T(std::forward<Args>(args)...);
  }

  template <typename T, typename... Args> void assign(relativ_ptr<T> &ptr, Args &&...args) {
    // TODO canonical relativ_ptr assumed here,
    //      should we allow assigning to ptrs within historical regions too?
    //      have to be in cherrypicked circumstances?
    assert(live_region());
    assert(&ptr > baseaddr());
    assert(&ptr < baseaddr() + capacity());
    ptr = intern<T>(std::forward(args)...);
  }
};

} // namespace shilos
