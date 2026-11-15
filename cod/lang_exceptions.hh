//===--- lang_exceptions.hh - Intrinsic exception categories --------===//
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace cod::language {

enum class ExceptionCategory : std::uint64_t {
  system = 0,
  out_of_memory = 1,
  region_capacity_exceeded = 2,
  invalid_capacity = 3,
  null_pointer = 4,
  missing_return = 5,
  unsound_runtime_typing = 6,
  statically_unknown_type = 7,
  index_out_of_bounds = 8,
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
    ExceptionDescriptor{ExceptionCategory::system, "Exception", false},
    ExceptionDescriptor{ExceptionCategory::out_of_memory, "OutOfMemoryError", false},
    ExceptionDescriptor{ExceptionCategory::region_capacity_exceeded, "RegionCapacityExceededError", false},
    ExceptionDescriptor{ExceptionCategory::invalid_capacity, "InvalidCapacityError", false},
    ExceptionDescriptor{ExceptionCategory::null_pointer, "NullPointerException", true},
    ExceptionDescriptor{ExceptionCategory::missing_return, "MissingReturnError", false},
    ExceptionDescriptor{ExceptionCategory::unsound_runtime_typing, "UnsoundRuntimeTypingError", false},
    ExceptionDescriptor{ExceptionCategory::statically_unknown_type, "StaticallyUnknownTypeException", true},
    ExceptionDescriptor{ExceptionCategory::index_out_of_bounds, "IndexOutOfBoundsError", false},
};
inline constexpr size_t concrete_exception_count = exception_categories.size() - 1;

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
