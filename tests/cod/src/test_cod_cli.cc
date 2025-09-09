//===--- tests/cod/src/test_cod_cli.cc - CoD CLI Tests ------------------===//
//
// CoD Project - Tests for cod command-line interface
// Tests argument parsing, help output, and basic CLI functionality
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

CodResult runCod(const std::vector<std::string> &args) {
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

void testHelpOption() {
  std::cout << "Testing --help option...\n";

  auto result = runCod({"--help"});
  assert(result.exit_code == 0);
  assert(result.stdout_output.find("Usage:") != std::string::npos);
  assert(result.stdout_output.find("-e, --eval") != std::string::npos);
  assert(result.stdout_output.find("-w, --works") != std::string::npos);

  // Test short form
  result = runCod({"-h"});
  assert(result.exit_code == 0);
  assert(result.stdout_output.find("Usage:") != std::string::npos);

  std::cout << "✓ Help option tests passed\n";
}

void testInvalidArguments() {
  std::cout << "Testing invalid arguments...\n";

  // Test unknown argument
  auto result = runCod({"--unknown"});
  assert(result.exit_code == 1);
  assert(result.stderr_output.find("Unknown argument") != std::string::npos);

  // Test missing argument for -w
  result = runCod({"-w"});
  assert(result.exit_code == 1);
  assert(result.stderr_output.find("requires a path argument") != std::string::npos);

  // Test missing argument for -e
  result = runCod({"-e"});
  assert(result.exit_code == 1);
  assert(result.stderr_output.find("requires an expression argument") != std::string::npos);

  // Test missing argument for --project
  result = runCod({"--project"});
  assert(result.exit_code == 1);
  assert(result.stderr_output.find("requires a path argument") != std::string::npos);

  std::cout << "✓ Invalid argument tests passed\n";
}

void testArgumentParsing() {
  std::cout << "Testing argument parsing...\n";

  // Create a temporary directory for testing
  auto temp_dir = fs::temp_directory_path() / "cod_cli_test";
  fs::create_directories(temp_dir);

  // Create a minimal CodProject.yaml
  std::ofstream project_file(temp_dir / "CodProject.yaml");
  project_file << "name: test_project\nversion: 1.0.0\n";
  project_file.close();

  // Test with custom works path (should fail gracefully since we don't have full project setup)
  auto works_path = temp_dir / "test.dbmr";
  auto result = runCod({"--project", temp_dir.string(), "-w", works_path.string(), "-e", "1 + 1"});
  // This might fail due to missing dependencies, but should not fail on argument parsing
  assert(result.stderr_output.find("Unknown argument") == std::string::npos);
  assert(result.stderr_output.find("requires a") == std::string::npos);

  // Clean up
  fs::remove_all(temp_dir);

  std::cout << "✓ Argument parsing tests passed\n";
}

int main() {
  std::cout << "Running CoD CLI tests...\n\n";

  try {
    testHelpOption();
    testInvalidArguments();
    testArgumentParsing();

    std::cout << "\n✔ All CLI tests passed!\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Test failed: " << e.what() << "\n";
    return 1;
  }
}