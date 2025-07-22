#pragma once

#include <shilos.hh>

#include <fstream>
#include <iostream>

namespace yaml_cmp {

using shilos::yaml::Node;

// Compare two scalar nodes for equality
inline bool scalar_equal(const Node &a, const Node &b) {
  return shilos::yaml::format_yaml(a) == shilos::yaml::format_yaml(b);
}

// Check if expected is a subset of actual
inline bool yaml_subset(const Node &expected, const Node &actual) {
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
      const auto &key = e_entry.key; // Keep as string_view - caller must keep Document alive
      auto it = a_map.find(key);
      if (it == a_map.end() || !yaml_subset(e_entry.value, it->value))
        return false;
    }
    return true;
  }
  return false;
}

// Check if two nodes are equal (bidirectional subset)
inline bool yaml_equal(const Node &a, const Node &b) { return yaml_subset(a, b) && yaml_subset(b, a); }

// Compare authored document with expected structure
inline bool compare_authored_with_expected(const shilos::yaml::Document &authored_doc,
                                           const shilos::yaml::Document &expected_doc, bool subset_mode = false) {
  const Node &authored_node = authored_doc.root();
  const Node &expected_node = expected_doc.root();

  return subset_mode ? yaml_subset(expected_node, authored_node) : yaml_equal(authored_node, expected_node);
}

// Compare multi-document authored document with expected structures
inline bool compare_multi_document(const shilos::yaml::Document &authored_doc,
                                   const std::vector<shilos::yaml::Document> &expected_docs, bool subset_mode = false) {
  if (authored_doc.documentCount() != expected_docs.size()) {
    return false;
  }

  for (size_t i = 0; i < authored_doc.documentCount(); ++i) {
    const Node &authored_node = authored_doc.root(i);
    const Node &expected_node = expected_docs[i].root();

    bool matches = subset_mode ? yaml_subset(expected_node, authored_node) : yaml_equal(authored_node, expected_node);
    if (!matches) {
      return false;
    }
  }
  return true;
}

// Compare authored document with expected document from file
inline bool compare_authored_with_expected(const shilos::yaml::Document &authored_doc, const std::string &expected_file,
                                           bool subset_mode = false) {
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
  return compare_authored_with_expected(authored_doc, expected_doc, subset_mode);
}

// Compare multi-document authored with expected documents from file
inline bool compare_multi_document(const shilos::yaml::Document &authored_doc, const std::string &expected_file,
                                   bool subset_mode = false) {
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

    bool matches = subset_mode ? yaml_subset(expected_node, authored_node) : yaml_equal(authored_node, expected_node);
    if (!matches) {
      return false;
    }
  }
  return true;
}

} // namespace yaml_cmp
