
#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

#include "./relativ.hh"

namespace shilos {

using std::intptr_t;
using std::ptrdiff_t;

class memory_stake {
public:
  virtual intptr_t base_ptr() { return base_ptr_; }
  virtual ptrdiff_t capacity() { return 0; }

  // allocate a memory block within this stake's interesting address range
  virtual void *allocate(size_t bytes, size_t align = 128) = 0;

  // all assignment (including clearing with nullptr) to (relative) ptrs inside this stake
  // should go through this method, to facilitate correct reference counting
  template <typename _Tp> _Tp &assign_ptr(relativ_ptr<_Tp> *holder, const _Tp *target) {
    const _Tp *curr = holder->get();
    if (curr != target) {
      if (target)
        increase_ref(reinterpret_cast<intptr_t>(target));
      *holder = target;
      if (curr)
        decrease_ref(reinterpret_cast<intptr_t>(curr));
    }
    return *holder;
  }

  template <typename _Tp, typename... Args> _Tp &new_held(relativ_ptr<_Tp> *holder, Args &&...args) {
#ifndef NDEBUG
    {
      const auto holder_ptr = reinterpret_cast<intptr_t>(holder);
      if (capacity() <= 0)
        throw std::logic_error("!?new object in empty stake?!");
      const auto base_ptr = this->base_ptr();
      const auto end_ptr = base_ptr + this->capacity();
      if (holder_ptr < base_ptr || holder_ptr >= end_ptr)
        throw std::out_of_range("!?holder out of stake interests?!");
    }
#endif
    // alloc mem block within
    _Tp *target = static_cast<_Tp *>(this->allocate(sizeof(_Tp), alignof(_Tp)));
    // placement new with args
    new (target) _Tp(std::forward<Args>(args)...);
    // assign the ptr to target holder
    assign_ptr(holder, target);
    // return the instance reference
    return *holder;
  }

  virtual ~memory_stake(){};

protected:
  // stake-wide reference counting callbacks
  virtual void increase_ref(intptr_t ptr) {}
  virtual void decrease_ref(intptr_t ptr) {}

  intptr_t base_ptr_;
};

//
// absolute ptr to some instance held by a \c memory_stake
//
template <typename _Tp, std::derived_from<memory_stake> _MsTp> class held_ptr {
public:
  typedef _Tp element_type;
  typedef _MsTp stake_type;

private:
  const stake_type *stake_;
  const intptr_t offset_;

public:
  held_ptr(const stake_type *stake, intptr_t offset) : stake_(stake), offset_(offset) {}

  _Tp *get() noexcept { return reinterpret_cast<_Tp *>(stake_->base_ptr() + offset_); }
  const _Tp *get() const noexcept { return reinterpret_cast<_Tp *>(stake_->base_ptr() + offset_); }

  _Tp &operator*() noexcept { return *get(); }
  const _Tp &operator*() const noexcept { return *get(); }

  _Tp *operator->() noexcept { return get(); }
  const _Tp *operator->() const noexcept { return get(); }
};

} // namespace shilos
