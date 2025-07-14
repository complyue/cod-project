#include "shilos/prelude.hh"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using shilos::yaml::Node;
namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Helper – read entire file into std::string
// ---------------------------------------------------------------------------
static std::string read_file(const fs::path &p) {
  std::ifstream ifs(p);
  if (!ifs)
    throw std::runtime_error("yaml-cmp: cannot open file: " + p.string());
  std::stringstream ss;
  ss << ifs.rdbuf();
  return ss.str();
}

// ---------------------------------------------------------------------------
// Recursive subset comparison of YAML nodes.
//  • Scalars must match exactly.
//  • Sequences – expected sequence size must not exceed actual, elements are
//    compared positionally for the prefix.
//  • Maps – every key in the expected map must exist in the actual map and the
//    corresponding value must be a subset recursively.
// ---------------------------------------------------------------------------
static bool yaml_subset(const Node &expected, const Node &actual);

static bool scalar_equal(const Node &a, const Node &b) {
  // Quick path – rely on formatted YAML string for scalars.
  return shilos::yaml::format_yaml(a) == shilos::yaml::format_yaml(b);
}

static bool yaml_subset(const Node &expected, const Node &actual) {
  using shilos::yaml::Map;
  using shilos::yaml::Sequence;

  // Null – require both null
  if (expected.IsNull())
    return actual.IsNull();

  // Scalar path -------------------------------------------------------------
  if (expected.IsScalar()) {
    if (!actual.IsScalar())
      return false;
    return scalar_equal(expected, actual);
  }

  // Sequence path -----------------------------------------------------------
  if (expected.IsSequence()) {
    if (!actual.IsSequence())
      return false;
    const auto &e_seq = std::get<Sequence>(expected.value);
    const auto &a_seq = std::get<Sequence>(actual.value);
    if (e_seq.size() > a_seq.size())
      return false; // actual sequence shorter than expected prefix
    for (size_t i = 0; i < e_seq.size(); ++i) {
      if (!yaml_subset(e_seq[i], a_seq[i]))
        return false;
    }
    return true;
  }

  // Map path ----------------------------------------------------------------
  if (expected.IsMap()) {
    if (!actual.IsMap())
      return false;
    const auto &e_map = std::get<Map>(expected.value);
    const auto &a_map = std::get<Map>(actual.value);
    // For every key in expected map ensure it is present in actual map and
    // value matches recursively.
    for (const auto &e_entry : e_map) {
      const std::string &key = e_entry.key;
      auto it = a_map.find(key);
      if (it == a_map.end())
        return false; // key missing
      if (!yaml_subset(e_entry.value, it->value))
        return false;
    }
    return true;
  }

  // Unhandled node categories (should not happen)
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
    const std::string expected_text = read_file(expected_path);
    const std::string actual_text = read_file(actual_path);

    Node expected_node = Node::Load(expected_text);
    Node actual_node = Node::Load(actual_text);

    bool ok;
    if (subset_mode) {
      ok = yaml_subset(expected_node, actual_node);
    } else {
      ok = yaml_subset(expected_node, actual_node) && yaml_subset(actual_node, expected_node);
    }

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
