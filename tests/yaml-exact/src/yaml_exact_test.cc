#include "shilos.hh"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

using namespace shilos::yaml;

std::string read_file(const std::string &path) {
  std::ifstream file(path);
  if (!file) {
    throw std::runtime_error("Could not open file: " + path);
  }
  return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

bool test_exact_printing_single_file(const std::string &file_path) {
  try {
    // Read original YAML
    std::string original = read_file(file_path);

    // Parse with exact-preserving parser
    YamlDocument doc = YamlDocument::Parse(original);

    // Print exactly
    std::string exact_output = doc.format_exact();

    // Compare with original
    return original == exact_output;

  } catch (const std::exception &e) {
    std::cerr << "Error processing file '" << file_path << "': " << e.what() << std::endl;
    return false;
  }
}

void show_usage(const char *program_name) {
  std::cout << "Usage: " << program_name << " <yaml-file>" << std::endl;
  std::cout << "       " << program_name << " --test-all" << std::endl;
  std::cout << std::endl;
  std::cout << "Test YAML exact printing functionality." << std::endl;
  std::cout << "Verifies that parsing and re-printing preserves the original format exactly." << std::endl;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    show_usage(argv[0]);
    return 1;
  }

  std::string arg = argv[1];

  if (arg == "--help" || arg == "-h") {
    show_usage(argv[0]);
    return 0;
  }

  if (arg == "--test-all") {
    // Run comprehensive tests on predefined test data
    try {
      std::cout << "=== YAML Exact Printing Tests ===" << std::endl;

      // Test basic functionality first
      std::cout << "\nTesting basic functionality..." << std::endl;
      std::string simple_yaml = "key: value\n";
      YamlDocument simple_doc = YamlDocument::Parse(simple_yaml);

      const Node &root = simple_doc.root();
      if (!root.IsMap()) {
        throw std::runtime_error("Failed to parse simple YAML as map");
      }
      std::cout << "✓ Basic parsing works" << std::endl;

      // Test the exact printing functionality on predefined files
      std::string test_dir = "test-data/";

      std::vector<std::pair<std::string, std::string>> test_files = {
          {"Simple YAML parsing", test_dir + "simple_test.yaml"},
          {"Basic YAML with comments", test_dir + "basic_with_comments.yaml"},
          {"Complex formatting", test_dir + "complex_formatting.yaml"},
          {"Whitespace edge cases", test_dir + "whitespace_edge_cases.yaml"},
          {"Advanced YAML constructs", test_dir + "advanced_constructs.yaml"}};

      bool all_passed = true;
      for (const auto &[test_name, file_path] : test_files) {
        std::cout << "Testing: " << test_name << "..." << std::endl;

        if (test_exact_printing_single_file(file_path)) {
          std::cout << "✓ " << test_name << " passed - exact match!" << std::endl;
        } else {
          std::cout << "✗ " << test_name << " failed - output differs from original" << std::endl;
          all_passed = false;
        }
      }

      if (all_passed) {
        std::cout << "\n✓ All exact printing tests passed!" << std::endl;
        return 0;
      } else {
        std::cout << "\n✗ Some tests failed!" << std::endl;
        return 1;
      }

    } catch (const std::exception &e) {
      std::cerr << "Test failed: " << e.what() << std::endl;
      return 1;
    }
  } else {
    // Single file test mode
    std::string file_path = arg;

    if (test_exact_printing_single_file(file_path)) {
      // Silent success for use in scripts
      return 0;
    } else {
      return 1;
    }
  }
}
