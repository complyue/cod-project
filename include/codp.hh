#pragma once

#include "shilos.hh" // IWYU pragma: keep

#include "shilos/list.hh"      // IWYU pragma: keep
#include "shilos/list_yaml.hh" // IWYU pragma: keep

#include "shilos/vector.hh"      // IWYU pragma: keep
#include "shilos/vector_yaml.hh" // IWYU pragma: keep

#include "shilos/dict.hh"      // IWYU pragma: keep
#include "shilos/dict_yaml.hh" // IWYU pragma: keep

namespace cod::project {

using namespace shilos;

class CodDep {
private:
  UUID uuid_;
  regional_str name_;
  regional_str repo_url_;
  regional_fifo<regional_str> branches_;

public:
  template <typename RT>
  CodDep(memory_region<RT> &mr, const UUID &uuid, std::string_view name, std::string_view repo_url)
      : uuid_(uuid), name_(mr, name), repo_url_(mr, repo_url), branches_(mr) {}

  // Deleted special members
  CodDep(const CodDep &) = delete;
  CodDep(CodDep &&) = delete;
  CodDep &operator=(const CodDep &) = delete;
  CodDep &operator=(CodDep &&) = delete;

  UUID uuid() const { return uuid_; }
  const regional_str &name() const { return name_; }
  const regional_str &repo_url() const { return repo_url_; }
  const regional_fifo<regional_str> &branches() const { return branches_; }
  regional_fifo<regional_str> &branches() { return branches_; }
};

class CodProject {
public:
  static constexpr UUID TYPE_UUID = UUID("9B27863B-8997-4158-AC34-38512484EDFB");

private:
  UUID uuid_;
  regional_str name_;
  regional_str repo_url_;
  regional_fifo<CodDep> deps_;

public:
  template <typename RT>
  CodProject(memory_region<RT> &mr, const UUID &uuid, std::string_view name, std::string_view repo_url)
      : uuid_(uuid), name_(mr, name), repo_url_(mr, repo_url), deps_(mr) {}

  template <typename RT> CodProject(memory_region<RT> &mr) : uuid_(), name_(), repo_url_(), deps_(mr) {}

  // Deleted special members
  CodProject(const CodProject &) = delete;
  CodProject(CodProject &&) = delete;
  CodProject &operator=(const CodProject &) = delete;
  CodProject &operator=(CodProject &&) = delete;

  UUID uuid() const { return uuid_; }

  regional_str &name() { return name_; }
  const regional_str &name() const { return name_; }

  regional_fifo<CodDep> &deps() { return deps_; }
  const regional_fifo<CodDep> &deps() const { return deps_; }

  regional_str &repo_url() { return repo_url_; }
  const regional_str &repo_url() const { return repo_url_; }

  // ---------------------------------------------------------------------
  // Convenience helpers to add & access dependencies ---------------------
  // ---------------------------------------------------------------------

  template <typename RT>
  CodDep &addDep(memory_region<RT> &mr, const UUID &uuid, std::string_view name, std::string_view repo_url) {
    deps_.enque(mr, uuid, name, repo_url);
    return *deps_.back();
  }

  // Unsafe if list empty – caller responsibility
  CodDep &lastDep() { return *deps_.back(); }
  const CodDep &lastDep() const { return *deps_.back(); }

  // -----------------------------------------------------------------------
  // YAML (de)serialisation helpers – implemented via free functions to
  // satisfy the yaml::YamlConvertible concept (ADL visible).
  // -----------------------------------------------------------------------

  template <typename RT> void reload_from_yaml(memory_region<RT> &mr, const yaml::Node &node) {
    // Destroy current contents then reconstruct in-place via from_yaml.
    this->~CodProject();
    from_yaml<CodProject>(mr, node, this);
  }
};

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
