//===--- lang_native_runtime.hh - Native system-service boundary ----===//
#pragma once

#include "lang_exceptions.hh"

#include <string>

namespace cod::language {

inline std::string native_runtime_source() {
  std::string source = "#include <cstdint>\nenum class CoDException : std::uint64_t {\n";
  for (const auto &category : exception_categories)
    source += std::string(category.name) + " = " + std::to_string(exception_id(category.category)) + ",\n";
  source += "};\nstatic constexpr const char *cod_exception_names[] = {\n";
  for (const auto &category : exception_categories) source += "\"" + std::string(category.name) + "\",\n";
  source += "};\n";
  source += R"CPP(
#include <cstdio>
#include <cstdlib>
#include <cstring>
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
void *cod_lang_callable_create(std::uint64_t fixed, std::uint64_t count, void *const *children,
                               const std::uint64_t *offsets, std::uint64_t *status) {
  auto bytes = fixed;
  if (bytes > std::numeric_limits<std::size_t>::max()) {
    status[0] = static_cast<std::uint64_t>(CoDException::OutOfMemoryError);
    return nullptr;
  }
  for (std::uint64_t i = 0; i < count; ++i) {
    if (!children[i]) continue;
    const auto child_bytes = *static_cast<const std::uint64_t *>(children[i]);
    if (child_bytes > std::numeric_limits<std::size_t>::max() - bytes) {
      status[0] = static_cast<std::uint64_t>(CoDException::OutOfMemoryError);
      return nullptr;
    }
    bytes += child_bytes;
  }
  auto *image = static_cast<unsigned char *>(std::malloc(static_cast<std::size_t>(bytes)));
  if (!image) { status[0] = static_cast<std::uint64_t>(CoDException::OutOfMemoryError); return nullptr; }
  std::memcpy(image, &bytes, sizeof(bytes));
  auto position = fixed;
  for (std::uint64_t i = 0; i < count; ++i) {
    if (!children[i]) {
      const std::uint64_t absent = 0;
      std::memcpy(image + offsets[i], &absent, sizeof(absent));
      continue;
    }
    const auto child_bytes = *static_cast<const std::uint64_t *>(children[i]);
    std::memcpy(image + offsets[i], &position, sizeof(position));
    std::memcpy(image + position, children[i], static_cast<std::size_t>(child_bytes));
    position += child_bytes;
  }
  return image;
}
void *cod_lang_callable_copy(const void *source, std::uint64_t *status) {
  const auto bytes = *static_cast<const std::uint64_t *>(source);
  auto *image = std::malloc(static_cast<std::size_t>(bytes));
  if (!image) { status[0] = static_cast<std::uint64_t>(CoDException::OutOfMemoryError); return nullptr; }
  std::memcpy(image, source, static_cast<std::size_t>(bytes));
  return image;
}
void cod_lang_callable_free(void *image) { std::free(image); }
void *cod_lang_defer_allocate(std::uint64_t bytes, std::uint64_t *status) {
  if (bytes > std::numeric_limits<std::size_t>::max()) {
    status[0] = static_cast<std::uint64_t>(CoDException::OutOfMemoryError);
    return nullptr;
  }
  auto *record = std::malloc(static_cast<std::size_t>(bytes));
  if (!record) status[0] = static_cast<std::uint64_t>(CoDException::OutOfMemoryError);
  return record;
}
void cod_lang_defer_free(void *record) { std::free(record); }
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
    status[0] = static_cast<std::uint64_t>(CoDException::OutOfMemoryError);
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
std::uint64_t cod_lang_exception_cause(std::uint64_t cause, std::uint64_t index, std::uint64_t *result,
                                      std::uint64_t check_bounds) {
  const auto requested = index;
  for (;;) {
    if (check_bounds && !cause) {
      result[0] = requested - index;
      return 0;
    }
    // Physical history integrity remains checked independently of source bounds policy.
    if (!cause || !cod_causes || cause > cod_causes->size()) runtime_failure("invalid execution-local exception cause");
    const auto value = (*cod_causes)[cause - 1];
    if (!index) {
      result[0] = value.site; result[1] = value.requested; result[2] = value.available;
      result[3] = value.cause; result[4] = value.category;
      return 1;
    }
    cause = value.cause;
    --index;
  }
}
[[noreturn]] void cod_lang_uncaught(std::uint64_t category, std::uint64_t site,
                                  std::uint64_t requested, std::uint64_t available, std::uint64_t cause) {
  const auto print = [](const char *prefix, const CoDCause &value) {
    if (!value.category || value.category >= sizeof(cod_exception_names) / sizeof(*cod_exception_names))
      runtime_failure("invalid intrinsic exception category");
    const char *name = cod_exception_names[value.category];
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
  return source;
}

} // namespace cod::language
