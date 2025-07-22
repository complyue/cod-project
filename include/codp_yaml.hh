#pragma once

#include "codp.hh"

#include "shilos/dict_yaml.hh"   // IWYU pragma: keep
#include "shilos/list_yaml.hh"   // IWYU pragma: keep
#include "shilos/str_yaml.hh"    // IWYU pragma: keep
#include "shilos/vector_yaml.hh" // IWYU pragma: keep

namespace cod::project {

// ==========================================================================
// YAML SERIALISATION IMPLEMENTATION FOR CodDep & CodProject
// --------------------------------------------------------------------------

inline yaml::Node to_yaml(const CodDep &dep, yaml::YamlAuthor &author) {
  auto m = author.createMap();
  author.setMapValue(m, "uuid", author.createString(dep.uuid().to_string()));
  author.setMapValue(m, "name", author.createString(std::string_view(dep.name())));
  author.setMapValue(m, "repo_url", author.createString(std::string_view(dep.repo_url())));
  if (!dep.path().empty()) {
    author.setMapValue(m, "path", author.createString(std::string_view(dep.path())));
  }

  if (!dep.branches().empty()) {
    auto seq = author.createSequence();
    for (const auto &br : dep.branches()) {
      author.pushToSequence(seq, author.createString(std::string_view(br)));
    }
    author.setMapValue(m, "branches", seq);
  }
  return m;
}

inline yaml::Node to_yaml(const CodProject &proj, yaml::YamlAuthor &author) {
  auto m = author.createMap();
  author.setMapValue(m, "uuid", author.createString(proj.uuid().to_string()));
  author.setMapValue(m, "name", author.createString(std::string_view(proj.name())));
  author.setMapValue(m, "repo_url", author.createString(std::string_view(proj.repo_url())));

  if (!proj.branches().empty()) {
    auto seq = author.createSequence();
    for (const auto &br : proj.branches()) {
      author.pushToSequence(seq, author.createString(std::string_view(br)));
    }
    author.setMapValue(m, "branches", seq);
  }

  if (!proj.deps().empty()) {
    auto seq = author.createSequence();
    for (const CodDep &d : proj.deps()) {
      author.pushToSequence(seq, to_yaml(d, author));
    }
    author.setMapValue(m, "deps", seq);
  }
  return m;
}

// --------------------------------------------------------------------------
// Deserialisation – free functions picked up by ADL.
// --------------------------------------------------------------------------

template <typename RT>
  requires(shilos::ValidMemRegionRootType<RT>)
void from_yaml(shilos::memory_region<RT> &mr, const yaml::Node &node, CodDep *raw_ptr) {
  if (!node.IsMap()) {
    throw yaml::TypeError("CodDep YAML node must be a mapping");
  }
  const auto &map = std::get<yaml::Map>(node.value);

  auto fetch_scalar = [&](const std::string &key) -> std::string {
    auto it = map.find(key);
    if (it == map.end()) {
      throw yaml::MissingFieldError("Missing key '" + key + "' in CodDep");
    }
    if (!it->value.IsScalar()) {
      std::string actual_type =
          (it->value.IsNull() ? "null"
                              : (it->value.IsMap() ? "map" : (it->value.IsSequence() ? "sequence" : "non-scalar")));
      throw yaml::TypeError("Expected scalar for key '" + key + "' in CodDep, got " + actual_type +
                            " with value: " + yaml::format_yaml(it->value));
    }
    try {
      return it->value.asString();
    } catch (const std::exception &e) {
      throw yaml::TypeError("Failed to parse string value for key '" + key + "' in CodDep: " + e.what());
    }
  };

  UUID uuid(fetch_scalar("uuid"));
  std::string name = fetch_scalar("name");
  std::string repo_url = fetch_scalar("repo_url");

  // Optional path
  std::string path;
  auto it_path = map.find("path");
  if (it_path != map.end()) {
    if (!it_path->value.IsScalar()) {
      throw yaml::TypeError("'path' must be a scalar");
    }
    path = it_path->value.asString();
  }

  // Construct in-place.
  new (raw_ptr) CodDep(mr, uuid, name, repo_url, path);

  // Optional branches.
  auto it_br = map.find("branches");
  if (it_br != map.end()) {
    if (!it_br->value.IsSequence()) {
      throw yaml::TypeError("'branches' must be a sequence");
    }
    for (const auto &br_node : std::get<yaml::Sequence>(it_br->value.value)) {
      raw_ptr->branches().enque(mr, br_node.asString());
    }
  }
}

template <typename RT>
  requires(shilos::ValidMemRegionRootType<RT>)
void from_yaml(shilos::memory_region<RT> &mr, const yaml::Node &node, CodProject *raw_ptr) {
  if (!node.IsMap()) {
    throw yaml::TypeError("CodProject YAML root must be a mapping");
  }
  const auto &map = std::get<yaml::Map>(node.value);

  auto fetch_scalar = [&](const std::string &key) -> std::string {
    auto it = map.find(key);
    if (it == map.end()) {
      throw yaml::MissingFieldError("Missing key '" + key + "' in CodProject");
    }
    if (!it->value.IsScalar()) {
      std::string actual_type =
          (it->value.IsNull() ? "null"
                              : (it->value.IsMap() ? "map" : (it->value.IsSequence() ? "sequence" : "non-scalar")));
      throw yaml::TypeError("Expected scalar for key '" + key + "' in CodProject, got " + actual_type +
                            " with value: " + yaml::format_yaml(it->value));
    }
    try {
      return it->value.asString();
    } catch (const std::exception &e) {
      throw yaml::TypeError("Failed to parse string value for key '" + key + "' in CodProject: " + e.what());
    }
  };

  UUID uuid(fetch_scalar("uuid"));
  std::string name = fetch_scalar("name");
  std::string repo_url = fetch_scalar("repo_url");

  // Construct CodProject in-place.
  new (raw_ptr) CodProject(mr, uuid, name, repo_url);

  // branches sequence (optional)
  auto it_branches = map.find("branches");
  if (it_branches != map.end()) {
    if (!it_branches->value.IsSequence()) {
      throw yaml::TypeError("'branches' must be a sequence");
    }
    for (const auto &br_node : std::get<yaml::Sequence>(it_branches->value.value)) {
      raw_ptr->branches().enque(mr, br_node.asString());
    }
  }

  // deps sequence (optional)
  auto it_deps = map.find("deps");
  if (it_deps != map.end()) {
    if (!it_deps->value.IsSequence()) {
      throw yaml::TypeError("'deps' must be a sequence");
    }
    for (const auto &dep_node : std::get<yaml::Sequence>(it_deps->value.value)) {
      // Allocate CodDep via from_yaml helper.
      raw_ptr->deps().emplace_init(mr, [&](CodDep *dst) { from_yaml(mr, dep_node, dst); });
    }
  }
}

// ---------------------------------------------------------------------------
// Helper utilities (non-member)
// ---------------------------------------------------------------------------

inline std::string repo_url_to_key(std::string_view url) {
  std::string key;
  key.reserve(url.size());
  for (char c : url) {
    switch (c) {
    case ':':
    case '/':
    case '\\':
    case '.':
    case '@':
      key += '_';
      break;
    default:
      key += c;
    }
  }
  return key;
}

// ==========================================================================
// YAML SERIALISATION FOR MANIFEST CLASSES
// --------------------------------------------------------------------------

inline yaml::Node to_yaml(const CodManifestEntry &entry, yaml::YamlAuthor &author) {
  auto m = author.createMap();
  author.setMapValue(m, "uuid", author.createString(entry.uuid().to_string()));
  author.setMapValue(m, "repo_url", author.createString(std::string_view(entry.repo_url())));
  if (!entry.branch().empty()) {
    author.setMapValue(m, "branch", author.createString(std::string_view(entry.branch())));
  }
  if (!entry.commit().empty()) {
    author.setMapValue(m, "commit", author.createString(std::string_view(entry.commit())));
  }
  return m;
}

inline yaml::Node to_yaml(const CodManifest &manifest, yaml::YamlAuthor &author) {
  auto m = author.createMap();

  // Root section
  auto root_map = author.createMap();
  author.setMapValue(root_map, "uuid", author.createString(manifest.root_uuid().to_string()));
  author.setMapValue(root_map, "repo_url", author.createString(std::string_view(manifest.root_repo_url())));
  author.setMapValue(m, "root", root_map);

  // Locals section
  if (!manifest.locals().empty()) {
    auto locals_map = author.createMap();
    for (const auto &[uuid_str, path_str] : manifest.locals()) {
      author.setMapValue(locals_map, std::string_view(uuid_str), author.createString(std::string_view(path_str)));
    }
    author.setMapValue(m, "locals", locals_map);
  }

  // Resolved section
  if (!manifest.resolved().empty()) {
    auto resolved_seq = author.createSequence();
    for (const CodManifestEntry &entry : manifest.resolved()) {
      author.pushToSequence(resolved_seq, to_yaml(entry, author));
    }
    author.setMapValue(m, "resolved", resolved_seq);
  }

  return m;
}

// --------------------------------------------------------------------------
// Deserialisation for manifest classes
// --------------------------------------------------------------------------

template <typename RT>
  requires(shilos::ValidMemRegionRootType<RT>)
void from_yaml(shilos::memory_region<RT> &mr, const yaml::Node &node, CodManifestEntry *raw_ptr) {
  if (!node.IsMap()) {
    throw yaml::TypeError("CodManifestEntry YAML node must be a mapping");
  }
  const auto &map = std::get<yaml::Map>(node.value);

  auto fetch_scalar = [&](const std::string &key) -> std::string {
    auto it = map.find(key);
    if (it == map.end()) {
      throw yaml::MissingFieldError("Missing key '" + key + "' in CodManifestEntry");
    }
    if (!it->value.IsScalar()) {
      throw yaml::TypeError("Expected scalar for key '" + key + "'");
    }
    return it->value.asString();
  };

  auto fetch_optional_scalar = [&](const std::string &key) -> std::string {
    auto it = map.find(key);
    if (it == map.end()) {
      return "";
    }
    if (!it->value.IsScalar()) {
      throw yaml::TypeError("Expected scalar for key '" + key + "'");
    }
    return it->value.asString();
  };

  UUID uuid(fetch_scalar("uuid"));
  std::string repo_url = fetch_scalar("repo_url");
  std::string branch = fetch_optional_scalar("branch");
  std::string commit = fetch_optional_scalar("commit");

  // Construct in-place
  new (raw_ptr) CodManifestEntry(mr, uuid, repo_url, branch, commit);
}

template <typename RT>
  requires(shilos::ValidMemRegionRootType<RT>)
void from_yaml(shilos::memory_region<RT> &mr, const yaml::Node &node, CodManifest *raw_ptr) {
  if (!node.IsMap()) {
    throw yaml::TypeError("CodManifest YAML root must be a mapping");
  }
  const auto &map = std::get<yaml::Map>(node.value);

  // Parse root section
  auto it_root = map.find("root");
  if (it_root == map.end()) {
    throw yaml::MissingFieldError("Missing 'root' section in CodManifest");
  }
  if (!it_root->value.IsMap()) {
    throw yaml::TypeError("'root' must be a mapping");
  }
  const auto &root_map = std::get<yaml::Map>(it_root->value.value);

  auto fetch_scalar_from_map = [&](const yaml::Map &m, const std::string &key) -> std::string {
    auto it = m.find(key);
    if (it == m.end()) {
      throw yaml::MissingFieldError("Missing key '" + key + "' in root section");
    }
    if (!it->value.IsScalar()) {
      std::string actual_type =
          (it->value.IsNull() ? "null"
                              : (it->value.IsMap() ? "map" : (it->value.IsSequence() ? "sequence" : "non-scalar")));
      throw yaml::TypeError("Expected scalar for key '" + key + "' in root section, got " + actual_type +
                            " with value: " + yaml::format_yaml(it->value));
    }
    try {
      return it->value.asString();
    } catch (const std::exception &e) {
      throw yaml::TypeError("Failed to parse string value for key '" + key + "' in root section: " + e.what());
    }
  };

  UUID root_uuid(fetch_scalar_from_map(root_map, "uuid"));
  std::string root_repo_url = fetch_scalar_from_map(root_map, "repo_url");

  // Construct CodManifest in-place
  new (raw_ptr) CodManifest(mr, root_uuid, root_repo_url);

  // Parse locals section (optional)
  auto it_locals = map.find("locals");
  if (it_locals != map.end()) {
    if (!it_locals->value.IsMap()) {
      throw yaml::TypeError("'locals' must be a mapping");
    }
    const auto &locals_map = std::get<yaml::Map>(it_locals->value.value);
    for (const auto &[uuid_str, path_node] : locals_map) {
      if (!path_node.IsScalar()) {
        throw yaml::TypeError("local path must be a scalar");
      }
      std::string path_str = path_node.asString();
      UUID uuid(uuid_str);
      raw_ptr->addLocal(mr, uuid, path_str);
    }
  }

  // Parse resolved section (optional)
  auto it_resolved = map.find("resolved");
  if (it_resolved != map.end()) {
    if (!it_resolved->value.IsSequence()) {
      throw yaml::TypeError("'resolved' must be a sequence");
    }
    for (const auto &entry_node : std::get<yaml::Sequence>(it_resolved->value.value)) {
      // Allocate CodManifestEntry via from_yaml helper
      raw_ptr->resolved().emplace_init(mr, [&](CodManifestEntry *dst) { from_yaml(mr, entry_node, dst); });
    }
  }
}

} // namespace cod::project
