#include "shilos.hh"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

using namespace shilos::yaml;

std::string read_file(const std::string &path) {
  std::ifstream file(path);
  if (!file) {
    throw std::runtime_error("Could not open file: " + path);
  }
  return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

void print_node_tree(const Node &node, int depth = 0, const std::string &key = "") {
  std::string indent(depth * 2, ' ');

  if (!key.empty()) {
    std::cout << indent << "KEY: \"" << key << "\" -> ";
  } else if (depth > 0) {
    std::cout << indent << "ITEM: ";
  } else {
    std::cout << "ROOT: ";
  }

  if (node.IsNull()) {
    std::cout << "NULL" << std::endl;
  } else if (node.IsScalar()) {
    if (auto b = std::get_if<bool>(&node.value)) {
      std::cout << "BOOL: " << (*b ? "true" : "false") << std::endl;
    } else if (auto i = std::get_if<int64_t>(&node.value)) {
      std::cout << "INT: " << *i << std::endl;
    } else if (auto d = std::get_if<double>(&node.value)) {
      std::cout << "DOUBLE: " << *d << std::endl;
    } else if (auto s = std::get_if<std::string_view>(&node.value)) {
      std::cout << "STRING: \"" << *s << "\"" << std::endl;
    }
  } else if (node.IsSequence()) {
    const auto &seq = std::get<Sequence>(node.value);
    std::cout << "SEQUENCE (" << seq.size() << " items)" << std::endl;
    for (size_t i = 0; i < seq.size(); ++i) {
      std::cout << indent << "  [" << i << "]:" << std::endl;
      print_node_tree(seq[i], depth + 2);
    }
  } else if (node.IsMap()) {
    const auto &map = std::get<Map>(node.value);
    std::cout << "MAP (" << map.size() << " entries)" << std::endl;
    for (const auto &entry : map) {
      print_node_tree(entry.value, depth + 1, std::string(entry.key));
    }
  } else {
    std::cout << "UNKNOWN TYPE" << std::endl;
  }
}

std::string get_tree_output(const Node &node) {
  std::ostringstream oss;

  // Redirect cout to our stringstream temporarily
  std::streambuf *old_cout = std::cout.rdbuf();
  std::cout.rdbuf(oss.rdbuf());

  print_node_tree(node);

  // Restore cout
  std::cout.rdbuf(old_cout);

  return oss.str();
}

bool compare_with_expected(const std::string &yaml_file, const std::string &expected_file) {
  try {
    // Parse YAML file
    std::string content = read_file(yaml_file);
    YamlDocument doc = YamlDocument::Parse(content);

    // Get actual tree output
    std::string actual_output = get_tree_output(doc.root());

    // Read expected output
    std::string expected_output = read_file(expected_file);

    // Trim whitespace from both for comparison
    auto trim = [](const std::string &str) {
      size_t start = str.find_first_not_of(" \t\n\r");
      if (start == std::string::npos)
        return std::string{};
      size_t end = str.find_last_not_of(" \t\n\r");
      return str.substr(start, end - start + 1);
    };

    std::string actual_trimmed = trim(actual_output);
    std::string expected_trimmed = trim(expected_output);

    if (actual_trimmed == expected_trimmed) {
      return true;
    } else {
      std::cout << "=== COMPARISON FAILED ===" << std::endl;
      std::cout << "YAML file: " << yaml_file << std::endl;
      std::cout << "Expected file: " << expected_file << std::endl;
      std::cout << std::endl;
      std::cout << "--- EXPECTED ---" << std::endl;
      std::cout << expected_output << std::endl;
      std::cout << "--- ACTUAL ---" << std::endl;
      std::cout << actual_output << std::endl;
      std::cout << "--- END COMPARISON ---" << std::endl;
      return false;
    }

  } catch (const std::exception &e) {
    std::cerr << "Error comparing files: " << e.what() << std::endl;
    return false;
  }
}

void show_usage(const char *program_name) {
  std::cout << "Usage: " << program_name << " <yaml-file>" << std::endl;
  std::cout << "       " << program_name << " --test-all" << std::endl;
  std::cout << "       " << program_name << " --compare <yaml-file> <expected-file>" << std::endl;
  std::cout << std::endl;
  std::cout << "Parse YAML file and display the internal node tree structure." << std::endl;
  std::cout << "This tool helps debug YAML parsing issues by showing exactly" << std::endl;
  std::cout << "how the parser interprets the input file." << std::endl;
  std::cout << std::endl;
  std::cout << "Options:" << std::endl;
  std::cout << "  --compare: Compare parsed tree output against expected output file" << std::endl;
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

  if (arg == "--compare") {
    if (argc < 4) {
      std::cerr << "Error: --compare requires two arguments: <yaml-file> <expected-file>" << std::endl;
      show_usage(argv[0]);
      return 1;
    }

    std::string yaml_file = argv[2];
    std::string expected_file = argv[3];

    if (compare_with_expected(yaml_file, expected_file)) {
      return 0; // Success - silent for use in test scripts
    } else {
      return 1; // Comparison failed
    }
  }

  if (arg == "--test-all") {
    // Run comprehensive tests on predefined test data
    try {
      std::cout << "=== YAML Pretty Print Tests ===" << std::endl;

      // Test basic functionality first
      std::cout << "\nTesting basic functionality..." << std::endl;
      std::string simple_yaml = "key: value\n";
      YamlDocument simple_doc = YamlDocument::Parse(simple_yaml);

      const Node &root = simple_doc.root();
      if (!root.IsMap()) {
        throw std::runtime_error("Failed to parse simple YAML as map");
      }
      std::cout << "✓ Basic parsing works" << std::endl;

      // Test on various YAML structures
      std::string test_dir = "test-data/";

      std::vector<std::pair<std::string, std::string>> test_files = {{"Simple YAML", test_dir + "simple.yaml"},
                                                                     {"Nested structures", test_dir + "nested.yaml"},
                                                                     {"Mixed types", test_dir + "mixed.yaml"}};

      bool all_passed = true;
      for (const auto &[test_name, file_path] : test_files) {
        std::cout << "\nTesting: " << test_name << std::endl;
        std::cout << "File: " << file_path << std::endl;

        try {
          if (std::filesystem::exists(file_path)) {
            std::string content = read_file(file_path);
            YamlDocument doc = YamlDocument::Parse(content);

            std::cout << "=== PARSED TREE ===" << std::endl;
            print_node_tree(doc.root());
            std::cout << "=== END TREE ===" << std::endl;

            std::cout << "✓ " << test_name << " parsed successfully" << std::endl;
          } else {
            std::cout << "⚠ " << test_name << " - file not found (skipping)" << std::endl;
          }
        } catch (const std::exception &e) {
          std::cout << "✗ " << test_name << " failed: " << e.what() << std::endl;
          all_passed = false;
        }
      }

      if (all_passed) {
        std::cout << "\n✓ All parsing tests completed!" << std::endl;
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
    // Single file mode
    std::string file_path = arg;

    try {
      std::string content = read_file(file_path);
      std::cout << "=== FILE: " << file_path << " ===" << std::endl;
      std::cout << "=== ORIGINAL CONTENT ===" << std::endl;
      std::cout << content << std::endl;

      std::cout << "=== PARSED TREE ===" << std::endl;
      YamlDocument doc = YamlDocument::Parse(content);
      print_node_tree(doc.root());

      std::cout << "=== FORMAT_YAML OUTPUT ===" << std::endl;
      std::cout << format_yaml(doc.root()) << std::endl;

      return 0;
    } catch (const std::exception &e) {
      std::cerr << "Error: " << e.what() << std::endl;
      return 1;
    }
  }
}
