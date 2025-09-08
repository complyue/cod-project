#include "codp_commands.hh"

int cmd_init(int argc, char **argv, int argi, const fs::path &project_path) {
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

  fs::path actual_project_path = project_path.empty() ? fs::current_path() : project_path;

  // Check if CodProject.yaml already exists
  fs::path project_yaml = actual_project_path / "CodProject.yaml";
  if (fs::exists(project_yaml)) {
    std::cerr << "Error: CodProject.yaml already exists at " << project_yaml << std::endl;
    return 1;
  }

  // Use provided UUID or generate a new one
  if (uuid_str.empty()) {
    uuid_str = UUID().to_string();
  }

  // Create new project
  auto_region<CodProject> region(1024 * 1024, UUID(uuid_str), name, repo_url);
  CodProject *project = region->root().get();

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
