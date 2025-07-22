#include "codp.hh"
#include "codp_yaml.hh"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <string_view>
#include <unordered_set>

using namespace shilos;
using namespace cod::project;
namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// End custom YAML loader
// ---------------------------------------------------------------------------

static void usage() {
  std::cerr << "codp solve [--project <path>] (default)\n"
               "codp update [--project <path>]\n"
               "codp init [--project <path>] [--uuid <uuid>] <name> <repo_url> <branch>...\n"
               "codp add [--project <path>] <repo_url> <branch>... [--uuid <uuid>]\n"
               "codp rm [--project <path>] <uuid-or-name>"
            << std::endl;
}

static fs::path home_dir() {
#ifdef _WIN32
  const char *home = std::getenv("USERPROFILE");
#else
  const char *home = std::getenv("HOME");
#endif
  if (!home) {
    throw std::runtime_error("Cannot determine HOME directory");
  }
  return fs::path(home);
}

// Walk upwards from start until CodProject.yaml found or root reached.
static std::optional<fs::path> find_project_dir(fs::path start) {
  start = fs::absolute(start);
  for (fs::path p = start; !p.empty(); p = p.parent_path()) {
    if (fs::exists(p / "CodProject.yaml")) {
      return p;
    }
    if (p == p.root_path())
      break;
  }
  return std::nullopt;
}

// Ensure directory exists (mkdir -p style)
static void ensure_dir(const fs::path &p) {
  std::error_code ec;
  fs::create_directories(p, ec);
  if (ec) {
    throw std::runtime_error("Failed to create directory: " + p.string() + ": " + ec.message());
  }
}

static void ensure_bare_repo(const std::string &url, const fs::path &bare_path) {
  if (fs::exists(bare_path)) {
    // Fetch updates
    std::string cmd = "git -C " + bare_path.string() + " fetch --all --prune";
    std::system(cmd.c_str());
  } else {
    ensure_dir(bare_path.parent_path());
    std::string cmd = "git clone --mirror " + url + " " + bare_path.string();
    int rc = std::system(cmd.c_str());
    if (rc != 0) {
      throw std::runtime_error("git clone failed for " + url);
    }
  }
}

static bool is_remote_repo_url(std::string_view url) {
  return url.starts_with("http://") || url.starts_with("https://") || url.starts_with("ssh://") ||
         url.starts_with("git@") || url.starts_with("ssh:");
}

// Validate that branches are provided
static void validate_branches(const regional_fifo<regional_str> &branches, const std::string &context) {
  if (branches.empty()) {
    throw std::runtime_error(context + ": at least one branch must be specified");
  }
}

// Find dependency by UUID or name
static const CodDep *find_dependency(const CodProject *project, const std::string &identifier) {
  for (const CodDep &dep : project->deps()) {
    if (dep.uuid().to_string() == identifier) {
      return &dep;
    }
    if (std::string_view(dep.name()) == identifier) {
      return &dep;
    }
  }
  return nullptr;
}

