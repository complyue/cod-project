
#pragma once

#include <concepts>
#include <cstdint>

namespace shilos {

using std::intptr_t;
using std::uintptr_t;

class memory_stake {
public:
  uintptr_t base_ptr() { return this->_base_ptr; }

protected:
  uintptr_t _base_ptr;
};

template <typename _Tp, std::derived_from<memory_stake> _MsTp> class held_ptr {
public:
  typedef _Tp element_type;
  typedef _MsTp stake_type;

  const stake_type *stake;
  const uintptr_t offset;

  _Tp *get() const noexcept {
    return static_cast<_Tp *>(stake->base_ptr() + offset);
  }
  _Tp *operator->() const noexcept {
    return static_cast<_Tp *>(stake->base_ptr() + offset);
  }
};

} // namespace shilos
