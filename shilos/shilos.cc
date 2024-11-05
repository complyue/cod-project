
#include <algorithm>
#include <cstring>
#include <memory>
#include <new>

#include "shilos.hh"

namespace shilos {

namespace {

// CAVEATS:
//   keep this internal struct trivially copyable/movable, and dtor-free
struct memory_region {
  intptr_t baseaddr;
  intptr_t capacity;
};

struct MemoryRegionManager {
  static constexpr std::size_t INITIAL_CAPACITY = 1024000;
  static constexpr std::size_t CAPACITY_INCREMENT = 1024000;

  memory_region *buffer_;
  std::size_t size_;
  std::size_t capacity_;

  MemoryRegionManager() : buffer_(nullptr), size_(0), capacity_(0) {}

  ~MemoryRegionManager() {
    if (buffer_) {
      std::allocator<memory_region> sys_mem;
      sys_mem.deallocate(buffer_, capacity_);
    }
  }

  void register_region(const intptr_t baseaddr, const intptr_t capacity) {
    if (!buffer_) [[unlikely]] { // the very first registration
      std::allocator<memory_region> sys_mem;
      capacity_ = INITIAL_CAPACITY;
      buffer_ = sys_mem.allocate(capacity_);
      new (buffer_) memory_region{baseaddr, capacity};
      size_ = 1;
      return;
    }

    assert(capacity_ > 0);
    memory_region *const prev_end_pos = buffer_ + size_;
    memory_region *insert_pos =
        std::lower_bound(buffer_, prev_end_pos, baseaddr,
                         [](const memory_region &region, const intptr_t addr) { return region.baseaddr < addr; });
    if (size_ < capacity_) [[likely]] { // usual case
      std::memmove(insert_pos + 1, insert_pos, sizeof(memory_region) * (prev_end_pos - insert_pos));
    } else [[unlikely]] { // exceeding capacity
      std::allocator<memory_region> sys_mem;
      const std::size_t new_capacity = capacity_ + CAPACITY_INCREMENT;
      if (new_capacity < capacity_) [[unlikely]] // Simple overflow check
        throw std::bad_alloc();
      memory_region *const new_buffer = sys_mem.allocate(new_capacity);
      memory_region *const new_insert_pos = new_buffer + (insert_pos - buffer_);
      std::memcpy(new_buffer, buffer_, sizeof(memory_region) * (insert_pos - buffer_));
      if (insert_pos < prev_end_pos)
        std::memcpy(new_insert_pos + 1, insert_pos, sizeof(memory_region) * (prev_end_pos - insert_pos));
      sys_mem.deallocate(buffer_, capacity_);
      buffer_ = new_buffer;
      capacity_ = new_capacity;
      insert_pos = new_insert_pos;
    }
    new (insert_pos) memory_region{baseaddr, capacity};
    ++size_;
  }

  void unregister_region(const intptr_t baseaddr) {
    assert(buffer_);
    memory_region *const end_pos = buffer_ + size_;
    memory_region *const region =
        std::lower_bound(buffer_, end_pos, baseaddr,
                         [](const memory_region &region, const intptr_t &addr) { return region.baseaddr < addr; });
    assert(region >= buffer_);
    assert(region - buffer_ < size_);
    if (region < end_pos - 1) {
      std::memmove(region, region + 1, sizeof(memory_region) * (end_pos - region - 1));
    }
    --size_;
  }

  intptr_t base_of(const intptr_t ptr) {
    memory_region *const region =
        std::lower_bound(buffer_, buffer_ + size_, ptr,
                         [](const memory_region &region, const intptr_t &addr) { return region.baseaddr < addr; }) -
        1;
    assert(region >= buffer_);
    assert(region - buffer_ < size_);
    return region->baseaddr;
  }
};

// thread wide singleton memory region manager
thread_local MemoryRegionManager managed_regions;

} // namespace

extern "C" {

//
intptr_t _stake_base_of(const void *const ptr) {
  //
  return managed_regions.base_of(reinterpret_cast<const intptr_t>(ptr));
}

//
void register_memory_region(const intptr_t baseaddr, const intptr_t capacity) {
  //
  managed_regions.register_region(baseaddr, capacity);
}

//
void unregister_memory_region(const intptr_t baseaddr) {
  //
  managed_regions.unregister_region(baseaddr);
}

//
}

} // namespace shilos
