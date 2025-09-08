#include "codp_commands.hh"

int cmd_solve(int argc, char **argv, int argi, const fs::path &project_path) {
  // cmd == "solve" - proceed with dependency resolution

  fs::path project_yaml = project_path / "CodProject.yaml";
  if (!fs::exists(project_yaml)) {
    std::cerr << "CodProject.yaml not found at " << project_yaml << std::endl;
    return 1;
  }

  try {
    yaml::Document doc(project_yaml.string());

    const yaml::Node &root = doc.root();

    // Allocate region (1 MB) and construct project from YAML
    auto_region<CodProject> region(1024 * 1024);
    CodProject *project = region->root().get();
    cod::project::from_yaml<CodProject>(*region, root, project);

    // Validate branches requirement
    validate_branches(project->branches(), "Project");
    for (const CodDep &dep : project->deps()) {
      validate_branches(dep.branches(), "Dependency " + dep.uuid().to_string());
    }

    // Prepare git cache directories
    fs::path repos_root = home_dir() / ".cod" / "pkgs" / "repos";

    // Iterate over deps + self repo
    auto process_repo = [&](const std::string &url) {
      if (!is_remote_repo_url(url))
        return; // skip local/dummy urls for test scenarios
      std::string key = repo_url_to_key(url);
      fs::path bare = repos_root / (key + ".git");
      ensure_bare_repo(url, bare);
    };

    process_repo(std::string(std::string_view(project->repo_url())));
    for (const CodDep &dep : project->deps()) {
      if (dep.path().empty()) {
        process_repo(std::string(std::string_view(dep.repo_url())));
      }
    }

    std::cout << "✔ Repositories synchronised." << std::endl;

    // -----------------------------------------------------------------
    // Generate CodManifest.yaml using regional CodManifest class
    // -----------------------------------------------------------------

    // Create manifest region and populate it
    auto_region<CodManifest> manifest_region(1024 * 1024, project->uuid(), std::string_view(project->repo_url()));
    CodManifest *manifest = manifest_region->root().get();

    // Collect dependencies recursively using temporary data structures
    std::unordered_set<std::string> visited;
    std::unordered_map<std::string, std::string> locals;                           // uuid -> path
    std::vector<std::tuple<UUID, std::string, std::string, std::string>> resolved; // uuid, repo_url, branch, commit

    std::function<void(const fs::path &, const CodProject *)> collect_deps = [&](const fs::path &proj_dir,
                                                                                 const CodProject *proj) {
      for (const CodDep &dep : proj->deps()) {
        std::string uuid_str = dep.uuid().to_string();
        if (visited.contains(uuid_str))
          continue;
        visited.insert(uuid_str);

        if (!dep.path().empty()) {
          // Local dependency
          fs::path dep_path = std::string(std::string_view(dep.path()));
          if (dep_path.is_relative())
            dep_path = fs::absolute(proj_dir / dep_path);
          dep_path = fs::weakly_canonical(dep_path);

          // Store relative path from project root
          fs::path relative_path = fs::relative(dep_path, project_path);
          locals[uuid_str] = relative_path.string();

          // Load and recurse into dep project
          fs::path dep_yaml_path = dep_path / "CodProject.yaml";
          try {
            yaml::Document dep_doc(dep_yaml_path.string());

            const yaml::Node &dep_root = dep_doc.root();
            auto_region<CodProject> dep_region(1024 * 1024);
            CodProject *dep_proj = dep_region->root().get();
            cod::project::from_yaml<CodProject>(*dep_region, dep_root, dep_proj);
            collect_deps(dep_path, dep_proj);
          } catch (const yaml::Exception &err) {
            std::cerr << "YAML Error: " << err.what() << std::endl;
            std::cerr << "Stack trace:\n" << err.stack_trace() << std::endl;
            throw std::runtime_error(std::string("Failed to parse ") + dep_yaml_path.string() + ": " + err.what());
          } catch (const std::exception &e) {
            // Skip deps that can't be loaded
            std::cerr << "Warning: Failed to load dependency " << dep_yaml_path << ": " << e.what() << std::endl;
          }
        } else {
          // Remote dependency - use first branch as the resolved branch
          std::string resolved_branch;
          if (!dep.branches().empty()) {
            resolved_branch = std::string(std::string_view(*dep.branches().front()));
          }
          resolved.emplace_back(dep.uuid(), std::string(std::string_view(dep.repo_url())), resolved_branch, "");
        }
      }
    };

    collect_deps(project_path, project);

    // Now populate the manifest with collected data
    for (const auto &[uuid_str, path] : locals) {
      UUID uuid(uuid_str);
      manifest->addLocal(*manifest_region, uuid, path);
    }

    for (const auto &[uuid, repo_url, branch, commit] : resolved) {
      manifest->addResolved(*manifest_region, uuid, repo_url, branch, commit);
    }

    // Generate YAML using the new to_yaml function
    try {
      yaml::Document doc("CodManifest.yaml", [&](yaml::YamlAuthor &author) {
        auto root = to_yaml(*manifest, author);
        author.addRoot(root);
      });
      std::cout << "✔ CodManifest.yaml generated at " << (project_path / "CodManifest.yaml") << std::endl;
    } catch (const yaml::AuthorError &err) {
      std::cerr << "YAML Author Error: " << err.what() << std::endl;
      std::cerr << "Stack trace:\n" << err.stack_trace() << std::endl;
      throw std::runtime_error(std::string("Failed to author manifest YAML: ") + err.what());
    }
  } catch (const yaml::ParseError &err) {
    std::cerr << "YAML Parse Error: " << err.what() << std::endl;
    std::cerr << "Stack trace:\n" << err.stack_trace() << std::endl;
    throw std::runtime_error(std::string("Failed to parse ") + project_yaml.string() + ": " + err.what());
  }

  return 0;
}

