#pragma once

#include "./region.hh"

#include <concepts>
#include <iomanip>
#include <map>
#include <optional>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

// Forward declarations
namespace shilos {

namespace yaml {
// Forward declarations
struct Node;
struct ParseState;

template <typename T, typename RT>
concept YamlConvertible = requires(T t, const yaml::Node &node, memory_region<RT> &mr, regional_ptr<T> &to_ptr) {
  { t.to_yaml() } -> std::same_as<yaml::Node>;
  { T::from_yaml(mr, node) } -> std::same_as<global_ptr<T, std::remove_cvref_t<decltype(t)>>>;
  { T::from_yaml(mr, node, to_ptr) } -> std::same_as<void>;
};

using Map = std::map<std::string, Node>;
using Sequence = std::vector<Node>;

std::optional<Node> parse_value(ParseState &state);

// Parsing state
struct ParseState {
  std::string_view input;
  size_t pos = 0;
  size_t line = 1;
  size_t column = 1;

  explicit ParseState(std::string_view str) : input(str) {}
};

// Complete YAML node type definition
struct Node {
  using Value = std::variant<std::monostate, // null
                             bool, int64_t, double, std::string, Sequence, Map>;

  Value value;

  Node() = default;
  explicit Node(Value v) : value(std::move(v)) {}

  static Node Load(const std::string_view yaml_str);

  // Helper constructors
  Node(std::nullptr_t) : value(std::monostate{}) {}
  explicit Node(bool b) : value(b) {}
  explicit Node(int64_t i) : value(i) {}
  explicit Node(double d) : value(d) {}
  explicit Node(const std::string s) : value(s) {}
  explicit Node(const std::string_view s) : value(std::string(s)) {}
  explicit Node(const char *s) : value(std::string(s)) {}
  explicit Node(const Sequence &seq) : value(seq) {}
  explicit Node(const Map &map) : value(map) {}
};

// Formatting state
struct FormatState {
  std::ostringstream output;
  int indent = 0;
};

// Forward declarations
std::optional<Node> parse_value(ParseState &state);

void format_value(FormatState &state, const Node &node);

std::ostream &operator<<(std::ostream &os, const Node &node);

} // namespace yaml
} // namespace shilos
