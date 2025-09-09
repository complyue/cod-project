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
    : project_root_path(mr.allocate<std::string>()), toolchain_version(mr.allocate<std::string>()),
      build_config(mr.allocate<std::unordered_map<std::string, std::string>>()),
      build_cache(std::make_unique<cache::BuildCache>(project_root)) {

  // Initialize the allocated objects
  new (project_root_path.get()) std::string(project_root.string());
  new (toolchain_version.get()) std::string("clang-18");
  new (build_config.get()) std::unordered_map<std::string, std::string>();

  // Initialize default build configuration
  build_config->emplace("optimization", "-O2");
  build_config->emplace("debug_info", "-g");
  build_config->emplace("std_version", "-std=c++20");
  build_config->emplace("warnings", "-Wall -Wextra");
}

void WorksRoot::set_project_root(const fs::path &root) {
  *project_root_path = root.string();
  // Recreate build cache with new project root
  build_cache = std::make_unique<cache::BuildCache>(root);
}

fs::path WorksRoot::get_project_root() const { return fs::path(*project_root_path); }

void WorksRoot::set_toolchain_version(const std::string &version) { *toolchain_version = version; }

std::string WorksRoot::get_toolchain_version() const { return *toolchain_version; }

void WorksRoot::set_build_config(const std::string &key, const std::string &value) { (*build_config)[key] = value; }

std::string WorksRoot::get_build_config(const std::string &key) const {
  auto it = build_config->find(key);
  return (it != build_config->end()) ? it->second : std::string{};
}

} // namespace cod