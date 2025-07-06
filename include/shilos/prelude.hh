#pragma once

#include <concepts>
#include <cstring>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <shilos/iopd.hh>

namespace shilos {

class UUID {
private:
  uint8_t data_[16];

  constexpr static void byte_to_hex(std::uint8_t byte, char *output) {
    constexpr std::string_view hex_chars = "0123456789ABCDEF";
    output[0] = hex_chars[byte >> 4];
    output[1] = hex_chars[byte & 0x0F];
  }

  constexpr static std::uint8_t hex_to_byte(char c) {
    if (c >= '0' && c <= '9')
      return c - '0';
    if (c >= 'a' && c <= 'f')
      return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
      return c - 'A' + 10;
    throw std::invalid_argument(std::string("Invalid hex character: [") + c + "]");
  }

public:
  // Generate a random UUID
  UUID() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint8_t> dist(0, 255);
    for (auto &byte : data_) {
      byte = dist(gen);
    }
    // Adjust UUID fields according to RFC 4122:
    // - Set version to 4 (random UUID)
    data_[6] = (data_[6] & 0x0F) | 0x40;
    // - Set variant to 2 (RFC 4122 variant)
    data_[8] = (data_[8] & 0x3F) | 0x80;
  }

  // Construct from string_view
  explicit constexpr UUID(std::string_view str) {
    if (str.size() != 36)
      throw std::invalid_argument("Invalid UUID string size");
    size_t ui = 0;
    for (size_t i = 0; i < 36;) {
      if (i == 8 || i == 13 || i == 18 || i == 23) {
        if (str[i] != '-')
          throw std::invalid_argument("Invalid UUID format");
        i++;
      } else {
        data_[ui++] = (hex_to_byte(str[i]) << 4) | hex_to_byte(str[i + 1]);
        i += 2;
      }
    }
  }

  constexpr std::string to_string() const {
    char result[36];
    size_t idx = 0;
    for (size_t i = 0; i < 16; ++i) {
      byte_to_hex(data_[i], &result[idx]);
      idx += 2;
      // Add dashes at the appropriate positions
      if (i == 3 || i == 5 || i == 7 || i == 9) {
        result[idx++] = '-';
      }
    }
    return std::string(result, 36);
  }

  auto operator<=>(const UUID &other) const = default;
  bool operator==(const UUID &other) const = default;

  static UUID parse(const std::string &str) { return UUID(std::string_view(str)); }
  static UUID parse(std::string_view str) { return UUID(str); }
};

inline std::ostream &operator<<(std::ostream &os, const shilos::UUID &uuid) { return os << uuid.to_string(); }

template <typename RT>
concept ValidMemRegionRootType = requires {
  { RT::TYPE_UUID } -> std::same_as<const UUID &>;
};

template <typename VT, typename RT> class global_ptr;
template <typename VT> class regional_ptr;
class regional_str;
template <typename VT> class regional_fifo;
template <typename VT> class regional_lifo;
template <typename RT>
  requires ValidMemRegionRootType<RT>
class memory_region;

namespace yaml {

// YAML exception hierarchy
class Exception : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

class ParseError : public Exception {
public:
  using Exception::Exception;
};

class TypeError : public Exception {
public:
  using Exception::Exception;
};

class MissingFieldError : public Exception {
public:
  using Exception::Exception;
};

class RangeError : public Exception {
public:
  using Exception::Exception;
};

struct Node;

using Map = iopd<Node>;
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

  // Assignment operators for different types
  Node &operator=(const std::string &s) {
    value = s;
    return *this;
  }
  Node &operator=(std::string_view s) {
    value = std::string(s);
    return *this;
  }
  Node &operator=(const char *s) {
    value = std::string(s);
    return *this;
  }
  Node &operator=(bool b) {
    value = b;
    return *this;
  }
  Node &operator=(int64_t i) {
    value = i;
    return *this;
  }
  Node &operator=(int i) {
    value = static_cast<int64_t>(i);
    return *this;
  } // Handle plain int (potential data loss for large values)
  Node &operator=(double d) {
    value = d;
    return *this;
  }
  Node &operator=(std::nullptr_t) {
    value = std::monostate{};
    return *this;
  }

