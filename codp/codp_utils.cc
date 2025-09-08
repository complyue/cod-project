#include "codp_commands.hh"

void usage() {
  std::cerr << "codp solve [--project <path>] (default)\n"
               "codp update [--project <path>]\n"
               "codp init [--project <path>] [--uuid <uuid>] <name> <repo_url> <branch>...\n"
               "codp add [--project <path>] <repo_url> <branch>... [--uuid <uuid>]\n"
               "codp rm [--project <path>] <uuid-or-name>"
            << std::endl;
}

fs::path home_dir() {
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
std::optional<fs::path> find_project_dir(fs::path start) {
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
void ensure_dir(const fs::path &p) {
  std::error_code ec;
  fs::create_directories(p, ec);
  if (ec) {
    throw std::runtime_error("Failed to create directory: " + p.string() + ": " + ec.message());
  }
}

void ensure_bare_repo(const std::string &url, const fs::path &bare_path) {
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

bool is_remote_repo_url(std::string_view url) {
  return url.starts_with("http://") || url.starts_with("https://") || url.starts_with("ssh://") ||
         url.starts_with("git@") || url.starts_with("ssh:");
}

// Validate that branches are provided
void validate_branches(const regional_fifo<regional_str> &branches, const std::string &context) {
  if (branches.empty()) {
    throw std::runtime_error(context + ": at least one branch must be specified");
  }
}

// Find dependency by UUID or name
const CodDep *find_dependency(const CodProject *project, const std::string &identifier) {
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

// Test function with known location for address capture
void errThrowingFunction() { //
  throw shilos::yaml::TypeError("Test error from errThrowingFunction");
}

// Dump debug info for the test function
void dumpTestDebugInfo(std::ostream &os) {
  void *func_addr = (void *)errThrowingFunction;
  os << "Obtained address of errThrowingFunction: " << func_addr << std::endl;
  shilos::dumpDebugInfo(func_addr, os);
  os << std::endl;

  os << "Test errThrowingFunction() call...\n";
  errThrowingFunction();
}
