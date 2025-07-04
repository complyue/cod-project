#pragma once

#include "codp.hh"

namespace cod::project {

// Standalone YAML functions for CodDep
inline yaml::Node to_yaml(const CodDep &dep) noexcept {
  yaml::Node node;
  auto &map = std::get<yaml::Map>(node.value = yaml::Map{});

  map.emplace("uuid", dep.uuid().to_string());
  map.emplace("name", std::string(static_cast<std::string_view>(dep.name())));
  if (!dep.repo_url().empty()) {
    map.emplace("repo_url", std::string(static_cast<std::string_view>(dep.repo_url())));
  }

  yaml::Node branches_node;
  auto &branches_seq = std::get<yaml::Sequence>(branches_node.value = yaml::Sequence{});
  for (const auto &branch : dep.branches()) {
    yaml::Node branch_node;
    branch_node.value = std::string(static_cast<std::string_view>(branch));
    branches_seq.push_back(branch_node);
  }
  map.emplace("branches", branches_node);

  return node;
}

template <typename RT> global_ptr<CodDep, RT> from_yaml(memory_region<RT> &mr, const yaml::Node &node) {
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
        dep->branches().enque(mr, std::get<std::string>(branch_node.value));
      }
    }
  }

  return dep;
}

template <typename RT> void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_ptr<CodDep> &to_ptr) {
  // Allocate uninitialized CodDep
  auto dep_ptr = mr.template allocate<CodDep>();
  // Use raw pointer version to construct directly
  from_yaml(mr, node, dep_ptr.get());
  // Assign the raw pointer to the regional_ptr (assumes to_ptr is in memory region)
  to_ptr = dep_ptr.get();
}

// Standalone YAML functions for CodProject
inline yaml::Node to_yaml(const CodProject &project) noexcept {
  yaml::Node node;
  auto &map = std::get<yaml::Map>(node.value = yaml::Map{});

  map.emplace("uuid", project.uuid().to_string());
  map.emplace("name", std::string(static_cast<std::string_view>(project.name())));

  yaml::Node deps_node;
  auto &deps_seq = std::get<yaml::Sequence>(deps_node.value = yaml::Sequence{});
  for (const auto &dep : project.deps()) {
    deps_seq.push_back(to_yaml(dep));
  }
  map.emplace("dependencies", deps_node);

  return node;
}

template <typename RT> global_ptr<CodProject, RT> from_yaml(memory_region<RT> &mr, const yaml::Node &node) {
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
  if (deps_it != map.end()) {
    const auto &deps_node = deps_it->second;

    // Handle sequence format
    if (std::holds_alternative<yaml::Sequence>(deps_node.value)) {
      const auto &seq = std::get<yaml::Sequence>(deps_node.value);
      for (const auto &dep_node : seq) {
        // Try to deserialize as CodDep object first
        if (std::holds_alternative<yaml::Map>(dep_node.value)) {
          // Allocate uninitialized CodDep
          auto dep_ptr = mr.template allocate<CodDep>();
          // Use raw pointer version to construct directly
          from_yaml(mr, dep_node, dep_ptr.get());
          
          // Add the dependency with constructor arguments
          project->deps().enque(mr, dep_ptr->uuid(), static_cast<std::string_view>(dep_ptr->name()),
                                static_cast<std::string_view>(dep_ptr->repo_url()));
          // Access the newly created CodDep and copy branches
          CodDep *new_dep = project->deps().back();
          if (new_dep) {
            // Copy branches from parsed dep to new_dep
            for (const auto &branch : dep_ptr->branches()) {
              new_dep->branches().enque(mr, static_cast<std::string_view>(branch));
            }
          }
        } else {
          // Handle string format parsing (existing logic)
          std::string dep_str = dep_node.as<std::string>();

          // Parse simple UUID format
          if (auto eq_pos = dep_str.find('='); eq_pos == std::string_view::npos) {
            UUID dep_uuid(dep_str);
            project->deps().enque(mr, dep_uuid, "", "");
          }
          // Parse name=uuid[:repo_url][#branches] format
          else {
            std::string name(dep_str.substr(0, eq_pos));
            std::string rest(dep_str.substr(eq_pos + 1));

            // Extract UUID (required)
            auto colon_pos = rest.find(':');
            auto hash_pos = rest.find('#');
            auto uuid_end = std::min(colon_pos, hash_pos);
            std::string uuid_str(rest.substr(0, uuid_end));

            // Extract optional repo URL
            std::string_view repo_url;
            if (colon_pos != std::string_view::npos && colon_pos < hash_pos) {
              repo_url = rest.substr(colon_pos + 1, hash_pos != std::string_view::npos ? hash_pos - colon_pos - 1
                                                                                       : std::string_view::npos);
            }

            // Create CodDep directly in the project's deps container
            project->deps().enque(mr, UUID(uuid_str), name, repo_url);

            // Extract optional branches and add them to the newly created CodDep
            // We need to access the last added element - this is a design limitation
            // For now, branches from string format won't be supported
            if (hash_pos != std::string_view::npos) {
              // TODO: Need a way to access the last added CodDep to add branches
              // This is a known limitation of the current design
            }
          }
        }
      }
    }
    // Handle map format
    else if (std::holds_alternative<yaml::Map>(deps_node.value)) {
      const auto &deps_map = std::get<yaml::Map>(deps_node.value);
      for (const auto &[name, dep_node] : deps_map) {
        // Allocate uninitialized CodDep
        auto dep_ptr = mr.template allocate<CodDep>();
        // Use raw pointer version to construct directly
        from_yaml(mr, dep_node, dep_ptr.get());
        
        // Create CodDep directly in the project's deps container with parsed data
        project->deps().enque(mr, dep_ptr->uuid(), static_cast<std::string_view>(dep_ptr->name()),
                              static_cast<std::string_view>(dep_ptr->repo_url()));
        // Access the newly created CodDep and copy branches
        CodDep *new_dep = project->deps().back();
        if (new_dep) {
          // Copy branches from parsed dep to new_dep
          for (const auto &branch : dep_ptr->branches()) {
            new_dep->branches().enque(mr, static_cast<std::string_view>(branch));
          }
        }
      }
    }
  }

  return project;
}

