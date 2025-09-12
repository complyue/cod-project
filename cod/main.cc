//===--- cod/main.cc - Compile-on-Demand REPL Driver ----------------===//
//
// CoD Project - Compile-on-Demand REPL without JIT
// Implements build-and-run REPL with persistent DBMR workspace
//
//===----------------------------------------------------------------------===//

#include "cod.hh"
#include "cod_cache.hh"

#include "codp.hh"
#include "codp_yaml.hh"

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Tool.h"
#include "clang/Driver/ToolChain.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/raw_ostream.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <system_error>
#include <unistd.h>
#include <vector>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <limits.h>
#endif

namespace fs = std::filesystem;
using namespace shilos;
using namespace cod;
using namespace cod::project;

// Helper class to access LLVM Driver APIs for toolchain detection
struct CodReplConfig {
  fs::path works_path = ".cod/works.dbmr";
  fs::path project_root;
  std::string repl_scope = "main.hh";
  std::string works_root_type_qualified = "cod::WorksRoot";
  std::string works_root_type_header = "cod.hh";
  size_t dbmr_capacity = 64 * 1024 * 1024;    // 64MB default
  std::optional<std::string> eval_expression; // For -e/--eval mode
  bool enable_cache = true;
  bool force_rebuild = false;
  bool verbose = false;
  std::chrono::hours cache_max_age = std::chrono::hours(24 * 7); // 1 week
};

class CodTool {
private:
  std::string executable_path_;

public:
  CodTool() {
    // this is often used, get it once
    executable_path_ = llvm::sys::fs::getMainExecutable(nullptr, nullptr);
  }

  const char *getClangProgramPath() const { return executable_path_.c_str(); }

  std::string getResourcesPath() const { return clang::driver::Driver::GetResourcesPath(executable_path_, ""); }

  std::vector<std::string> buildLinkerArgs(const CodReplConfig &config) const {
    std::vector<std::string> args;

    // Force use of lld from the same toolchain
    args.push_back("-fuse-ld=lld");

    // Add toolchain lib directory to rpath for libc++abi.1.dylib
    // Use LLVM Driver API to get resource directory from cod's path
    std::string ResourceDir = getResourcesPath();

    if (!ResourceDir.empty()) {
      // The lib directory is typically at the same level as the resource directory
      // ResourceDir is like '/path/to/build/lib/clang/18', so we need to go up two levels to get '/path/to/build/lib'
      fs::path toolchain_lib = fs::path(ResourceDir).parent_path().parent_path();
      if (fs::exists(toolchain_lib)) {
        std::string rpath_arg = "-Wl,-rpath," + fs::absolute(toolchain_lib).string();
        args.push_back(rpath_arg);
        if (config.verbose) {
          std::cerr << "[DEBUG] Adding rpath: " << fs::absolute(toolchain_lib).string() << "\n";
        }
      }
    }

    // Add shilos library from toolchain
    if (!ResourceDir.empty()) {
      // The lib directory is typically at the same level as the resource directory
      // ResourceDir is like '/path/to/build/lib/clang/18', so we need to go up two levels to get '/path/to/build/lib'
      fs::path toolchain_lib = fs::path(ResourceDir).parent_path().parent_path();
      if (fs::exists(toolchain_lib)) {
        args.push_back("-L" + fs::absolute(toolchain_lib).string());
        args.push_back("-lshilos");
        if (config.verbose) {
          std::cerr << "[DEBUG] Adding library path: " << fs::absolute(toolchain_lib).string() << "\n";
        }
      }
    }

    return args;
  }

  bool compileAndRun(const CodReplConfig &config, const std::string &submission) const;
  void runRepl(const CodReplConfig &config) const;
};

static void printUsage(const char *prog_name) {
  std::cout << "Usage: " << prog_name << " [OPTIONS] [EXPRESSION]\n"
            << "\n"
            << "Compile-on-Demand REPL - Build-and-run REPL without JIT\n"
            << "\n"
            << "Options:\n"
            << "  -w, --works PATH    Workspace DBMR file path (default: ./.cod/works.dbmr)\n"
            << "  --project PATH      Project root directory (default: auto-detect)\n"
            << "  -e, --eval EXPR     Evaluate expression/statement and exit\n"
            << "  -h, --help          Show this help message\n"
            << "  --no-cache          Disable build cache\n"
            << "  --force-rebuild     Force rebuild (ignore cache)\n"
            << "  -v, --verbose       Enable verbose debug output\n"
            << "  --cache-max-age=HOURS Set cache expiration time in hours\n"
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
    // Project root extracted from project_ptr

    // Use project configuration if available
    // Extract configuration from the parsed project
    if (project_ptr) {
      // For now, we'll manually check for works_root_type_header in the YAML
      // This is a simplified approach until full project config integration
      auto root_node = doc.root();
      if (root_node.IsMap()) {
        const auto &map = std::get<yaml::Map>(root_node.value);
        auto it = std::find_if(map.begin(), map.end(),
                               [](const auto &entry) { return entry.key == "works_root_type_header"; });
        if (it != map.end() && it->value.IsScalar()) {
          config.works_root_type_header = it->value.asString();
        }
      }
    }
    return config;
  } catch (const std::exception &e) {
    std::cerr << "Error loading config: " << e.what() << "\n";
    return std::nullopt;
  }
}

