#pragma once

#include "shilos.hh"

namespace cod::project {

using namespace shilos;

class VersionConstraint {
public:
  enum class Type { Exact, Least, Below };

  Type type_;
  int16_t version_[3];
};

class CodDep {
public:
  regional_ptr<UUID> uuid_;
  regional_ptr<regional_str> name_;
  regional_list<VersionConstraint> constraints_;
};

class CodProject {
public:
  static constexpr UUID TYPE_UUID = UUID("9B27863B-8997-4158-AC34-38512484EDFB");

protected:
  UUID uuid_;
  regional_ptr<regional_str> name_;
  int16_t version_[3];

  regional_ptr<regional_list<CodDep>> deps_;

public:
  template <typename RT>                                     //
  CodProject(memory_region<RT> *mr, std::string_view name)   //
      : uuid_(), name_(mr->afford(name)), version_{1, 0, 0}, //
        deps_(mr->template create<regional_list<CodDep>>()){};

  template <typename RT, typename... Args>
    requires(std::is_constructible_v<VersionConstraint, Args> && ...)        //
  CodProject(memory_region<RT> *mr, const UUID &uuid, std::string_view name, //
             const int16_t version[3], Args &&...deps)
      : uuid_(uuid), name_(mr->afford(name)),         //
        version_{version[0], version[1], version[2]}, //
        deps_(mr->template create<regional_list<CodDep>>()) {
    (..., deps_->prepend(mr, std::forward<Args>(deps)));
  };

  regional_str &name() { return *name_; }
  const regional_str &name() const { return *name_; }
};

} // namespace cod::project