  // Type checking methods
  bool IsScalar() const {
    return std::holds_alternative<bool>(value) || std::holds_alternative<int64_t>(value) ||
           std::holds_alternative<double>(value) || std::holds_alternative<std::string>(value);
  }

  bool IsSequence() const { return std::holds_alternative<Sequence>(value); }

  bool IsMap() const { return std::holds_alternative<Map>(value); }

  bool IsNull() const { return std::holds_alternative<std::monostate>(value); }

  // Size method for sequences and maps
  size_t size() const {
    if (auto seq = std::get_if<Sequence>(&value)) {
      return seq->size();
    } else if (auto map = std::get_if<Map>(&value)) {
      return map->size();
    }
    return 0;
  }

  // push_back for sequences
  void push_back(const Node &node) {
    if (auto seq = std::get_if<Sequence>(&value)) {
      seq->push_back(node);
    } else {
      // Convert to sequence if not already
      value = Sequence{};
      std::get<Sequence>(value).push_back(node);
    }
  }

  void push_back(int64_t i) { push_back(Node(i)); }

  void push_back(const std::string &s) { push_back(Node(s)); }

  template <typename T> T as() const {
    if constexpr (std::is_same_v<T, std::string>) {
      if (auto s = std::get_if<std::string>(&value)) {
        return *s;
      }
      throw TypeError("Expected string value");
    } else if constexpr (std::is_same_v<T, int64_t>) {
      if (auto i = std::get_if<int64_t>(&value)) {
        return *i;
      }
      throw TypeError("Expected integer value");
    } else if constexpr (std::is_same_v<T, int>) {
      if (auto i = std::get_if<int64_t>(&value)) {
        return static_cast<int>(*i);
      }
      throw TypeError("Expected integer value");
    } else if constexpr (std::is_same_v<T, double>) {
      if (auto d = std::get_if<double>(&value)) {
        return *d;
      }
      throw TypeError("Expected double value");
    } else if constexpr (std::is_same_v<T, bool>) {
      if (auto b = std::get_if<bool>(&value)) {
        return *b;
      }
      throw TypeError("Expected bool value");
    }
    throw TypeError("Unsupported type conversion");
  }

  const Node &operator[](const std::string &key) const {
    if (auto map = std::get_if<Map>(&value)) {
      return map->at(key);
    }
    throw TypeError("Expected map value");
  }

  Node &operator[](const std::string &key) {
    if (auto map = std::get_if<Map>(&value)) {
      return (*map)[key];
    }
    throw TypeError("Expected map value");
  }

  // Indexing by integer for sequences
  const Node &operator[](size_t index) const {
    if (auto seq = std::get_if<Sequence>(&value)) {
      if (index >= seq->size()) {
        throw RangeError("Index out of range");
      }
      return (*seq)[index];
    }
    throw TypeError("Expected sequence value");
  }

  Node &operator[](size_t index) {
    if (auto seq = std::get_if<Sequence>(&value)) {
      if (index >= seq->size()) {
        throw RangeError("Index out of range");
      }
      return (*seq)[index];
    }
    throw TypeError("Expected sequence value");
  }

  Map::const_iterator find(const std::string &key) const {
    if (auto map = std::get_if<Map>(&value)) {
      return map->find(key);
    }
    throw TypeError("Expected map value");
  }

  Map::const_iterator end() const {
    if (auto map = std::get_if<Map>(&value)) {
      return map->end();
    }
    throw TypeError("Expected map value");
  }
};

void format_yaml(std::ostream &os, const Node &node, int indent = 0);
std::ostream &operator<<(std::ostream &os, const Node &node);
std::string format_yaml(const Node &node);

// Standalone Load function
inline Node Load(const std::string_view yaml_str) { return Node::Load(yaml_str); }

inline Node Load(const std::string &yaml_str) { return Node::Load(yaml_str); }

template <typename T, typename RT>
concept YamlConvertible = requires(T t, const yaml::Node &node, memory_region<RT> &mr, T *raw_ptr) {
  { to_yaml(t) } noexcept -> std::same_as<yaml::Node>;
  { from_yaml<T>(mr, node, raw_ptr) } -> std::same_as<void>;
};

} // namespace yaml

} // namespace shilos
