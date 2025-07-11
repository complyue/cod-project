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

inline yaml::Node to_yaml(const CodDep &dep) noexcept {
  yaml::Node m(yaml::Map{});
  m["uuid"] = dep.uuid().to_string();
  m["name"] = std::string_view(dep.name());
  m["repo_url"] = std::string_view(dep.repo_url());

  if (!dep.branches().empty()) {
    yaml::Node seq(yaml::Sequence{});
    for (const auto &br : dep.branches()) {
      seq.push_back(std::string(std::string_view(br)));
    }
    m["branches"] = seq;
  }
  return m;
}

inline yaml::Node to_yaml(const CodProject &proj) noexcept {
  yaml::Node m(yaml::Map{});
  m["uuid"] = proj.uuid().to_string();
  m["name"] = std::string_view(proj.name());
  m["repo_url"] = std::string_view(proj.repo_url());

  if (!proj.deps().empty()) {
    yaml::Node seq(yaml::Sequence{});
    for (const CodDep &d : proj.deps()) {
      seq.push_back(to_yaml(d));
    }
    m["deps"] = seq;
  }
  return m;
}

// --------------------------------------------------------------------------
// Deserialisation – free functions picked up by ADL.
// --------------------------------------------------------------------------

template <typename T, typename RT>
  requires(std::same_as<T, CodDep> && shilos::ValidMemRegionRootType<RT>)
void from_yaml(shilos::memory_region<RT> &mr, const yaml::Node &node, T *raw_ptr) {
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
      throw yaml::TypeError("Expected scalar for key '" + key + "'");
    }
    return it->value.as<std::string>();
  };

  UUID uuid(fetch_scalar("uuid"));
  std::string name = fetch_scalar("name");
  std::string repo_url = fetch_scalar("repo_url");

  // Construct in-place.
  new (raw_ptr) T(mr, uuid, name, repo_url);

  // Optional branches.
  auto it_br = map.find("branches");
  if (it_br != map.end()) {
    if (!it_br->value.IsSequence()) {
      throw yaml::TypeError("'branches' must be a sequence");
    }
    for (const auto &br_node : std::get<yaml::Sequence>(it_br->value.value)) {
      raw_ptr->branches().enque(mr, br_node.as<std::string>());
    }
  }
}

template <typename T, typename RT>
  requires(std::same_as<T, CodProject> && shilos::ValidMemRegionRootType<RT>)
void from_yaml(shilos::memory_region<RT> &mr, const yaml::Node &node, T *raw_ptr) {
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
      throw yaml::TypeError("Expected scalar for key '" + key + "'");
    }
    return it->value.as<std::string>();
  };

  UUID uuid(fetch_scalar("uuid"));
  std::string name = fetch_scalar("name");
  std::string repo_url = fetch_scalar("repo_url");

  // Construct CodProject in-place.
  new (raw_ptr) T(mr, uuid, name, repo_url);

  // deps sequence (optional)
  auto it_deps = map.find("deps");
  if (it_deps != map.end()) {
    if (!it_deps->value.IsSequence()) {
      throw yaml::TypeError("'deps' must be a sequence");
    }
    for (const auto &dep_node : std::get<yaml::Sequence>(it_deps->value.value)) {
      // Allocate CodDep via from_yaml helper.
      raw_ptr->deps().emplace_init(mr, [&](CodDep *dst) { from_yaml<CodDep>(mr, dep_node, dst); });
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

} // namespace cod::project
