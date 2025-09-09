#pragma once

#include "codp.hh"

#include "shilos/dict_yaml.hh"   // IWYU pragma: keep
#include "shilos/list_yaml.hh"   // IWYU pragma: keep
#include "shilos/str_yaml.hh"    // IWYU pragma: keep
#include "shilos/vector_yaml.hh" // IWYU pragma: keep

namespace cod::project {

// ==========================================================================
// YAML SERIALISATION IMPLEMENTATION FOR CodDep & CodProject
// --------------------------------------------------------------------------

inline yaml::Node to_yaml(const CodDep &dep, yaml::YamlAuthor &author) {
  auto m = author.createMap();

  // Parse description comments into individual lines
  std::vector<std::string_view> description_lines;
  std::string desc_str = std::string(dep.description());
  if (!desc_str.empty()) {
    size_t start = 0;
    size_t end = desc_str.find('\n');
    while (end != std::string::npos) {
      std::string line = desc_str.substr(start, end - start);
      if (!line.empty()) {
        description_lines.push_back(author.createStringView("# " + line));
      }
      start = end + 1;
      end = desc_str.find('\n', start);
    }
    // Add last line
    std::string last_line = desc_str.substr(start);
    if (!last_line.empty()) {
      description_lines.push_back(author.createStringView("# " + last_line));
    }
  }

  // Parse field-specific trailing comments
  std::string_view name_comment;
  std::string name_comment_str = std::string(dep.name_comment());
  if (!name_comment_str.empty()) {
    name_comment = author.createStringView("# " + name_comment_str);
  }

  std::string_view repo_url_comment;
  std::string repo_url_comment_str = std::string(dep.repo_url_comment());
  if (!repo_url_comment_str.empty()) {
    repo_url_comment = author.createStringView("# " + repo_url_comment_str);
  }

  std::string_view path_comment;
  std::string path_comment_str = std::string(dep.path_comment());
  if (!path_comment_str.empty()) {
    path_comment = author.createStringView("# " + path_comment_str);
  }

  // Add UUID field with description
  author.setMapValue(m, "uuid", author.createString(dep.uuid().to_string()), description_lines, "");

  // Add name field with trailing comment if present
  author.setMapValue(m, "name", author.createString(std::string_view(dep.name())), {}, name_comment);

  // Add repo_url field with trailing comment if present
  author.setMapValue(m, "repo_url", author.createString(std::string_view(dep.repo_url())), {}, repo_url_comment);

  // Add path field if present with trailing comment
  if (!dep.path().empty()) {
    author.setMapValue(m, "path", author.createString(std::string_view(dep.path())), {}, path_comment);
  }

  // Add branches sequence if present
  if (!dep.branches().empty()) {
    auto seq = author.createDashSequence();
    for (const auto &br : dep.branches()) {
      author.pushToSequence(seq, author.createString(std::string_view(br)));
    }
    author.setMapValue(m, "branches", seq);
  }

  return m;
}

inline yaml::Node to_yaml(const CodProject &proj, yaml::YamlAuthor &author) {
  // Add document header comments
  std::string header_str = std::string(proj.header());
  if (!header_str.empty()) {
    size_t start = 0;
    size_t end = header_str.find('\n');
    while (end != std::string::npos) {
      std::string line = header_str.substr(start, end - start);
      if (!line.empty()) {
        author.addDocumentHeaderComment("# " + line);
      }
      start = end + 1;
      end = header_str.find('\n', start);
    }
    // Add last line
    std::string last_line = header_str.substr(start);
    if (!last_line.empty()) {
      author.addDocumentHeaderComment("# " + last_line);
    }
  }

  auto m = author.createMap();

  // Parse field-specific trailing comments
  std::string_view name_comment;
  std::string name_comment_str = std::string(proj.name_comment());
  if (!name_comment_str.empty()) {
    name_comment = author.createStringView("# " + name_comment_str);
  }

  std::string_view repo_url_comment;
  std::string repo_url_comment_str = std::string(proj.repo_url_comment());
  if (!repo_url_comment_str.empty()) {
    repo_url_comment = author.createStringView("# " + repo_url_comment_str);
  }

  // Add UUID field
  author.setMapValue(m, "uuid", author.createString(proj.uuid().to_string()));

  // Add name field with trailing comment if present
  author.setMapValue(m, "name", author.createString(std::string_view(proj.name())), {}, name_comment);

  // Add repo_url field with trailing comment if present
  author.setMapValue(m, "repo_url", author.createString(std::string_view(proj.repo_url())), {}, repo_url_comment);

  // Add branches sequence if present
  if (!proj.branches().empty()) {
    auto seq = author.createDashSequence();
    for (const auto &br : proj.branches()) {
      author.pushToSequence(seq, author.createString(std::string_view(br)));
    }
    author.setMapValue(m, "branches", seq);
  }

  // Add works.root_type if provided
  if (!proj.works_root_type_qualified().empty() || !proj.works_root_type_header().empty()) {
    auto works_map = author.createMap();
    auto rt_map = author.createMap();
    if (!proj.works_root_type_qualified().empty()) {
      author.setMapValue(rt_map, "qualified", author.createString(std::string_view(proj.works_root_type_qualified())));
    }
    if (!proj.works_root_type_header().empty()) {
      author.setMapValue(rt_map, "header", author.createString(std::string_view(proj.works_root_type_header())));
    }
    if (!std::get<yaml::Map>(rt_map.value).empty()) {
      author.setMapValue(works_map, "root_type", rt_map);
    }
    if (!std::get<yaml::Map>(works_map.value).empty()) {
      author.setMapValue(m, "works", works_map);
    }
  }

  // Add repl.scope if provided
  if (!proj.repl_scope().empty()) {
    auto repl_map = author.createMap();
    author.setMapValue(repl_map, "scope", author.createString(std::string_view(proj.repl_scope())));
    author.setMapValue(m, "repl", repl_map);
  }

  // Add dependencies sequence if present
  if (!proj.deps().empty()) {
    auto seq = author.createDashSequence();
    for (const CodDep &d : proj.deps()) {
      author.pushToSequence(seq, to_yaml(d, author));
    }
    author.setMapValue(m, "deps", seq);
  }

  return m;
}

// --------------------------------------------------------------------------
// Deserialisation – free functions picked up by ADL.
// --------------------------------------------------------------------------

template <typename RT>
  requires(shilos::ValidMemRegionRootType<RT>)
void from_yaml(shilos::memory_region<RT> &mr, const yaml::Node &node, CodDep *raw_ptr) {
  if (!node.IsMap()) {
    throw yaml::TypeError("CodDep YAML node must be a mapping");
  }
  const auto &map = std::get<yaml::Map>(node.value);

  auto fetch_scalar = [&](const std::string &key) -> std::string {
    auto it = std::find_if(map.begin(), map.end(), [&](const auto &entry) { return entry.key == key; });
    if (it == map.end()) {
      throw yaml::MissingFieldError("Missing key '" + key + "' in CodDep");
    }
    if (!it->value.IsScalar()) {
      std::string actual_type =
          (it->value.IsNull() ? "null"
                              : (it->value.IsMap() ? "map" : (it->value.IsSequence() ? "sequence" : "non-scalar")));
      throw yaml::TypeError("Expected scalar for key '" + key + "' in CodDep, got " + actual_type +
                            " with value: " + yaml::format_yaml(it->value));
    }
    try {
      return it->value.asString();
    } catch (const std::exception &e) {
      throw yaml::TypeError("Failed to parse string value for key '" + key + "' in CodDep: " + e.what());
    }
  };

  UUID uuid(fetch_scalar("uuid"));
  std::string name = fetch_scalar("name");
  std::string repo_url = fetch_scalar("repo_url");

  // Optional path
  std::string path;
  auto it_path = std::find_if(map.begin(), map.end(), [](const auto &entry) { return entry.key == "path"; });
  if (it_path != map.end()) {
    if (!it_path->value.IsScalar()) {
      throw yaml::TypeError("'path' must be a scalar");
    }
    path = it_path->value.asString();
  }

  // Extract comments from the map if available
  std::string description;
  std::string name_comment;
  std::string repo_url_comment;
  std::string path_comment;

  // Extract field-specific comments
  for (const auto &entry : map) {
    if (entry.key == "name") {
      // Extract trailing comment from name field
      if (!entry.trailing_comment.empty()) {
        name_comment = std::string(entry.trailing_comment);
        // Remove the "# " prefix if present
        if (name_comment.size() > 2 && name_comment.substr(0, 2) == "# ") {
          name_comment = name_comment.substr(2);
        }
      }
    } else if (entry.key == "repo_url") {
      // Extract trailing comment from repo_url field
      if (!entry.trailing_comment.empty()) {
        repo_url_comment = std::string(entry.trailing_comment);
        // Remove the "# " prefix if present
        if (repo_url_comment.size() > 2 && repo_url_comment.substr(0, 2) == "# ") {
          repo_url_comment = repo_url_comment.substr(2);
        }
      }
    } else if (entry.key == "path") {
      // Extract trailing comment from path field
      if (!entry.trailing_comment.empty()) {
        path_comment = std::string(entry.trailing_comment);
        // Remove the "# " prefix if present
        if (path_comment.size() > 2 && path_comment.substr(0, 2) == "# ") {
          path_comment = path_comment.substr(2);
        }
      }
    } else if (entry.key == "uuid") {
      // Extract leading comments from uuid field as description
      for (const auto &comment : entry.leading_comments) {
        if (!description.empty()) {
          description += "\n";
        }
        std::string comment_str = std::string(comment);
        // Remove the "# " prefix if present
        if (comment_str.size() > 2 && comment_str.substr(0, 2) == "# ") {
          description += comment_str.substr(2);
        } else {
          description += comment_str;
        }
      }
    }
  }

  // Construct in-place with field-specific comments
  new (raw_ptr) CodDep(mr, uuid, name, repo_url, path, description, name_comment, repo_url_comment, path_comment);

  // Optional branches.
  auto it_br = std::find_if(map.begin(), map.end(), [](const auto &entry) { return entry.key == "branches"; });
  if (it_br != map.end()) {
    if (!it_br->value.IsSequence()) {
      throw yaml::TypeError("'branches' must be a sequence");
    }
    for (const auto &seq_item : std::get<yaml::DashSequence>(it_br->value.value)) {
      const auto &br_node = seq_item.value;
      raw_ptr->branches().enque(mr, br_node.asString());
    }
  }
}

template <typename RT>
  requires(shilos::ValidMemRegionRootType<RT>)
void from_yaml(shilos::memory_region<RT> &mr, const yaml::Node &node, CodProject *raw_ptr) {
  if (!node.IsMap()) {
    throw yaml::TypeError("CodProject YAML root must be a mapping");
  }
  const auto &map = std::get<yaml::Map>(node.value);

  auto fetch_scalar = [&](const std::string &key) -> std::string {
    auto it = std::find_if(map.begin(), map.end(), [&](const auto &entry) { return entry.key == key; });
    if (it == map.end()) {
      throw yaml::MissingFieldError("Missing key '" + key + "' in CodProject");
    }
    if (!it->value.IsScalar()) {
      std::string actual_type =
          (it->value.IsNull() ? "null"
                              : (it->value.IsMap() ? "map" : (it->value.IsSequence() ? "sequence" : "non-scalar")));
      throw yaml::TypeError("Expected scalar for key '" + key + "' in CodProject, got " + actual_type +
                            " with value: " + yaml::format_yaml(it->value));
    }
    try {
      return it->value.asString();
    } catch (const std::exception &e) {
      throw yaml::TypeError("Failed to parse string value for key '" + key + "' in CodProject: " + e.what());
    }
  };

  UUID uuid(fetch_scalar("uuid"));
  std::string name = fetch_scalar("name");
  std::string repo_url = fetch_scalar("repo_url");

  // Extract document-level comments if available
  std::string header;
  std::string name_comment;
  std::string repo_url_comment;

  // Optional CoD fields
  std::string repl_scope;
  std::string works_root_type_qualified;
  std::string works_root_type_header;

  // Extract field-specific comments and optional sections
  for (const auto &entry : map) {
    if (entry.key == "name") {
      if (!entry.trailing_comment.empty()) {
        name_comment = std::string(entry.trailing_comment);
        if (name_comment.size() > 2 && name_comment.substr(0, 2) == "# ") {
          name_comment = name_comment.substr(2);
        }
      }
    } else if (entry.key == "repo_url") {
      if (!entry.trailing_comment.empty()) {
        repo_url_comment = std::string(entry.trailing_comment);
        if (repo_url_comment.size() > 2 && repo_url_comment.substr(0, 2) == "# ") {
          repo_url_comment = repo_url_comment.substr(2);
        }
      }
    } else if (entry.key == "repl") {
      if (!entry.value.IsMap()) {
        throw yaml::TypeError("'repl' must be a mapping");
      }
      const auto &repl_map = std::get<yaml::Map>(entry.value.value);
      auto it_scope = std::find_if(repl_map.begin(), repl_map.end(), [](const auto &kv) { return kv.key == "scope"; });
      if (it_scope != repl_map.end()) {
        if (!it_scope->value.IsScalar()) {
          throw yaml::TypeError("'repl.scope' must be a scalar");
        }
        repl_scope = it_scope->value.asString();
      }
    } else if (entry.key == "works") {
      if (!entry.value.IsMap()) {
        throw yaml::TypeError("'works' must be a mapping");
      }
      const auto &works_map = std::get<yaml::Map>(entry.value.value);
      auto it_rt =
          std::find_if(works_map.begin(), works_map.end(), [](const auto &kv) { return kv.key == "root_type"; });
      if (it_rt != works_map.end()) {
        if (!it_rt->value.IsMap()) {
          throw yaml::TypeError("'works.root_type' must be a mapping");
        }
        const auto &rt_map = std::get<yaml::Map>(it_rt->value.value);
        auto it_q = std::find_if(rt_map.begin(), rt_map.end(), [](const auto &kv) { return kv.key == "qualified"; });
        if (it_q != rt_map.end()) {
          if (!it_q->value.IsScalar()) {
            throw yaml::TypeError("'works.root_type.qualified' must be a scalar");
          }
          works_root_type_qualified = it_q->value.asString();
        }
        auto it_h = std::find_if(rt_map.begin(), rt_map.end(), [](const auto &kv) { return kv.key == "header"; });
        if (it_h != rt_map.end()) {
          if (!it_h->value.IsScalar()) {
            throw yaml::TypeError("'works.root_type.header' must be a scalar");
          }
          works_root_type_header = it_h->value.asString();
        }
      }
    }
  }

  // Construct CodProject in-place with field-specific comments and CoD fields
  new (raw_ptr) CodProject(mr, uuid, name, repo_url, header, name_comment, repo_url_comment, repl_scope,
                           works_root_type_qualified, works_root_type_header);

  // branches sequence (optional)
  auto it_branches = std::find_if(map.begin(), map.end(), [](const auto &entry) { return entry.key == "branches"; });
  if (it_branches != map.end()) {
    if (!it_branches->value.IsSequence()) {
      throw yaml::TypeError("'branches' must be a sequence");
    }
    for (const auto &seq_item : std::get<yaml::DashSequence>(it_branches->value.value)) {
      const auto &br_node = seq_item.value;
      raw_ptr->branches().enque(mr, br_node.asString());
    }
  }

  // deps sequence (optional)
  auto it_deps = std::find_if(map.begin(), map.end(), [](const auto &entry) { return entry.key == "deps"; });
  if (it_deps != map.end()) {
    if (!it_deps->value.IsSequence()) {
      throw yaml::TypeError("'deps' must be a sequence");
    }
    for (const auto &seq_item : std::get<yaml::DashSequence>(it_deps->value.value)) {
      const auto &dep_node = seq_item.value;
      raw_ptr->deps().emplace_init(mr, [&](CodDep *dst) { from_yaml(mr, dep_node, dst); });
    }
  }
}

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

// ==========================================================================
// YAML SERIALISATION FOR MANIFEST CLASSES
// --------------------------------------------------------------------------

inline yaml::Node to_yaml(const CodManifestEntry &entry, yaml::YamlAuthor &author) {
  auto m = author.createMap();
  author.setMapValue(m, "uuid", author.createString(entry.uuid().to_string()));
  author.setMapValue(m, "repo_url", author.createString(std::string_view(entry.repo_url())));
  if (!entry.branch().empty()) {
    author.setMapValue(m, "branch", author.createString(std::string_view(entry.branch())));
  }
  if (!entry.commit().empty()) {
    author.setMapValue(m, "commit", author.createString(std::string_view(entry.commit())));
  }
  return m;
}

inline yaml::Node to_yaml(const CodManifest &manifest, yaml::YamlAuthor &author) {
  auto m = author.createMap();

  // Root section
  auto root_map = author.createMap();
  author.setMapValue(root_map, "uuid", author.createString(manifest.root_uuid().to_string()));
  author.setMapValue(root_map, "repo_url", author.createString(std::string_view(manifest.root_repo_url())));
  author.setMapValue(m, "root", root_map);

  // Locals section
  if (!manifest.locals().empty()) {
    auto locals_map = author.createMap();
    for (const auto &[uuid_str, path_str] : manifest.locals()) {
      author.setMapValue(locals_map, std::string_view(uuid_str), author.createString(std::string_view(path_str)));
    }
    author.setMapValue(m, "locals", locals_map);
  }

  // Resolved section
  if (!manifest.resolved().empty()) {
    auto resolved_seq = author.createDashSequence();
    for (const CodManifestEntry &entry : manifest.resolved()) {
      author.pushToSequence(resolved_seq, to_yaml(entry, author));
    }
    author.setMapValue(m, "resolved", resolved_seq);
  }

  return m;
}

// --------------------------------------------------------------------------
// Deserialisation for manifest classes
// --------------------------------------------------------------------------

template <typename RT>
  requires(shilos::ValidMemRegionRootType<RT>)
void from_yaml(shilos::memory_region<RT> &mr, const yaml::Node &node, CodManifestEntry *raw_ptr) {
  if (!node.IsMap()) {
    throw yaml::TypeError("CodManifestEntry YAML node must be a mapping");
  }
  const auto &map = std::get<yaml::Map>(node.value);

  auto fetch_scalar = [&](const std::string &key) -> std::string {
    auto it = std::find_if(map.begin(), map.end(), [&](const auto &entry) { return entry.key == key; });
    if (it == map.end()) {
      throw yaml::MissingFieldError("Missing key '" + key + "' in CodManifestEntry");
    }
    if (!it->value.IsScalar()) {
      throw yaml::TypeError("Expected scalar for key '" + key + "'");
    }
    return it->value.asString();
  };

  auto fetch_optional_scalar = [&](const std::string &key) -> std::string {
    auto it = std::find_if(map.begin(), map.end(), [&](const auto &entry) { return entry.key == key; });
    if (it == map.end()) {
      return "";
    }
    if (!it->value.IsScalar()) {
      throw yaml::TypeError("Expected scalar for key '" + key + "'");
    }
    return it->value.asString();
  };

  UUID uuid(fetch_scalar("uuid"));
  std::string repo_url = fetch_scalar("repo_url");
  std::string branch = fetch_optional_scalar("branch");
  std::string commit = fetch_optional_scalar("commit");

  // Construct in-place
  new (raw_ptr) CodManifestEntry(mr, uuid, repo_url, branch, commit);
}

template <typename RT>
  requires(shilos::ValidMemRegionRootType<RT>)
void from_yaml(shilos::memory_region<RT> &mr, const yaml::Node &node, CodManifest *raw_ptr) {
  if (!node.IsMap()) {
    throw yaml::TypeError("CodManifest YAML root must be a mapping");
  }
  const auto &map = std::get<yaml::Map>(node.value);

  // Parse root section
  auto it_root = std::find_if(map.begin(), map.end(), [](const auto &entry) { return entry.key == "root"; });
  if (it_root == map.end()) {
    throw yaml::MissingFieldError("Missing 'root' section in CodManifest");
  }
  if (!it_root->value.IsMap()) {
    throw yaml::TypeError("'root' must be a mapping");
  }
  const auto &root_map = std::get<yaml::Map>(it_root->value.value);

  auto fetch_scalar_from_map = [&](const yaml::Map &m, const std::string &key) -> std::string {
    auto it = std::find_if(m.begin(), m.end(), [&](const auto &entry) { return entry.key == key; });
    if (it == m.end()) {
      throw yaml::MissingFieldError("Missing key '" + key + "' in root section");
    }
    if (!it->value.IsScalar()) {
      std::string actual_type =
          (it->value.IsNull() ? "null"
                              : (it->value.IsMap() ? "map" : (it->value.IsSequence() ? "sequence" : "non-scalar")));
      throw yaml::TypeError("Expected scalar for key '" + key + "' in root section, got " + actual_type +
                            " with value: " + yaml::format_yaml(it->value));
    }
    try {
      return it->value.asString();
    } catch (const std::exception &e) {
      throw yaml::TypeError("Failed to parse string value for key '" + key + "' in root section: " + e.what());
    }
  };

  UUID root_uuid(fetch_scalar_from_map(root_map, "uuid"));
  std::string root_repo_url = fetch_scalar_from_map(root_map, "repo_url");

  // Construct CodManifest in-place
  new (raw_ptr) CodManifest(mr, root_uuid, root_repo_url);

  // Parse locals section (optional)
  auto it_locals = std::find_if(map.begin(), map.end(), [](const auto &entry) { return entry.key == "locals"; });
  if (it_locals != map.end()) {
    if (!it_locals->value.IsMap()) {
      throw yaml::TypeError("'locals' must be a mapping");
    }
    const auto &locals_map = std::get<yaml::Map>(it_locals->value.value);
    for (const auto &entry : locals_map) {
      const auto &uuid_str = entry.key;
      const auto &path_node = entry.value;
      if (!path_node.IsScalar()) {
        throw yaml::TypeError("local path must be a scalar");
      }
      std::string path_str = path_node.asString();
      UUID uuid(uuid_str);
      raw_ptr->addLocal(mr, uuid, path_str);
    }
  }

  // Parse resolved section (optional)
  auto it_resolved = std::find_if(map.begin(), map.end(), [](const auto &entry) { return entry.key == "resolved"; });
  if (it_resolved != map.end()) {
    if (!it_resolved->value.IsSequence()) {
      throw yaml::TypeError("'resolved' must be a sequence");
    }
    for (const auto &seq_item : std::get<yaml::DashSequence>(it_resolved->value.value)) {
      const auto &entry_node = seq_item.value;
      // Allocate CodManifestEntry via from_yaml helper
      raw_ptr->resolved().emplace_init(mr, [&](CodManifestEntry *dst) { from_yaml(mr, entry_node, dst); });
    }
  }
}

} // namespace cod::project
