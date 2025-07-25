#include <shilos.hh>
#include <shilos/di.hh>

#include <cassert>
#include <cxxabi.h>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <typeinfo>
#include <unistd.h>
#include <vector>

using namespace shilos;

// Helper functions to trigger exceptions at different stack levels
// Using volatile to prevent inlining and ensure visibility in stack traces
volatile int stack_depth_counter = 0;

__attribute__((noinline)) void deep_program_function_level3() {
  // Add some work to prevent optimization/inlining
  stack_depth_counter += 3;
  std::cout << "  -> Entering deep_program_function_level3 (depth=" << stack_depth_counter << ")" << std::endl;

  // Directly throw a ParseError to test stack trace capture at throw point
  throw yaml::ParseError("Program call stack test error", "test_file.yaml", 123, 45);
}

__attribute__((noinline)) void deep_program_function_level2() {
  stack_depth_counter += 2;
  std::cout << "  -> Entering deep_program_function_level2 (depth=" << stack_depth_counter << ")" << std::endl;
  deep_program_function_level3();
}

__attribute__((noinline)) void deep_program_function_level1() {
  stack_depth_counter += 1;
  std::cout << "  -> Entering deep_program_function_level1 (depth=" << stack_depth_counter << ")" << std::endl;
  deep_program_function_level2();
}

__attribute__((noinline)) void trigger_program_exception_with_stack() {
  std::cout << "Triggering program exception through nested calls..." << std::endl;
  deep_program_function_level1();
}

__attribute__((noinline)) void deep_author_function_level3() {
  stack_depth_counter += 30;
  std::cout << "  -> Entering deep_author_function_level3 (depth=" << stack_depth_counter << ")" << std::endl;
  // Simply throw an AuthorError to demonstrate stack trace capture
  throw yaml::AuthorError("stacktrace_test_output.yaml", "Forced authoring error for stack trace test");
}

__attribute__((noinline)) void deep_author_function_level2() {
  stack_depth_counter += 20;
  std::cout << "  -> Entering deep_author_function_level2 (depth=" << stack_depth_counter << ")" << std::endl;
  deep_author_function_level3();
}

__attribute__((noinline)) void deep_author_function_level1() {
  stack_depth_counter += 10;
  std::cout << "  -> Entering deep_author_function_level1 (depth=" << stack_depth_counter << ")" << std::endl;
  deep_author_function_level2();
}

__attribute__((noinline)) void trigger_author_exception_with_stack() {
  std::cout << "Triggering author exception through nested calls..." << std::endl;
  deep_author_function_level1();
}

__attribute__((noinline)) void deep_conversion_function_level3() {
  stack_depth_counter += 300;
  std::cout << "  -> Entering deep_conversion_function_level3 (depth=" << stack_depth_counter << ")" << std::endl;
  // Force a type error to demonstrate stack trace capture
  throw yaml::TypeError("Forced type conversion error for stack trace test: cannot convert 'not_a_number' to integer");
}

__attribute__((noinline)) void deep_conversion_function_level2() {
  stack_depth_counter += 200;
  std::cout << "  -> Entering deep_conversion_function_level2 (depth=" << stack_depth_counter << ")" << std::endl;
  deep_conversion_function_level3();
}

__attribute__((noinline)) void deep_conversion_function_level1() {
  stack_depth_counter += 100;
  std::cout << "  -> Entering deep_conversion_function_level1 (depth=" << stack_depth_counter << ")" << std::endl;
  deep_conversion_function_level2();
}

__attribute__((noinline)) void trigger_conversion_exception_with_stack() {
  std::cout << "Triggering type conversion exception through nested calls..." << std::endl;
  deep_conversion_function_level1();
}

struct StackTraceTest {
  std::string name;
  std::function<void()> trigger_function;
};

std::string demangle_type_name(const char *name) {
  int status;
  char *demangled = abi::__cxa_demangle(name, nullptr, nullptr, &status);
  if (status == 0) {
    std::string result(demangled);
    free(demangled);
    return result;
  } else {
    return std::string(name);
  }
}

