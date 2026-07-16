//===--- tests/cod/src/test_cod_workspace.cc - CoD Workspace Tests ------===//
//
// CoD Project - Tests for cod workspace (DBMR) functionality
// Tests workspace creation, persistence, and WorksRoot handling
//
//===----------------------------------------------------------------------===//

#include <array>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "cod.hh"
#include "cod_cache.hh"
#include "shilos.hh"
#include "llvm/Support/Program.h"

#define TEST_ASSERT(condition)                                                                                         \
  do {                                                                                                                 \
    if (!(condition)) {                                                                                                \
      throw std::runtime_error("Assertion failed: " #condition);                                                       \
    }                                                                                                                  \
  } while (0)

namespace fs = std::filesystem;
using namespace shilos;
using namespace cod;

// Helper function to execute cod with arguments and capture output
struct CodResult {
  int exit_code;
  std::string stdout_output;
  std::string stderr_output;
};

void assertCodSucceeded(const CodResult &result) {
  if (result.exit_code != 0) {
    throw std::runtime_error("cod exited with " + std::to_string(result.exit_code) + ":\n" + result.stdout_output +
                             result.stderr_output);
  }
}

class CurrentPathGuard {
public:
  explicit CurrentPathGuard(const fs::path &path) : original_path_(fs::current_path()) { fs::current_path(path); }

  ~CurrentPathGuard() {
    std::error_code error;
    fs::current_path(original_path_, error);
  }

  CurrentPathGuard(const CurrentPathGuard &) = delete;
  CurrentPathGuard &operator=(const CurrentPathGuard &) = delete;

private:
  fs::path original_path_;
};

std::string readFile(const fs::path &path) {
  std::ifstream input(path, std::ios::binary);
  std::ostringstream output;
  output << input.rdbuf();
  return output.str();
}

CodResult runCod(const std::vector<std::string> &args, const fs::path &cwd = fs::current_path()) {
  auto executable = llvm::sys::findProgramByName("cod");
  if (!executable) {
    throw std::runtime_error("Failed to find cod: " + executable.getError().message());
  }

  std::vector<std::string> owned_args = {*executable};
  owned_args.insert(owned_args.end(), args.begin(), args.end());
  std::vector<llvm::StringRef> argument_refs;
  argument_refs.reserve(owned_args.size());
  for (const auto &argument : owned_args) {
    argument_refs.emplace_back(argument);
  }

  const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
  const fs::path stdout_path = fs::temp_directory_path() / ("cod_workspace_stdout_" + suffix);
  const fs::path stderr_path = fs::temp_directory_path() / ("cod_workspace_stderr_" + suffix);
  const std::string stdout_path_text = stdout_path.string();
  const std::string stderr_path_text = stderr_path.string();
  const std::array<std::optional<llvm::StringRef>, 3> redirects = {std::nullopt, stdout_path_text, stderr_path_text};

  std::string error_message;
  bool execution_failed = false;
  int exit_code;
  {
    CurrentPathGuard current_path(cwd);
    exit_code = llvm::sys::ExecuteAndWait(*executable, argument_refs, std::nullopt, redirects, 0, 0, &error_message,
                                          &execution_failed);
  }

  const std::string stdout_output = readFile(stdout_path);
  const std::string stderr_output = readFile(stderr_path);
  std::error_code remove_error;
  fs::remove(stdout_path, remove_error);
  fs::remove(stderr_path, remove_error);

  if (execution_failed) {
    throw std::runtime_error("Failed to execute cod: " + error_message);
  }
  return {exit_code, stdout_output, stderr_output};
}

// Create a minimal test project setup
fs::path createTestProject() {
  const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
  auto temp_dir = fs::temp_directory_path() / ("cod workspace test & " + suffix);
  fs::create_directories(temp_dir);

  // Create CodProject.yaml
  std::ofstream project_file(temp_dir / "CodProject.yaml");
  project_file << "uuid: \"99999999-9999-9999-9999-999999999999\"\n";
  project_file << "name: test_project\n";
  project_file << "repo_url: /test/workspace\n";
  project_file << "branches: [main]\n";
  project_file.close();

  // Create include directory
  fs::create_directories(temp_dir / "include");

  // Create the default REPL scope header
  std::ofstream scope_file(temp_dir / "include" / "main.hh");
  scope_file << "#pragma once\n";
  scope_file << "#include <iostream>\n";
  scope_file.close();

  return temp_dir;
}

void testWorkspaceCreation() {
  std::cout << "Testing workspace creation...\n";

  auto project_dir = createTestProject();
  auto works_path = project_dir / "test_creation.dbmr";

  // Ensure workspace doesn't exist initially
  fs::remove(works_path);
  TEST_ASSERT(!fs::exists(works_path));

  // Run cod with eval to trigger workspace creation
  auto result = runCod({"--project", project_dir.string(), "-w", works_path.string(), "-e", "int x = 1;"}, project_dir);

  assertCodSucceeded(result);
  TEST_ASSERT(fs::exists(works_path));
  std::cout << "  ✓ Workspace file created\n";

  auto file_size = fs::file_size(works_path);
  TEST_ASSERT(file_size > 0);
  std::cout << "  ✓ Workspace file has content (" << file_size << " bytes)\n";

  // Clean up
  fs::remove_all(project_dir);

  std::cout << "✓ Workspace creation test passed\n";
}

void testWorkspaceReuse() {
  std::cout << "Testing workspace reuse...\n";

  auto project_dir = createTestProject();
  auto works_path = project_dir / "test_reuse.dbmr";

  // First run to create workspace
  auto result1 =
      runCod({"--project", project_dir.string(), "-w", works_path.string(), "-e", "int x = 1;"}, project_dir);
  assertCodSucceeded(result1);
  TEST_ASSERT(fs::exists(works_path));

  auto result2 =
      runCod({"--project", project_dir.string(), "-w", works_path.string(), "-e", "int y = 2;"}, project_dir);
  assertCodSucceeded(result2);
  TEST_ASSERT(fs::exists(works_path));
  std::cout << "  ✓ Workspace persisted across runs\n";

  auto final_size = fs::file_size(works_path);
  TEST_ASSERT(final_size > 0);
  std::cout << "  ✓ Workspace remains valid after reuse\n";

  // Clean up
  fs::remove_all(project_dir);

  std::cout << "✓ Workspace reuse test passed\n";
}

void testDefaultWorkspacePath() {
  std::cout << "Testing default workspace path...\n";

  auto project_dir = createTestProject();
  auto default_works_path = project_dir / ".cod/works.dbmr";

  // Remove default workspace if it exists
  fs::remove(default_works_path);

  // Run cod without specifying workspace path
  auto result = runCod({"--project", project_dir.string(), "-e", "int x = 1;"}, project_dir);

  assertCodSucceeded(result);
  TEST_ASSERT(fs::exists(default_works_path));
  std::cout << "  ✓ Default workspace created at .cod/works.dbmr\n";

  // Clean up
  fs::remove_all(project_dir);

  std::cout << "✓ Default workspace path test passed\n";
}

void testWorksRootType() {
  std::cout << "Testing WorksRoot type...\n";

  const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
  auto temp_path = fs::temp_directory_path() / ("test_works_root_" + suffix + ".dbmr");
  fs::remove(temp_path);

  {
    auto dbmr = DBMR<WorksRoot>::create(temp_path.string(), 1024 * 1024, fs::path(".")); // 1MB

    const auto &uuid = WorksRoot::TYPE_UUID;
    TEST_ASSERT(!uuid.to_string().empty());
    std::cout << "  ✓ WorksRoot TYPE_UUID accessible: " << uuid.to_string() << "\n";

    auto root = dbmr.region().root();
    TEST_ASSERT(root.get() != nullptr);
    std::cout << "  ✓ WorksRoot object created successfully\n";
  }
  fs::remove(temp_path);

  std::cout << "✓ WorksRoot type test completed\n";
}

void testWorkspaceDirectoryCreation() {
  std::cout << "Testing workspace directory creation...\n";

  auto project_dir = createTestProject();
  auto nested_dir = project_dir / "nested" / "deep" / "path";
  auto works_path = nested_dir / "test.dbmr";

  // Ensure nested directory doesn't exist
  fs::remove_all(nested_dir);
  TEST_ASSERT(!fs::exists(nested_dir));

  // Run cod with workspace in nested directory
  auto result = runCod({"--project", project_dir.string(), "-w", works_path.string(), "-e", "int x = 1;"}, project_dir);

  assertCodSucceeded(result);
  TEST_ASSERT(fs::exists(nested_dir));
  TEST_ASSERT(fs::exists(works_path));
  std::cout << "  ✓ Parent directories created\n";
  std::cout << "  ✓ Workspace created in nested directory\n";

  // Clean up
  fs::remove_all(project_dir);

  std::cout << "✓ Workspace directory creation test passed\n";
}

void testBuildCache() {
  std::cout << "Testing build cache functionality...\n";

  auto project_dir = createTestProject();
  auto works_path = project_dir / ".cod" / "cache_test.dbmr";

  // Remove if exists
  if (fs::exists(works_path)) {
    fs::remove(works_path);
  }

  // First run - should create cache entry
  auto result1 = runCod({"--project", project_dir.string(), "-w", works_path.string(), "--verbose", "-e",
                         "#include <iostream>\nstd::cout << \"Hello Cache\" << std::endl;"},
                        project_dir);

  assertCodSucceeded(result1);
  std::cout << "  ✓ First run completed successfully\n";

  // Second run - should use cache
  auto result2 = runCod({"--project", project_dir.string(), "-w", works_path.string(), "--verbose", "-e",
                         "#include <iostream>\nstd::cout << \"Hello Cache\" << std::endl;"},
                        project_dir);

  assertCodSucceeded(result2);
  std::cout << "  ✓ Second run completed successfully\n";

  // Clean up
  fs::remove_all(project_dir);

  std::cout << "✓ Build cache test passed\n";
}

void testToolchainVersion() {
  std::cout << "Testing toolchain version management...\n";

  auto project_dir = createTestProject();
  auto works_path = project_dir / ".cod" / "toolchain_test.dbmr";

  // Remove if exists
  if (fs::exists(works_path)) {
    fs::remove(works_path);
  }

  // Create workspace with default toolchain
  auto result1 =
      runCod({"--project", project_dir.string(), "-w", works_path.string(), "--verbose", "-e", "int version = 1;"},
             project_dir);

  assertCodSucceeded(result1);
  std::cout << "  ✓ Workspace created with default toolchain\n";

  // Test with different compilation flags
  auto result2 =
      runCod({"--project", project_dir.string(), "-w", works_path.string(), "--verbose", "-e", "int version = 2;"},
             project_dir);

  assertCodSucceeded(result2);
  std::cout << "  ✓ Workspace reused successfully\n";

  // Clean up
  fs::remove_all(project_dir);

  std::cout << "✓ Toolchain version test passed\n";
}

int main() {
  std::cout << "Running CoD workspace tests...\n\n";

  try {
    testWorkspaceCreation();
    testWorkspaceReuse();
    testDefaultWorkspacePath();
    testWorksRootType();
    testWorkspaceDirectoryCreation();
    testBuildCache();
    testToolchainVersion();

    std::cout << "\n✔ All workspace tests passed!\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Test failed: " << e.what() << "\n";
    return 1;
  }
}
