
#include "shilos.hh"
#include "shilos/iops.hh"

#include <cctype>
#include <cmath>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace shilos {
namespace yaml {

// Constants for magic numbers
constexpr size_t MAX_LOOKAHEAD = 200; // Maximum lookahead for efficient mapping detection

// Indentation comparison result
enum class IndentRelation { Less, Equal, Greater, Incompatible };

// Parser state that tracks the source string for creating string_views
struct ParseState {
  std::string_view input;
  size_t line_begin_pos = 0;
  size_t pos = 0;
  size_t line = 1;
  size_t column = 1;
  iops<std::string> owned_strings; // Storage for strings that need escaping (deduplicated)

  // Track indentation for basic compatibility checking
  std::string_view last_indent; // Last indentation seen for basic validation

  // Anchor/alias tracking
  std::unordered_map<std::string_view, Node> anchors; // Maps anchor names to their nodes

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

  // Validate indentation compatibility - only throws on truly incompatible indentation
  void validate_indentation(std::string_view current_indent) {
    if (current_indent.empty())
      return;

    // Simple compatibility check with the last indentation seen
    // We allow mixed tabs/spaces as long as they form valid prefix relationships
    if (!last_indent.empty() && current_indent != last_indent) {
      // Check if either is a prefix of the other
      bool is_compatible = false;

      if (current_indent.size() >= last_indent.size()) {
        // Check if last is a prefix of current
        is_compatible = current_indent.substr(0, last_indent.size()) == last_indent;
      } else {
        // Check if current is a prefix of last
        is_compatible = last_indent.substr(0, current_indent.size()) == current_indent;
      }

      if (!is_compatible) {
        throw ParseError("Incompatible indentation at line " + std::to_string(line) +
                         " - indentation cannot be consistently compared with previous levels");
      }
    }

    last_indent = current_indent;
  }
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
bool looks_like_mapping_efficient(const ParseState &state);
Node parse_multiline_scalar(ParseState &state);
Node parse_alias(ParseState &state);
Node parse_anchored_value(ParseState &state);
Node parse_tagged_value(ParseState &state);

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
  size_t start_line = state.line; // Store the starting line for accurate error reporting
  state.advance();                // Skip opening quote

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
      case 'u': {
        // Unicode escape sequence - simplified handling for common cases
        // TODO: Full Unicode escape support for production use
        result += escaped; // For now, just include the character
        break;
      }
      default:
        // Invalid escape sequence
        throw ParseError("Invalid escape sequence '\\" + std::string(1, escaped) + "' at line " +
                         std::to_string(state.line) + ", column " + std::to_string(state.column));
        break;
      }
    } else {
      result += state.current();
    }
    state.advance();
  }

  if (state.current() == quote_char) {
    state.advance(); // Skip closing quote
  } else {
    // Reached end of input without finding closing quote
    throw ParseError("Unclosed quoted string starting at line " + std::to_string(start_line) + " - missing closing " +
                     std::string(1, quote_char) + " quote");
  }

  // If no escaping was needed, we could potentially return a view to the original
  // but for simplicity, we'll always store quoted strings as owned strings
  return state.store_owned_string(std::move(result));
}

// Check if a string looks like a URL scheme (http, https, ftp, etc.)
bool is_likely_url_scheme(std::string_view text) {
  if (text.empty())
    return false;

  // Common URL schemes
  return text == "http" || text == "https" || text == "ftp" || text == "ftps" || text == "file" || text == "mailto" ||
         text == "tel" || text == "ssh" || text == "git" || text == "ws" || text == "wss";
}

