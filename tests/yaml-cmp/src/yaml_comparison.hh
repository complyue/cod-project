#pragma once

#include <shilos.hh>

#include <algorithm>
#include <fstream>
#include <iostream>

namespace yaml_cmp {

using shilos::yaml::DashSequence;
using shilos::yaml::Map;
using shilos::yaml::Node;
using shilos::yaml::SimpleSequence;

// Helper function to find a key in a vector-based map
inline auto find_in_map(const Map &map, std::string_view key) {
  return std::find_if(map.begin(), map.end(), [key](const auto &entry) { return entry.key == key; });
}

// Forward declaration
inline bool yaml_subset(const Node &expected, const Node &actual, bool ignore_comments);

// Compare two scalar nodes for equality
inline bool scalar_equal(const Node &a, const Node &b, bool ignore_comments = false) {
  if (ignore_comments) {
    // When ignoring comments, we compare the actual values only
    // This is a simplified implementation - in a real implementation,
    // we would need to parse the YAML and compare the semantic values
    return shilos::yaml::format_yaml(a) == shilos::yaml::format_yaml(b);
  }
  return shilos::yaml::format_yaml(a) == shilos::yaml::format_yaml(b);
}

// Helper function to compare sequence nodes
inline bool compare_sequence_nodes(const Node &a, const Node &b, bool ignore_comments = false) {
  // Handle SimpleSequence vs SimpleSequence
  if (const auto *a_simple = std::get_if<SimpleSequence>(&a.value)) {
    if (const auto *b_simple = std::get_if<SimpleSequence>(&b.value)) {
      if (a_simple->size() != b_simple->size())
        return false;
      for (size_t i = 0; i < a_simple->size(); ++i) {
        if (!yaml_subset((*a_simple)[i], (*b_simple)[i], ignore_comments))
          return false;
      }
      return true;
    }
  }

  // Handle DashSequence vs DashSequence
  if (const auto *a_dash = std::get_if<DashSequence>(&a.value)) {
    if (const auto *b_dash = std::get_if<DashSequence>(&b.value)) {
      if (a_dash->size() != b_dash->size())
        return false;
      for (size_t i = 0; i < a_dash->size(); ++i) {
        if (!yaml_subset((*a_dash)[i].value, (*b_dash)[i].value, ignore_comments))
          return false;
      }
      return true;
    }
  }

  // Handle SimpleSequence vs DashSequence (compare as if both were SimpleSequence)
  if (const auto *a_simple = std::get_if<SimpleSequence>(&a.value)) {
    if (const auto *b_dash = std::get_if<DashSequence>(&b.value)) {
      if (a_simple->size() != b_dash->size())
        return false;
      for (size_t i = 0; i < a_simple->size(); ++i) {
        if (!yaml_subset((*a_simple)[i], (*b_dash)[i].value, ignore_comments))
          return false;
      }
      return true;
    }
  }

  // Handle DashSequence vs SimpleSequence (compare as if both were SimpleSequence)
  if (const auto *a_dash = std::get_if<DashSequence>(&a.value)) {
    if (const auto *b_simple = std::get_if<SimpleSequence>(&b.value)) {
      if (a_dash->size() != b_simple->size())
        return false;
      for (size_t i = 0; i < a_dash->size(); ++i) {
        if (!yaml_subset((*a_dash)[i].value, (*b_simple)[i], ignore_comments))
          return false;
      }
      return true;
    }
  }

  return false;
}

// Check if expected is a subset of actual
inline bool yaml_subset(const Node &expected, const Node &actual, bool ignore_comments = false) {
  if (expected.IsNull())
    return actual.IsNull();

  if (expected.IsScalar()) {
    return actual.IsScalar() && scalar_equal(expected, actual, ignore_comments);
  }

  if (expected.IsSequence()) {
    if (!actual.IsSequence())
      return false;
    return compare_sequence_nodes(expected, actual, ignore_comments);
  }

  if (expected.IsMap()) {
    if (!actual.IsMap())
      return false;
    const auto &e_map = std::get<Map>(expected.value);
    const auto &a_map = std::get<Map>(actual.value);
    for (const auto &e_entry : e_map) {
      const auto &key = e_entry.key; // Keep as string_view - caller must keep Document alive
      auto it = find_in_map(a_map, key);
      if (it == a_map.end() || !yaml_subset(e_entry.value, it->value, ignore_comments))
        return false;
    }
    return true;
  }
  return false;
}

// Check if two nodes are equal (bidirectional subset)
inline bool yaml_equal(const Node &a, const Node &b, bool ignore_comments = false) {
  return yaml_subset(a, b, ignore_comments) && yaml_subset(b, a, ignore_comments);
}

// Compare authored document with expected structure
inline bool compare_authored_with_expected(const shilos::yaml::Document &authored_doc,
                                           const shilos::yaml::Document &expected_doc, bool subset_mode = false,
                                           bool ignore_comments = false) {
  const Node &authored_node = authored_doc.root();
  const Node &expected_node = expected_doc.root();

  return subset_mode ? yaml_subset(expected_node, authored_node, ignore_comments)
                     : yaml_equal(authored_node, expected_node, ignore_comments);
}

// Compare multi-document authored document with expected structures
inline bool compare_multi_document(const shilos::yaml::Document &authored_doc,
                                   const std::vector<shilos::yaml::Document> &expected_docs, bool subset_mode = false,
                                   bool ignore_comments = false) {
  if (authored_doc.documentCount() != expected_docs.size()) {
    return false;
  }

  for (size_t i = 0; i < authored_doc.documentCount(); ++i) {
    const Node &authored_node = authored_doc.root(i);
    const Node &expected_node = expected_docs[i].root();

    bool matches = subset_mode ? yaml_subset(expected_node, authored_node, ignore_comments)
                               : yaml_equal(authored_node, expected_node, ignore_comments);
    if (!matches) {
      return false;
    }
  }
  return true;
}

// Compare authored document with expected document from file
inline bool compare_authored_with_expected(const shilos::yaml::Document &authored_doc, const std::string &expected_file,
                                           bool subset_mode = false, bool ignore_comments = false) {
  // Read file content
  std::ifstream file(expected_file);
  if (!file.is_open()) {
    std::cerr << "Failed to open expected file: " << expected_file << std::endl;
    return false;
  }

  std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  file.close();

  // Load expected document from file content
  auto expected_result = shilos::yaml::Document::Parse(expected_file, content);
  if (std::holds_alternative<shilos::yaml::ParseError>(expected_result)) {
    std::cerr << "Failed to parse expected file: " << expected_file << std::endl;
    return false;
  }

  auto expected_doc = std::get<shilos::yaml::Document>(std::move(expected_result));
  return compare_authored_with_expected(authored_doc, expected_doc, subset_mode, ignore_comments);
}

// Compare multi-document authored with expected documents from file
inline bool compare_multi_document(const shilos::yaml::Document &authored_doc, const std::string &expected_file,
                                   bool subset_mode = false, bool ignore_comments = false) {
  // Read file content
  std::ifstream file(expected_file);
  if (!file.is_open()) {
    std::cerr << "Failed to open expected file: " << expected_file << std::endl;
    return false;
  }

  std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  file.close();

  // Load expected documents from file content
  auto expected_result = shilos::yaml::Document::Parse(expected_file, content);
  if (std::holds_alternative<shilos::yaml::ParseError>(expected_result)) {
    std::cerr << "Failed to parse expected file: " << expected_file << std::endl;
    return false;
  }

  auto expected_doc = std::get<shilos::yaml::Document>(std::move(expected_result));

  if (authored_doc.documentCount() != expected_doc.documentCount()) {
    return false;
  }

  for (size_t i = 0; i < authored_doc.documentCount(); ++i) {
    const Node &authored_node = authored_doc.root(i);
    const Node &expected_node = expected_doc.root(i);

    bool matches = subset_mode ? yaml_subset(expected_node, authored_node, ignore_comments)
                               : yaml_equal(authored_node, expected_node, ignore_comments);
    if (!matches) {
      return false;
    }
  }
  return true;
}

} // namespace yaml_cmp