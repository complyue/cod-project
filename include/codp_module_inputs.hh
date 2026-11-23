//===--- codp_module_inputs.hh - Checked package source inputs --------===//
#pragma once

#include "codp_yaml.hh"

#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace cod::project {

struct PackageSourceRoot {
  UUID package;
  std::filesystem::path directory;
};

namespace module_inputs {

namespace fs = std::filesystem;

struct Requirement {
  UUID package;
  std::string repository;
  fs::path local;
};

struct PackageDeclaration {
  UUID package;
  std::string repository;
  std::vector<Requirement> requirements;
};

inline PackageDeclaration read_project(const fs::path &directory) {
  const auto file = directory / "CodProject.yaml";
  try {
    yaml::Document document(file.string());
    auto_region<CodProject> region(1024 * 1024);
    auto *project = region->root().get();
    from_yaml(*region, document.root(), project);
    PackageDeclaration declaration{project->uuid(), std::string(project->repo_url()), {}};
    for (const auto &dependency : project->deps())
      declaration.requirements.push_back(
          {dependency.uuid(), std::string(dependency.repo_url()), fs::path(std::string(dependency.path()))});
    return declaration;
  } catch (const std::exception &error) {
    throw std::runtime_error("cannot read package declaration '" + file.string() + "': " + error.what());
  }
}

inline bool present(const fs::path &file) {
  std::error_code error;
  const auto status = fs::symlink_status(file, error);
  if (status.type() == fs::file_type::not_found) return false;
  if (error) throw fs::filesystem_error("cannot inspect package input", file, error);
  return fs::exists(status);
}

inline fs::path source_directory(const fs::path &path) {
  if (path.empty()) throw std::runtime_error("empty package source directory");
  const auto canonical = fs::canonical(path);
  if (!fs::is_directory(canonical))
    throw std::runtime_error("package source is not a directory: '" + canonical.string() + "'");
  return canonical;
}

struct Lock {
  std::map<UUID, fs::path> locals;
  std::map<UUID, std::string> remote;
};

inline Lock read_lock(const fs::path &root, const PackageDeclaration &declaration) {
  const auto file = root / "CodManifest.yaml";
  if (!present(file)) {
    if (declaration.requirements.empty()) return {};
    throw std::runtime_error("missing CodManifest.yaml for package '" + root.string() + "'");
  }
  try {
    yaml::Document document(file.string());
    auto_region<CodManifest> region(1024 * 1024);
    auto *manifest = region->root().get();
    from_yaml(*region, document.root(), manifest);
    if (manifest->root_uuid() != declaration.package || std::string_view(manifest->root_repo_url()) != declaration.repository)
      throw std::runtime_error("manifest root does not match CodProject.yaml");
    Lock lock;
    for (const auto &[key, value] : manifest->locals()) {
      const auto package = UUID(std::string_view(key));
      const auto path = fs::path(std::string(value));
      if (path.empty() || package == declaration.package ||
          !lock.locals.emplace(package, source_directory(root / path)).second)
        throw std::runtime_error("invalid local resolution for package " + package.to_string());
    }
    for (const auto &entry : manifest->resolved()) {
      const auto package = entry.uuid();
      if (package == declaration.package || lock.locals.contains(package) ||
          !lock.remote.emplace(package, std::string(entry.commit())).second)
        throw std::runtime_error("competing manifest resolutions for package " + package.to_string());
    }
    return lock;
  } catch (const std::exception &error) {
    throw std::runtime_error("cannot read package manifest '" + file.string() + "': " + error.what());
  }
}

} // namespace module_inputs

inline std::optional<std::filesystem::path> find_source_project(const std::filesystem::path &entry) {
  auto directory = std::filesystem::canonical(entry).parent_path();
  while (!directory.empty()) {
    if (module_inputs::present(directory / "CodProject.yaml")) return directory;
    const auto parent = directory.parent_path();
    if (parent == directory) break;
    directory = parent;
  }
  return {};
}

inline std::vector<PackageSourceRoot> checked_package_sources(const std::filesystem::path &project_root) {
  using namespace module_inputs;
  const auto root = source_directory(project_root);
  auto root_project = read_project(root);
  const auto lock = read_lock(root, root_project);
  struct PackageVisit {
    PackageSourceRoot source;
    std::vector<Requirement> requirements;
  };
  std::vector<PackageVisit> visits;
  visits.push_back({{root_project.package, root}, std::move(root_project.requirements)});
  std::set<UUID> discovered{root_project.package};
  std::map<UUID, std::string> required_repositories;
  for (size_t next = 0; next < visits.size(); ++next) {
    // Appending a child can grow the visit list.
    const auto directory = visits[next].source.directory;
    const auto requirements = std::move(visits[next].requirements);
    for (const auto &requirement : requirements) {
      const auto id = requirement.package.to_string();
      if (!requirement.repository.empty()) {
        const auto [known, inserted] = required_repositories.emplace(requirement.package, requirement.repository);
        if (!inserted && known->second != requirement.repository)
          throw std::runtime_error("competing repository requirements for package " + id);
      }
      const auto local = lock.locals.find(requirement.package);
      if (local == lock.locals.end()) {
        const auto remote = lock.remote.find(requirement.package);
        if (remote == lock.remote.end())
          throw std::runtime_error("no manifest resolution for package " + id);
        if (remote->second.empty())
          throw std::runtime_error("remote package has no pinned commit: " + id);
        throw std::runtime_error("prepared remote source cannot yet be verified for package " + id);
      }
      if (!requirement.local.empty() && source_directory(directory / requirement.local) != local->second)
        throw std::runtime_error("manifest does not match the declared local override for package " + id);
      if (!discovered.insert(requirement.package).second) continue;
      auto declaration = read_project(local->second);
      if (declaration.package != requirement.package)
        throw std::runtime_error("local package UUID does not match " + id + " at '" + local->second.string() + "'");
      visits.push_back({{declaration.package, local->second}, std::move(declaration.requirements)});
    }
  }
  if (visits.size() - 1 != lock.locals.size() || !lock.remote.empty())
    throw std::runtime_error("manifest contains sources outside the project's dependency closure");
  std::vector<PackageSourceRoot> sources;
  sources.reserve(visits.size());
  for (auto &visit : visits) sources.push_back(std::move(visit.source));
  return sources;
}

// The private host exchange carries raw native path bytes with explicit sizes.
// Construct the complete payload before writing anything to the channel.
inline std::string encode_package_sources(const std::vector<PackageSourceRoot> &sources) {
  std::string result = "CODPKG01";
  const auto word = [&](std::uint64_t value) {
    for (unsigned shift = 0; shift != 64; shift += 8)
      result.push_back(static_cast<char>((value >> shift) & 0xff));
  };
  word(sources.size());
  for (const auto &source : sources) {
    result += source.package.to_string(); // The UUID spelling is always 36 ASCII bytes.
    const auto bytes = source.directory.u8string();
    word(bytes.size());
    result.append(reinterpret_cast<const char *>(bytes.data()), bytes.size());
  }
  return result;
}

} // namespace cod::project
