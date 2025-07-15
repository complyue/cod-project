
#include "shilos.hh"
#include "shilos/iops.hh"

#include <cctype>
#include <cstring>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace shilos {
namespace yaml {

// Indentation comparison result
enum class IndentRelation { Less, Equal, Greater, Incompatible };

// Parser state that tracks the source string for creating string_views
struct ParseState {
  std::string_view input;
  size_t line_begin_pos = 0;
  size_t pos = 0;
  size_t line = 1;
  size_t column = 1;
  iops owned_strings; // Storage for strings that need escaping (deduplicated)

  explicit ParseState(std::string_view str) : input(str) {}

  char current() const { return (pos < input.size()) ? input[pos] : '\0'; }

  char peek(size_t offset = 1) const { return (pos + offset < input.size()) ? input[pos + offset] : '\0'; }

  void advance() {
    if (pos < input.size()) {
      if (input[pos] == '\n') {
        line++;
        column = 1;
        pos++;
        line_begin_pos = pos; // Update line_begin_pos to start of new line
      } else {
        column++;
        pos++;
      }
    }
  }

  void skip_whitespace_inline() {
    while (pos < input.size() && (input[pos] == ' ' || input[pos] == '\t')) {
      advance();
    }
  }

  void skip_whitespace_and_newlines() {
    while (pos < input.size() &&
           (input[pos] == ' ' || input[pos] == '\t' || input[pos] == '\n' || input[pos] == '\r')) {
      advance();
    }
  }

  void skip_to_end_of_line() {
    while (pos < input.size() && input[pos] != '\n') {
      advance();
    }
  }

  // Get current line's indentation (from line_begin_pos to current non-whitespace pos)
  std::string_view current_line_indentation() const { return make_view(line_begin_pos, pos); }

  bool at_end() const { return pos >= input.size(); }

  // Create a string_view from the current position to end_pos
  std::string_view make_view(size_t start_pos, size_t end_pos) const {
    if (start_pos >= input.size() || end_pos > input.size() || start_pos >= end_pos) {
      return std::string_view{};
    }
    return input.substr(start_pos, end_pos - start_pos);
  }

