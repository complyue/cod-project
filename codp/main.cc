
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>

#include "codp.hh"
#include "codp_manifest.hh"
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

static std::string slurp_file(const fs::path &p) {
  std::ifstream ifs(p);
  if (!ifs) {
    throw std::runtime_error("Failed to open file: " + p.string());
  }
  std::stringstream ss;
  ss << ifs.rdbuf();
  return ss.str();
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

    std::string yaml_text = ::slurp_file(project_yaml);
    auto doc = yaml::YamlDocument::Parse(std::string(yaml_text));
    const yaml::Node &root = doc.root();

    // Allocate region (1 MB) and construct project from YAML
    auto_region<CodProject> region(1024 * 1024);
    CodProject *project = region->root().get();
    from_yaml(*region, root, project);

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
    // Generate CodManifest.yaml (local deps only stage)
    // -----------------------------------------------------------------

    yaml::Node manifest_node = generate_manifest(project_path);

    fs::path manifest_path = project_path / "CodManifest.yaml";
    std::ofstream ofs(manifest_path);
    if (!ofs) {
      throw std::runtime_error("Cannot write CodManifest.yaml at " + manifest_path.string());
    }
    ofs << yaml::format_yaml(manifest_node) << std::endl;

    std::cout << "✔ CodManifest.yaml generated at " << manifest_path << std::endl;

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
