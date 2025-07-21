
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string_view>
#include <unordered_set>

#include "codp.hh"
#include "codp_yaml.hh"

using namespace shilos;
using namespace cod::project;
namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// End custom YAML loader
// ---------------------------------------------------------------------------

static void usage() {
  std::cerr << "codp solve [--project <path>] (default)\n"
               "codp update [--project <path>]"
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

int main(int argc, char **argv) {
  std::string_view cmd = "solve";
  int argi = 1;
  if (argc >= 2 && argv[1][0] != '-') {
    cmd = argv[1];
    ++argi;
  }

  if (cmd != "solve" && cmd != "update") {
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
    } else {
      usage();
      return 1;
    }
  }

  if (project_path.empty()) {
    auto maybe = find_project_dir(fs::current_path());
    if (!maybe) {
      std::cerr << "Error: could not find CodProject.yaml in current directory or any parent." << std::endl;
      return 1;
    }
    project_path = *maybe;
  }

  fs::path project_yaml = project_path / "CodProject.yaml";
  if (!fs::exists(project_yaml)) {
    std::cerr << "CodProject.yaml not found at " << project_yaml << std::endl;
    return 1;
  }

  try {
    if (cmd == "update") {
      std::cerr << "update command is not yet implemented." << std::endl;
      return 0;
    }

    auto result = yaml::YamlDocument::Read(project_yaml.string());
    vswitch(
        result,
        [&](const yaml::ParseError &err) {
          throw std::runtime_error("Failed to parse " + project_yaml.string() + ": " + err.what());
        },
        [&](const yaml::YamlDocument &doc) {
          const yaml::Node &root = doc.root();

          // Allocate region (1 MB) and construct project from YAML
          auto_region<CodProject> region(1024 * 1024);
          CodProject *project = region->root().get();
          cod::project::from_yaml<CodProject>(*region, root, project);

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
          std::unordered_map<std::string, std::string> locals; // uuid -> path
          std::vector<std::tuple<UUID, std::string, std::string, std::string>>
              resolved; // uuid, repo_url, branch, commit

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
                  auto dep_result = yaml::YamlDocument::Read(dep_yaml_path.string());
                  vswitch(
                      dep_result,
                      [&](const yaml::ParseError &err) {
                        throw std::runtime_error("Failed to parse " + dep_yaml_path.string() + ": " + err.what());
                      },
                      [&](const yaml::YamlDocument &doc) {
                        const yaml::Node &dep_root = doc.root();
                        auto_region<CodProject> dep_region(1024 * 1024);
                        CodProject *dep_proj = dep_region->root().get();
                        cod::project::from_yaml<CodProject>(*dep_region, dep_root, dep_proj);
                        collect_deps(dep_path, dep_proj);
                      });
                } catch (const std::exception &e) {
                  // Skip deps that can't be loaded
                  std::cerr << "Warning: Failed to load dependency " << dep_yaml_path << ": " << e.what() << std::endl;
                }
              } else {
                // Remote dependency
                resolved.emplace_back(dep.uuid(), std::string(std::string_view(dep.repo_url())), "", "");
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
          auto yaml_result = yaml::YamlDocument::Write("CodManifest.yaml", [&](yaml::YamlAuthor &author) {
            auto root = to_yaml(*manifest, author);
            author.addRoot(root);
          });

          vswitch(
              yaml_result,
              [&](const yaml::ParseError &err) {
                throw std::runtime_error("Failed to generate manifest YAML: " + std::string(err.what()));
              },
              [&](const yaml::AuthorError &err) {
                throw std::runtime_error("Failed to author manifest YAML: " + std::string(err.what()));
              },
              [&](const yaml::YamlDocument &manifest_doc) {
                fs::path manifest_path = project_path / "CodManifest.yaml";
                std::ofstream ofs(manifest_path);
                if (!ofs) {
                  throw std::runtime_error("Cannot write CodManifest.yaml at " + manifest_path.string());
                }
                ofs << yaml::format_yaml(manifest_doc.root()) << std::endl;
                std::cout << "✔ CodManifest.yaml generated at " << manifest_path << std::endl;
              });
        });

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
