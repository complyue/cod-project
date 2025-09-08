#include "codp_commands.hh"

int cmd_rm(int argc, char **argv, int argi, const fs::path &project_path) {
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

    // Save data from original project before overwriting
    UUID orig_uuid = project->uuid();
    std::string orig_name(project->name());
    std::string orig_repo_url(project->repo_url());

    // Save branches
    std::vector<std::string> orig_branches;
    for (const regional_str &branch : project->branches()) {
      orig_branches.emplace_back(branch);
    }

    // Save deps (except the target)
    struct SavedDep {
      UUID uuid;
      std::string name;
      std::string repo_url;
      std::string path;
      std::vector<std::string> branches;
    };

    std::vector<SavedDep> saved_deps;
    for (const CodDep &dep : project->deps()) {
      if (dep.uuid() != target_dep->uuid()) {
        SavedDep saved;
        saved.uuid = dep.uuid();
        saved.name = std::string(dep.name());
        saved.repo_url = std::string(dep.repo_url());
        saved.path = std::string(dep.path());
        for (const regional_str &branch : dep.branches()) {
          saved.branches.emplace_back(branch);
        }
        saved_deps.push_back(std::move(saved));
      }
    }

    // Create new project at root location
    CodProject *new_project = region->root().get();
    new (new_project) CodProject(*region, orig_uuid, orig_name, orig_repo_url);

    // Copy branches
    for (const std::string &branch : orig_branches) {
      new_project->branches().enque(*region, branch);
    }

    // Copy saved deps
    for (const SavedDep &saved : saved_deps) {
      new_project->deps().emplace_init(*region, [&](CodDep *new_dep) {
        new (new_dep) CodDep(*region, saved.uuid, saved.name, saved.repo_url, saved.path);

        // Copy branches
        for (const std::string &branch : saved.branches) {
          new_dep->branches().enque(*region, branch);
        }
      });
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
