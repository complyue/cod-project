#include "yaml_comparison.hh"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using shilos::yaml::Node;
namespace fs = std::filesystem;

static std::string read_file(const fs::path &p) {
  std::ifstream ifs(p);
  if (!ifs)
    throw std::runtime_error("yaml-cmp: cannot open file: " + p.string());
  std::stringstream ss;
  ss << ifs.rdbuf();
  return ss.str();
}

static void usage() { std::cerr << "Usage: yaml-cmp [--subset] <expected.yaml> <actual.yaml>\n"; }

int main(int argc, char **argv) {
  bool subset_mode = false;
  int argi = 1;
  if (argc >= 2 && std::string_view(argv[1]) == "--subset") {
    subset_mode = true;
    ++argi;
  }
  if (argc - argi != 2) {
    usage();
    return 1;
  }
  fs::path expected_path = argv[argi];
  fs::path actual_path = argv[argi + 1];
  try {
    auto expected_result = shilos::yaml::YamlDocument::Read(expected_path.string());
    auto actual_result = shilos::yaml::YamlDocument::Read(actual_path.string());

    shilos::vswitch(
        expected_result,
        [&](const shilos::yaml::ParseError &expected_err) {
          std::cerr << "yaml-cmp error parsing expected file: " << expected_err.what() << "\n";
          std::exit(1);
        },
        [&](const shilos::yaml::YamlDocument &expected_doc) {
          shilos::vswitch(
              actual_result,
              [&](const shilos::yaml::ParseError &actual_err) {
                std::cerr << "yaml-cmp error parsing actual file: " << actual_err.what() << "\n";
                std::exit(1);
              },
              [&](const shilos::yaml::YamlDocument &actual_doc) {
                const Node &expected_node = expected_doc.root();
                const Node &actual_node = actual_doc.root();
                bool ok = subset_mode ? yaml_cmp::yaml_subset(expected_node, actual_node)
                                      : yaml_cmp::yaml_equal(expected_node, actual_node);
                if (!ok) {
                  std::cerr << "yaml-cmp: comparison FAILED\n";
                  std::cerr << "--- expected (subset=" << (subset_mode ? "yes" : "no") << ") ---\n";
                  std::cerr << shilos::yaml::format_yaml(expected_node) << "\n";
                  std::cerr << "--- actual ---\n";
                  std::cerr << shilos::yaml::format_yaml(actual_node) << "\n";
                  std::exit(1);
                }
              });
        });
  } catch (const std::exception &e) {
    std::cerr << "yaml-cmp error: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
