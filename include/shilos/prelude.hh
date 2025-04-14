#pragma once

#include "./uuid.hh"

#include <concepts>
#include <cstring>
#include <map>
#include <string_view>
#include <variant>
#include <vector>

namespace shilos {

template <typename RT>
concept ValidMemRegionRootType = requires {
  { RT::TYPE_UUID } -> std::same_as<const UUID &>;
};

template <typename VT, typename RT> class global_ptr;
template <typename VT> class regional_ptr;
class regional_str;
template <typename VT> class regional_list;
template <typename RT>
  requires ValidMemRegionRootType<RT>
class memory_region;

namespace yaml {

struct Node;

using Map = std::map<std::string, Node>;
using Sequence = std::vector<Node>;

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

void format_yaml(std::ostream &os, const Node &node, int indent = 0);
std::ostream &operator<<(std::ostream &os, const Node &node);
std::string format_yaml(const Node &node);

template <typename T, typename RT>
concept YamlConvertible = requires(T t, const yaml::Node &node, memory_region<RT> &mr, regional_ptr<T> &to_ptr) {
  { t.to_yaml() } -> std::same_as<yaml::Node>;
  { T::from_yaml(mr, node) } -> std::same_as<global_ptr<T, RT>>;
  { T::from_yaml(mr, node, to_ptr) } -> std::same_as<void>;
};

} // namespace yaml

} // namespace shilos