template <typename RT> void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_ptr<CodProject> &to_ptr) {
  // Allocate uninitialized CodProject
  auto project_ptr = mr.template allocate<CodProject>();
  // Use raw pointer version to construct directly
  from_yaml(mr, node, project_ptr.get());
  // Assign the raw pointer to the regional_ptr (assumes to_ptr is in memory region)
  to_ptr = project_ptr.get();
}

template <typename RT> void from_yaml(memory_region<RT> &mr, const yaml::Node &node, CodDep *raw_ptr) {
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

  new (raw_ptr) CodDep(mr, uuid, name, repo_url);

  auto branches_it = map.find("branches");
  if (branches_it != map.end() && std::holds_alternative<yaml::Sequence>(branches_it->second.value)) {
    for (const auto &branch_node : std::get<yaml::Sequence>(branches_it->second.value)) {
      if (std::holds_alternative<std::string>(branch_node.value)) {
        raw_ptr->branches().enque(mr, std::get<std::string>(branch_node.value));
      }
    }
  }
}

template <typename RT> void from_yaml(memory_region<RT> &mr, const yaml::Node &node, CodProject *raw_ptr) {
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

  new (raw_ptr) CodProject(mr, uuid, name);

  auto deps_it = map.find("dependencies");
  if (deps_it != map.end()) {
    const auto &deps_node = deps_it->second;

    // Handle sequence format
    if (std::holds_alternative<yaml::Sequence>(deps_node.value)) {
      const auto &seq = std::get<yaml::Sequence>(deps_node.value);
      for (const auto &dep_node : seq) {
        // Use raw pointer version for direct construction
        if (std::holds_alternative<yaml::Map>(dep_node.value)) {
          // Allocate uninitialized CodDep
          auto dep_ptr = mr.template allocate<CodDep>();
          // Use raw pointer version to construct directly
          from_yaml(mr, dep_node, dep_ptr.get());
          
          // Add to dependencies using the constructed data
          raw_ptr->deps().enque(mr, dep_ptr->uuid(), static_cast<std::string_view>(dep_ptr->name()),
                                static_cast<std::string_view>(dep_ptr->repo_url()));

          // Copy branches to the new dependency
          CodDep *new_dep = raw_ptr->deps().back();
          if (new_dep) {
            for (const auto &branch : dep_ptr->branches()) {
              new_dep->branches().enque(mr, static_cast<std::string_view>(branch));
            }
          }
        } else {
          // Handle string format parsing (existing logic)
          std::string dep_str = dep_node.as<std::string>();

          // Parse simple UUID format
          if (auto eq_pos = dep_str.find('='); eq_pos == std::string_view::npos) {
            UUID dep_uuid(dep_str);
            raw_ptr->deps().enque(mr, dep_uuid, "", "");
          }
          // Parse name=uuid[:repo_url][#branches] format
          else {
            std::string name(dep_str.substr(0, eq_pos));
            std::string rest(dep_str.substr(eq_pos + 1));

            // Extract UUID (required)
            auto colon_pos = rest.find(':');
            auto hash_pos = rest.find('#');
            auto uuid_end = std::min(colon_pos, hash_pos);
            std::string uuid_str(rest.substr(0, uuid_end));

            // Extract optional repo URL
            std::string_view repo_url;
            if (colon_pos != std::string_view::npos && colon_pos < hash_pos) {
              repo_url = rest.substr(colon_pos + 1, hash_pos != std::string_view::npos ? hash_pos - colon_pos - 1
                                                                                       : std::string_view::npos);
            }

            // Create CodDep directly in the project's deps container
            raw_ptr->deps().enque(mr, UUID(uuid_str), name, repo_url);
          }
        }
      }
    }
    // Handle map format
    else if (std::holds_alternative<yaml::Map>(deps_node.value)) {
      const auto &deps_map = std::get<yaml::Map>(deps_node.value);
      for (const auto &[name, dep_node] : deps_map) {
        // Allocate uninitialized CodDep
        auto dep_ptr = mr.template allocate<CodDep>();
        // Use raw pointer version to construct directly
        from_yaml(mr, dep_node, dep_ptr.get());
        
        // Add to dependencies using the constructed data
        raw_ptr->deps().enque(mr, dep_ptr->uuid(), static_cast<std::string_view>(dep_ptr->name()),
                              static_cast<std::string_view>(dep_ptr->repo_url()));

        // Copy branches to the new dependency
        CodDep *new_dep = raw_ptr->deps().back();
        if (new_dep) {
          for (const auto &branch : dep_ptr->branches()) {
            new_dep->branches().enque(mr, static_cast<std::string_view>(branch));
          }
        }
      }
    }
  }
}

} // namespace cod::project