static bool ensureDbmrExists(const CodReplConfig &config) {
  if (fs::exists(config.works_path)) {
    // Load existing workspace and check if project root matches
    try {
      auto dbmr = DBMR<WorksRoot>(config.works_path.string(), 0);
      auto root = dbmr.region().root();
      auto stored_project_root = root->get_project_root();

      // Convert both paths to absolute for comparison
      auto stored_abs = fs::absolute(stored_project_root);
      auto config_abs = fs::absolute(config.project_root);

      if (stored_abs != config_abs) {
        std::cerr << "Warning: Existing workspace has different project root\n";
        std::cerr << "  Stored: " << stored_project_root << "\n";
        std::cerr << "  Requested: " << config.project_root << "\n";
        std::cerr << "  Recreating workspace with new project root...\n";

        // Remove existing workspace and recreate
        fs::remove(config.works_path);
      } else {
        std::cout << "Using existing workspace: " << config.works_path << "\n";
        std::cout << "Project root: " << stored_project_root << "\n";
        return true;
      }
    } catch (const std::exception &e) {
      std::cerr << "Error opening existing workspace: " << e.what() << "\n";
      return false;
    }
  }

  try {
    // Create parent directories
    fs::create_directories(config.works_path.parent_path());

    // Create DBMR with WorksRoot
    auto dbmr = DBMR<WorksRoot>::create(config.works_path.string(), config.dbmr_capacity, config.project_root);

    // WorksRoot is already initialized with the correct project root
    auto root = dbmr.region().root();

    std::cout << "Created new workspace: " << config.works_path << " (" << config.dbmr_capacity / (1024 * 1024)
              << "MB)\n";
    std::cout << "Project root: " << root->get_project_root() << "\n";
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error creating workspace: " << e.what() << "\n";
    return false;
  }
}

// AST visitor to analyze submission structure
class SubmissionAnalyzer : public clang::RecursiveASTVisitor<SubmissionAnalyzer> {
public:
  struct AnalysisResult {
    std::vector<clang::Stmt *> statements;
    clang::Expr *final_expression = nullptr;
    bool has_final_expression = false;
  };

  AnalysisResult result;

  bool VisitCompoundStmt(clang::CompoundStmt *CS) {
    // Analyze the compound statement to find statements and final expression
    auto children = CS->children();
    std::vector<clang::Stmt *> stmts(children.begin(), children.end());

    if (!stmts.empty()) {
      // Check if the last statement is an expression statement
      if (auto *ES = clang::dyn_cast<clang::Expr>(stmts.back())) {
        // The last statement is an expression - this could be our final expression
        result.final_expression = ES;
        result.has_final_expression = true;
        // Add all statements except the last one
        result.statements.assign(stmts.begin(), stmts.end() - 1);
      } else {
        // No final expression, all are statements
        result.statements = stmts;
        result.has_final_expression = false;
      }
    }
    return true;
  }
};

// Simplified analysis result structure
struct SubmissionAnalysis {
  std::vector<std::string> statements;
  std::string final_expression;
  bool has_final_expression = false;
};

