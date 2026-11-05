//===--- lang_native_runtime.hh - Native system-service boundary ----===//
#pragma once

#include <string>

namespace cod::language {

inline std::string native_runtime_source(bool regions) {
  std::string source = R"CPP(
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>

[[noreturn]] static void runtime_failure(const char *message) {
  std::fprintf(stderr, "%s\n", message);
  std::exit(70);
}
extern "C" {
[[noreturn]] void cod_lang_fail(const char *message) { runtime_failure(message); }
void cod_lang_print_i64(std::int64_t value) { std::printf("%lld\n", static_cast<long long>(value)); }
void cod_lang_print_u64(std::uint64_t value) { std::printf("%llu\n", static_cast<unsigned long long>(value)); }
void cod_lang_print_f32(float value) { std::printf("%.9g\n", static_cast<double>(value)); }
void cod_lang_print_f64(double value) { std::printf("%.17g\n", value); }
void cod_lang_print_bool(std::uint32_t value) { std::puts(value ? "true" : "false"); }
}
)CPP";
  if (!regions) return source;
  source += R"CPP(
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
)CPP";
  source += R"CPP(
extern "C" {
void *cod_lang_region_new(std::uint64_t capacity) {
  try { return CoDRegion::alloc_region(region_capacity(capacity)); }
  catch (const std::exception &error) { runtime_failure(error.what()); }
}
void cod_lang_region_free(void *region) { CoDRegion::free_region(static_cast<CoDRegion *>(region)); }
std::uint64_t cod_lang_region_alloc(void *owner, std::uint64_t bytes, std::uint64_t alignment) {
  auto &region = *static_cast<CoDRegion *>(owner);
  try {
    if (bytes > std::numeric_limits<std::size_t>::max()) throw std::bad_alloc();
    auto *storage = region.allocate(static_cast<std::size_t>(bytes), static_cast<std::size_t>(alignment));
    return static_cast<std::uint64_t>(static_cast<std::byte *>(storage) - reinterpret_cast<std::byte *>(&region));
  } catch (const std::exception &error) { runtime_failure(error.what()); }
}
std::uint64_t cod_lang_region_used(void *region) { return static_cast<CoDRegion *>(region)->occupation(); }
std::uint64_t cod_lang_region_capacity(void *region) { return static_cast<CoDRegion *>(region)->capacity(); }
}
)CPP";
  return source;
}

} // namespace cod::language
