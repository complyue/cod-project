#include "codp_commands.hh"

int cmd_add(int argc, char **argv, int argi, const fs::path &project_path) {
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

    std::string dep_name;

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
        dep_name = std::string(dep_project->name()); // Use name from local CodProject.yaml
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

    // If dep_name is still empty (remote repo or local without name), generate one
    if (dep_name.empty()) {
      if (is_remote_repo_url(repo_url)) {
        // Extract repo name from URL
        fs::path repo_path(repo_url);
        dep_name = repo_path.stem().string();
        if (dep_name.empty()) {
          dep_name = "dep-" + uuid_str.substr(0, 8);
        }
      } else {
        // For local dependencies, use directory name as fallback
        fs::path local_path(repo_url);
        dep_name = local_path.filename().string();
        if (dep_name.empty()) {
          dep_name = "local-dep";
        }
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
                       std::string_view(new_dep->repo_url()), std::string_view(new_dep->path()),
                       std::string_view(new_dep->description()), std::string_view(new_dep->name_comment()),
                       std::string_view(new_dep->repo_url_comment()), std::string_view(new_dep->path_comment()));

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
