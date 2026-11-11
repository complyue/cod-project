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
#include <memory>
#include <new>
#include <vector>

struct CoDCause {
  std::uint64_t category, site, requested, available, cause;
};
// Immutable, execution-local histories. Numeric handles survive regional relocation.
// A trivial TLS pointer avoids first-use destructor registration on OOM paths.
static thread_local std::vector<CoDCause> *cod_causes = nullptr;
extern "C" const char *cod_lang_exception_site(std::uint64_t site);
extern "C" void cod_lang_exception_finish() noexcept {
  delete cod_causes;
  cod_causes = nullptr;
}

[[noreturn]] static void runtime_failure(const char *message, const char *context = nullptr) {
  if (context) std::fprintf(stderr, "%s: ", context);
  std::fprintf(stderr, "%s\n", message);
  cod_lang_exception_finish();
  std::exit(70);
}
extern "C" {
[[noreturn]] void cod_lang_fail(const char *message) { runtime_failure(message); }
std::uint64_t cod_lang_exception_chain(std::uint64_t category, std::uint64_t site,
                                       std::uint64_t requested, std::uint64_t available,
                                       std::uint64_t cause, std::uint64_t *status) {
  try {
    std::unique_ptr<std::vector<CoDCause>> initial;
    if (!cod_causes) initial = std::make_unique<std::vector<CoDCause>>();
    auto &records = cod_causes ? *cod_causes : *initial;
    if (records.size() == records.max_size()) throw std::bad_alloc();
    records.push_back({category, site, requested, available, cause});
    if (initial) cod_causes = initial.release();
    return static_cast<std::uint64_t>(records.size());
  } catch (const std::bad_alloc &) {
    status[0] = 1;
    return 0;
  }
}
std::uint64_t cod_lang_exception_count(std::uint64_t cause) {
  std::uint64_t count = 0;
  while (cause) {
    if (!cod_causes || cause > cod_causes->size()) runtime_failure("invalid execution-local exception cause");
    cause = (*cod_causes)[cause - 1].cause;
    ++count;
  }
  return count;
}
void cod_lang_exception_cause(std::uint64_t cause, std::uint64_t index, std::uint64_t *result) {
  for (;;) {
    if (!cause || !cod_causes || cause > cod_causes->size()) runtime_failure("exception cause index out of bounds");
    const auto value = (*cod_causes)[cause - 1];
    if (!index) {
      result[0] = value.site; result[1] = value.requested; result[2] = value.available;
      result[3] = value.cause; result[4] = value.category;
      return;
    }
    cause = value.cause;
    --index;
  }
}
[[noreturn]] void cod_lang_uncaught(std::uint64_t category, std::uint64_t site,
                                  std::uint64_t requested, std::uint64_t available, std::uint64_t cause) {
  const auto print = [](const char *prefix, const CoDCause &value) {
    const char *name = value.category == 1 ? "OutOfMemory" :
                       value.category == 2 ? "RegionCapacityExceeded" :
                       value.category == 3 ? "InvalidCapacity" : "NullPointerException";
    if (const char *context = cod_lang_exception_site(value.site)) std::fprintf(stderr, "%s: ", context);
    std::fprintf(stderr, "%s %s (site=%llu, requested=%llu, available=%llu)\n", prefix, name,
                 static_cast<unsigned long long>(value.site), static_cast<unsigned long long>(value.requested),
                 static_cast<unsigned long long>(value.available));
  };
  print("uncaught", {category, site, requested, available, cause});
  while (cause) {
    if (!cod_causes || cause > cod_causes->size()) runtime_failure("invalid execution-local exception cause");
    const auto value = (*cod_causes)[cause - 1];
    print("caused by", value);
    cause = value.cause;
  }
  cod_lang_exception_finish();
  std::exit(70);
}
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

)CPP";
  source += R"CPP(
extern "C" {
void *cod_lang_region_new(std::uint64_t capacity, std::uint64_t *status) {
  if (capacity < sizeof(CoDRegionRoot) || capacity > std::numeric_limits<std::size_t>::max() - sizeof(CoDRegion)) {
    status[0] = 3;
    return nullptr;
  }
  try { return CoDRegion::alloc_region(static_cast<std::size_t>(capacity)); }
  catch (const std::bad_alloc &) { status[0] = 1; return nullptr; }
}
void cod_lang_region_free(void *region) { CoDRegion::free_region(static_cast<CoDRegion *>(region)); }
std::uint64_t cod_lang_region_root(void *owner) {
  auto &region = *static_cast<CoDRegion *>(owner);
  return static_cast<std::uint64_t>(reinterpret_cast<std::byte *>(region.root().get()) -
                                    reinterpret_cast<std::byte *>(&region));
}
std::uint64_t cod_lang_region_reserve_root(void *owner, std::uint64_t bytes, std::uint64_t alignment,
                                          std::uint64_t *status) {
  auto &region = *static_cast<CoDRegion *>(owner);
  // The empty native root marks the start of the LLVM-defined root storage.
  // Reserve its remaining bytes in place; no extra root object or pointer table.
  const auto offset = cod_lang_region_root(owner);
  if (bytes < sizeof(CoDRegionRoot) || !alignment ||
      reinterpret_cast<std::uintptr_t>(region.root().get()) % alignment)
    runtime_failure("unsupported native root layout");
  try {
    if (bytes - sizeof(CoDRegionRoot) > std::numeric_limits<std::size_t>::max()) throw std::bad_alloc();
    region.allocate(static_cast<std::size_t>(bytes - sizeof(CoDRegionRoot)), 1);
    return offset;
  } catch (const std::bad_alloc &) {
    status[0] = 2;
    status[1] = region.free_capacity() + sizeof(CoDRegionRoot);
    return 0;
  }
}
std::uint64_t cod_lang_region_alloc(void *owner, std::uint64_t bytes, std::uint64_t alignment, std::uint64_t *status) {
  auto &region = *static_cast<CoDRegion *>(owner);
  try {
    if (bytes > std::numeric_limits<std::size_t>::max()) throw std::bad_alloc();
    auto *storage = region.allocate(static_cast<std::size_t>(bytes), static_cast<std::size_t>(alignment));
    return static_cast<std::uint64_t>(static_cast<std::byte *>(storage) - reinterpret_cast<std::byte *>(&region));
  } catch (const std::bad_alloc &) {
    status[0] = 2;
    status[1] = region.capacity() - region.occupation();
    return 0;
  }
}
std::uint64_t cod_lang_region_used(void *region) { return static_cast<CoDRegion *>(region)->occupation(); }
std::uint64_t cod_lang_region_capacity(void *region) { return static_cast<CoDRegion *>(region)->capacity(); }
}
)CPP";
  return source;
}

} // namespace cod::language
