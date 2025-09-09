#pragma once

#include "cod_cache.hh"
#include "shilos.hh" // IWYU pragma: keep
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace cod {

namespace fs = std::filesystem;
using namespace shilos;

// Forward declarations
namespace cache {
class BuildCache;
}

// Enhanced WorksRoot type for Compile-on-Demand DBMR workspaces.
// - Satisfies ValidMemRegionRootType concept via TYPE_UUID reference.
// - Constructible only in-region via memory_region<WorksRoot>&.
// - No copy/move semantics, zero-cost relocation compatible.
// - Manages build cache and workspace state.
struct WorksRoot {
  // TYPE_UUID must be available as a const UUID& expression to satisfy the concept.
  static const shilos::UUID TYPE_UUID_INSTANCE; // storage
  static const shilos::UUID &TYPE_UUID;         // reference, required by concept

  // Workspace state stored in memory region
  regional_ptr<std::string> project_root_path;
  regional_ptr<std::string> toolchain_version;
  regional_ptr<std::unordered_map<std::string, std::string>> build_config;

  // Build cache (not stored in region, managed separately)
  std::unique_ptr<cache::BuildCache> build_cache;

  // Constructible only with a memory_region reference
  explicit WorksRoot(memory_region<WorksRoot> &mr, const fs::path &project_root);

  WorksRoot(const WorksRoot &) = delete;
  WorksRoot(WorksRoot &&) = delete;
  WorksRoot &operator=(const WorksRoot &) = delete;
  WorksRoot &operator=(WorksRoot &&) = delete;

  // Workspace management
  void set_project_root(const fs::path &root);
  fs::path get_project_root() const;

  void set_toolchain_version(const std::string &version);
  std::string get_toolchain_version() const;

  void set_build_config(const std::string &key, const std::string &value);
  std::string get_build_config(const std::string &key) const;

  // Cache access
  cache::BuildCache &get_build_cache() { return *build_cache; }
  const cache::BuildCache &get_build_cache() const { return *build_cache; }
};

// Inline definitions to avoid ODR issues; UUID chosen arbitrarily but stable.
inline const shilos::UUID WorksRoot::TYPE_UUID_INSTANCE = shilos::UUID("D8E5A5E3-8B9C-4A07-9AFB-4EAD56A29F17");
inline const shilos::UUID &WorksRoot::TYPE_UUID = WorksRoot::TYPE_UUID_INSTANCE;

} // namespace cod
