#pragma once

#include "shilos.hh" // IWYU pragma: keep

#include "shilos/dict.hh"   // IWYU pragma: keep
#include "shilos/list.hh"   // IWYU pragma: keep
#include "shilos/vector.hh" // IWYU pragma: keep

namespace cod::project {

using namespace shilos;

class CodDep {
private:
  UUID uuid_;
  regional_str name_;
  regional_str repo_url_;
  regional_fifo<regional_str> branches_;
  regional_str path_; // NEW: optional local path for dependency

public:
  template <typename RT>
  CodDep(memory_region<RT> &mr, const UUID &uuid, std::string_view name, std::string_view repo_url,
         std::string_view path = "")
      : uuid_(uuid), name_(mr, name), repo_url_(mr, repo_url), branches_(mr), path_(mr, path) {}

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
  const regional_str &path() const { return path_; }
  bool has_path() const { return !path_.empty(); }
};

class CodProject {
public:
  static constexpr UUID TYPE_UUID = UUID("9B27863B-8997-4158-AC34-38512484EDFB");

private:
  UUID uuid_;
  regional_str name_;
  regional_str repo_url_;
  regional_fifo<regional_str> branches_;
  regional_fifo<CodDep> deps_;

public:
  template <typename RT>
  CodProject(memory_region<RT> &mr, const UUID &uuid, std::string_view name, std::string_view repo_url)
      : uuid_(uuid), name_(mr, name), repo_url_(mr, repo_url), branches_(mr), deps_(mr) {}

  template <typename RT> CodProject(memory_region<RT> &mr) : uuid_(), name_(), repo_url_(), branches_(mr), deps_(mr) {}

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

  regional_fifo<regional_str> &branches() { return branches_; }
  const regional_fifo<regional_str> &branches() const { return branches_; }
};

// ==========================================================================
// MANIFEST-RELATED CLASSES
// --------------------------------------------------------------------------

class CodManifestEntry {
public:
  static constexpr UUID TYPE_UUID = UUID("F1E2D3C4-B5A6-7890-FEDC-BA0987654321");

private:
  UUID uuid_;
  regional_str repo_url_;
  regional_str branch_;
  regional_str commit_;

public:
  // Default constructor for vector storage
  CodManifestEntry() = default;

  template <typename RT>
  CodManifestEntry(memory_region<RT> &mr, const UUID &uuid, std::string_view repo_url, std::string_view branch = "",
                   std::string_view commit = "")
      : uuid_(uuid), repo_url_(mr, repo_url), branch_(mr, branch), commit_(mr, commit) {}

  // Deleted special members
  CodManifestEntry(const CodManifestEntry &) = delete;
  CodManifestEntry(CodManifestEntry &&) = delete;
  CodManifestEntry &operator=(const CodManifestEntry &) = delete;
  CodManifestEntry &operator=(CodManifestEntry &&) = delete;

  UUID uuid() const { return uuid_; }
  const regional_str &repo_url() const { return repo_url_; }
  const regional_str &branch() const { return branch_; }
  const regional_str &commit() const { return commit_; }
  regional_str &branch() { return branch_; }
  regional_str &commit() { return commit_; }
};

class CodManifest {
public:
  static constexpr UUID TYPE_UUID = UUID("A1B2C3D4-E5F6-7890-ABCD-EF0123456789");

private:
  UUID root_uuid_;
  regional_str root_repo_url_;
  regional_dict<regional_str, regional_str> locals_; // uuid -> path
  regional_vector<CodManifestEntry> resolved_;

public:
  template <typename RT>
  CodManifest(memory_region<RT> &mr, const UUID &root_uuid, std::string_view root_repo_url)
      : root_uuid_(root_uuid), root_repo_url_(mr, root_repo_url), locals_(mr), resolved_(mr) {}

  template <typename RT>
  CodManifest(memory_region<RT> &mr) : root_uuid_(), root_repo_url_(mr), locals_(mr), resolved_(mr) {}

  // Deleted special members
  CodManifest(const CodManifest &) = delete;
  CodManifest(CodManifest &&) = delete;
  CodManifest &operator=(const CodManifest &) = delete;
  CodManifest &operator=(CodManifest &&) = delete;

  UUID root_uuid() const { return root_uuid_; }
  const regional_str &root_repo_url() const { return root_repo_url_; }
  const regional_dict<regional_str, regional_str> &locals() const { return locals_; }
  const regional_vector<CodManifestEntry> &resolved() const { return resolved_; }
  regional_dict<regional_str, regional_str> &locals() { return locals_; }
  regional_vector<CodManifestEntry> &resolved() { return resolved_; }

  // ---------------------------------------------------------------------
  // Convenience helpers to add & access manifest entries ----------------
  // ---------------------------------------------------------------------

  template <typename RT> void addLocal(memory_region<RT> &mr, const UUID &uuid, std::string_view path) {
    regional_str uuid_str(mr, uuid.to_string());
    regional_str path_str(mr, path);
    locals_.insert(mr, std::move(uuid_str), std::move(path_str));
  }

  template <typename RT>
  CodManifestEntry &addResolved(memory_region<RT> &mr, const UUID &uuid, std::string_view repo_url,
                                std::string_view branch = "", std::string_view commit = "") {
    resolved_.emplace_back(mr, uuid, repo_url, branch, commit);
    return resolved_.back();
  }
};

} // namespace cod::project
