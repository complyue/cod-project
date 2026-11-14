//===--- lang_exceptions.hh - Intrinsic exception categories --------===//
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

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
    ExceptionDescriptor{ExceptionCategory::system, "SystemException", false},
    ExceptionDescriptor{ExceptionCategory::out_of_memory, "OutOfMemory", false},
    ExceptionDescriptor{ExceptionCategory::region_capacity_exceeded, "RegionCapacityExceeded", false},
    ExceptionDescriptor{ExceptionCategory::invalid_capacity, "InvalidCapacity", false},
    ExceptionDescriptor{ExceptionCategory::null_pointer, "NullPointerException", true},
    ExceptionDescriptor{ExceptionCategory::missing_return, "MissingReturnException", false},
    ExceptionDescriptor{ExceptionCategory::unsound_runtime_typing, "UnsoundRuntimeTyping", false},
    ExceptionDescriptor{ExceptionCategory::statically_unknown_type, "StaticallyUnknownType", true},
};
inline constexpr size_t concrete_exception_count = exception_categories.size() - 1;

static_assert([] {
  for (size_t i = 0; i < exception_categories.size(); ++i)
    if (exception_id(exception_categories[i].category) != i) return false;
  return true;
}());

} // namespace cod::language
