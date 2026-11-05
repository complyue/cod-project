//===--- lang_region_runtime.hh - Native regional source support ----===//
#pragma once

#include <string_view>

namespace cod::language {

// Emitted only for programs that use regional data. The source prototype uses
// the existing allocator and self-relative fields; this is not a DBMR ABI.
inline constexpr std::string_view region_runtime = R"CPP(
#include <shilos/region.hh>
#include <memory>

struct CoDRegionRoot {
  static constexpr shilos::UUID TYPE_UUID{"d5b09c4c-21dd-4b88-90bb-31e057c6c627"};
  explicit CoDRegionRoot(shilos::memory_region<CoDRegionRoot> &) {}
  CoDRegionRoot(const CoDRegionRoot &) = delete;
  CoDRegionRoot(CoDRegionRoot &&) = delete;
  CoDRegionRoot &operator=(const CoDRegionRoot &) = delete;
  CoDRegionRoot &operator=(CoDRegionRoot &&) = delete;
};
using CoDRegion = shilos::memory_region<CoDRegionRoot>;
using CoDRegionOwner = shilos::auto_region<CoDRegionRoot>;

static std::size_t region_capacity(std::uint64_t bytes) {
  if (bytes < sizeof(CoDRegionRoot) || bytes > std::numeric_limits<std::size_t>::max() - sizeof(CoDRegion))
    runtime_failure("region capacity out of range");
  return static_cast<std::size_t>(bytes);
}
template<class T> static auto region_pointer(CoDRegion &region, std::uint64_t offset) {
  if (!offset) return region.template null<T>();
  return region.cast_ptr(reinterpret_cast<T *>(reinterpret_cast<std::byte *>(&region) + offset));
}
template<class T> static const T &region_read(CoDRegion &region, std::uint64_t offset) {
  if (!offset) runtime_failure("null regional reference");
  const auto pointer = region_pointer<T>(region, offset);
  return *pointer;
}
template<class T> static auto region_offset(CoDRegion &region, const T *pointer) {
  if (!pointer) return std::uint64_t(0);
  const auto checked = region.cast_ptr(pointer);
  return static_cast<std::uint64_t>(reinterpret_cast<const std::byte *>(checked.get()) -
                                    reinterpret_cast<const std::byte *>(&region));
}
template<class T> struct CoDArray {
  shilos::regional_ptr<T> data;
  std::uint64_t size;
  CoDArray(CoDRegion &region, std::uint64_t count = 0, T initial = {}) : data(), size(count) {
    if (count > region.free_capacity() / sizeof(T)) throw std::bad_alloc();
    if (count) {
      auto *storage = region.allocate<T>(static_cast<std::size_t>(count));
      for (std::uint64_t i = 0; i < count; ++i) std::construct_at(storage + i, initial);
      data = region.cast_ptr(storage);
    }
  }
  CoDArray(const CoDArray &) = delete;
  CoDArray(CoDArray &&) = delete;
  CoDArray &operator=(const CoDArray &) = delete;
  CoDArray &operator=(CoDArray &&) = delete;
};
template<class T> static const T &array_read(CoDRegion &region, std::uint64_t offset, std::uint64_t index) {
  const auto &array = region_read<CoDArray<T>>(region, offset);
  if (index >= array.size) runtime_failure("regional array index out of bounds");
  return array.data.get()[index];
}
template<class T> static T array_store(CoDRegion &region, std::uint64_t offset, std::uint64_t index, T value) {
  if (!offset) runtime_failure("null regional reference");
  auto array = region_pointer<CoDArray<T>>(region, offset);
  if (index >= array->size) runtime_failure("regional array index out of bounds");
  array->data.get()[index] = value;
  return value;
}
template<class T> static auto array_reference(CoDRegion &region, std::uint64_t offset, std::uint64_t index) {
  const auto &array = region_read<CoDArray<T>>(region, offset);
  if (index >= array.size) runtime_failure("regional array index out of bounds");
  return region_offset(region, array.data.get() + index);
}
)CPP";

} // namespace cod::language
