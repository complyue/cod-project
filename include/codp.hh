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

  yaml::Node to_yaml() const noexcept {
    yaml::Node node;
    auto &map = std::get<yaml::Map>(node.value = yaml::Map{});

    map.emplace("uuid", uuid_.to_string());
    map.emplace("name", std::string(static_cast<std::string_view>(name_)));
    if (!repo_url_.empty()) {
      map.emplace("repo_url", std::string(static_cast<std::string_view>(repo_url_)));
    }

    yaml::Node branches_node;
    auto &branches_seq = std::get<yaml::Sequence>(branches_node.value = yaml::Sequence{});
    for (const auto &branch : branches_) {
      yaml::Node branch_node;
      branch_node.value = std::string(static_cast<std::string_view>(branch));
      branches_seq.push_back(branch_node);
    }
    map.emplace("branches", branches_node);

    return node;
  }

  template <typename RT> static global_ptr<CodDep, RT> from_yaml(memory_region<RT> &mr, const yaml::Node &node) {
    if (!std::holds_alternative<yaml::Map>(node.value)) {
      throw yaml::TypeError("Expected mapping for CodDep");
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

    std::string_view repo_url;
    if (auto repo_url_it = map.find("repo_url"); repo_url_it != map.end()) {
      repo_url = std::get<std::string>(repo_url_it->second.value);
    }

    auto dep = mr.template create<CodDep>(uuid, name, repo_url);

    auto branches_it = map.find("branches");
    if (branches_it != map.end() && std::holds_alternative<yaml::Sequence>(branches_it->second.value)) {
      for (const auto &branch_node : std::get<yaml::Sequence>(branches_it->second.value)) {
        if (std::holds_alternative<std::string>(branch_node.value)) {
          append_to(dep->branches_, mr, std::get<std::string>(branch_node.value));
        }
      }
    }

    return dep;
  }
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

  yaml::Node to_yaml() const noexcept {
    yaml::Node node;
    auto &map = std::get<yaml::Map>(node.value = yaml::Map{});

    map.emplace("uuid", uuid_.to_string());
    map.emplace("name", std::string(static_cast<std::string_view>(name_)));

    yaml::Node deps_node;
    auto &deps_seq = std::get<yaml::Sequence>(deps_node.value = yaml::Sequence{});
    for (const auto &dep : deps_) {
      yaml::Node dep_node;
      auto &dep_map = std::get<yaml::Map>(dep_node.value = yaml::Map{});
      dep_map.emplace("uuid", dep.uuid_.to_string());
      dep_map.emplace("name", std::string(static_cast<std::string_view>(dep.name_)));
      dep_map.emplace("repo_url", std::string(static_cast<std::string_view>(dep.repo_url_)));

      yaml::Node branches_node;
      auto &branches_seq = std::get<yaml::Sequence>(branches_node.value = yaml::Sequence{});
      for (const auto &branch : dep.branches_) {
        yaml::Node branch_node;
        branch_node.value = std::string(static_cast<std::string_view>(branch));
        branches_seq.push_back(branch_node);
      }
      dep_map.emplace("branches", branches_node);

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

    auto project = mr.template create<CodProject>(uuid, name);

    auto deps_it = map.find("dependencies");
    // several formats for dependencies:
    // * a sequence of:
    //   - single uuid str
    //   - <name>=<uuid>
    //   - <name>=<uuid>:<repo_url>
    //   - <name>=<uuid>:<repo_url>#<branch1>,<branch2>,...
    // * a map with name as key, with explicit uuid, repo_url and branches
    if (deps_it != map.end()) {
      const auto &deps_node = deps_it->second;

      // Handle sequence format
      if (std::holds_alternative<yaml::Sequence>(deps_node.value)) {
        const auto &seq = std::get<yaml::Sequence>(deps_node.value);
        for (const auto &dep : seq) {
          std::string dep_str = dep.as<std::string>();

          // Parse simple UUID format
          if (auto eq_pos = dep_str.find('='); eq_pos == std::string_view::npos) {
            project->add_dependency({}, UUID(dep_str));
          }
          // Parse name=uuid[:repo_url][#branches] format
          else {
            std::string name(dep_str.substr(0, eq_pos));
            std::string rest(dep_str.substr(eq_pos + 1));

            // Extract UUID (required)
            auto colon_pos = rest.find(':');
            auto hash_pos = rest.find('#');
            auto uuid_end = std::min(colon_pos, hash_pos);
            std::string uuid(rest.substr(0, uuid_end));

            // Extract optional repo URL
            std::string_view repo_url;
            if (colon_pos != std::string_view::npos && colon_pos < hash_pos) {
              repo_url = rest.substr(colon_pos + 1, hash_pos != std::string_view::npos ? hash_pos - colon_pos - 1
                                                                                       : std::string_view::npos);
            }

            // Extract optional branches
            std::vector<std::string_view> branches;
            if (hash_pos != std::string_view::npos) {
              auto branches_str = rest.substr(hash_pos + 1);
              size_t start = 0;
              size_t end = branches_str.find(',');
              while (start != std::string_view::npos) {
                branches.emplace_back(branches_str.substr(start, end - start));
                start = end == std::string_view::npos ? end : end + 1;
                end = branches_str.find(',', start);
              }
            }

            project->add_dependency(name, UUID(uuid), repo_url, branches);
          }
        }
      }
      // Handle map format
      else if (std::holds_alternative<yaml::Map>(deps_node.value)) {
        const auto &map = std::get<yaml::Map>(deps_node.value);
        for (const auto &[name, dep] : map) {
          auto uuid = dep.as<yaml::Map>().at("uuid").as<std::string>();
          std::string repo_url;
          const auto &dep_map = dep.as<yaml::Map>();
          if (auto repo_it = dep_map.find("repo_url"); repo_it != dep.as<yaml::Map>().end()) {
            repo_url = repo_it->second.as<std::string>();
          }

          std::vector<std::string> branches;
          if (auto branches_it = dep_map.find("branches"); branches_it != dep.as<yaml::Map>().end()) {
            for (const auto &branch : branches_it->second.as<yaml::Sequence>()) {
              branches.push_back(branch.as<std::string>());
            }
          }

          project->add_dependency(name, UUID(uuid), repo_url, branches);
        }
      } else {
        throw std::runtime_error("Invalid dependencies format - must be sequence or map");
      }
    }

    return project;
  }
};

} // namespace cod::project
