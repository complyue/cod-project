#include "shilos/prelude.hh"

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

static bool yaml_subset(const Node &expected, const Node &actual);

static bool scalar_equal(const Node &a, const Node &b) {
  return shilos::yaml::format_yaml(a) == shilos::yaml::format_yaml(b);
}

static bool yaml_subset(const Node &expected, const Node &actual) {
  using shilos::yaml::Map;
  using shilos::yaml::Sequence;

  if (expected.IsNull())
    return actual.IsNull();

  if (expected.IsScalar()) {
    return actual.IsScalar() && scalar_equal(expected, actual);
  }

  if (expected.IsSequence()) {
    if (!actual.IsSequence())
      return false;
    const auto &e_seq = std::get<Sequence>(expected.value);
    const auto &a_seq = std::get<Sequence>(actual.value);
    if (e_seq.size() > a_seq.size())
      return false;
    for (size_t i = 0; i < e_seq.size(); ++i)
      if (!yaml_subset(e_seq[i], a_seq[i]))
        return false;
    return true;
  }

  if (expected.IsMap()) {
    if (!actual.IsMap())
      return false;
    const auto &e_map = std::get<Map>(expected.value);
    const auto &a_map = std::get<Map>(actual.value);
    for (const auto &e_entry : e_map) {
      const std::string &key = e_entry.key;
      auto it = a_map.find(key);
      if (it == a_map.end() || !yaml_subset(e_entry.value, it->value))
        return false;
    }
    return true;
  }
  return false;
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
    Node expected_node = Node::Load(read_file(expected_path));
    Node actual_node = Node::Load(read_file(actual_path));
    bool ok = subset_mode ? yaml_subset(expected_node, actual_node)
                          : (yaml_subset(expected_node, actual_node) && yaml_subset(actual_node, expected_node));
    if (!ok) {
      std::cerr << "yaml-cmp: comparison FAILED\n";
      std::cerr << "--- expected (subset=" << (subset_mode ? "yes" : "no") << ") ---\n";
      std::cerr << shilos::yaml::format_yaml(expected_node) << "\n";
      std::cerr << "--- actual ---\n";
      std::cerr << shilos::yaml::format_yaml(actual_node) << "\n";
      return 1;
    }
  } catch (const std::exception &e) {
    std::cerr << "yaml-cmp error: " << e.what() << "\n";
    return 1;
  }
  return 0;
} 