// Helper function to analyze submission structure
static SubmissionAnalysis analyzeSubmissionStructure(const std::string &submission) {
  SubmissionAnalysis result;

  // Handle single line input differently
  if (submission.find('\n') == std::string::npos) {
    // Single line - split by semicolons and analyze each part
    std::string trimmed = submission;
    trimmed.erase(0, trimmed.find_first_not_of(" \t"));
    trimmed.erase(trimmed.find_last_not_of(" \t") + 1);

    if (trimmed.empty()) {
      return result;
    }

    // Split by semicolons
    std::vector<std::string> parts;
    std::istringstream iss(trimmed);
    std::string part;

    while (std::getline(iss, part, ';')) {
      // Trim each part
      part.erase(0, part.find_first_not_of(" \t"));
      part.erase(part.find_last_not_of(" \t") + 1);
      if (!part.empty()) {
        parts.push_back(part);
      }
    }

    if (parts.empty()) {
      return result;
    }

    // Check if the last part looks like a statement or expression
    const std::string &last_part = parts.back();
    bool last_is_statement =
        (last_part.find("int ") == 0 || last_part.find("auto ") == 0 || last_part.find("const ") == 0 ||
         last_part.find("if ") == 0 || last_part.find("for ") == 0 || last_part.find("while ") == 0 ||
         last_part.find("return ") == 0 || last_part.find("std::cout") == 0);

    if (parts.size() == 1) {
      // Single part - if it's not clearly a statement, treat as expression
      if (last_is_statement) {
        result.statements = parts;
        result.has_final_expression = false;
      } else {
        result.final_expression = last_part;
        result.has_final_expression = true;
      }
    } else {
      // Multiple parts - last part is expression if not clearly a statement
      if (!last_is_statement) {
        result.statements.assign(parts.begin(), parts.end() - 1);
        result.final_expression = last_part;
        result.has_final_expression = true;
      } else {
        result.statements = parts;
        result.has_final_expression = false;
      }
    }

    return result;
  }

  // Multi-line input - split by lines
  std::istringstream iss(submission);
  std::string line;
  std::vector<std::string> lines;

  while (std::getline(iss, line)) {
    // Trim whitespace
    line.erase(0, line.find_first_not_of(" \t"));
    line.erase(line.find_last_not_of(" \t") + 1);
    if (!line.empty()) {
      lines.push_back(line);
    }
  }

  if (lines.empty()) {
    return result;
  }

  // Check if the last line is an expression (doesn't end with ; or })
  const std::string &last_line = lines.back();
  bool is_statement = (last_line.back() == ';' || last_line.back() == '}' || last_line.find("if ") == 0 ||
                       last_line.find("for ") == 0 || last_line.find("while ") == 0 || last_line.find("return ") == 0 ||
                       last_line.find("int ") == 0 || last_line.find("auto ") == 0 || last_line.find("const ") == 0);

  if (!is_statement && lines.size() > 1) {
    // Last line is likely an expression, previous lines are statements
    result.statements.assign(lines.begin(), lines.end() - 1);
    result.final_expression = last_line;
    result.has_final_expression = true;
  } else {
    // All lines are statements
    result.statements = lines;
    result.has_final_expression = false;
  }

  return result;
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

  // Use AST-based analysis to determine structure
  auto analysis = analyzeSubmissionStructure(submission);

  // Add all statements first
  for (const auto &stmt : analysis.statements) {
    oss << "    " << stmt;
    // Add semicolon if not already present and not ending with }
    if (!stmt.empty() && stmt.back() != ';' && stmt.back() != '}') {
      oss << ";";
    }
    oss << "\n";
  }

  // Handle final expression - wrap in cout if it exists
  if (analysis.has_final_expression && !analysis.final_expression.empty()) {
    oss << "    std::cout << (" << analysis.final_expression << ") << std::endl;\n";
  }

  oss << "  }\n";
  oss << "  // End user submission\n";
  oss << "\n";
  oss << "  return 0;\n";
  oss << "}\n";

  return oss.str();
}

static fs::path getTempDir(const CodReplConfig &config) { return config.project_root / ".cod/repl"; }

static std::vector<std::string> buildCompilerArgs(const CodReplConfig &config) {
  std::vector<std::string> args;

  // Note: Do not include the compiler path here - it's handled by BitcodeCompiler
  args.push_back("-std=c++20");
  args.push_back("-stdlib=libc++");

  // Add include path for current project
  auto project_include = config.project_root / "include";
  if (fs::exists(project_include)) {
    args.push_back("-I" + project_include.string());
  }

  // If we're in a test directory or the current project doesn't have cod.hh,
  // also add the main CoD project include directory
  auto cod_header = project_include / "cod.hh";
  if (!fs::exists(cod_header)) {
    // Try to find the main CoD project root by looking for a parent directory
    // that contains both include/cod.hh and build/ directories
    auto current = config.project_root;
    while (!current.empty() && current != current.parent_path()) {
      auto parent_include = current.parent_path() / "include";
      auto parent_cod_header = parent_include / "cod.hh";
      auto parent_build = current.parent_path() / "build";

      if (fs::exists(parent_cod_header) && fs::exists(parent_build)) {
        args.push_back("-I" + parent_include.string());
        break;
      }
      current = current.parent_path();
    }
  }

  // Note: Library paths and libraries are NOT included here
  // They belong in linker arguments, not compiler arguments for bitcode generation

  args.push_back("-O2");
  args.push_back("-g");
  return args;
}

