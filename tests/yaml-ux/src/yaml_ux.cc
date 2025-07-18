#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "shilos.hh"

namespace fs = std::filesystem;
using namespace shilos::yaml;

std::string read_file(const fs::path &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file: " + path.string());
  }
  return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

struct ErrorTest {
  std::string name;
  std::string filename;
  std::string content;
  std::string expected_error_pattern;
};

void run_error_test(const ErrorTest &test, bool verbose) {
  if (verbose) {
    std::cout << "\n=== Testing: " << test.name << " ===" << std::endl;
    std::cout << "File: " << test.filename << std::endl;
    std::cout << "Content:\n" << test.content << std::endl;
  }

  auto result = YamlDocument::Parse(test.filename, test.content);

  shilos::vswitch(
      result,
      [&](const ParseError &err) {
        if (verbose) {
          std::cout << "✓ Parse error caught (as expected):" << std::endl;
          std::cout << "  Error message: " << err.what() << std::endl;
          std::cout << "  Filename: " << err.filename() << std::endl;
          std::cout << "  Line: " << err.line() << std::endl;
          std::cout << "  Column: " << err.column() << std::endl;
          std::cout << "  Message: " << err.message() << std::endl;
          std::cout << "  VS Code clickable format: " << err.what() << std::endl;
        } else {
          std::cout << "\033[0;32m✓\033[0m " << test.name << " - " << err.what() << std::endl;
        }
      },
      [&](const YamlDocument &doc) {
        if (verbose) {
          std::cout << "✗ Unexpected success - expected parsing error!" << std::endl;
        } else {
          std::cout << "\033[0;31m✗\033[0m " << test.name << " - Unexpected success!" << std::endl;
        }
      });
}

void run_file_test(const fs::path &test_file, bool verbose) {
  if (verbose) {
    std::cout << "\n=== Testing file: " << test_file.filename() << " ===" << std::endl;
    std::cout << "Full path: " << test_file << std::endl;
  }

  try {
    auto result = YamlDocument::Read(test_file.string());
    if (verbose) {
      std::string content = read_file(test_file);
      std::cout << "Content:\n" << content << std::endl;
    }

    shilos::vswitch(
        result,
        [&](const ParseError &err) {
          if (verbose) {
            std::cout << "✓ Parse error caught:" << std::endl;
            std::cout << "  VS Code clickable format: " << err.what() << std::endl;
            std::cout << "  Structured info:" << std::endl;
            std::cout << "    - Filename: " << err.filename() << std::endl;
            std::cout << "    - Line: " << err.line() << std::endl;
            std::cout << "    - Column: " << err.column() << std::endl;
            std::cout << "    - Message: " << err.message() << std::endl;
          } else {
            std::cout << "\033[0;32m✓\033[0m " << test_file.filename().string() << " - " << err.what() << std::endl;
          }
        },
        [&](const YamlDocument &doc) {
          if (verbose) {
            std::cout << "✓ Parse successful - showing formatted output:" << std::endl;
            std::cout << format_yaml(doc.root()) << std::endl;
          } else {
            std::cout << "\033[0;32m✓\033[0m " << test_file.filename().string() << " - Parse successful" << std::endl;
          }
        });
  } catch (const std::exception &e) {
    if (verbose) {
      std::cout << "✗ File access error: " << e.what() << std::endl;
    } else {
      std::cout << "\033[0;31m✗\033[0m " << test_file.filename().string() << " - File access error" << std::endl;
    }
  }
}

int main(int argc, char *argv[]) {
  bool verbose = false;
  std::vector<std::string> file_args;

  // Parse command line arguments
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--verbose" || arg == "-v") {
      verbose = true;
    } else if (arg == "--help" || arg == "-h") {
      std::cout << "Usage: " << argv[0] << " [OPTIONS] [FILES...]" << std::endl;
      std::cout << "Options:" << std::endl;
      std::cout << "  -v, --verbose    Show detailed output" << std::endl;
      std::cout << "  -h, --help       Show this help message" << std::endl;
      std::cout << "Files:" << std::endl;
      std::cout << "  If no files specified, runs built-in error tests" << std::endl;
      std::cout << "  If files specified, tests those files" << std::endl;
      return 0;
    } else {
      file_args.push_back(arg);
    }
  }

  std::cout << "=== YAML UX Error Reporting Test Suite ===" << std::endl;
  if (verbose) {
    std::cout << "This test suite demonstrates improved error reporting for YAML parsing" << std::endl;
    std::cout << "with VS Code-compatible clickable links format." << std::endl;
  }

  if (!file_args.empty()) {
    // Test specific files provided as arguments
    if (verbose) {
      std::cout << "\n--- Testing specified files ---" << std::endl;
    }
    for (const auto &file_arg : file_args) {
      fs::path test_file = file_arg;
      run_file_test(test_file, verbose);
    }
    return 0;
  }

  // Run built-in error tests
  if (verbose) {
    std::cout << "\n--- Running built-in error tests ---" << std::endl;
  } else {
    std::cout << "\nBuilt-in error tests:" << std::endl;
  }

  std::vector<ErrorTest> error_tests = {
      {"Incompatible Indentation", "config/server.yaml", "server:\n  name: web-server\n\tport: 8080",
       "Incompatible indentation"},

      {"Unclosed Quoted String", "data/user.yaml", "name: \"John Doe\ndescription: Missing closing quote",
       "Unclosed quoted string"},

      {"Invalid Escape Sequence", "templates/message.yaml", "message: \"Hello \\x world\"", "Invalid escape sequence"},

      {"Empty Key in Mapping", "settings/app.yaml", ": value\ndebug: true", "Empty or missing key"},

      {"Missing Colon After Key", "config/database.yaml", "database:\n  host: localhost\n  port 5432\n  user: admin",
       "Expected ':' after key"},

      {"Empty Alias Name", "references/aliases.yaml", "default: &anchor value\nother: *", "Empty alias name"},

      {"Undefined Alias", "references/broken.yaml", "main: *undefined_alias", "Undefined alias"},

      {"Empty Anchor Name", "references/anchors.yaml", "value: & something", "Empty anchor name"},

      {"Empty Tag Name", "types/tagged.yaml", "value: !! something", "Empty tag name"},

      {"Invalid Type Tag", "types/conversion.yaml", "number: !!int \"not a number\"",
       "!!int tag applied to non-integer value"},

      {"Unterminated JSON Object", "json/object.yaml", "data: {key: value, other: incomplete",
       "Unterminated JSON object"},

      {"Unterminated JSON Array", "json/array.yaml", "items: [1, 2, 3, incomplete", "Unterminated JSON array"}};

  for (const auto &test : error_tests) {
    run_error_test(test, verbose);
  }

  // Test files from test-data directory if it exists
  fs::path test_data_dir = "test-data";
  if (fs::exists(test_data_dir) && fs::is_directory(test_data_dir)) {
    if (verbose) {
      std::cout << "\n=== Testing files from test-data directory ===" << std::endl;
    } else {
      std::cout << "\nTest-data files:" << std::endl;
    }
    for (const auto &entry : fs::directory_iterator(test_data_dir)) {
      if (entry.is_regular_file() && entry.path().extension() == ".yaml") {
        run_file_test(entry.path(), verbose);
      }
    }
  }

  if (verbose) {
    std::cout << "\n=== Test Suite Complete ===" << std::endl;
    std::cout << "All error messages above should be clickable in VS Code terminal" << std::endl;
    std::cout << "Format: filename:line:column: message" << std::endl;
  } else {
    std::cout << "\n✓ Test suite complete. Use --verbose for detailed output." << std::endl;
  }

  return 0;
}
