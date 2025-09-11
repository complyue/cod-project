#pragma once

#include <iostream>
#include <string>

// ANSI color codes for consistent test output formatting
namespace test_colors {
constexpr const char *RED = "\033[0;31m";
constexpr const char *GREEN = "\033[0;32m";
constexpr const char *YELLOW = "\033[0;33m";
constexpr const char *NC = "\033[0m"; // No Color
} // namespace test_colors

// Standardized test output functions
class TestLogger {
public:
  static void log_test(const std::string &test_name) {
    std::cout << test_colors::YELLOW << "Testing " << test_name << "..." << test_colors::NC << std::endl;
  }

  static void log_pass(const std::string &message) {
    std::cout << test_colors::GREEN << "✓ " << message << test_colors::NC << std::endl;
  }

  static void log_fail(const std::string &message) {
    std::cerr << test_colors::RED << "✗ " << message << test_colors::NC << std::endl;
  }

  static void log_header(const std::string &test_suite_name) {
    std::cout << test_colors::GREEN << "=== " << test_suite_name << " ===" << test_colors::NC << std::endl;
  }

  static void log_summary(int passed, int total, const std::string &test_type = "tests") {
    std::cout << std::endl;
    if (passed == total) {
      std::cout << test_colors::GREEN << "✔ All " << test_type << " passed! (" << passed << "/" << total << ")"
                << test_colors::NC << std::endl;
    } else {
      int failed = total - passed;
      std::cout << test_colors::RED << "✗ Some " << test_type << " failed. (" << failed << "/" << total << " failures)"
                << test_colors::NC << std::endl;
    }
  }
};