// CodTool method implementations
bool CodTool::compileAndRun(const CodReplConfig &config, const std::string &submission) const {
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

    bool success = false;

    // Open workspace
    auto dbmr = DBMR<WorksRoot>(config.works_path.string(), 0);

    // Create build cache separately (not part of workspace state)
    cache::BuildCache cache(config.project_root, config.verbose);

    // Build compiler arguments
    auto compiler_args = cod::CompilerArgs(buildCompilerArgs(config));
    std::string project_snapshot = "temp"; // For temp files, use simple snapshot

    std::optional<fs::path> cached_bitcode;

    // Try cache lookup if enabled and not forcing rebuild
    if (config.enable_cache && !config.force_rebuild) {
      cached_bitcode = cache.lookup(source_path.string(), compiler_args, project_snapshot);
    }

    // Generate bitcode if not cached
    if (!cached_bitcode) {
      cached_bitcode = cache.generate_bitcode(source_path.string(), compiler_args);

      if (!cached_bitcode) {
        std::cerr << "Error: Failed to generate bitcode\n";
        fs::remove(source_path);
        return false;
      }

      // Store in cache for future use
      if (config.enable_cache) {
        cache.store(source_path.string(), *cached_bitcode, compiler_args, project_snapshot);
      }
    }

    // Link bitcode to executable
    cod::cache::BitcodeCompiler compiler;
    std::vector<fs::path> bitcode_files = {*cached_bitcode};
    auto linker_args = cod::LinkerArgs(buildLinkerArgs(config));

    if (!compiler.link_bitcode(bitcode_files, binary_path.string(), linker_args)) {
      std::cerr << "Error: Failed to link bitcode\n";
      fs::remove(source_path);
      return false;
    }

    // Set environment for runner
    setenv("COD_WORKS_PATH", config.works_path.c_str(), 1);

    // Execute the compiled program
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
        success = (WEXITSTATUS(run_status) == 0);
      } else {
        std::cerr << "Program terminated abnormally\n";
        success = false;
      }
    } else {
      std::cerr << "Error: Failed to fork for execution\n";
      success = false;
    }

    // Clean up temporary files
    std::error_code ec;
    fs::remove(source_path, ec);
    fs::remove(binary_path, ec);

    return success;
  } catch (const std::exception &e) {
    std::cerr << "Error during compile and run: " << e.what() << "\n";
    return false;
  }
}

void CodTool::runRepl(const CodReplConfig &config) const {
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
      config.project_root = fs::absolute(argv[++i]);
    } else if (arg == "-e" || arg == "--eval") {
      if (i + 1 >= argc) {
        std::cerr << "Error: " << arg << " requires an expression argument\n";
        return 1;
      }
      std::string expr_arg = argv[++i];
      if (expr_arg == "-") {
        // Read from stdin
        std::string line;
        std::ostringstream oss;
        while (std::getline(std::cin, line)) {
          oss << line << "\n";
        }
        config.eval_expression = oss.str();
        // Remove trailing newline if present
        if (!config.eval_expression->empty() && config.eval_expression->back() == '\n') {
          config.eval_expression->pop_back();
        }
      } else {
        config.eval_expression = expr_arg;
      }
    } else if (arg == "--no-cache") {
      config.enable_cache = false;
    } else if (arg == "--force-rebuild") {
      config.force_rebuild = true;
    } else if (arg == "-v" || arg == "--verbose") {
      config.verbose = true;
    } else if (arg.starts_with("--cache-max-age=")) {
      int hours = std::stoi(arg.substr(16));
      config.cache_max_age = std::chrono::hours(hours);
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
    config.project_root = fs::absolute(*detected);
  }

  // Load project configuration
  auto loaded_config = loadConfig(config.project_root);
  if (!loaded_config) {
    std::cerr << "Error: Failed to load project configuration\n";
    return 1;
  }
  // Preserve command-line settings when merging with loaded config
  auto eval_expr = config.eval_expression;
  auto works_path = config.works_path;
  auto project_root = config.project_root;
  auto enable_cache = config.enable_cache;
  auto force_rebuild = config.force_rebuild;
  auto verbose = config.verbose;
  auto cache_max_age = config.cache_max_age;
  config = *loaded_config;

  // Restore command-line settings
  config.eval_expression = eval_expr;
  if (works_path != "./.cod/works.dbmr")
    config.works_path = works_path;
  if (!project_root.empty()) {
    config.project_root = project_root;
    // Update works_path to be relative to the project root
    if (config.works_path == ".cod/works.dbmr") {
      config.works_path = fs::path(project_root) / ".cod/works.dbmr";
    }
  }
  config.enable_cache = enable_cache;
  config.force_rebuild = force_rebuild;
  config.verbose = verbose;
  config.cache_max_age = cache_max_age;

  // Ensure DBMR workspace exists
  if (!ensureDbmrExists(config)) {
    std::cerr << "Error: Failed to initialize workspace\n";
    return 1;
  }

  // Create cod tool
  CodTool cod_tool;

  // Check if we're in eval mode or REPL mode
  if (config.eval_expression.has_value()) {
    // Eval mode: compile and run the expression, then exit
    bool success = cod_tool.compileAndRun(config, *config.eval_expression);
    return success ? 0 : 1;
  } else {
    // Interactive REPL mode
    cod_tool.runRepl(config);
    return 0;
  }
}
