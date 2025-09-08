#include "codp_commands.hh"

int main(int argc, char **argv) {
  std::string_view cmd = "solve";
  int argi = -1;

  fs::path project_path;

  // Parse global options anywhere and detect subcommand as first recognized non-option token
  for (int i = 1; i < argc; ++i) {
    std::string_view a = argv[i];
    if (a == "--project") {
      if (i + 1 >= argc) {
        usage();
        return 1;
      }
      project_path = fs::path(argv[++i]);
      continue;
    }

    // First recognized non-option token becomes subcommand; otherwise treat as positional for default command
    if (!a.empty() && a[0] != '-') {
      if (a == "solve" || a == "update" || a == "init" || a == "add" || a == "rm" || a == "debug") {
        cmd = a;
        argi = i + 1;
      } else if (argi == -1) {
        // No subcommand specified yet; default remains 'solve' and this is the first positional for it
        argi = i;
      }
      continue;
    }

    // Unknown global options are ignored here and left for subcommand-specific parsing
  }

  if (argi == -1) {
    // No positional args encountered; start after program name
    argi = 1;
  }

  if (cmd != "solve" && cmd != "update" && cmd != "init" && cmd != "add" && cmd != "rm" && cmd != "debug") {
    usage();
    return 1;
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
