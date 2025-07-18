#pragma once

#include "codp.hh"
#include "codp_yaml.hh"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace cod::project {
namespace fs = std::filesystem;

// Internal helper – read whole file into string.
inline std::string slurp_file(const fs::path &p) {
  std::ifstream ifs(p);
  if (!ifs) {
    throw std::runtime_error("Failed to open file: " + p.string());
  }
  std::stringstream ss;
  ss << ifs.rdbuf();
  return ss.str();
}

struct ManifestEntry {
  UUID uuid;
  std::string repo_url;
  std::string branch;
  std::string commit;
};

struct ManifestData {
  std::unordered_map<std::string, std::string> locals; // uuid -> path
  std::vector<ManifestEntry> resolved;
};

inline void collect_deps(const fs::path &proj_dir, const fs::path &root_dir, const CodProject *proj, ManifestData &out,
                         std::unordered_set<std::string> &visited) {
  for (const CodDep &dep : proj->deps()) {
    std::string uuid_str = dep.uuid().to_string();
    if (visited.contains(uuid_str))
      continue;
    visited.insert(uuid_str);

    if (!dep.path().empty()) {
      fs::path dep_path = std::string(std::string_view(dep.path()));
      if (dep_path.is_relative())
        dep_path = fs::absolute(proj_dir / dep_path);
      dep_path = fs::weakly_canonical(dep_path);

      // Store relative path from project root instead of absolute path
      fs::path relative_path = fs::relative(dep_path, root_dir);
      out.locals[uuid_str] = relative_path.string();

      // Load dep project YAML and recurse
      fs::path dep_yaml_path = dep_path / "CodProject.yaml";
      std::string yaml_text = slurp_file(dep_yaml_path);
      auto dep_result = yaml::YamlDocument::Parse(dep_yaml_path.string(), std::string(yaml_text));
      shilos::vswitch(
          dep_result,
          [&](const yaml::ParseError &err) {
            throw std::runtime_error("Failed to parse " + dep_yaml_path.string() + ": " + err.what());
          },
          [&](const yaml::YamlDocument &doc) {
            const yaml::Node &dep_root = doc.root();
            auto_region<CodProject> dep_region(1024 * 1024);
            CodProject *dep_proj = dep_region->root().get();
            from_yaml(*dep_region, dep_root, dep_proj);

            collect_deps(dep_path, root_dir, dep_proj, out, visited);
          });
    } else {
      // Remote dependency – record minimal information (branch/commit TBD)
      ManifestEntry me{dep.uuid(), std::string(std::string_view(dep.repo_url())), "", ""};
      out.resolved.push_back(std::move(me));
    }
  }
}

inline yaml::YamlDocument generate_manifest(const fs::path &project_dir) {
  std::string yaml_text = slurp_file(project_dir / "CodProject.yaml");
  auto root_result = yaml::YamlDocument::Parse((project_dir / "CodProject.yaml").string(), std::string(yaml_text));
  return shilos::vswitch(
      root_result,
      [&](const yaml::ParseError &err) -> yaml::YamlDocument {
        throw std::runtime_error("Failed to parse " + (project_dir / "CodProject.yaml").string() + ": " + err.what());
      },
      [&](const yaml::YamlDocument &doc) -> yaml::YamlDocument {
        const yaml::Node &root_node = doc.root();
        auto_region<CodProject> region(1024 * 1024);
        CodProject *project = region->root().get();
        from_yaml(*region, root_node, project);

        ManifestData data;
        std::unordered_set<std::string> visited;
        collect_deps(project_dir, project_dir, project, data, visited);

        // Use YamlAuthor to create the manifest document
        auto author_result = yaml::YamlDocument::Write("manifest.yaml", [&](yaml::YamlAuthor &author) -> yaml::Node {
          yaml::Node manifest = author.create_map();
          yaml::Node root_map = author.create_map();

          // Use YamlAuthor to create string nodes
          root_map["uuid"] = author.create_string(project->uuid().to_string());
          root_map["repo_url"] = author.create_string(std::string_view(project->repo_url()));
          manifest["root"] = root_map;

          if (!data.locals.empty()) {
            yaml::Node locals_map = author.create_map();
            for (const auto &kv : data.locals) {
              std::string key_str = kv.first;
              std::string value_str = kv.second;
              locals_map[key_str] = author.create_string(value_str);
            }
            manifest["locals"] = locals_map;
          }

          yaml::Node resolved_seq = author.create_sequence();
          for (const auto &entry : data.resolved) {
            yaml::Node m = author.create_map();
            m["uuid"] = author.create_string(entry.uuid.to_string());
            m["repo_url"] = author.create_string(entry.repo_url);
            if (!entry.branch.empty())
              m["branch"] = author.create_string(entry.branch);
            if (!entry.commit.empty())
              m["commit"] = author.create_string(entry.commit);
            resolved_seq.push_back(m);
          }
          manifest["resolved"] = resolved_seq;
          return manifest;
        });

        // Handle the AuthorResult from Write() and return the document
        return shilos::vswitch(
            author_result,
            [&](const yaml::ParseError &err) -> yaml::YamlDocument {
              throw std::runtime_error(std::string("Failed to create manifest: ") + err.what());
            },
            [&](yaml::YamlDocument &authored_doc) -> yaml::YamlDocument { return std::move(authored_doc); });
      });
}

} // namespace cod::project