int cmd_update(int argc, char **argv, int argi, const fs::path &project_path) {
  fs::path manifest_yaml = project_path / "CodManifest.yaml";

  // Check if CodManifest.yaml exists
  if (!fs::exists(manifest_yaml)) {
    std::cerr << "Error: CodManifest.yaml not found. Run 'codp solve' first." << std::endl;
    return 1;
  }

  try {
    // Load existing manifest
    yaml::Document manifest_doc(manifest_yaml);
    auto_region<CodManifest> manifest_region(1024 * 1024);
    CodManifest *manifest = manifest_region->root().get();
    from_yaml(*manifest_region, manifest_doc.root(), manifest);

    std::cout << "Updating dependencies..." << std::endl;

    // Update each resolved dependency
    for (CodManifestEntry &entry : manifest->resolved()) {
      std::string repo_url = std::string(std::string_view(entry.repo_url()));
      std::string branch = std::string(std::string_view(entry.branch()));

      if (branch.empty()) {
        std::cout << "Skipping " << repo_url << " (no branch specified)" << std::endl;
        continue;
      }

      std::cout << "Updating " << repo_url << " (" << branch << ")..." << std::endl;

      // Ensure bare repo exists and is up to date
      std::string repo_key = repo_url_to_key(repo_url);
      fs::path bare_repo_path = home_dir() / ".cod" / "pkgs" / "repos" / repo_key;

      try {
        ensure_bare_repo(repo_url, bare_repo_path);
      } catch (const std::exception &e) {
        std::cerr << "Warning: Failed to update " << repo_url << ": " << e.what() << std::endl;
        continue;
      }

      // Get latest commit hash for the branch
      std::string git_cmd = "git --git-dir=" + bare_repo_path.string() + " rev-parse " + branch;
      FILE *pipe = popen(git_cmd.c_str(), "r");
      if (!pipe) {
        std::cerr << "Warning: Failed to get commit hash for " << repo_url << " (" << branch << ")" << std::endl;
        continue;
      }

      char commit_hash[41];
      if (fgets(commit_hash, sizeof(commit_hash), pipe) != nullptr) {
        // Remove newline
        size_t len = strlen(commit_hash);
        if (len > 0 && commit_hash[len - 1] == '\n') {
          commit_hash[len - 1] = '\0';
        }

        // Update the commit hash in the manifest entry
        new (&entry.commit()) regional_str(*manifest_region, commit_hash);
        std::cout << "  Updated to commit " << commit_hash << std::endl;
      } else {
        std::cerr << "Warning: Failed to read commit hash for " << repo_url << " (" << branch << ")" << std::endl;
      }
      pclose(pipe);
    }

    // Write updated manifest back to file
    try {
      yaml::Document doc("CodManifest.yaml", [&](yaml::YamlAuthor &author) {
        auto root = to_yaml(*manifest, author);
        author.addRoot(root);
      });
      std::cout << "✔ Updated CodManifest.yaml" << std::endl;
    } catch (const yaml::AuthorError &err) {
      std::cerr << "YAML Author Error: " << err.what() << std::endl;
      std::cerr << "Stack trace:\n" << err.stack_trace() << std::endl;
      throw std::runtime_error(std::string("Failed to author manifest YAML: ") + err.what());
    }

  } catch (const yaml::ParseError &err) {
    std::cerr << "YAML Parse Error: " << err.what() << std::endl;
    std::cerr << "Stack trace:\n" << err.stack_trace() << std::endl;
    return 1;
  } catch (const std::exception &err) {
    std::cerr << "Error: " << err.what() << std::endl;
    return 1;
  }

  return 0;
}

int cmd_debug(int argc, char **argv, int argi, const fs::path &project_path) {
  dumpTestDebugInfo(std::cerr);
  return 0;
}
