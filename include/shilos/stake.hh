
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
  virtual intptr_t base_ptr() { return this->_base_ptr; }
  virtual ptrdiff_t capacity() { return 0; }

  // allocate a memory block within this stake's interesting address range
  virtual void *allocate(size_t size, size_t align = 128) = 0;

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
    void *held_block = this->allocate(sizeof(_Tp), alignof(_Tp));
    // placement new with args
    new (held_block) _Tp(std::forward<Args>(args)...);
    // assign the ptr to target holder
    *holder = static_cast<_Tp *>(held_block);
    // return the instance reference
    return *holder;
  }

  virtual ~memory_stake(){};

protected:
  intptr_t _base_ptr;
};

//
// absolute ptr to some instance held by a \c memory_stake
//
template <typename _Tp, std::derived_from<memory_stake> _MsTp> class held_ptr {
public:
  typedef _Tp element_type;
  typedef _MsTp stake_type;

private:
  const stake_type *stake;
  const intptr_t offset;

public:
  held_ptr(const stake_type *stake_a, intptr_t offset_a) : stake(stake_a), offset(offset_a) {}

  _Tp *get() noexcept { return reinterpret_cast<_Tp *>(stake->base_ptr() + offset); }
  const _Tp *get() const noexcept { return reinterpret_cast<_Tp *>(stake->base_ptr() + offset); }

  _Tp &operator*() noexcept { return *get(); }
  const _Tp &operator*() const noexcept { return *get(); }

  _Tp *operator->() noexcept { return get(); }
  const _Tp *operator->() const noexcept { return get(); }
};

} // namespace shilos
