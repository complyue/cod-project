#include <shilos.hh>

#include <iostream>

using namespace shilos;

std::string read_file(const std::string &path) {
  std::ifstream file(path);
  if (!file) {
    throw std::runtime_error("Could not open file: " + path);
  }
  return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

void print_node_tree(const yaml::Node &node, int depth = 0, const std::string &key = "") {
  std::string indent(depth * 2, ' ');

  if (!key.empty()) {
    std::cout << indent << "KEY: \"" << key << "\" -> ";
  } else if (depth > 0) {
    std::cout << indent << "ITEM: ";
  } else {
    std::cout << "ROOT: ";
  }

  // Print leading comments if any
  if (!node.leading_comments.empty()) {
    std::cout << "LEADING_COMMENT: [";
    for (size_t i = 0; i < node.leading_comments.size(); ++i) {
      if (i > 0)
        std::cout << ", ";
      std::cout << "\"" << node.leading_comments[i] << "\"";
    }
    std::cout << "] ";
  }

  if (node.IsNull()) {
    std::cout << "NULL";
  } else if (node.IsScalar()) {
    if (auto b = std::get_if<bool>(&node.value)) {
      std::cout << "BOOL: " << (*b ? "true" : "false");
    } else if (auto i = std::get_if<int64_t>(&node.value)) {
      std::cout << "INT: " << *i;
    } else if (auto d = std::get_if<double>(&node.value)) {
      std::cout << "DOUBLE: " << *d;
    } else if (auto s = std::get_if<std::string_view>(&node.value)) {
      std::cout << "STRING: \"" << *s << "\"";
    }
  } else if (node.IsSequence()) {
    const auto &seq = std::get<yaml::Sequence>(node.value);
    std::cout << "SEQUENCE (" << seq.size() << " items)";
  } else if (node.IsMap()) {
    const auto &map = std::get<yaml::Map>(node.value);
    std::cout << "MAP (" << map.size() << " entries)";
  } else {
    std::cout << "UNKNOWN TYPE";
  }

  // Print trailing comment if any
  if (!node.trailing_comment.empty()) {
    std::cout << " TRAILING_COMMENT: \"" << node.trailing_comment << "\"";
  }

  std::cout << std::endl;

  // Recursively print children
  if (node.IsSequence()) {
    const auto &seq = std::get<yaml::Sequence>(node.value);
    for (size_t i = 0; i < seq.size(); ++i) {
      std::cout << indent << "  [" << i << "]:" << std::endl;
      print_node_tree(seq[i], depth + 2);
    }
  } else if (node.IsMap()) {
    const auto &map = std::get<yaml::Map>(node.value);
    for (const auto &entry : map) {
      print_node_tree(entry.value, depth + 1, std::string(entry.key));
    }
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
    auto result = yaml::Document::Read(yaml_file);
    vswitch(
        result,
        [](const yaml::ParseError &err) {
          std::cerr << "Error: " << err.what() << std::endl;
          std::exit(1);
        },
        [](const yaml::Document &doc) {
          print_node_tree(doc.root());

          std::cout << "=== FORMAT_YAML OUTPUT ===" << std::endl;
          std::cout << format_yaml(doc.root()) << std::endl;
        });

    return 0;
  }

  if (arg == "--basic-test") {
    std::cout << "=== YAML Basic Pretty Print Tests ===" << std::endl;
    std::string simple_yaml = "key: value\n";
    auto simple_result = yaml::Document::Parse("<basic-test>", simple_yaml);
    return vswitch(
        simple_result,
        [](const yaml::ParseError &err) {
          std::cerr << "Failed to parse simple YAML: " + std::string(err.what()) << std::endl;
          return 1;
        },
        [](const yaml::Document &doc) {
          const yaml::Node &root = doc.root();
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
      auto doc_result = yaml::Document::Read(file_path);
      vswitch(
          doc_result,
          [](const yaml::ParseError &err) {
            std::cerr << "Error: " << err.what() << std::endl;
            std::exit(1);
          },
          [](const yaml::Document &doc) { std::cout << format_yaml(doc.root()) << std::endl; });
      return 0;
    } catch (const std::exception &e) {
      std::cerr << "Error: " << e.what() << std::endl;
      return 1;
    }
  }
}