int main(int argc, char **argv) {
  std::string_view cmd = "solve";
  int argi = 1;
  if (argc >= 2 && argv[1][0] != '-') {
    cmd = argv[1];
    ++argi;
  }

  if (cmd != "solve" && cmd != "update" && cmd != "init" && cmd != "add" && cmd != "rm") {
    usage();
    return 1;
  }

  fs::path project_path;

  // parse optional --project <path>
  for (int i = argi; i < argc; ++i) {
    if (std::string_view(argv[i]) == "--project") {
      if (i + 1 >= argc) {
        usage();
        return 1;
      }
      project_path = fs::path(argv[i + 1]);
      ++i; // skip path
    }
  }

  try {
    if (cmd == "init") {
      // codp init [--uuid <uuid>] <name> <repo_url> <branch>...
      std::string uuid_str;
      std::string name;
      std::string repo_url;
      std::vector<std::string> branches;

      // Parse arguments, skipping already processed --project
      int pos = 0;
      for (int i = argi; i < argc; ++i) {
        std::string_view arg = argv[i];
        if (arg == "--project") {
          // Already processed in outer loop, skip both flag and value
          ++i; // skip value
          continue;
        }
        if (arg == "--uuid") {
          if (i + 1 >= argc) {
            std::cerr << "Error: --uuid requires a value" << std::endl;
            return 1;
          }
          uuid_str = argv[i + 1];
          ++i; // skip value
          continue;
        }

        // Positional arguments
        if (pos == 0) {
          name = std::string(arg);
        } else if (pos == 1) {
          repo_url = std::string(arg);
        } else {
          branches.emplace_back(arg);
        }
        ++pos;
      }

      if (name.empty() || repo_url.empty() || branches.empty()) {
        std::cerr << "Error: init requires <name> <repo_url> <branch>..." << std::endl;
        usage();
        return 1;
      }

      if (project_path.empty()) {
        project_path = fs::current_path();
      }

      // Check if CodProject.yaml already exists
      fs::path project_yaml = project_path / "CodProject.yaml";
      if (fs::exists(project_yaml)) {
        std::cerr << "Error: CodProject.yaml already exists at " << project_yaml << std::endl;
        return 1;
      }

      // Use provided UUID or generate a new one
      if (uuid_str.empty()) {
        uuid_str = UUID().to_string();
      }

      // Create new project
      auto_region<CodProject> region(1024 * 1024);
      CodProject *project = region->root().get();
      new (project) CodProject(*region, UUID(uuid_str), name, repo_url);

      // Add branches
      for (const std::string &branch : branches) {
        project->branches().enque(*region, branch);
      }

      validate_branches(project->branches(), "Project");

      // Save the project
      try {
        yaml::Document doc("CodProject.yaml", [&](yaml::YamlAuthor &author) {
          auto root = to_yaml(*project, author);
          author.addRoot(root);
        });
        // Document is automatically written to file by the constructor
      } catch (const yaml::Exception &err) {
        std::cerr << "YAML Error: " << err.what() << std::endl;
        std::cerr << "Stack trace:\n" << err.stack_trace() << std::endl;
        throw std::runtime_error(std::string("Failed to author project YAML: ") + err.what());
      }

      std::cout << "✔ Created CodProject.yaml at " << project_yaml << std::endl;
      return 0;
    }

    if (project_path.empty()) {
      auto maybe = find_project_dir(fs::current_path());
      if (!maybe) {
        std::cerr << "Error: could not find CodProject.yaml in current directory or any parent." << std::endl;
        return 1;
      }
      project_path = *maybe;
    }

    if (cmd == "add") {
      // codp add [--uuid <uuid>] <repo_url> <branch>...
      std::string uuid_str;
      std::string repo_url;
      std::vector<std::string> branches;

      // Parse arguments
      int i = argi;
      while (i < argc) {
        if (std::string_view(argv[i]) == "--project") {
          i += 2;
          continue;
        }
        if (std::string_view(argv[i]) == "--uuid") {
          if (i + 1 >= argc) {
            std::cerr << "Error: --uuid requires a value" << std::endl;
            return 1;
          }
          uuid_str = argv[i + 1];
          i += 2;
        } else if (repo_url.empty()) {
          repo_url = argv[i];
          i++;
        } else {
          branches.push_back(argv[i]);
          i++;
        }
      }

      if (repo_url.empty() || branches.empty()) {
        std::cerr << "Error: add requires <repo_url> <branch>..." << std::endl;
        usage();
        return 1;
      }

      // Load existing project
      try {
        yaml::Document doc((project_path / "CodProject.yaml").string());

        auto_region<CodProject> region(1024 * 1024);
        CodProject *project = region->root().get();
        cod::project::from_yaml<CodProject>(*region, doc.root(), project);

        // If UUID not provided, check if repo_url is a local path
        if (uuid_str.empty()) {
          if (is_remote_repo_url(repo_url)) {
            throw std::runtime_error("UUID is required for remote repositories");
          }

          fs::path dep_path = repo_url;
          if (dep_path.is_relative()) {
            dep_path = project_path / dep_path;
          }

          fs::path dep_yaml = dep_path / "CodProject.yaml";
          if (!fs::exists(dep_yaml)) {
            throw std::runtime_error("CodProject.yaml not found at " + dep_yaml.string());
          }

          try {
            yaml::Document dep_doc(dep_yaml.string());

            auto_region<CodProject> dep_region(1024 * 1024);
            CodProject *dep_project = dep_region->root().get();
            cod::project::from_yaml<CodProject>(*dep_region, dep_doc.root(), dep_project);
            uuid_str = dep_project->uuid().to_string();
          } catch (const yaml::Exception &err) {
            std::cerr << "YAML Error: " << err.what() << std::endl;
            std::cerr << "Stack trace:\n" << err.stack_trace() << std::endl;
            throw std::runtime_error(std::string("Failed to parse ") + dep_yaml.string() + ": " + err.what());
          }
        }

        UUID new_uuid(uuid_str);

        // Check if dependency already exists
        for (const CodDep &dep : project->deps()) {
          if (dep.uuid() == new_uuid) {
            throw std::runtime_error("dependency with UUID " + uuid_str + " already exists");
          }
        }

        // Create new dependency with a default name based on repo URL
        std::string dep_name;
        if (is_remote_repo_url(repo_url)) {
          // Extract repo name from URL
          fs::path repo_path(repo_url);
          dep_name = repo_path.stem().string();
          if (dep_name.empty()) {
            dep_name = "dep-" + uuid_str.substr(0, 8);
          }
        } else {
          // For local dependencies, use directory name
          fs::path local_path(repo_url);
          dep_name = local_path.filename().string();
          if (dep_name.empty()) {
            dep_name = "local-dep";
          }
        }
        CodDep *new_dep = new (region->allocate<CodDep>()) CodDep(*region, new_uuid, dep_name, repo_url, "");

        // Add branches
        for (const std::string &branch : branches) {
          new_dep->branches().enque(*region, branch);
        }

        validate_branches(new_dep->branches(), "Dependency");

        // Add to project
        project->deps().emplace_init(*region, [&](CodDep *dep) {
          new (dep) CodDep(*region, new_dep->uuid(), std::string_view(new_dep->name()),
                           std::string_view(new_dep->repo_url()), std::string_view(new_dep->path()));

          // Copy branches
          for (const regional_str &branch : new_dep->branches()) {
            dep->branches().enque(*region, std::string_view(branch));
          }
        });

        // Save the project
        try {
          yaml::Document doc("CodProject.yaml", [&](yaml::YamlAuthor &author) {
            auto root = to_yaml(*project, author);
            author.addRoot(root);
          });
          // Document is automatically written to file by the constructor
        } catch (const yaml::AuthorError &err) {
          std::cerr << "YAML Author Error: " << err.what() << std::endl;
          std::cerr << "Stack trace:\n" << err.stack_trace() << std::endl;
          throw std::runtime_error(std::string("Failed to author project YAML: ") + err.what());
        }

        std::cout << "✔ Added dependency " << uuid_str << " to CodProject.yaml" << std::endl;
      } catch (const yaml::Exception &err) {
        std::cerr << "YAML Error: " << err.what() << std::endl;
        std::cerr << "Stack trace:\n" << err.stack_trace() << std::endl;
        throw std::runtime_error(std::string("Failed to parse CodProject.yaml: ") + err.what());
      }

      return 0;
    }

    if (cmd == "rm") {
      // codp rm <uuid-or-name>
      int required_args_start = argi;
      for (int i = argi; i < argc; ++i) {
        if (std::string_view(argv[i]) == "--project") {
          required_args_start = i + 2;
          break;
        }
      }

      if (required_args_start >= argc) {
        std::cerr << "Error: rm requires <uuid-or-name>" << std::endl;
        usage();
        return 1;
      }

      std::string identifier = argv[required_args_start];

      // Load existing project
      try {
        yaml::Document doc((project_path / "CodProject.yaml").string());

        auto_region<CodProject> region(1024 * 1024);
        CodProject *project = region->root().get();
        cod::project::from_yaml<CodProject>(*region, doc.root(), project);

        // Find dependency by UUID or name
        const CodDep *target_dep = find_dependency(project, identifier);
        if (!target_dep) {
          throw std::runtime_error("dependency with identifier '" + identifier + "' not found");
        }

        // Create new project with updated deps
        CodProject *new_project = region->root().get();
        new (new_project) CodProject(*region, project->uuid(), std::string_view(project->name()),
                                     std::string_view(project->repo_url()));

        // Copy branches
        for (const regional_str &branch : project->branches()) {
          new_project->branches().enque(*region, std::string_view(branch));
        }

        // Copy deps except the target
        for (const CodDep &dep : project->deps()) {
          if (dep.uuid() != target_dep->uuid()) {
            CodDep *new_dep =
                new (region->allocate<CodDep>()) CodDep(*region, dep.uuid(), std::string_view(dep.name()),
                                                        std::string_view(dep.repo_url()), std::string_view(dep.path()));

            // Copy branches
            for (const regional_str &branch : dep.branches()) {
              new_dep->branches().enque(*region, std::string_view(branch));
            }

            new_project->deps().emplace_init(*region, [&](CodDep *dep) {
              new (dep) CodDep(*region, new_dep->uuid(), std::string_view(new_dep->name()),
                               std::string_view(new_dep->repo_url()), std::string_view(new_dep->path()));
            });
          }
        }

        // Save the project
        try {
          yaml::Document doc("CodProject.yaml", [&](yaml::YamlAuthor &author) {
            auto root = to_yaml(*new_project, author);
            author.addRoot(root);
          });
          // Document is automatically written to file by the constructor
        } catch (const yaml::AuthorError &err) {
          std::cerr << "YAML Author Error: " << err.what() << std::endl;
          std::cerr << "Stack trace:\n" << err.stack_trace() << std::endl;
          throw std::runtime_error(std::string("Failed to author project YAML: ") + err.what());
        }

        std::cout << "✔ Removed dependency " << identifier << " from CodProject.yaml" << std::endl;
      } catch (const yaml::ParseError &err) {
        std::cerr << "YAML Parse Error: " << err.what() << std::endl;
        std::cerr << "Stack trace:\n" << err.stack_trace() << std::endl;
        throw std::runtime_error(std::string("Failed to parse CodProject.yaml: ") + err.what() + "\nStack trace:\n" +
                                 err.stack_trace());
      }

      return 0;
    }

    if (cmd == "update") {
      std::cerr << "update command is not yet implemented." << std::endl;
      return 0;
    }

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
      auto_region<CodManifest> manifest_region(1024 * 1024);
      CodManifest *manifest = manifest_region->root().get();
      new (manifest) CodManifest(*manifest_region, project->uuid(), std::string_view(project->repo_url()));

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

  } catch (const yaml::Exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    std::cerr << "Stack trace:\n" << e.stack_trace() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