  // Store an owned string and return a view to it (for escaped strings)
  std::string_view store_owned_string(std::string str) { return owned_strings.insert(std::move(str)); }
};

// Compare two indentations to determine their relationship
IndentRelation compare_indentation(std::string_view current, std::string_view base) {
  // Empty base means we're at the root level
  if (base.empty()) {
    return current.empty() ? IndentRelation::Equal : IndentRelation::Greater;
  }

  // If current is shorter than base, it must be less indented
  if (current.size() < base.size()) {
    return IndentRelation::Less;
  }

  // If current is longer, check if it starts with base
  if (current.size() > base.size()) {
    if (current.substr(0, base.size()) == base) {
      return IndentRelation::Greater;
    } else {
      return IndentRelation::Incompatible;
    }
  }

  // Same length - check if they're identical
  return (current == base) ? IndentRelation::Equal : IndentRelation::Incompatible;
}

// Check if current indentation is compatible with (equal to or greater than) base
bool is_indentation_compatible(std::string_view current, std::string_view base) {
  auto relation = compare_indentation(current, base);
  return relation == IndentRelation::Equal || relation == IndentRelation::Greater;
}

// Check if current indentation is strictly greater than base
bool is_indentation_greater(std::string_view current, std::string_view base) {
  return compare_indentation(current, base) == IndentRelation::Greater;
}

// Forward declarations
Node parse_document(ParseState &state);
Node parse_value(ParseState &state);
Node parse_mapping(ParseState &state);
Node parse_sequence(ParseState &state);
Node parse_json_mapping(ParseState &state);
Node parse_json_sequence(ParseState &state);
Node parse_json_value(ParseState &state);
std::string_view parse_scalar(ParseState &state);

void advance_to_next_content(ParseState &state) {
  while (!state.at_end()) {
    // Skip whitespace at start of line
    state.skip_whitespace_inline();

    if (state.current() == '#') {
      // Skip comment line
      state.skip_to_end_of_line();
      if (state.current() == '\n') {
        state.advance(); // This will update line_begin_pos
      }
    } else if (state.current() == '\n' || state.current() == '\r') {
      // Skip empty line
      state.advance(); // This will update line_begin_pos
    } else {
      // Found content - pos is now at first non-whitespace char
      // line_begin_pos to pos represents the indentation
      break;
    }
  }
}

bool is_yaml_special_char(char c) {
  return c == ':' || c == '-' || c == '?' || c == '|' || c == '>' || c == '[' || c == ']' || c == '{' || c == '}' ||
         c == '#' || c == '&' || c == '*' || c == '!' || c == '\'' || c == '"';
}

std::string_view parse_quoted_string(ParseState &state) {
  char quote_char = state.current();
  state.advance(); // Skip opening quote

  std::string result;

  while (!state.at_end() && state.current() != quote_char) {
    if (state.current() == '\\') {
      state.advance();
      if (state.at_end())
        break;

      char escaped = state.current();
      switch (escaped) {
      case 'n':
        result += '\n';
        break;
      case 't':
        result += '\t';
        break;
      case 'r':
        result += '\r';
        break;
      case '\\':
        result += '\\';
        break;
      case '"':
        result += '"';
        break;
      case '\'':
        result += '\'';
        break;
      default:
        result += escaped;
        break;
      }
    } else {
      result += state.current();
    }
    state.advance();
  }

  if (state.current() == quote_char) {
    state.advance(); // Skip closing quote
  }

  // If no escaping was needed, we could potentially return a view to the original
  // but for simplicity, we'll always store quoted strings as owned strings
  return state.store_owned_string(std::move(result));
}

std::string_view parse_unquoted_scalar(ParseState &state) {
  size_t start_pos = state.pos;

  while (!state.at_end()) {
    char c = state.current();

    // Stop at newline, comment, or special YAML characters that end a scalar
    if (c == '\n' || c == '\r' || c == '#') {
      break;
    }

    // Stop at colon followed by space (indicates mapping) - but only if we're not at start
    if (state.pos > start_pos && c == ':' && (state.peek() == ' ' || state.peek() == '\n' || state.peek() == '\0')) {
      break;
    }

    state.advance();
  }

  size_t end_pos = state.pos;

  // Trim trailing whitespace
  while (end_pos > start_pos && (state.input[end_pos - 1] == ' ' || state.input[end_pos - 1] == '\t')) {
    end_pos--;
  }

  return state.make_view(start_pos, end_pos);
}

Node parse_scalar_value(std::string_view scalar_text) {
  if (scalar_text.empty() || scalar_text == "null" || scalar_text == "~") {
    return Node(std::monostate{});
  }

  if (scalar_text == "true" || scalar_text == "yes" || scalar_text == "on") {
    return Node(true);
  }

  if (scalar_text == "false" || scalar_text == "no" || scalar_text == "off") {
    return Node(false);
  }

  // Try to parse as number (only if it looks numeric)
  if (!scalar_text.empty() && (std::isdigit(scalar_text[0]) ||
                               (scalar_text[0] == '-' && scalar_text.size() > 1 && std::isdigit(scalar_text[1])))) {
    bool is_number = true;
    bool has_dot = false;

    for (size_t i = 0; i < scalar_text.size(); ++i) {
      char c = scalar_text[i];
      if (i == 0 && c == '-') {
        continue; // Allow leading minus
      } else if (c == '.' && !has_dot) {
        has_dot = true;
      } else if (!std::isdigit(c)) {
        is_number = false;
        break;
      }
    }

    if (is_number && scalar_text != "-") { // Don't parse standalone "-" as number
      try {
        if (has_dot) {
          return Node(std::stod(std::string(scalar_text)));
        } else {
          return Node(static_cast<int64_t>(std::stoll(std::string(scalar_text))));
        }
      } catch (...) {
        // Fall through to string
      }
    }
  }

  // Default to string_view for any unrecognized scalar
  return Node(scalar_text);
}

std::string_view parse_scalar(ParseState &state) {
  if (state.current() == '"' || state.current() == '\'') {
    return parse_quoted_string(state);
  } else {
    return parse_unquoted_scalar(state);
  }
}

Node parse_sequence(ParseState &state) {
  Node node;
  node.value = Sequence{};
  auto &seq = std::get<Sequence>(node.value);

  while (!state.at_end()) {
    advance_to_next_content(state);
    if (state.at_end())
      break;

    // Check for list item marker
    if (state.current() == '-' && (state.peek() == ' ' || state.peek() == '\n' || state.peek() == '\0')) {
      // This is a list item, parse it as part of this sequence

      state.advance(); // Skip '-'
      state.skip_whitespace_inline();

      if (state.current() == '\n' || state.at_end()) {
        // Empty list item
        seq.push_back(Node(std::monostate{}));
        if (state.current() == '\n')
          state.advance();
      } else {
        // Parse the list item value - let parse_value detect natural indentation
        Node item = parse_value(state);
        seq.push_back(item);
      }
    } else {
      // Not a list item, we're done with this sequence
      break;
    }
  }

  return node;
}

Node parse_mapping(ParseState &state) {
  Node node;
  node.value = Map{};
  auto &map = std::get<Map>(node.value);

  // Track the minimum indentation required for keys in this mapping
  std::string_view min_key_indent;

  while (!state.at_end()) {
    advance_to_next_content(state);
    if (state.at_end())
      break;

    std::string_view current_indent = state.current_line_indentation();

    // Check if this is actually a sequence item (starts with -)
    if (state.current() == '-' && (state.peek() == ' ' || state.peek() == '\n' || state.peek() == '\0')) {
      // This is a sequence, not a mapping, so we're done with this mapping
      break;
    }

    // Set minimum key indentation from first key
    if (min_key_indent.empty()) {
      min_key_indent = current_indent;
    } else {
      // Check if this key meets the minimum indentation requirement
      auto relation = compare_indentation(current_indent, min_key_indent);
      if (relation == IndentRelation::Less || relation == IndentRelation::Incompatible) {
        // Indentation is less than minimum, this key belongs to a parent level
        break;
      }
      // Keys can have equal or greater indentation than the minimum and still be siblings
    }

    // Parse key
    std::string_view key = parse_scalar(state);
    if (key.empty())
      break;

    state.skip_whitespace_inline();

    // Expect colon
    if (state.current() != ':') {
      throw ParseError("Expected ':' after key '" + std::string(key) + "' at line " + std::to_string(state.line));
    }

    state.advance(); // Skip ':'
    state.skip_whitespace_inline();

    // Parse value
    Node value;
    if (state.current() == '\n' || state.at_end()) {
      // Value is on next line(s) or null
      if (state.current() == '\n')
        state.advance();
      advance_to_next_content(state);

      std::string_view next_indent = state.current_line_indentation();
      auto next_relation = compare_indentation(next_indent, current_indent);
      if (state.at_end() || next_relation != IndentRelation::Greater) {
        // No value or value at same/lower indentation = null
        value = Node(std::monostate{});
      } else {
        // Value on next line with higher indentation - let parse_value detect natural indentation
        value = parse_value(state);
      }
    } else {
      // Value on same line
      if (state.current() == '"' || state.current() == '\'') {
        // Quoted strings should be treated as strings directly (even if empty)
        std::string_view str = parse_quoted_string(state);
        value = Node(std::string_view(str));
      } else {
        value = parse_value(state);
      }

      // Skip to end of line
      state.skip_whitespace_inline();
      if (state.current() == '\n')
        state.advance();
    }

    map.emplace(key, value);
  }

  return node;
}

Node parse_value(ParseState &state) {
  advance_to_next_content(state);
  if (state.at_end()) {
    return Node(std::monostate{});
  }

  // Check for sequence (list starting with -)
  if (state.current() == '-' && (state.peek() == ' ' || state.peek() == '\n' || state.peek() == '\0')) {
    // Parse as sequence
    return parse_sequence(state);
  }

  state.skip_whitespace_inline();

  // Check for inline JSON-style structures
  if (state.current() == '{') {
    return parse_json_mapping(state);
  }

  if (state.current() == '[') {
    return parse_json_sequence(state);
  }

  // Check if this looks like a mapping (contains key: value pairs)
  // Save current position to restore if it's not a mapping
  size_t saved_pos = state.pos;
  size_t saved_line = state.line;
  size_t saved_column = state.column;

  bool looks_like_mapping = false;
  bool in_quotes = false;
  char quote_char = '\0';

  // Look for ':' followed by space/newline to determine if it's a mapping
  // But respect quote boundaries - don't look for colons inside quoted strings
  while (!state.at_end()) {
    char c = state.current();

    if (c == '\n' || c == '\r' || c == '#') {
      break;
    }

    // Handle quotes
    if (!in_quotes && (c == '"' || c == '\'')) {
      in_quotes = true;
      quote_char = c;
    } else if (in_quotes && c == quote_char) {
      in_quotes = false;
      quote_char = '\0';
    } else if (!in_quotes && c == ':') {
      char next = state.peek();
      if (next == ' ' || next == '\n' || next == '\r' || next == '\0') {
        looks_like_mapping = true;
        break;
      }
    }
    state.advance();
  }

  // Restore position
  state.pos = saved_pos;
  state.line = saved_line;
  state.column = saved_column;

  if (looks_like_mapping) {
    return parse_mapping(state);
  }

  // Otherwise, parse as scalar
  if (state.current() == '"' || state.current() == '\'') {
    // Quoted strings should be treated as strings directly (even if empty)
    std::string_view str = parse_quoted_string(state);
    return Node(std::string_view(str));
  } else {
    // Unquoted scalars go through scalar value parsing
    std::string_view scalar_text = parse_unquoted_scalar(state);
    return parse_scalar_value(scalar_text);
  }
}

// JSON value parsing that stops at commas and closing brackets
Node parse_json_value(ParseState &state) {
  state.skip_whitespace_and_newlines();

  // Handle JSON arrays
  if (state.current() == '[') {
    return parse_json_sequence(state);
  }

  // Handle JSON objects
  if (state.current() == '{') {
    return parse_json_mapping(state);
  }

  // Handle quoted strings
  if (state.current() == '"' || state.current() == '\'') {
    std::string_view str = parse_quoted_string(state);
    return Node(std::string_view(str));
  }

  // Handle other values (numbers, booleans, null)
  std::string value_str;
  while (!state.at_end()) {
    char c = state.current();
    // Stop at JSON delimiters
    if (c == ',' || c == '}' || c == ']' || c == '\n' || c == '\r' || c == ' ' || c == '\t') {
      break;
    }
    value_str += c;
    state.advance();
  }

  return parse_scalar_value(std::string_view(value_str));
}

// JSON-style parsing for backward compatibility
Node parse_json_mapping(ParseState &state) {
  state.advance(); // Skip '{'

  Node node;
  node.value = Map{};
  auto &map = std::get<Map>(node.value);

  while (!state.at_end() && state.current() != '}') {
    state.skip_whitespace_and_newlines();
    if (state.current() == '}')
      break;

    // Parse key
    std::string key;
    if (state.current() == '"' || state.current() == '\'') {
      key = std::string(parse_quoted_string(state));
    } else {
      // For JSON, unquoted keys should be parsed more carefully
      while (!state.at_end() && state.current() != ':' && state.current() != ' ' && state.current() != '\t' &&
             state.current() != '\n') {
        key += state.current();
        state.advance();
      }
    }

    state.skip_whitespace_and_newlines();

    // Expect colon
    if (state.current() != ':') {
      throw ParseError("Expected ':' in JSON mapping");
    }
    state.advance();

    state.skip_whitespace_and_newlines();

    // Parse value - need to parse JSON values specially to avoid comma issues
    Node value = parse_json_value(state);
    map.emplace(state.store_owned_string(std::move(key)), value);

    state.skip_whitespace_and_newlines();

    // Skip comma if present
    if (state.current() == ',') {
      state.advance();
    }
  }

  if (state.current() == '}') {
    state.advance();
  }

  return node;
}

Node parse_json_sequence(ParseState &state) {
  state.advance(); // Skip '['

  Node node;
  node.value = Sequence{};
  auto &seq = std::get<Sequence>(node.value);

  while (!state.at_end() && state.current() != ']') {
    state.skip_whitespace_and_newlines();
    if (state.current() == ']')
      break;

    Node item = parse_json_value(state);
    seq.push_back(item);

    state.skip_whitespace_and_newlines();

    // Skip comma if present
    if (state.current() == ',') {
      state.advance();
    }
  }

  if (state.current() == ']') {
    state.advance();
  }

  return node;
}

Node parse_document(ParseState &state) {
  advance_to_next_content(state);
  if (state.at_end()) {
    return Node(std::monostate{});
  }

  return parse_value(state);
}

// YamlDocument implementation
YamlDocument::YamlDocument(std::string source) : source_(std::move(source)) {
  ParseState state{source_};
  // Note: unordered_set automatically deduplicates strings and maintains stable references

  try {
    root_ = parse_document(state);
    // Transfer ownership of escaped strings from ParseState to this document
    // This ensures all string_views in nodes remain valid for the document's lifetime
    owned_strings_ = std::move(state.owned_strings);
  } catch (const std::exception &e) {
    throw ParseError("YAML parse error at line " + std::to_string(state.line) + ", column " +
                     std::to_string(state.column) + ": " + e.what());
  }
}

YamlDocument YamlDocument::Parse(std::string source) { return YamlDocument(std::move(source)); }

void format_yaml(std::ostream &os, const Node &node, int indent) {
  std::visit(
      [&os, indent](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
          os << "null";
        } else if constexpr (std::is_same_v<T, bool>) {
          os << (arg ? "true" : "false");
        } else if constexpr (std::is_same_v<T, int64_t>) {
          os << arg;
        } else if constexpr (std::is_same_v<T, double>) {
          os << arg;
        } else if constexpr (std::is_same_v<T, std::string_view>) {
          // Quote string if it contains special characters or looks like other types
          bool needs_quotes = arg.empty() || arg == "true" || arg == "false" || arg == "null" ||
                              arg.find(':') != std::string_view::npos || arg.find('#') != std::string_view::npos ||
                              arg.find('\n') != std::string_view::npos || arg.find('"') != std::string_view::npos ||
                              (std::isdigit(arg[0]) || arg[0] == '-');

          if (needs_quotes) {
            os << "\"";
            for (char c : std::string(arg)) { // Convert string_view to string for iteration
              if (c == '"')
                os << "\\\"";
              else if (c == '\\')
                os << "\\\\";
              else if (c == '\n')
                os << "\\n";
              else if (c == '\t')
                os << "\\t";
              else if (c == '\r')
                os << "\\r";
              else
                os << c;
            }
            os << "\"";
          } else {
            os << std::string(arg); // Convert string_view to string for output
          }
        } else if constexpr (std::is_same_v<T, Sequence>) {
          if (indent == 0) {
            // Root level sequence
            for (size_t i = 0; i < arg.size(); ++i) {
              if (i > 0)
                os << "\n";
              os << "- ";
              format_yaml(os, arg[i], 2);
            }
          } else {
            // Nested sequence - use standard YAML list format
            for (size_t i = 0; i < arg.size(); ++i) {
              if (i > 0)
                os << "\n" << std::string(indent, ' ');
              os << "- ";
              format_yaml(os, arg[i], indent + 2);
            }
          }
        } else if constexpr (std::is_same_v<T, Map>) {
          bool first = true;
          for (const auto &entry : arg) {
            if (!first) {
              os << "\n" << std::string(indent, ' ');
            }
            first = false;

            os << std::string(entry.key) << ": ";

            // Check if value should be on next line
            if (std::holds_alternative<Map>(entry.value.value) || std::holds_alternative<Sequence>(entry.value.value)) {
              os << "\n" << std::string(indent + 2, ' ');
              format_yaml(os, entry.value, indent + 2);
            } else {
              format_yaml(os, entry.value, 0);
            }
          }
        }
      },
      node.value);
}

std::ostream &operator<<(std::ostream &os, const Node &node) {
  format_yaml(os, node, 0);
  return os;
}

std::string format_yaml(const Node &node) {
  std::ostringstream oss;
  format_yaml(oss, node, 0);
  return oss.str();
}

} // namespace yaml
} // namespace shilos
