#pragma once

#include "shilos.hh"

namespace cod::project {

using namespace shilos;

struct Version {
  int16_t major, minor, patch;

  Version(int16_t v1 = 0, int16_t v2 = 0, int16_t v3 = 0) : major(v1), minor(v2), patch(v3) {}

  Version(std::string_view str) : major(0), minor(0), patch(0) {
    size_t pos = 0;
    size_t next = str.find('.');

    if (next != std::string_view::npos) {
      major = std::stoi(std::string(str.substr(pos, next - pos)));
      pos = next + 1;
      next = str.find('.', pos);
      if (next != std::string_view::npos) {
        minor = std::stoi(std::string(str.substr(pos, next - pos)));
        pos = next + 1;
        patch = std::stoi(std::string(str.substr(pos)));
      } else {
        minor = std::stoi(std::string(str.substr(pos)));
      }
    } else {
      major = std::stoi(std::string(str.substr(pos)));
    }
  }

  std::string toString() const { return std::format("{}.{}.{}", major, minor, patch); }
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

  regional_list<CodDep> deps_;

public:
  template <typename RT>                                   //
  CodProject(memory_region<RT> &mr, std::string_view name) //
      : uuid_(), version_(1, 0, 0), deps_() {
    mr.afford_to(name_, name);
  };

  template <typename RT, typename... Args>
    requires(std::is_constructible_v<VersionConstraint, Args> && ...)        //
  CodProject(memory_region<RT> &mr, const UUID &uuid, std::string_view name, //
             const Version version, Args &&...deps)
      : uuid_(uuid), name_(mr, name), version_(version), deps_() {
    (..., append_to(deps_, mr, std::forward<Args>(deps)));
  };

  // Deleted special members
  CodProject(const CodProject &) = delete;
  CodProject(CodProject &&) = delete;
  CodProject &operator=(const CodProject &) = delete;
  CodProject &operator=(CodProject &&) = delete;

  UUID uuid() const { return uuid_; }
  Version version() const { return version_; }

  regional_str &name() { return name_; }
  const regional_str &name() const { return name_; }

  regional_list<CodDep> &deps() { return deps_; }
  const regional_list<CodDep> &deps() const { return deps_; }

  template <typename RT, typename... Args> void addDep(memory_region<RT> &mr, CodDep dep) {
    append_to(deps_, mr, dep); //
  }

  yaml::Node to_yaml() const noexcept {
    yaml::Node node;
    auto &map = std::get<yaml::Map>(node.value = yaml::Map{});

    map.emplace("uuid", uuid_.to_string());
    map.emplace("name", std::string(static_cast<std::string_view>(name_)));

    map.emplace("version", version_.toString());

    yaml::Node deps_node;
    auto &deps_seq = std::get<yaml::Sequence>(deps_node.value = yaml::Sequence{});
    for (const auto &dep : deps_) {
      yaml::Node dep_node;
      auto &dep_map = std::get<yaml::Map>(dep_node.value = yaml::Map{});
      dep_map.emplace("uuid", dep.uuid_.to_string());
      dep_map.emplace("name", std::string(static_cast<std::string_view>(dep.name_)));
      deps_seq.push_back(dep_node);
    }
    map.emplace("dependencies", deps_node);

    return node;
  }

  template <typename RT> static global_ptr<CodProject, RT> from_yaml(memory_region<RT> &mr, const yaml::Node &node) {
    if (!std::holds_alternative<yaml::Map>(node.value)) {
      throw yaml::TypeError("Expected mapping for CodProject");
    }

    const auto &map = std::get<yaml::Map>(node.value);

    auto uuid_it = map.find("uuid");
    if (uuid_it == map.end()) {
      throw yaml::MissingFieldError("Missing 'uuid' field");
    }
    UUID uuid(std::get<std::string>(uuid_it->second.value));

    auto name_it = map.find("name");
    if (name_it == map.end()) {
      throw yaml::MissingFieldError("Missing 'name' field");
    }
    std::string_view name = std::get<std::string>(name_it->second.value);

    auto version_it = map.find("version");
    if (version_it == map.end()) {
      throw yaml::MissingFieldError("Missing 'version' field");
    }
    const auto &version_node = version_it->second;
    if (!std::holds_alternative<std::string>(version_node.value)) {
      throw yaml::TypeError("Expected string for version");
    }
    Version version(std::get<std::string>(version_node.value));

    auto project = mr.template make<CodProject>(uuid, name, version);

    auto deps_it = map.find("dependencies");
    if (deps_it != map.end()) {
      if (!std::holds_alternative<yaml::Sequence>(deps_it->second.value)) {
        throw yaml::TypeError("Expected sequence for dependencies");
      }

      for (const auto &dep_node : std::get<yaml::Sequence>(deps_it->second.value)) {
        if (!std::holds_alternative<yaml::Map>(dep_node.value)) {
          throw yaml::TypeError("Expected mapping for dependency");
        }
        const auto &dep_map = std::get<yaml::Map>(dep_node.value);

        CodDep dep;
        dep.uuid_ = UUID(std::get<std::string>(dep_map.at("uuid").value));
        mr.afford_to(dep.name_, std::get<std::string>(dep_map.at("name").value));

        project->addDep(mr, dep);
      }
    }

    return project;
  }

  template <typename RT>
  static void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_ptr<CodProject> &to_ptr) {
    auto project = from_yaml(mr, node);
    to_ptr = project;
  }
};

} // namespace cod::project
