//===--- cod/main.cc - Compile-on-Demand REPL Driver ----------------===//
//
// CoD Project - Compile-on-Demand REPL without JIT
// Implements build-and-run REPL with persistent DBMR workspace
//
//===----------------------------------------------------------------------===//

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <system_error>
#include <unistd.h>
#include <vector>

#include "cod.hh"
#include "codp.hh"
#include "codp_yaml.hh"
#include "shilos.hh"

namespace fs = std::filesystem;
using namespace shilos;
using namespace cod;
using namespace cod::project;

struct CodReplConfig {
  fs::path works_path = "./CodWorks.dbmr";
  fs::path project_root;
  std::string repl_scope = "main.hh";
  std::string works_root_type_qualified = "cod::WorksRoot";
  std::string works_root_type_header = "cod.hh";
  size_t dbmr_capacity = 64 * 1024 * 1024;    // 64MB default
  std::optional<std::string> eval_expression; // For -e/--eval mode
};

static void printUsage(const char *prog_name) {
  std::cout << "Usage: " << prog_name << " [OPTIONS] [EXPRESSION]\n"
            << "\n"
            << "Compile-on-Demand REPL - Build-and-run REPL without JIT\n"
            << "\n"
            << "Options:\n"
            << "  -w, --works PATH    Workspace DBMR file path (default: ./CodWorks.dbmr)\n"
            << "  --project PATH      Project root directory (default: auto-detect)\n"
            << "  -e, --eval EXPR     Evaluate expression/statement and exit\n"
            << "  -h, --help          Show this help message\n"
            << "\n"
            << "If no -e/--eval is specified, starts interactive REPL mode.\n"
            << "\n"
            << "REPL Commands:\n"
            << "  %quit               Exit the REPL\n"
            << "  %help               Show REPL help\n";
}

static void printReplHelp() {
  std::cout << "CoD REPL Help:\n"
            << "\n"
            << "Enter C++20 statements or expressions. Each submission is compiled\n"
            << "into a temporary executable and run immediately.\n"
            << "\n"
            << "Session state persists in the workspace DBMR file.\n"
            << "\n"
            << "Commands:\n"
            << "  %quit    - Exit the REPL\n"
            << "  %help    - Show this help\n"
            << "\n";
}

static std::optional<fs::path> findProjectRoot(fs::path start) {
  while (!start.empty() && start != start.parent_path()) {
    if (fs::exists(start / "CodProject.yaml")) {
      return start;
    }
    start = start.parent_path();
  }
  return std::nullopt;
}

