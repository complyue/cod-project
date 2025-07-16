#include "shilos.hh"
#include <filesystem>
#include <fstream>
#include <iomanip>
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

void show_usage(const char *program_name) {
  std::cout << "Usage: " << program_name << " <yaml-file>" << std::endl;
  std::cout << "       " << program_name << " --verbose <yaml-file>" << std::endl;
  std::cout << "       " << program_name << " --basic-test" << std::endl;
  std::cout << std::endl;
  std::cout << "Parse YAML file and output the formatted YAML." << std::endl;
  std::cout << "By default, outputs only the clean formatted YAML." << std::endl;
  std::cout << std::endl;
  std::cout << "Options:" << std::endl;
  std::cout << "  --verbose: Show detailed output with original content and parse tree" << std::endl;
  std::cout << "  --basic-test: Run built-in basic functionality tests" << std::endl;
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

  if (arg == "--verbose") {
    if (argc < 3) {
      std::cerr << "Error: --verbose requires one argument: <yaml-file>" << std::endl;
      show_usage(argv[0]);
      return 1;
    }

    std::string yaml_file = argv[2];

    std::string content = read_file(yaml_file);
    std::cout << "=== FILE: " << yaml_file << " ===" << std::endl;
    std::cout << "=== ORIGINAL CONTENT ===" << std::endl;
    std::cout << content << std::endl;

    std::cout << "=== PARSED TREE ===" << std::endl;
    auto result = YamlDocument::Parse(content);
    shilos::vswitch(
        result,
        [](const shilos::yaml::ParseError &err) {
          std::cerr << "Error: " << err.what() << std::endl;
          std::exit(1);
        },
        [](const YamlDocument &doc) {
          print_node_tree(doc.root());

          std::cout << "=== FORMAT_YAML OUTPUT ===" << std::endl;
          std::cout << format_yaml(doc.root()) << std::endl;
        });

    return 0;
  }

  if (arg == "--basic-test") {
    std::cout << "=== YAML Basic Pretty Print Tests ===" << std::endl;
    std::string simple_yaml = "key: value\n";
    auto simple_result = YamlDocument::Parse(simple_yaml);
    return shilos::vswitch(
        simple_result,
        [](const shilos::yaml::ParseError &err) {
          std::cerr << "Failed to parse simple YAML: " + std::string(err.what()) << std::endl;
          return 1;
        },
        [](const YamlDocument &doc) {
          const Node &root = doc.root();
          if (!root.IsMap()) {
            std::cerr << "Failed to parse simple YAML as map" << std::endl;
            return 2;
          }
          std::cout << "✓ Basic parsing works" << std::endl;
          return 0;
        });
  } else {
    // Single file mode - default behavior (format only)
    std::string file_path = arg;

    try {
      std::string content = read_file(file_path);
      auto doc_result = YamlDocument::Parse(content);
      shilos::vswitch(
          doc_result,
          [](const shilos::yaml::ParseError &err) {
            std::cerr << "Error: " << err.what() << std::endl;
            std::exit(1);
          },
          [](const YamlDocument &doc) { std::cout << format_yaml(doc.root()) << std::endl; });
      return 0;
    } catch (const std::exception &e) {
      std::cerr << "Error: " << e.what() << std::endl;
      return 1;
    }
  }
}
