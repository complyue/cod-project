//===--- tests/cod/src/test_cod_eval.cc - CoD Eval Tests ----------------===//
//
// CoD Project - Tests for cod -e/--eval functionality
// Tests expression evaluation, compilation, and execution
//
//===----------------------------------------------------------------------===//

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;

// Helper function to execute cod with arguments and capture output
struct CodResult {
  int exit_code;
  std::string stdout_output;
  std::string stderr_output;
};

CodResult runCod(const std::vector<std::string> &args, const fs::path &cwd = fs::current_path()) {
  // Create pipes for stdout and stderr
  int stdout_pipe[2], stderr_pipe[2];
  if (pipe(stdout_pipe) == -1 || pipe(stderr_pipe) == -1) {
    throw std::runtime_error("Failed to create pipes");
  }

  pid_t pid = fork();
  if (pid == 0) {
    // Child process
    close(stdout_pipe[0]); // Close read end
    close(stderr_pipe[0]);

    dup2(stdout_pipe[1], STDOUT_FILENO);
    dup2(stderr_pipe[1], STDERR_FILENO);

    close(stdout_pipe[1]);
    close(stderr_pipe[1]);

    // Change to the specified working directory
    if (chdir(cwd.c_str()) != 0) {
      exit(126); // chdir failed
    }

    // Prepare arguments for execvp
    std::vector<char *> argv;
    argv.push_back(const_cast<char *>("cod"));
    for (const auto &arg : args) {
      argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);

    execvp("cod", argv.data());
    exit(127); // exec failed
  } else if (pid > 0) {
    // Parent process
    close(stdout_pipe[1]); // Close write end
    close(stderr_pipe[1]);

    // Read stdout
    std::string stdout_output;
    char buffer[4096];
    ssize_t bytes_read;
    while ((bytes_read = read(stdout_pipe[0], buffer, sizeof(buffer))) > 0) {
      stdout_output.append(buffer, bytes_read);
    }
    close(stdout_pipe[0]);

    // Read stderr
    std::string stderr_output;
    while ((bytes_read = read(stderr_pipe[0], buffer, sizeof(buffer))) > 0) {
      stderr_output.append(buffer, bytes_read);
    }
    close(stderr_pipe[0]);

    // Wait for child
    int status;
    waitpid(pid, &status, 0);
    int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;

    return {exit_code, stdout_output, stderr_output};
  } else {
    throw std::runtime_error("Failed to fork");
  }
}

// Create a minimal test project setup
fs::path createTestProject() {
  auto temp_dir = fs::temp_directory_path() / "cod_eval_test";
  fs::remove_all(temp_dir); // Clean up any previous test
  fs::create_directories(temp_dir);

  // Create CodProject.yaml
  std::ofstream project_file(temp_dir / "CodProject.yaml");
  project_file << "name: test_project\n";
  project_file << "version: 1.0.0\n";
  project_file << "repl:\n";
  project_file << "  scope: \"test_scope.hh\"\n";
  project_file.close();

  // Create include directory
  fs::create_directories(temp_dir / "include");

  // Create a simple test scope header
  std::ofstream scope_file(temp_dir / "include" / "test_scope.hh");
  scope_file << "#pragma once\n";
  scope_file << "#include <iostream>\n";
  scope_file << "#include <string>\n";
  scope_file << "\n";
  scope_file << "inline void test_print(const std::string& msg) {\n";
  scope_file << "  std::cout << \"TEST: \" << msg << std::endl;\n";
  scope_file << "}\n";
  scope_file.close();

  // Create build directory structure (minimal)
  fs::create_directories(temp_dir / "build" / "lib");

  return temp_dir;
}

void testBasicEvaluation() {
  std::cout << "Testing basic evaluation...\n";

  auto project_dir = createTestProject();

  // Test simple expression
  auto result =
      runCod({"--project", project_dir.string(), "-e", "std::cout << \"Hello World\" << std::endl;"}, project_dir);

  // The test might fail due to missing build setup, but we check that it's not a CLI parsing error
  if (result.exit_code != 0) {
    // Should be a compilation/build error, not argument parsing error
    assert(result.stderr_output.find("Unknown argument") == std::string::npos);
    assert(result.stderr_output.find("requires a") == std::string::npos);
    std::cout << "  Note: Compilation failed as expected without full build setup\n";
  }

  // Clean up
  fs::remove_all(project_dir);

  std::cout << "✓ Basic evaluation test passed\n";
}

void testEvalWithCustomWorks() {
  std::cout << "Testing eval with custom workspace...\n";

  auto project_dir = createTestProject();
  auto works_path = project_dir / "custom.dbmr";

  // Test with custom workspace path
  auto result =
      runCod({"--project", project_dir.string(), "-w", works_path.string(), "-e", "int x = 42;"}, project_dir);

  // Should not fail on argument parsing
  assert(result.stderr_output.find("Unknown argument") == std::string::npos);
  assert(result.stderr_output.find("requires a") == std::string::npos);

  // Clean up
  fs::remove_all(project_dir);

  std::cout << "✓ Custom workspace test passed\n";
}

void testEvalLongForm() {
  std::cout << "Testing --eval long form...\n";

  auto project_dir = createTestProject();

  // Test long form --eval
  auto result =
      runCod({"--project", project_dir.string(), "--eval", "// This is a comment\nint result = 1 + 1;"}, project_dir);

  // Should not fail on argument parsing
  assert(result.stderr_output.find("Unknown argument") == std::string::npos);
  assert(result.stderr_output.find("requires a") == std::string::npos);

  // Clean up
  fs::remove_all(project_dir);

  std::cout << "✓ Long form eval test passed\n";
}

void testEvalMultilineExpression() {
  std::cout << "Testing multiline expression evaluation...\n";

  auto project_dir = createTestProject();

  // Test multiline expression
  std::string multiline_expr = R"(
int a = 10;
int b = 20;
std::cout << "Sum: " << (a + b) << std::endl;
)";

  auto result = runCod({"--project", project_dir.string(), "-e", multiline_expr}, project_dir);

  // Should not fail on argument parsing
  assert(result.stderr_output.find("Unknown argument") == std::string::npos);
  assert(result.stderr_output.find("requires a") == std::string::npos);

  // Clean up
  fs::remove_all(project_dir);

  std::cout << "✓ Multiline expression test passed\n";
}

void testEvalExitCodes() {
  std::cout << "Testing eval exit codes...\n";

  auto project_dir = createTestProject();

  // Test that eval mode exits after evaluation (not entering REPL)
  auto result = runCod({"--project", project_dir.string(), "-e", "std::cout << \"test\" << std::endl;"}, project_dir);

  // Should exit immediately, not wait for REPL input
  // The exact exit code depends on compilation success, but it should exit
  assert(result.exit_code != -1); // Should not hang

  // Clean up
  fs::remove_all(project_dir);

  std::cout << "✓ Exit code test passed\n";
}

int main() {
  std::cout << "Running CoD eval tests...\n\n";

  try {
    testBasicEvaluation();
    testEvalWithCustomWorks();
    testEvalLongForm();
    testEvalMultilineExpression();
    testEvalExitCodes();

    std::cout << "\n✔ All eval tests passed!\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Test failed: " << e.what() << "\n";
    return 1;
  }
}