static std::optional<CodReplConfig> loadConfig(const fs::path &project_root) {
  CodReplConfig config;
  config.project_root = project_root;

  auto config_path = project_root / "CodProject.yaml";
  if (!fs::exists(config_path)) {
    std::cerr << "Warning: CodProject.yaml not found at " << config_path << "\n";
    return config; // Return default config
  }

  try {
    auto region = auto_region<CodProject>(1024 * 1024); // 1MB for config parsing
    std::ifstream file(config_path);
    if (!file) {
      std::cerr << "Error: Cannot read " << config_path << "\n";
      return std::nullopt;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    auto doc_result = yaml::Document::Parse(config_path.string(), content);
    if (std::holds_alternative<yaml::ParseError>(doc_result)) {
      std::cerr << "Error: Failed to parse YAML in " << config_path << "\n";
      return std::nullopt;
    }

    auto &doc = std::get<yaml::Document>(doc_result);
    CodProject *project_ptr = region->root().get();
    cod::project::from_yaml<CodProject>(*region, doc.root(), project_ptr);
    auto &project = *project_ptr;

    // Use project configuration if available
    // For now, use defaults
    return config;
  } catch (const std::exception &e) {
    std::cerr << "Error loading config: " << e.what() << "\n";
    return std::nullopt;
  }
}

static bool ensureDbmrExists(const fs::path &works_path, size_t capacity) {
  if (fs::exists(works_path)) {
    return true;
  }

  try {
    // Create parent directories
    fs::create_directories(works_path.parent_path());

    // Create DBMR with WorksRoot
    auto dbmr = DBMR<WorksRoot>::create(works_path.string(), capacity);

    std::cout << "Created new workspace: " << works_path << " (" << capacity / (1024 * 1024) << "MB)\n";
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error creating workspace: " << e.what() << "\n";
    return false;
  }
}

static std::string generateRunnerSource(const CodReplConfig &config, const std::string &submission) {
  std::ostringstream oss;

  // Include the configured scope header
  oss << "#include <" << config.works_root_type_header << ">\n";
  oss << "#include <" << config.repl_scope << ">\n";
  oss << "\n";

  // Generate main function
  oss << "int main(int argc, const char** argv) {\n";
  oss << "  // CoD workspace path available via environment\n";
  oss << "  // Project scope can access it via getenv(\"COD_WORKS_PATH\")\n";
  oss << "\n";
  oss << "  // Begin user submission\n";
  oss << "  {\n";

  // Indent user code
  std::istringstream input(submission);
  std::string line;
  while (std::getline(input, line)) {
    oss << "    " << line << "\n";
  }

  oss << "  }\n";
  oss << "  // End user submission\n";
  oss << "\n";
  oss << "  return 0;\n";
  oss << "}\n";

  return oss.str();
}

static fs::path getTempDir(const CodReplConfig &config) { return config.project_root / ".cod-repl-temp"; }

static std::vector<std::string> buildCompilerArgs(const CodReplConfig &config) {
  std::vector<std::string> args;
  args.push_back("clang++");
  args.push_back("-std=c++20");
  args.push_back("-stdlib=libc++");
  args.push_back("-I" + (config.project_root / "include").string());

  // Add library paths and libraries
  auto lib_path = config.project_root / "build" / "lib";
  if (fs::exists(lib_path)) {
    args.push_back("-L" + lib_path.string());
    args.push_back("-lshilos");
  }

  args.push_back("-O2");
  args.push_back("-g");
  return args;
}

static bool compileAndRun(const CodReplConfig &config, const std::string &submission) {
  auto temp_dir = getTempDir(config);

  try {
    // Create temp directory
    fs::create_directories(temp_dir);

    // Generate source file
    auto source_path = temp_dir / "runner.cc";
    auto binary_path = temp_dir / "runner";

    std::ofstream source_file(source_path);
    if (!source_file) {
      std::cerr << "Error: Cannot create temporary source file\n";
      return false;
    }

    source_file << generateRunnerSource(config, submission);
    source_file.close();

    // Build compiler command
    auto compiler_args = buildCompilerArgs(config);
    compiler_args.push_back(source_path.string());
    compiler_args.push_back("-o");
    compiler_args.push_back(binary_path.string());

    // Convert to char* array for execvp
    std::vector<char *> argv;
    for (auto &arg : compiler_args) {
      argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);

    // Fork and compile
    pid_t compile_pid = fork();
    if (compile_pid == 0) {
      // Child process - execute compiler
      execvp(argv[0], argv.data());
      std::cerr << "Error: Failed to execute compiler\n";
      exit(1);
    } else if (compile_pid > 0) {
      // Parent process - wait for compilation
      int status;
      waitpid(compile_pid, &status, 0);

      if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        // Compilation successful, now run the binary
        // Set environment for runner
        setenv("COD_WORKS_PATH", config.works_path.c_str(), 1);

        pid_t run_pid = fork();
        if (run_pid == 0) {
          // Child process - execute binary
          execl(binary_path.c_str(), binary_path.c_str(), nullptr);
          std::cerr << "Error: Failed to execute binary\n";
          exit(1);
        } else if (run_pid > 0) {
          // Parent process - wait for execution
          int run_status;
          waitpid(run_pid, &run_status, 0);

          if (WIFEXITED(run_status)) {
            if (WEXITSTATUS(run_status) != 0) {
              std::cerr << "Program exited with code " << WEXITSTATUS(run_status) << "\n";
            }
          } else {
            std::cerr << "Program terminated abnormally\n";
          }
        } else {
          std::cerr << "Error: Failed to fork for execution\n";
          return false;
        }
      } else {
        std::cerr << "Compilation failed\n";
        return false;
      }
    } else {
      std::cerr << "Error: Failed to fork for compilation\n";
      return false;
    }

    // Clean up temporary files
    std::error_code ec;
    fs::remove(source_path, ec);
    fs::remove(binary_path, ec);

    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error during compile and run: " << e.what() << "\n";
    return false;
  }
}

