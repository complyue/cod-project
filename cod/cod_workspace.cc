//===--- cod/cod_workspace.cc - CoD Workspace Implementation -----------===//
//
// CoD Project - DBMR-backed workspace with build cache integration
// Implements WorksRoot with regional memory management
//
//===----------------------------------------------------------------------===//

#include "cod.hh"
#include "cod_cache.hh"
#include <cstdlib>
#include <stdexcept>

namespace cod {

// WorksRoot implementation
WorksRoot::WorksRoot(memory_region<WorksRoot> &mr, const fs::path &project_root)
    : project_root_path(mr.allocate<regional_str>()), toolchain_version(mr.allocate<regional_str>()),
      build_config(mr.allocate<regional_dict<regional_str, regional_str>>()),
      build_cache(std::make_unique<cache::BuildCache>(project_root)) {

  // Initialize the allocated objects
  new (project_root_path.get()) regional_str(mr, project_root.string());
  new (toolchain_version.get()) regional_str(mr, "clang-18");
  new (build_config.get()) regional_dict<regional_str, regional_str>(mr);

  // Initialize default build configuration
  build_config->insert_or_assign(mr, std::string_view("optimization"), std::string_view("-O2"));
  build_config->insert_or_assign(mr, std::string_view("debug_info"), std::string_view("-g"));
  build_config->insert_or_assign(mr, std::string_view("std_version"), std::string_view("-std=c++20"));
  build_config->insert_or_assign(mr, std::string_view("warnings"), std::string_view("-Wall -Wextra"));
}

void WorksRoot::set_project_root(const fs::path &root) {
  // Note: Cannot reassign regional_str directly, would need to recreate the object
  // For now, recreate build cache with new project root
  build_cache = std::make_unique<cache::BuildCache>(root);
}

fs::path WorksRoot::get_project_root() const { return fs::path(std::string_view(*project_root_path)); }

void WorksRoot::set_toolchain_version(const std::string &version) {
  // Note: Cannot reassign regional_str directly, would need to recreate the object
}

std::string WorksRoot::get_toolchain_version() const { return std::string(std::string_view(*toolchain_version)); }

void WorksRoot::set_build_config(const std::string &key, const std::string &value) {
  // Note: Cannot modify regional_dict directly without memory_region reference
}

std::string WorksRoot::get_build_config(const std::string &key) const {
  auto *value_ptr = build_config->find_value(std::string_view(key));
  return value_ptr ? std::string(std::string_view(*value_ptr)) : std::string{};
}

} // namespace cod
