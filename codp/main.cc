#include "codp_commands.hh"

int main(int argc, char **argv) {
  std::string_view cmd = "solve";
  int argi = 1;
  if (argc >= 2 && argv[1][0] != '-') {
    cmd = argv[1];
    ++argi;
  }

  if (cmd != "solve" && cmd != "update" && cmd != "init" && cmd != "add" && cmd != "rm" && cmd != "debug") {
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
    if (cmd == "debug") {
      return cmd_debug(argc, argv, argi, project_path);
    }

    if (cmd == "init") {
      return cmd_init(argc, argv, argi, project_path);
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
      return cmd_add(argc, argv, argi, project_path);
    }

    if (cmd == "rm") {
      return cmd_rm(argc, argv, argi, project_path);
    }

    if (cmd == "update") {
      return cmd_update(argc, argv, argi, project_path);
    }

    // cmd == "solve" - proceed with dependency resolution
    return cmd_solve(argc, argv, argi, project_path);

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
