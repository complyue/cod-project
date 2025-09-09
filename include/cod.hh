#pragma once

#include "shilos.hh" // IWYU pragma: keep

namespace cod {

// Minimal default WorksRoot type for Compile-on-Demand DBMR workspaces.
// - Satisfies ValidMemRegionRootType concept via TYPE_UUID reference.
// - Constructible only in-region via memory_region<WorksRoot>&.
// - No copy/move semantics, zero-cost relocation compatible.
struct WorksRoot {
  // TYPE_UUID must be available as a const UUID& expression to satisfy the concept.
  static const shilos::UUID TYPE_UUID_INSTANCE; // storage
  static const shilos::UUID &TYPE_UUID;         // reference, required by concept

  // Constructible only with a memory_region reference; additional args can be added later.
  explicit WorksRoot(shilos::memory_region<WorksRoot> & /*mr*/) noexcept {}

  WorksRoot(const WorksRoot &) = delete;
  WorksRoot(WorksRoot &&) = delete;
  WorksRoot &operator=(const WorksRoot &) = delete;
  WorksRoot &operator=(WorksRoot &&) = delete;
};

// Inline definitions to avoid ODR issues; UUID chosen arbitrarily but stable.
inline const shilos::UUID WorksRoot::TYPE_UUID_INSTANCE = shilos::UUID("D8E5A5E3-8B9C-4A07-9AFB-4EAD56A29F17");
inline const shilos::UUID &WorksRoot::TYPE_UUID = WorksRoot::TYPE_UUID_INSTANCE;

} // namespace cod
