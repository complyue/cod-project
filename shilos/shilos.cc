
#include <algorithm>
#include <cstring>
#include <memory>
#include <new>

#include "shilos.hh"

namespace shilos {

namespace {

struct managed_region : memory_region {
  memory_stake *stake;
};

class MemoryRegionManager {
private:
  static constexpr std::size_t INITIAL_CAPACITY = 8000;
  static constexpr std::size_t CAPACITY_INCREMENT = 30000;

  managed_region *regions_;
  std::size_t n_regions;
  std::size_t cap_regions;

public:
  MemoryRegionManager() : regions_(nullptr), n_regions(0), cap_regions(0) {}

  ~MemoryRegionManager() {
    if (regions_) {
      std::allocator<memory_region> sys_mem;
      sys_mem.deallocate(regions_, cap_regions);
    }
  }

  const managed_region *register_region(const intptr_t baseaddr, const intptr_t capacity, memory_stake *const stake) {
    if (!regions_) [[unlikely]] { // the very first registration
      std::allocator<managed_region> sys_mem;
      cap_regions = INITIAL_CAPACITY;
      regions_ = sys_mem.allocate(cap_regions);
      new (regions_) managed_region{{baseaddr, capacity, const_cast<memory_region *>(stake->live_region())}, stake};
      n_regions = 1;
      return regions_;
    }

    assert(cap_regions > 0);
    managed_region *const prev_end_pos = regions_ + n_regions;
    managed_region *insert_pos =
        std::lower_bound(regions_, prev_end_pos, baseaddr,
                         [](const managed_region &region, const intptr_t addr) { return region.baseaddr < addr; });
#ifndef NDEBUG
    if (insert_pos < prev_end_pos) {
      assert(baseaddr < insert_pos->baseaddr);
    }
#endif
    if (n_regions < cap_regions) [[likely]] { // usual case
      std::memmove(insert_pos + 1, insert_pos, sizeof(managed_region) * (prev_end_pos - insert_pos));
    } else [[unlikely]] { // exceeding capacity
      std::allocator<managed_region> sys_mem;
      const std::size_t new_capacity = cap_regions + CAPACITY_INCREMENT;
      if (new_capacity < cap_regions) [[unlikely]] // Simple overflow check
        throw std::bad_alloc();
      managed_region *const new_regions = sys_mem.allocate(new_capacity);
      managed_region *const new_insert_pos = new_regions + (insert_pos - regions_);
      std::memcpy(new_regions, regions_, sizeof(managed_region) * (insert_pos - regions_));
      if (insert_pos < prev_end_pos)
        std::memcpy(new_insert_pos + 1, insert_pos, sizeof(managed_region) * (prev_end_pos - insert_pos));
      sys_mem.deallocate(regions_, cap_regions);
      regions_ = new_regions;
      cap_regions = new_capacity;
      insert_pos = new_insert_pos;
    }
    new (insert_pos) managed_region{{baseaddr, capacity, const_cast<memory_region *>(stake->live_region())}, stake};
    ++n_regions;
    return insert_pos;
  }

  const managed_region *region_of(const intptr_t ptr) {
    managed_region *const end_pos = regions_ + n_regions;
    managed_region *const region =
        std::lower_bound(regions_, end_pos, ptr,
                         [](const managed_region &region, const intptr_t &addr) { return region.baseaddr < addr; });
    assert(region >= regions_);
    assert(region - regions_ < n_regions);
    if (region == end_pos) [[unlikely]] {
      return nullptr;
    }
    if (ptr < region->baseaddr || ptr - region->baseaddr >= region->capacity) [[unlikely]] {
      return nullptr;
    }
    return region;
  }

  void unregister_regions(memory_stake *const stake) {
    const memory_region *const live_region = stake->live_region();
    size_t cnt2rm = 0;
    for (const memory_region *rp = live_region; rp; rp = rp->prev)
      cnt2rm++;
    if (cnt2rm <= 0)
      return;

    managed_region *const end_pos = regions_ + n_regions;

    // there shouldn't be too many historic regions per memory_stake, so do stack alloca
    const memory_region **rps2rm = (const memory_region **)alloca(cnt2rm * sizeof(void *));

    // sort the ptrs of regions to remove, so we can overwrite the slots in an optimized way
    {
      const memory_region *rp = live_region;
      const memory_region **p = rps2rm;
      for (; rp; rp = rp->prev, p++) {
        // `memory_stake`s should always have their `live_region` history made by `assume_region()`,
        // i.e. getting ptrs to our `regions_`
        assert(regions_ <= rp && rp < end_pos);
        assert(static_cast<managed_region *>(rp)->stake == stake);
        *p = rp;
      }
    }
    std::sort(rps2rm, rps2rm + cnt2rm);

    // do move remaining items in our `regions_` array, those after the ones to be removed
    const memory_region **next_rp2rm = rps2rm;
    managed_region *write_pos = static_cast<managed_region *>(const_cast<memory_region *>(*next_rp2rm));
    next_rp2rm++;
    managed_region *read_pos = write_pos + 1;
    for (; read_pos < end_pos; write_pos++, read_pos++) {
      assert(regions_ <= write_pos && write_pos < end_pos);
      while (read_pos == *next_rp2rm) {
        assert(regions_ < read_pos && read_pos < end_pos);
        read_pos++;
        assert(next_rp2rm < rps2rm + cnt);
        next_rp2rm++;
      }
      *write_pos = *read_pos; // expect move of `regions_` items via trivial copy assignment
    }

    // decrease total number of regions, wrt those we've just removed
    n_regions -= cnt2rm;
  }
};

// thread wide singleton memory region manager
thread_local MemoryRegionManager managed_regions;

} // namespace

void memory_stake::assume_region(const intptr_t baseaddr, const intptr_t capacity) {
  assert(baseaddr > 0);
  assert(capacity > 0);
  live_region_ = managed_regions.register_region(baseaddr, capacity, this);
}

memory_stake::~memory_stake() {
  //
  managed_regions.unregister_regions(this);
}

extern "C" {

//
const memory_region *_region_of(const void *const ptr) {
  //
  return managed_regions.region_of(reinterpret_cast<const intptr_t>(ptr));
}

//
const memory_stake *_stake_of(const void *const ptr) {
  const managed_region *const region = managed_regions.region_of(reinterpret_cast<const intptr_t>(ptr));
  if (region)
    return region->stake;
  return nullptr;
}

//
}

} // namespace shilos