static void runRepl(const CodReplConfig &config) {
  std::cout << "CoD REPL - Compile-on-Demand without JIT\n";
  std::cout << "Workspace: " << config.works_path << "\n";
  std::cout << "Project: " << config.project_root << "\n";
  std::cout << "Scope: " << config.repl_scope << "\n";
  std::cout << "Type 'help' for help, '%quit' to exit.\n";
  std::cout << "\n";

  std::string input;
  std::string accumulated_input;

  while (true) {
    if (accumulated_input.empty()) {
      std::cout << "cod> ";
    } else {
      std::cout << "...  ";
    }

    if (!std::getline(std::cin, input)) {
      // EOF
      break;
    }

    // Trim whitespace
    input.erase(0, input.find_first_not_of(" \t"));
    input.erase(input.find_last_not_of(" \t") + 1);

    // Handle commands
    if (input == "%quit" || input == "quit") {
      break;
    }

    if (input == "%help" || input == "help") {
      printReplHelp();
      continue;
    }

    // Handle line continuation
    if (input.ends_with("\\")) {
      accumulated_input += input.substr(0, input.length() - 1) + "\n";
      continue;
    }

    // Complete input
    accumulated_input += input;

    if (!accumulated_input.empty()) {
      compileAndRun(config, accumulated_input);
    }

    accumulated_input.clear();
  }

  std::cout << "\nGoodbye!\n";
}

int main(int argc, const char **argv) {
  CodReplConfig config;

  // Parse command line arguments
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "-h" || arg == "--help") {
      printUsage(argv[0]);
      return 0;
    } else if (arg == "-w" || arg == "--works") {
      if (i + 1 >= argc) {
        std::cerr << "Error: " << arg << " requires a path argument\n";
        return 1;
      }
      config.works_path = argv[++i];
    } else if (arg == "--project") {
      if (i + 1 >= argc) {
        std::cerr << "Error: " << arg << " requires a path argument\n";
        return 1;
      }
      config.project_root = argv[++i];
    } else if (arg == "-e" || arg == "--eval") {
      if (i + 1 >= argc) {
        std::cerr << "Error: " << arg << " requires an expression argument\n";
        return 1;
      }
      config.eval_expression = argv[++i];
    } else {
      std::cerr << "Error: Unknown argument " << arg << "\n";
      printUsage(argv[0]);
      return 1;
    }
  }

  // Auto-detect project root if not specified
  if (config.project_root.empty()) {
    auto detected = findProjectRoot(fs::current_path());
    if (!detected) {
      std::cerr
          << "Error: Could not find CodProject.yaml. Please specify --project or run from a CoD project directory.\n";
      return 1;
    }
    config.project_root = *detected;
  }

  // Load project configuration
  auto loaded_config = loadConfig(config.project_root);
  if (!loaded_config) {
    std::cerr << "Error: Failed to load project configuration\n";
    return 1;
  }
  config = *loaded_config;

  // Ensure DBMR workspace exists
  if (!ensureDbmrExists(config.works_path, config.dbmr_capacity)) {
    std::cerr << "Error: Failed to initialize workspace\n";
    return 1;
  }

  // Check if we're in eval mode or REPL mode
  if (config.eval_expression.has_value()) {
    // Eval mode: compile and run the expression, then exit
    bool success = compileAndRun(config, *config.eval_expression);
    return success ? 0 : 1;
  } else {
    // Interactive REPL mode
    runRepl(config);
    return 0;
  }
}
