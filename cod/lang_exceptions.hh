//===--- lang_exceptions.hh - Intrinsic exception categories --------===//
#pragma once

#include "lang_intrinsic_catalog.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

namespace cod::language {

enum class ExceptionCategory : std::uint64_t {
#define COD_LANG_EXCEPTION_CATEGORY(key, spelling, id, checked) key = id,
  COD_LANG_INTRINSIC_EXCEPTIONS(COD_LANG_EXCEPTION_CATEGORY)
#undef COD_LANG_EXCEPTION_CATEGORY
};

constexpr size_t exception_id(ExceptionCategory category) {
  return static_cast<size_t>(category);
}

struct ExceptionDescriptor {
  ExceptionCategory category;
  const char *name;
  bool checked;
};

// Category zero is the common source type, not a concrete runtime failure.
// These identities are shared by vocabulary, native dispatch and services.
inline constexpr std::array exception_categories{
#define COD_LANG_EXCEPTION_DESCRIPTOR(key, spelling, id, checked) ExceptionDescriptor{ExceptionCategory::key, #spelling, checked != 0},
    COD_LANG_INTRINSIC_EXCEPTIONS(COD_LANG_EXCEPTION_DESCRIPTOR)
#undef COD_LANG_EXCEPTION_DESCRIPTOR
};
inline constexpr size_t concrete_exception_count = exception_categories.size() - 1;

struct CheckedExceptionCategory {
  std::string_view name;
  bool checked;
  std::optional<size_t> schema = {};
};

inline std::vector<CheckedExceptionCategory> intrinsic_exception_types() {
  std::vector<CheckedExceptionCategory> types;
  types.reserve(exception_categories.size());
  for (const auto &category : exception_categories) types.push_back({category.name, category.checked});
  return types;
}

static_assert([] {
  for (size_t i = 0; i < exception_categories.size(); ++i) {
    const auto &descriptor = exception_categories[i];
    if (exception_id(descriptor.category) != i) return false;
    const std::string_view name{descriptor.name};
    if (!i) {
      if (name != "Exception" || descriptor.checked) return false;
    } else if (!name.ends_with(descriptor.checked ? "Exception" : "Error")) return false;
  }
  return true;
}());

} // namespace cod::language
