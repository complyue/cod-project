#pragma once

#include "shilos.hh"

namespace cod::project {

using namespace shilos;

class CodDep {
public:
  UUID uuid_;
  regional_str name_;
  regional_str repo_url_;
  regional_list<regional_str> branches_;

  template <typename RT>
  CodDep(memory_region<RT> &mr, const UUID &uuid, std::string_view name, std::string_view repo_url)
      : uuid_(uuid), name_(mr, name), repo_url_(mr, repo_url), branches_(mr) {}

  // Deleted special members
  CodDep(const CodDep &) = delete;
  CodDep(CodDep &&) = delete;
  CodDep &operator=(const CodDep &) = delete;
  CodDep &operator=(CodDep &&) = delete;

  // YAML functionality moved to codp_yaml.hh
};

class CodProject {
public:
  static constexpr UUID TYPE_UUID = UUID("9B27863B-8997-4158-AC34-38512484EDFB");

protected:
  UUID uuid_;
  regional_str name_;
  regional_list<CodDep> deps_;

public:
  template <typename RT> CodProject(memory_region<RT> &mr, std::string_view name) : uuid_(), name_(mr, name), deps_(){};

  template <typename RT>
  CodProject(memory_region<RT> &mr, const UUID &uuid, std::string_view name) : uuid_(uuid), name_(mr, name), deps_(){};

  // Deleted special members
  CodProject(const CodProject &) = delete;
  CodProject(CodProject &&) = delete;
  CodProject &operator=(const CodProject &) = delete;
  CodProject &operator=(CodProject &&) = delete;

  UUID uuid() const { return uuid_; }

  regional_str &name() { return name_; }
  const regional_str &name() const { return name_; }

  regional_list<CodDep> &deps() { return deps_; }
  const regional_list<CodDep> &deps() const { return deps_; }

  template <typename RT>
  void addDep(memory_region<RT> &mr, const UUID &uuid, std::string_view name, std::string_view repo_url) {
    append_to(deps_, mr, uuid, name, repo_url);
  }

  // YAML functionality moved to codp_yaml.hh
};

} // namespace cod::project
