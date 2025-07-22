#pragma once

#include "./iopd.hh"
#include "./iops.hh"

#include <concepts>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

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

  std::string to_string() const {
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

template <class Variant, class... Fs> decltype(auto) vswitch(Variant &&v, Fs &&...fs) {
  struct visitor : Fs... {
    using Fs::operator()...;
  };
  return std::visit(visitor{std::forward<Fs>(fs)...}, std::forward<Variant>(v));
}

namespace yaml {

// YAML exception hierarchy
class Exception : public std::runtime_error {
private:
  std::string stack_trace_;

public:
  // Explicit constructors that capture stack trace immediately
  explicit Exception(const std::string &message);

  const std::string &stack_trace() const;
};

class ParseError : public Exception {
private:
  std::string filename_;
  size_t line_;
  size_t column_;
  std::string message_;

public:
  ParseError(std::string message, std::string filename = "", size_t line = 0, size_t column = 0)
      : Exception(FormatErrorMessage(filename, line, column, message)), filename_(std::move(filename)), line_(line),
        column_(column), message_(std::move(message)) {}

  const std::string &filename() const noexcept { return filename_; }
  size_t line() const noexcept { return line_; }
  size_t column() const noexcept { return column_; }
  const std::string &message() const noexcept { return message_; }

private:
  static std::string FormatErrorMessage(const std::string &filename, size_t line, size_t column,
                                        const std::string &message) {
    if (filename.empty()) {
      return message;
    }
    return filename + ":" + std::to_string(line) + ":" + std::to_string(column) + ": " + message;
  }
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

class AuthorError : public Exception {
private:
  std::string filename_;
  std::string message_;

public:
  AuthorError(std::string filename, std::string message)
      : Exception(FormatErrorMessage(filename, message)), filename_(std::move(filename)), message_(std::move(message)) {
  }

  // Legacy constructor for backward compatibility during transition
  explicit AuthorError(std::string message) : Exception(message), filename_(""), message_(std::move(message)) {}

  const std::string &filename() const noexcept { return filename_; }
  const std::string &message() const noexcept { return message_; }

private:
  static std::string FormatErrorMessage(const std::string &filename, const std::string &message) {
    if (filename.empty()) {
      return message;
    }
    return filename + ": " + message;
  }
};

// Forward declarations
struct Node;
class YamlDocument;
class YamlAuthor;

std::string format_yaml(const Node &node);

// Result type for noexcept YAML parsing
using ParseResult = std::variant<YamlDocument, ParseError>;
using AuthorResult = std::variant<YamlDocument, AuthorError>;

using Map = iopd<std::string_view, Node>;
using Sequence = std::vector<Node>;

// Forward declaration for parsing functions
struct ParseState;

// Complete YAML node type definition
struct Node {
  using Value = std::variant<std::monostate, // null
                             bool, int64_t, double, std::string_view, Sequence, Map>;

  Value value;

  // Allow YamlAuthor and YamlDocument to access private members for modification during authoring
  friend class YamlAuthor;
  friend class YamlDocument;

  // Allow parsing functions to access private string constructors
  friend Node parse_document(ParseState &state);
  friend Node parse_value(ParseState &state);
  friend Node parse_mapping(ParseState &state);
  friend Node parse_sequence(ParseState &state);
  friend Node parse_json_mapping(ParseState &state);
  friend Node parse_json_sequence(ParseState &state);
  friend Node parse_json_value(ParseState &state);
  friend Node parse_multiline_scalar(ParseState &state);
  friend Node parse_alias(ParseState &state);
  friend Node parse_anchored_value(ParseState &state);
  friend Node parse_tagged_value(ParseState &state);
  friend Node parse_scalar_value(std::string_view scalar_text);

  // Allow YAML parsing functions to access private constructors and assignment operators
  template <typename RT, typename K, typename V>
  friend void from_yaml(memory_region<RT> &mr, const yaml::Node &node, iopd<K, V> *raw_ptr);

  // Allow YamlAuthor to access private constructors and assignment operators
  friend class YamlAuthor;

  // Allow YamlDocument and parsing functions to access private constructors
  friend class YamlDocument;

  // Allow template functions in header files to access private members (parsing only)
  template <typename RT, typename K, typename V, typename F>
  friend void from_yaml(memory_region<RT> &mr, const yaml::Node &node, iopd<K, V> *raw_ptr, F make_key_callable);
  template <typename RT, typename K, typename V>
  friend void from_yaml(memory_region<RT> &mr, const yaml::Node &node, iopd<K, V> *raw_ptr);

  // Allow all from_yaml template functions to access private constructors
  template <typename RT, typename T> friend void from_yaml(memory_region<RT> &mr, const yaml::Node &node, T *raw_ptr);

public:
  Node() = default;
  Node(const Node &other) = default;
  Node(Node &&other) = default;
  explicit Node(Value v) : value(std::move(v)) {}

  // Helper constructors (non-string types)
  Node(std::nullptr_t) : value(std::monostate{}) {}
  explicit Node(bool b) : value(b) {}
  explicit Node(int64_t i) : value(i) {}
  explicit Node(double d) : value(d) {}
  explicit Node(const Sequence &seq) : value(seq) {}
  explicit Node(const Map &map) : value(map) {}

  // Assignment operators (default and non-string types) - public for internal use
  Node &operator=(const Node &other) = default;
  Node &operator=(Node &&other) = default;
  Node &operator=(bool b) {
    value = b;
    return *this;
  }
  Node &operator=(int i) {
    value = static_cast<int64_t>(i);
    return *this;
  }
  Node &operator=(int64_t i) {
    value = i;
    return *this;
  }
  Node &operator=(double d) {
    value = d;
    return *this;
  }
  Node &operator=(const Sequence &seq) {
    value = seq;
    return *this;
  }
  Node &operator=(const Map &map) {
    value = map;
    return *this;
  }

private:
  // Private string constructors - only accessible through YamlAuthor and parsing functions
  explicit Node(const std::string_view s) : value(s) {}
  Node(const std::string &s) : value(std::string_view(s)) {}
  explicit Node(const char *s) : value(std::string_view(s)) {}

  // Private string assignment operators - only accessible through YamlAuthor and parsing functions
  Node &operator=(std::string_view s) {
    value = s;
    return *this;
  }
  Node &operator=(const std::string &s) {
    value = std::string_view(s);
    return *this;
  }
  Node &operator=(const char *s) {
    value = std::string_view(s);
    return *this;
  }

public:
  // Type checking methods
  bool IsScalar() const {
    return std::holds_alternative<bool>(value) || std::holds_alternative<int64_t>(value) ||
           std::holds_alternative<double>(value) || std::holds_alternative<std::string_view>(value);
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

private:
  std::string describe_actual_type() const {
    if (std::holds_alternative<std::monostate>(value))
      return "null";
    if (std::holds_alternative<bool>(value))
      return "bool";
    if (std::holds_alternative<int64_t>(value))
      return "integer";
    if (std::holds_alternative<double>(value))
      return "double";
    if (std::holds_alternative<std::string_view>(value))
      return "string";
    if (std::holds_alternative<yaml::Sequence>(value))
      return "sequence";
    if (std::holds_alternative<yaml::Map>(value))
      return "map";
    return "unknown";
  }

public:
  // Convenience methods for common type conversions
  const Map &asMap() const {
    if (auto map = std::get_if<Map>(&value)) {
      return *map;
    }
    throw TypeError("Expected map value");
  }

  const Sequence &asSequence() const {
    if (auto seq = std::get_if<Sequence>(&value)) {
      return *seq;
    }
    throw TypeError("Expected sequence value");
  }

  std::string asString() const {
    if (auto s = std::get_if<std::string_view>(&value)) {
      return std::string(*s);
    }
    throw TypeError("Expected string value, got " + describe_actual_type());
  }

  bool asBool() const {
    if (auto b = std::get_if<bool>(&value)) {
      return *b;
    }
    throw TypeError("Expected bool value, got " + describe_actual_type());
  }

  int asInt() const {
    if (auto i = std::get_if<int64_t>(&value)) {
      return static_cast<int>(*i);
    }
    throw TypeError("Expected integer value, got " + describe_actual_type());
  }

  int64_t asInt64() const {
    if (auto i = std::get_if<int64_t>(&value)) {
      return *i;
    }
    throw TypeError("Expected integer value, got " + describe_actual_type());
  }

  double asDouble() const {
    if (auto d = std::get_if<double>(&value)) {
      return *d;
    }
    throw TypeError("Expected double value, got " + describe_actual_type());
  }

  float asFloat() const {
    if (auto d = std::get_if<double>(&value)) {
      return static_cast<float>(*d);
    }
    throw TypeError("Expected double value, got " + describe_actual_type());
  }

  const Node &operator[](std::string_view key) const {
    if (auto map = std::get_if<Map>(&value)) {
      return map->at(key);
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

  Map::const_iterator find(std::string_view key) const {
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

  bool contains(std::string_view key) const {
    if (auto map = std::get_if<Map>(&value)) {
      return map->find(key) != map->end();
    }
    throw TypeError("Expected map value");
  }
};

// YAML authoring interface for programmatic document creation
// Provides string tracking and node manipulation within a callback context
class YamlAuthor {
private:
  std::string filename_;
  iops<std::string> owned_strings_; // Tracks strings created during authoring
  std::vector<Node> roots_;         // Multiple root nodes for multi-document support

  friend class YamlDocument;

  explicit YamlAuthor(std::string filename) : filename_(std::move(filename)) {}

public:
  // Public constructor for testing purposes
  YamlAuthor() : filename_("test") {}

  // Non-copyable and non-movable to ensure string_view stability
  YamlAuthor(const YamlAuthor &) = delete;
  YamlAuthor &operator=(const YamlAuthor &) = delete;
  YamlAuthor(YamlAuthor &&) = delete;
  YamlAuthor &operator=(YamlAuthor &&) = delete;

  // String creation methods that return string_views backed by this author
  std::string_view createStringView(std::string str) { return owned_strings_.insert(std::move(str)); }

  std::string_view createStringView(std::string_view str) { return owned_strings_.insert(std::string(str)); }

  std::string_view createStringView(const char *str) { return owned_strings_.insert(std::string(str)); }

  // Node creation methods
  Node createString(std::string str) { return Node(owned_strings_.insert(std::move(str))); }

  Node createString(std::string_view str) { return Node(owned_strings_.insert(std::string(str))); }

  Node createString(const char *str) { return Node(owned_strings_.insert(std::string(str))); }

  // Scalar creation methods
  Node createScalar(bool value) { return Node(value); }

  Node createScalar(int value) { return Node(static_cast<int64_t>(value)); }

  Node createScalar(int64_t value) { return Node(value); }

  Node createScalar(double value) { return Node(value); }

  Node createScalar(float value) { return Node(static_cast<double>(value)); }

  // Container creation methods
  Node createMap() { return Node(Map{}); }

  Node createSequence() { return Node(Sequence{}); }

  // Node modification methods (only available during authoring)
  void setMapValue(Node &map_node, std::string_view key, const Node &value) {
    if (auto map = std::get_if<Map>(&map_node.value)) {
      (*map)[key] = value;
    } else {
      throw TypeError("Expected map value");
    }
  }

  void pushToSequence(Node &seq_node, const Node &value) {
    if (auto seq = std::get_if<Sequence>(&seq_node.value)) {
      seq->push_back(value);
    } else {
      throw TypeError("Expected sequence value");
    }
  }

  void assignNode(Node &target, const Node &source) { target.value = source.value; }

  // Add root document to the multi-document stream
  void addRoot(const Node &root) { roots_.push_back(root); }

  void addRoot(Node &&root) { roots_.push_back(std::move(root)); }

  // Access to filename for error reporting
  const std::string &filename() const noexcept { return filename_; }
};

// YAML document stream that owns the underlying string payload
// Supports multiple documents separated by --- or ...
// All yaml nodes must not outlive the document that created them
//
// YAML API Design Pattern - Two flavors for different error scenarios:
// - Exception-throwing flavor (Constructor-based): Throws exceptions directly when YAML formatting errors are
// unusual/unexpected
// - noexcept flavor (Static method-based): Returns Result variants when YAML formatting errors are expected/common
//
// This dual API pattern provides both fail-fast behavior (exception-throwing) and graceful degradation (noexcept)
// for different scenarios where formatting errors are either exceptional or routine.
class YamlDocument {
private:
  std::string source_;              // Owns the original YAML string for string_view lifetime
  std::vector<Node> documents_;     // Multiple documents in the stream
  iops<std::string> owned_strings_; // Owns escaped strings and keys that nodes reference (deduplicated)

  // Internal constructor for authoring - transfers ownership from YamlAuthor
  // Private - accessed by static methods (which are class members)
  YamlDocument(std::string filename, std::vector<Node> documents, iops<std::string> owned_strings);

  // Helper function to read file content for filename-only constructor
  static std::string read_file(const std::string &filename) {
    std::ifstream file(filename);
    if (!file) {
      throw ParseError("Failed to open file for reading", filename);
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  }

public:
  // Constructor for parsing from source - THROWS ParseError on failure
  YamlDocument(std::string filename, std::string source);

  // Constructor for parsing from file - THROWS ParseError on failure
  YamlDocument(std::string filename) : YamlDocument(filename, read_file(filename)) {}

  // Constructor for authoring with callback - THROWS AuthorError on failure
  // This is the throwing version of the authoring API
  template <typename AuthorCallback>
    requires std::invocable<AuthorCallback, YamlAuthor &>
  YamlDocument(std::string filename, AuthorCallback &&callback, bool write = true, bool overwrite = true) {
    try {
      YamlAuthor author(filename);

      // Call the user callback to create/manipulate nodes (void return)
      callback(author);

      // Transfer roots from author
      documents_ = std::move(author.roots_);

      // Ensure at least one document exists
      if (documents_.empty()) {
        throw AuthorError(filename, "No root documents created by callback");
      }

      // Transfer string ownership from author
      owned_strings_ = std::move(author.owned_strings_);

      // Generate YAML source from authored content
      std::ostringstream oss;
      for (size_t i = 0; i < documents_.size(); ++i) {
        if (i > 0) {
          oss << "---\n";
        }
        oss << format_yaml(documents_[i]);
      }
      source_ = oss.str();

      // Write to file if requested
      if (write) {
        // Check if file exists and overwrite is false
        if (!overwrite && std::filesystem::exists(filename)) {
          throw AuthorError(filename, "File already exists and overwrite is false");
        }

        // Create directory if it doesn't exist
        if (auto parent = std::filesystem::path(filename).parent_path(); !parent.empty()) {
          std::filesystem::create_directories(parent);
        }

        // Write YAML content to file
        std::ofstream ofs(filename);
        if (!ofs) {
          throw AuthorError(filename, "Failed to open file for writing");
        }

        // Write all documents
        for (size_t i = 0; i < documents_.size(); ++i) {
          if (i > 0) {
            ofs << "---\n";
          }
          ofs << format_yaml(documents_[i]);
        }

        if (!ofs) {
          throw AuthorError(filename, "Failed to write to file");
        }
      }
    } catch (const std::exception &e) {
      throw AuthorError(filename, "YAML authoring error: " + std::string(e.what()));
    }
  }

  // Non-copyable to ensure string_view stability - move allowed for variant usage
  YamlDocument(const YamlDocument &) = delete;
  YamlDocument &operator=(const YamlDocument &) = delete;
  YamlDocument(YamlDocument &&) = default;
  YamlDocument &operator=(YamlDocument &&) = delete;

  // Access to documents
  const std::vector<Node> &documents() const noexcept { return documents_; }
  std::vector<Node> &documents() noexcept { return documents_; }

  // Convenience methods for single-document use
  const Node &root() const {
    if (documents_.empty())
      throw std::runtime_error("No documents in YAML stream");
    return documents_[0];
  }
  Node &root() {
    if (documents_.empty())
      throw std::runtime_error("No documents in YAML stream");
    return documents_[0];
  }

  // Multi-document support - access root by index
  const Node &root(size_t index) const {
    if (index >= documents_.size())
      throw std::runtime_error("Document index out of range");
    return documents_[index];
  }
  Node &root(size_t index) {
    if (index >= documents_.size())
      throw std::runtime_error("Document index out of range");
    return documents_[index];
  }

  // Check if this is a multi-document stream
  bool isMultiDocument() const noexcept { return documents_.size() > 1; }
  size_t documentCount() const noexcept { return documents_.size(); }

  // Static parsing function - noexcept version that returns ParseResult
  // Use this when you prefer error handling via Result types instead of exceptions
  static ParseResult Parse(std::string filename, std::string source) noexcept;
  static ParseResult Parse(std::string filename, std::string_view source) noexcept {
    return Parse(std::move(filename), std::string(source));
  }

  // Static authoring function - noexcept version that returns AuthorResult
  // Use this when you prefer error handling via Result types instead of exceptions
  template <typename AuthorCallback>
    requires std::invocable<AuthorCallback, YamlAuthor &>
  static AuthorResult Write(std::string filename, AuthorCallback &&callback, bool write = true,
                            bool overwrite = true) noexcept {
    try {
      // Delegate to authoring constructor
      YamlDocument doc(filename, std::forward<AuthorCallback>(callback), write, overwrite);
      return AuthorResult{std::in_place_type<YamlDocument>, std::move(doc)};
    } catch (const AuthorError &e) {
      return AuthorError(e.filename(), e.message());
    } catch (const std::exception &e) {
      return AuthorError(filename, "YAML authoring error: " + std::string(e.what()));
    }
  }

  // Static read function - noexcept version that returns ParseResult
  // Use this when you prefer error handling via Result types instead of exceptions
  static ParseResult Read(const std::string &filepath) noexcept {
    try {
      return Parse(filepath, read_file(filepath));
    } catch (const std::exception &e) {
      return ParseError("Error reading file: " + std::string(e.what()), filepath);
    }
  }
};

void format_yaml(std::ostream &os, const Node &node, int indent = 0);
std::ostream &operator<<(std::ostream &os, const Node &node);
std::string format_yaml(const Node &node);

template <typename T, typename RT>
concept YamlConvertible =
    requires(T t, const yaml::Node &node, memory_region<RT> &mr, T *raw_ptr, yaml::YamlAuthor &author) {
      { to_yaml(t, author) } noexcept -> std::same_as<yaml::Node>;
      { from_yaml<T>(mr, node, raw_ptr) } -> std::same_as<void>;
    };

} // namespace yaml

} // namespace shilos
