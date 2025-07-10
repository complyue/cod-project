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
  regional_fifo<CodDep> deps_;

public:
  template <typename RT>
  CodProject(memory_region<RT> &mr, const UUID &uuid, std::string_view name)
      : uuid_(uuid), name_(mr, name), deps_(mr) {}

  template <typename RT> CodProject(memory_region<RT> &mr) : uuid_(), name_(mr), deps_(mr) {}

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

  template <typename RT>
  CodDep &addDep(memory_region<RT> &mr, const UUID &uuid, std::string_view name, std::string_view repo_url) {
    deps_.enque(mr, uuid, name, repo_url);
    return *deps_.back();
  }

  // Convenience: fetch last dependency added (unsafe if none present)
  CodDep &lastDep() { return *deps_.back(); }
  const CodDep &lastDep() const { return *deps_.back(); }
};

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