// Function to dump debug information programmatically
void dump_debug_info() {
  std::cout << "\n=== Debug Information Dump ===" << std::endl;

  // Get current executable path
  char exe_path[PATH_MAX];
  ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
  if (len != -1) {
    exe_path[len] = '\0';
    std::cout << "Executable: " << exe_path << std::endl;
  }

  // Get current working directory
  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) != nullptr) {
    std::cout << "Working directory: " << cwd << std::endl;
  }

  // Check if debug symbols are available
  std::cout << "Build type: ";
#ifdef NDEBUG
  std::cout << "Release (NDEBUG defined)" << std::endl;
#else
  std::cout << "Debug (NDEBUG not defined)" << std::endl;
#endif

  std::cout << "Compiler: " << __VERSION__ << std::endl;
  std::cout << "=== End Debug Information ===" << std::endl;
}

int main(int argc, char *argv[]) {
  bool verbose = false;

  // Parse command line arguments
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--verbose" || arg == "-v") {
      verbose = true;
    } else if (arg == "--help" || arg == "-h") {
      std::cout << "Usage: " << argv[0] << " [OPTIONS]" << std::endl;
      std::cout << "Options:" << std::endl;
      std::cout << "  -v, --verbose    Show detailed output including debug information" << std::endl;
      std::cout << "  -h, --help       Show this help message" << std::endl;
      std::cout << std::endl;
      std::cout << "This test verifies that yaml::Exception and its subclasses correctly" << std::endl;
      std::cout << "capture and report full stack traces when exceptions occur during" << std::endl;
      std::cout << "YAML parsing, authoring, and type conversion operations." << std::endl;
      return 0;
    }
  }

  if (verbose) {
    dump_debug_info();
  }

  std::cout << "=== YAML Exception Stack Trace Test Suite ===" << std::endl;
  std::cout << "Testing stack trace capture and verification..." << std::endl;

  std::vector<StackTraceTest> tests = {{"Program Call Stack - ParseError", trigger_program_exception_with_stack},
                                       {"Program Call Stack - AuthorError", trigger_author_exception_with_stack},
                                       {"Program Call Stack - TypeError", trigger_conversion_exception_with_stack}};

  std::cout << "\n--- Running Stack Trace Tests ---" << std::endl;

  bool all_passed = true;
  for (const auto &test : tests) {
    std::cout << "\n=== Stack Trace Test: " << test.name << " ===" << std::endl;

    try {
      test.trigger_function();
      std::cout << "\033[0;31m✗\033[0m " << test.name << " - No exception thrown!" << std::endl;
      all_passed = false;
      continue;
    } catch (const yaml::Exception &e) {
      std::cout << "✓ Exception caught successfully" << std::endl;
      std::cout << "  Exception type: " << demangle_type_name(typeid(e).name()) << std::endl;
      std::cout << "  Error message: " << e.what() << std::endl;
      std::cout << "  Stack trace:" << std::endl;

      const std::string &stack_trace = e.stack_trace();
      if (stack_trace.empty()) {
        std::cout << "    (Stack trace is empty)" << std::endl;
        std::cout << "\033[0;31m✗\033[0m " << test.name << " - Stack trace is empty!" << std::endl;
        all_passed = false;
        continue;
      } else {
        std::istringstream ss(stack_trace);
        std::string line;
        int line_count = 0;
        while (std::getline(ss, line)) {
          std::cout << "    " << line << std::endl;
          line_count++;
        }

        // Strictly verify that we have source-level debug information
        bool has_source_debug = stack_trace.find("/yaml_exc_trace_test.cc") != std::string::npos;

        if (has_source_debug) {
          std::cout << "  ✓ Source-level debug information found in stack trace" << std::endl;
        } else {
          std::cout << "  ✗ Source-level debug information NOT found in stack trace" << std::endl;
          all_passed = false;
        }

        std::cout << "  Stack trace length: " << stack_trace.length() << " characters, " << line_count << " lines"
                  << std::endl;
      }
    } catch (const std::exception &e) {
      std::cout << "\033[0;31m✗\033[0m " << test.name << " - Wrong exception type" << std::endl;
      all_passed = false;
    }
  }

  std::cout << "\n=== Test Suite Complete ===" << std::endl;

  if (!all_passed) {
    std::cout << "\033[0;31m✗\033[0m Some tests failed!" << std::endl;
    return 1;
  }

  std::cout << "\033[0;32m✓\033[0m All tests passed!" << std::endl;
  return 0;
}
