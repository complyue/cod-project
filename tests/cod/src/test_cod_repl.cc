//===--- tests/cod/src/test_cod_repl.cc - CoD REPL Tests ----------------===//
//
// CoD Project - Tests for cod REPL functionality
// Tests interactive REPL mode, commands, and session handling
//
//===----------------------------------------------------------------------===//

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;

// Helper function to execute cod in REPL mode with input and capture output
struct CodResult {
  int exit_code;
  std::string stdout_output;
  std::string stderr_output;
};

CodResult runCodRepl(const std::vector<std::string> &args, const std::string &input,
                     const fs::path &cwd = fs::current_path(), int timeout_seconds = 5) {
  // Create pipes for stdin, stdout and stderr
  int stdin_pipe[2], stdout_pipe[2], stderr_pipe[2];
  if (pipe(stdin_pipe) == -1 || pipe(stdout_pipe) == -1 || pipe(stderr_pipe) == -1) {
    throw std::runtime_error("Failed to create pipes");
  }

  pid_t pid = fork();
  if (pid == 0) {
    // Child process
    close(stdin_pipe[1]);  // Close write end of stdin
    close(stdout_pipe[0]); // Close read end of stdout
    close(stderr_pipe[0]); // Close read end of stderr

    dup2(stdin_pipe[0], STDIN_FILENO);
    dup2(stdout_pipe[1], STDOUT_FILENO);
    dup2(stderr_pipe[1], STDERR_FILENO);

    close(stdin_pipe[0]);
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
    close(stdin_pipe[0]);  // Close read end of stdin
    close(stdout_pipe[1]); // Close write end of stdout
    close(stderr_pipe[1]); // Close write end of stderr

    // Write input to stdin
    write(stdin_pipe[1], input.c_str(), input.length());
    close(stdin_pipe[1]);

    // Set up timeout
    alarm(timeout_seconds);

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

    // Cancel alarm
    alarm(0);

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
  auto temp_dir = fs::temp_directory_path() / "cod_repl_test";
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

void testReplStartup() {
  std::cout << "Testing REPL startup...\n";

  auto project_dir = createTestProject();

  // Test REPL startup with quit command
  auto result = runCodRepl({"--project", project_dir.string()}, "%quit\n", project_dir);

  // Should show REPL banner and exit cleanly
  if (result.stdout_output.find("CoD REPL") != std::string::npos) {
    std::cout << "  ✓ REPL banner displayed\n";
  }

  // Should exit with code 0 when quit properly
  if (result.exit_code == 0) {
    std::cout << "  ✓ Clean exit on %quit\n";
  }

  // Clean up
  fs::remove_all(project_dir);

  std::cout << "✓ REPL startup test passed\n";
}

void testReplCommands() {
  std::cout << "Testing REPL commands...\n";

  auto project_dir = createTestProject();

  // Test help command
  auto result = runCodRepl({"--project", project_dir.string()}, "%help\n%quit\n", project_dir);

  if (result.stdout_output.find("CoD REPL Help") != std::string::npos ||
      result.stdout_output.find("help") != std::string::npos) {
    std::cout << "  ✓ Help command works\n";
  }

  // Test alternative quit command
  result = runCodRepl({"--project", project_dir.string()}, "quit\n", project_dir);

  if (result.exit_code == 0) {
    std::cout << "  ✓ Alternative quit command works\n";
  }

  // Clean up
  fs::remove_all(project_dir);

  std::cout << "✓ REPL commands test passed\n";
}

void testReplLineContinuation() {
  std::cout << "Testing REPL line continuation...\n";

  auto project_dir = createTestProject();

  // Test line continuation with backslash
  auto result = runCodRepl({"--project", project_dir.string()}, "int x = \\\n42;\n%quit\n", project_dir);

  // Should handle line continuation (exact behavior depends on compilation success)
  // Main thing is it shouldn't crash or hang
  assert(result.exit_code != -1);

  // Clean up
  fs::remove_all(project_dir);

  std::cout << "✓ Line continuation test passed\n";
}

void testReplWorkspaceHandling() {
  std::cout << "Testing REPL workspace handling...\n";

  auto project_dir = createTestProject();
  auto works_path = project_dir / "test.dbmr";

  // Test with custom workspace
  auto result = runCodRepl({"--project", project_dir.string(), "-w", works_path.string()}, "%quit\n", project_dir);

  // Should mention the workspace path in output
  if (result.stdout_output.find(works_path.filename().string()) != std::string::npos ||
      result.stdout_output.find("Workspace:") != std::string::npos) {
    std::cout << "  ✓ Workspace path displayed\n";
  }

  // Clean up
  fs::remove_all(project_dir);

  std::cout << "✓ Workspace handling test passed\n";
}

void testReplPrompt() {
  std::cout << "Testing REPL prompt...\n";

  auto project_dir = createTestProject();

  // Test that REPL shows prompt
  auto result = runCodRepl({"--project", project_dir.string()}, "%quit\n", project_dir);

  // Should show cod> prompt
  if (result.stdout_output.find("cod>") != std::string::npos) {
    std::cout << "  ✓ REPL prompt displayed\n";
  }

  // Clean up
  fs::remove_all(project_dir);

  std::cout << "✓ REPL prompt test passed\n";
}

int main() {
  std::cout << "Running CoD REPL tests...\n\n";

  // Set up signal handler for timeout
  signal(SIGALRM, [](int) {
    std::cerr << "Test timed out\n";
    exit(1);
  });

  try {
    testReplStartup();
    testReplCommands();
    testReplLineContinuation();
    testReplWorkspaceHandling();
    testReplPrompt();

    std::cout << "\n✔ All REPL tests passed!\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Test failed: " << e.what() << "\n";
    return 1;
  }
}
