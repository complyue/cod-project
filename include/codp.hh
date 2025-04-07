#pragma once

#include "shilos.hh"

namespace cod::project {

using namespace shilos;

struct Version {
  int16_t major, minor, patch;
};

class VersionConstraint {
public:
  enum class Type { Exact, Least, Below };

  Type type_;
  Version version_;
};

class CodDep {
public:
  UUID uuid_;
  regional_str name_;
  regional_list<VersionConstraint> constraints_;
};

class CodProject {
public:
  static constexpr UUID TYPE_UUID = UUID("9B27863B-8997-4158-AC34-38512484EDFB");

protected:
  UUID uuid_;
  regional_str name_;
  Version version_;

  regional_ptr<regional_list<CodDep>> deps_;

public:
  template <typename RT>                                   //
  CodProject(memory_region<RT> &mr, std::string_view name) //
      : uuid_(), version_{1, 0, 0}, deps_() {
    mr.afford_to(name_, name);
  };

  template <typename RT, typename... Args>
    requires(std::is_constructible_v<VersionConstraint, Args> && ...)        //
  CodProject(memory_region<RT> &mr, const UUID &uuid, std::string_view name, //
             const Version version, Args &&...deps)
      : uuid_(uuid), version_(version), deps_() {
    mr.afford_to(name_, name);
    (..., append_to(deps_, mr, std::forward<Args>(deps)));
  };

  UUID uuid() const { return uuid_; }
  Version version() const { return version_; }

  regional_str &name() { return name_; }
  const regional_str &name() const { return name_; }

  regional_ptr<regional_list<CodDep>> &deps() { return deps_; }
  const regional_ptr<regional_list<CodDep>> &deps() const { return deps_; }

  template <typename RT, typename... Args> void addDep(memory_region<RT> &mr, CodDep dep) {
    //
    append_to(deps_, mr, dep);
  }

  //
};

} // namespace cod::project
