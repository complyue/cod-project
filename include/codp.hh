#pragma once

#include "shilos.hh"

namespace cod::project {

using namespace shilos;

class CodProject {
protected:
  relativ_ptr<relativ_str> name_;

public:
  static constexpr UUID TYPE_UUID = UUID("9B27863B-8997-4158-AC34-38512484EDFB");

  template <typename RT> CodProject(memory_region<RT> *mr, std::string_view name) : name_(mr->intern(name)){};

  relativ_str &name() { return *name_; }
  const relativ_str &name() const { return *name_; }
};

} // namespace cod::project