// Check if the current position looks like it's part of a time format (12:30, 14:45:30)
bool is_likely_time_format(const ParseState &state, size_t start_pos) {
  std::string_view text = state.make_view(start_pos, state.pos);

  // Simple heuristic: if it's 1-2 digits, it might be hours in a time format
  if (text.size() >= 1 && text.size() <= 2) {
    bool all_digits = true;
    for (char c : text) {
      if (!std::isdigit(c)) {
        all_digits = false;
        break;
      }
    }

    if (all_digits) {
      int value = std::stoi(std::string(text));
      // Hours are typically 0-23, minutes/seconds are 0-59
      return value >= 0 && value <= 59;
    }
  }

  return false;
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
    // and it's not part of a URL scheme or time format
    if (state.pos > start_pos && c == ':' && (state.peek() == ' ' || state.peek() == '\n' || state.peek() == '\0')) {
      // Check if this might be a URL scheme (like http:, https:, ftp:)
      std::string_view potential_key = state.make_view(start_pos, state.pos);
      if (is_likely_url_scheme(potential_key) || is_likely_time_format(state, start_pos)) {
        // Continue parsing - this colon is part of the scalar value
      } else {
        // This looks like a mapping key - stop here
        break;
      }
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

// Validate if a string represents a valid number according to YAML/JSON standards
bool is_valid_number(std::string_view text) {
  if (text.empty())
    return false;

  size_t i = 0;

  // Handle optional sign
  if (text[i] == '+' || text[i] == '-') {
    i++;
    if (i >= text.size())
      return false; // Just a sign is not a number
  }

  // Must have at least one digit
  if (i >= text.size() || !std::isdigit(text[i])) {
    return false;
  }

  // Parse integer part
  if (text[i] == '0') {
    // Leading zero only allowed for 0, 0.xxx, or 0e/E
    i++;
    if (i < text.size() && std::isdigit(text[i])) {
      return false; // Invalid: 01, 02, etc.
    }
  } else {
    // Non-zero digit followed by more digits
    while (i < text.size() && std::isdigit(text[i])) {
      i++;
    }
  }

  // Optional decimal part
  if (i < text.size() && text[i] == '.') {
    i++;
    if (i >= text.size() || !std::isdigit(text[i])) {
      return false; // Dot must be followed by digits
    }
    while (i < text.size() && std::isdigit(text[i])) {
      i++;
    }
  }

  // Optional exponent part
  if (i < text.size() && (text[i] == 'e' || text[i] == 'E')) {
    i++;
    if (i >= text.size())
      return false; // 'e' must be followed by something

    // Optional sign in exponent
    if (text[i] == '+' || text[i] == '-') {
      i++;
    }

    if (i >= text.size() || !std::isdigit(text[i])) {
      return false; // Exponent must have digits
    }

    while (i < text.size() && std::isdigit(text[i])) {
      i++;
    }
  }

  // Must have consumed entire string
  return i == text.size();
}

// Check if number contains decimal point or scientific notation
bool contains_decimal_or_scientific(std::string_view text) {
  return text.find('.') != std::string_view::npos || text.find('e') != std::string_view::npos ||
         text.find('E') != std::string_view::npos;
}

Node parse_scalar_value(std::string_view scalar_text) {
  // YAML 1.2 spec-compliant boolean/null detection with case-insensitive matching
  if (scalar_text.empty()) {
    return Node(std::monostate{});
  }

  // Convert to lowercase for case-insensitive comparison
  std::string lower_text;
  lower_text.reserve(scalar_text.size());
  for (char c : scalar_text) {
    lower_text += std::tolower(c);
  }

  // YAML 1.2 null values (case-insensitive)
  if (lower_text == "null" || lower_text == "~" || lower_text == "null") {
    return Node(std::monostate{});
  }

  // YAML 1.2 boolean values (case-insensitive)
  if (lower_text == "true" || lower_text == "yes" || lower_text == "on" || lower_text == "y") {
    return Node(true);
  }

  if (lower_text == "false" || lower_text == "no" || lower_text == "off" || lower_text == "n") {
    return Node(false);
  }

  // Try to parse as number with comprehensive validation
  if (!scalar_text.empty() && is_valid_number(scalar_text)) {
    try {
      if (contains_decimal_or_scientific(scalar_text)) {
        double value = std::stod(std::string(scalar_text));
        // Check for overflow/underflow
        if (std::isfinite(value)) {
          return Node(value);
        }
      } else {
        // Integer parsing with overflow checking
        try {
          int64_t value = std::stoll(std::string(scalar_text));
          return Node(value);
        } catch (const std::out_of_range &) {
          // Try as double if integer overflows
          double value = std::stod(std::string(scalar_text));
          if (std::isfinite(value)) {
            return Node(value);
          }
        }
      }
    } catch (const std::invalid_argument &) {
      // Not a valid number, fall through to string
    } catch (const std::out_of_range &) {
      // Number too large, fall through to string
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

    // Validate indentation consistency
    state.validate_indentation(current_indent);

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
    if (key.empty()) {
      throw ParseError("Empty or missing key in YAML mapping at line " + std::to_string(state.line) + ", column " +
                       std::to_string(state.column));
    }

    state.skip_whitespace_inline();

    // Expect colon
    if (state.current() != ':') {
      throw ParseError("Expected ':' after key '" + std::string(key) + "' at line " + std::to_string(state.line) +
                       ", column " + std::to_string(state.column));
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

      // Validate indentation consistency for the value
      state.validate_indentation(next_indent);

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

// Efficient mapping detection using limited lookahead to avoid O(n²) performance
// Only looks ahead a reasonable distance to find mapping indicators
bool looks_like_mapping_efficient(const ParseState &state) {
  size_t pos = state.pos;
  bool in_quotes = false;
  char quote_char = '\0';
  size_t chars_scanned = 0;

  // Scan ahead looking for key: value pattern within reasonable limits
  while (pos < state.input.size() && chars_scanned < MAX_LOOKAHEAD) {
    char c = state.input[pos];

    // Stop at line boundaries or comments - these end potential mappings
    if (c == '\n' || c == '\r' || c == '#') {
      break;
    }

    // Handle quoted strings - don't look for colons inside quotes
    if (!in_quotes && (c == '"' || c == '\'')) {
      in_quotes = true;
      quote_char = c;
    } else if (in_quotes && c == quote_char) {
      // Check for escape sequences
      if (pos > 0 && state.input[pos - 1] != '\\') {
        in_quotes = false;
        quote_char = '\0';
      }
    } else if (!in_quotes && c == ':') {
      // Found colon outside quotes - check if it's followed by whitespace
      char next = (pos + 1 < state.input.size()) ? state.input[pos + 1] : '\0';
      if (next == ' ' || next == '\t' || next == '\n' || next == '\r' || next == '\0') {
        return true; // This looks like a mapping
      }
    }

    pos++;
    chars_scanned++;
  }

  return false; // No mapping pattern found within reasonable distance
}

// Parse YAML multi-line scalars (literal | and folded >)
Node parse_multiline_scalar(ParseState &state) {
  char indicator = state.current();
  state.advance(); // Skip '|' or '>'

  // Parse optional style indicators (+, -, or digit for explicit indentation)
  bool strip_final_newlines = false;
  bool keep_final_newlines = false;

  while (!state.at_end() && state.current() != '\n') {
    if (state.current() == '+') {
      keep_final_newlines = true;
      state.advance();
    } else if (state.current() == '-') {
      strip_final_newlines = true;
      state.advance();
    } else if (std::isdigit(state.current())) {
      // Explicit indentation indicator - just skip for now
      state.advance();
    } else if (state.current() == ' ' || state.current() == '\t') {
      state.advance();
    } else {
      break;
    }
  }

  // Skip to end of indicator line
  state.skip_to_end_of_line();
  if (state.current() == '\n') {
    state.advance();
  }

  // Get the base indentation (indentation of the first content line)
  advance_to_next_content(state);
  if (state.at_end()) {
    return Node(std::string_view(""));
  }

  std::string_view base_indent = state.current_line_indentation();
  std::string result;

  // Parse multi-line content
  while (!state.at_end()) {
    std::string_view line_indent = state.current_line_indentation();

    // Check if this line belongs to the multi-line scalar
    if (!line_indent.empty() && !is_indentation_greater(line_indent, base_indent)) {
      // This line has less indentation, so it's not part of the scalar
      break;
    }

    // Extract content line (removing base indentation)
    std::string line_content;
    if (line_indent.size() >= base_indent.size()) {
      // Skip the base indentation
      state.pos += base_indent.size();
    }

    // Read the rest of the line
    size_t line_start = state.pos;
    while (!state.at_end() && state.current() != '\n') {
      state.advance();
    }

    std::string_view line_text = state.make_view(line_start, state.pos);

    if (indicator == '|') {
      // Literal scalar - preserve newlines and spaces
      if (!result.empty()) {
        result += '\n';
      }
      result += std::string(line_text);
    } else {
      // Folded scalar - fold newlines into spaces, preserve blank lines
      if (line_text.empty()) {
        // Empty line - preserve as paragraph break
        if (!result.empty()) {
          result += '\n';
        }
      } else {
        // Non-empty line - fold with previous
        if (!result.empty() && result.back() != '\n') {
          result += ' ';
        }
        result += std::string(line_text);
      }
    }

    // Move to next line
    if (state.current() == '\n') {
      state.advance();
    }

    advance_to_next_content(state);
  }

  // Apply final newline handling
  if (strip_final_newlines) {
    while (!result.empty() && result.back() == '\n') {
      result.pop_back();
    }
  } else if (!keep_final_newlines && !result.empty() && result.back() == '\n') {
    // Default behavior: single final newline
    while (result.size() > 1 && result[result.size() - 2] == '\n') {
      result.pop_back();
    }
  }

  return Node(state.store_owned_string(std::move(result)));
}

// Parse YAML alias (*alias)
Node parse_alias(ParseState &state) {
  state.advance(); // Skip '*'

  // Parse alias name
  std::string alias_name;
  while (!state.at_end() && !std::isspace(state.current()) && state.current() != ',' && state.current() != '}' &&
         state.current() != ']' && state.current() != ':' && state.current() != '#') {
    alias_name += state.current();
    state.advance();
  }

  if (alias_name.empty()) {
    throw ParseError("Empty alias name at line " + std::to_string(state.line) + ", column " +
                     std::to_string(state.column));
  }

  // Look up the anchor
  std::string_view alias_key = state.store_owned_string(std::move(alias_name));
  auto it = state.anchors.find(alias_key);
  if (it == state.anchors.end()) {
    throw ParseError("Undefined alias '" + std::string(alias_key) + "' at line " + std::to_string(state.line));
  }

  return it->second; // Return copy of the anchored node
}

// Parse YAML anchored value (&anchor value)
Node parse_anchored_value(ParseState &state) {
  state.advance(); // Skip '&'

  // Parse anchor name
  std::string anchor_name;
  while (!state.at_end() && !std::isspace(state.current()) && state.current() != ',' && state.current() != '}' &&
         state.current() != ']' && state.current() != ':' && state.current() != '#') {
    anchor_name += state.current();
    state.advance();
  }

  if (anchor_name.empty()) {
    throw ParseError("Empty anchor name at line " + std::to_string(state.line) + ", column " +
                     std::to_string(state.column));
  }

  // Skip whitespace before value
  state.skip_whitespace_inline();

  // Parse the anchored value
  Node value;
  if (state.current() == '\n' || state.at_end()) {
    // Anchor with null value
    value = Node(std::monostate{});
  } else {
    // Parse the actual value
    value = parse_value(state);
  }

  // Store the anchor
  std::string_view anchor_key = state.store_owned_string(std::move(anchor_name));
  state.anchors[anchor_key] = value;

  return value;
}

// Parse YAML explicit type tag (!!type value)
Node parse_tagged_value(ParseState &state) {
  state.advance(); // Skip first '!'
  state.advance(); // Skip second '!'

  // Parse tag name
  std::string tag_name;
  while (!state.at_end() && !std::isspace(state.current()) && state.current() != ',' && state.current() != '}' &&
         state.current() != ']') {
    tag_name += state.current();
    state.advance();
  }

  if (tag_name.empty()) {
    throw ParseError("Empty tag name at line " + std::to_string(state.line) + ", column " +
                     std::to_string(state.column));
  }

  // Skip whitespace before value
  state.skip_whitespace_inline();

  // Parse the tagged value
  Node value = parse_value(state);

  // Apply type conversion based on tag
  if (tag_name == "str") {
    // Force string type
    if (auto scalar = std::get_if<std::string_view>(&value.value)) {
      return Node(*scalar);
    } else {
      throw ParseError("!!str tag applied to non-scalar value at line " + std::to_string(state.line));
    }
  } else if (tag_name == "int") {
    // Force integer type
    if (auto scalar = std::get_if<std::string_view>(&value.value)) {
      try {
        int64_t int_value = std::stoll(std::string(*scalar));
        return Node(int_value);
      } catch (...) {
        throw ParseError("!!int tag applied to non-integer value '" + std::string(*scalar) + "' at line " +
                         std::to_string(state.line));
      }
    } else {
      throw ParseError("!!int tag applied to non-scalar value at line " + std::to_string(state.line));
    }
  } else if (tag_name == "float") {
    // Force float type
    if (auto scalar = std::get_if<std::string_view>(&value.value)) {
      try {
        double float_value = std::stod(std::string(*scalar));
        return Node(float_value);
      } catch (...) {
        throw ParseError("!!float tag applied to non-float value '" + std::string(*scalar) + "' at line " +
                         std::to_string(state.line));
      }
    } else {
      throw ParseError("!!float tag applied to non-scalar value at line " + std::to_string(state.line));
    }
  } else if (tag_name == "bool") {
    // Force boolean type
    if (auto scalar = std::get_if<std::string_view>(&value.value)) {
      std::string str_value = std::string(*scalar);
      if (str_value == "true" || str_value == "yes" || str_value == "on" || str_value == "1") {
        return Node(true);
      } else if (str_value == "false" || str_value == "no" || str_value == "off" || str_value == "0") {
        return Node(false);
      } else {
        throw ParseError("!!bool tag applied to non-boolean value '" + str_value + "' at line " +
                         std::to_string(state.line));
      }
    } else {
      throw ParseError("!!bool tag applied to non-scalar value at line " + std::to_string(state.line));
    }
  } else if (tag_name == "null") {
    // Force null type
    return Node(std::monostate{});
  } else {
    // Unknown tag - keep original value but warn
    // For now, just return the original value
    return value;
  }
}

Node parse_value(ParseState &state) {
  advance_to_next_content(state);
  if (state.at_end()) {
    return Node(std::monostate{});
  }

  // Check for alias (*alias)
  if (state.current() == '*') {
    return parse_alias(state);
  }

  // Check for explicit type tag (!!type)
  if (state.current() == '!' && state.peek() == '!') {
    return parse_tagged_value(state);
  }

  // Check for sequence (list starting with -)
  if (state.current() == '-' && (state.peek() == ' ' || state.peek() == '\n' || state.peek() == '\0')) {
    // Parse as sequence
    return parse_sequence(state);
  }

  state.skip_whitespace_inline();

  // Check for anchor (&anchor)
  if (state.current() == '&') {
    return parse_anchored_value(state);
  }

  // Check for inline JSON-style structures
  if (state.current() == '{') {
    return parse_json_mapping(state);
  }

  if (state.current() == '[') {
    return parse_json_sequence(state);
  }

  // Check if this looks like a mapping using efficient limited lookahead
  if (looks_like_mapping_efficient(state)) {
    return parse_mapping(state);
  }

  // Check for multi-line scalars (literal | and folded >)
  if (state.current() == '|' || state.current() == '>') {
    return parse_multiline_scalar(state);
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
      throw ParseError("Expected ':' after key '" + key + "' in JSON mapping at line " + std::to_string(state.line) +
                       ", column " + std::to_string(state.column));
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
  } else {
    throw ParseError("Unterminated JSON object - missing closing '}' at line " + std::to_string(state.line) +
                     ", column " + std::to_string(state.column));
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
  } else {
    throw ParseError("Unterminated JSON array - missing closing ']' at line " + std::to_string(state.line) +
                     ", column " + std::to_string(state.column));
  }

  return node;
}

// Check if current position is at a document separator (--- or ...)
bool is_document_separator(ParseState &state) {
  if (state.pos + 2 >= state.input.size())
    return false;

  // Check for "---" or "..."
  if ((state.input[state.pos] == '-' && state.input[state.pos + 1] == '-' && state.input[state.pos + 2] == '-') ||
      (state.input[state.pos] == '.' && state.input[state.pos + 1] == '.' && state.input[state.pos + 2] == '.')) {

    // Must be followed by whitespace, newline, or end of input
    if (state.pos + 3 >= state.input.size())
      return true;
    char next = state.input[state.pos + 3];
    return next == ' ' || next == '\t' || next == '\n' || next == '\r';
  }

  return false;
}

// Skip document separator and any following content on the same line
void skip_document_separator(ParseState &state) {
  if (is_document_separator(state)) {
    state.advance(); // Skip first character
    state.advance(); // Skip second character
    state.advance(); // Skip third character

    // Skip rest of line
    state.skip_to_end_of_line();
    if (state.current() == '\n') {
      state.advance();
    }
  }
}

Node parse_document(ParseState &state) {
  advance_to_next_content(state);

  // Skip document start separator if present
  if (is_document_separator(state)) {
    skip_document_separator(state);
    advance_to_next_content(state);
  }

  if (state.at_end()) {
    return Node(std::monostate{});
  }

  return parse_value(state);
}

// Parse multiple documents from a YAML stream
std::vector<Node> parse_document_stream(ParseState &state) {
  std::vector<Node> documents;

  while (!state.at_end()) {
    advance_to_next_content(state);
    if (state.at_end())
      break;

    // Parse a single document
    Node doc = parse_document(state);
    documents.push_back(std::move(doc));

    // Skip to next document or end
    advance_to_next_content(state);

    // Check for document end separator
    if (is_document_separator(state)) {
      skip_document_separator(state);
      advance_to_next_content(state);
    }
  }

  return documents;
}

// YamlDocument implementation - now supports multiple documents
YamlDocument::YamlDocument(std::string source) : source_(std::move(source)) {
  ParseState state{source_};

  try {
    documents_ = parse_document_stream(state);
    // Transfer ownership of escaped strings from ParseState to this document
    // This ensures all string_views in nodes remain valid for the document's lifetime
    owned_strings_ = std::move(state.owned_strings);
  } catch (const std::exception &e) {
    throw ParseError("YAML parse error at line " + std::to_string(state.line) + ", column " +
                     std::to_string(state.column) + ": " + e.what());
  }
}

YamlDocument YamlDocument::Parse(std::string source) { return YamlDocument(std::move(source)); }

// Forward declarations for formatting functions
void format_scalar(std::ostream &os, const std::string_view &str);
void format_sequence(std::ostream &os, const Sequence &seq, int indent);
void format_mapping(std::ostream &os, const Map &map, int indent);

void format_scalar(std::ostream &os, const std::string_view &str) {
  // Quote string if it contains special characters or looks like other types
  bool needs_quotes = str.empty() || str == "true" || str == "false" || str == "null" ||
                      str.find(':') != std::string_view::npos || str.find('#') != std::string_view::npos ||
                      str.find('\n') != std::string_view::npos || str.find('"') != std::string_view::npos;

  // Quote strings that look numeric but aren't valid numbers
  if (!needs_quotes && !str.empty() && (std::isdigit(str[0]) || str[0] == '-' || str[0] == '+')) {
    needs_quotes = !is_valid_number(str);
  }

  if (needs_quotes) {
    os << "\"";
    for (char c : str) {
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
    os << str;
  }
}

void format_sequence(std::ostream &os, const Sequence &seq, int indent) {
  if (indent == 0) {
    // Root level sequence
    for (size_t i = 0; i < seq.size(); ++i) {
      if (i > 0)
        os << "\n";
      os << "- ";
      format_yaml(os, seq[i], 2);
    }
  } else {
    // Nested sequence - use standard YAML list format
    for (size_t i = 0; i < seq.size(); ++i) {
      if (i > 0)
        os << "\n" << std::string(indent, ' ');
      os << "- ";
      format_yaml(os, seq[i], indent + 2);
    }
  }
}

void format_mapping(std::ostream &os, const Map &map, int indent) {
  bool first = true;
  for (const auto &entry : map) {
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
          format_scalar(os, arg);
        } else if constexpr (std::is_same_v<T, Sequence>) {
          format_sequence(os, arg, indent);
        } else if constexpr (std::is_same_v<T, Map>) {
          format_mapping(os, arg, indent);
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
