
#include <algorithm>
#include <concepts>
#include <cstring>
#include <iomanip>
#include <map>
#include <memory>
#include <new>
#include <optional>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#include "shilos.hh"
#include "shilos/yaml.hh"

namespace shilos {
namespace yaml {

std::optional<Node> parse_value(ParseState &state) {
  // Skip whitespace
  while (state.pos < state.input.size() && (state.input[state.pos] == ' ' || state.input[state.pos] == '\t' ||
                                            state.input[state.pos] == '\n' || state.input[state.pos] == '\r')) {
    if (state.input[state.pos] == '\n') {
      state.line++;
      state.column = 1;
    } else {
      state.column++;
    }
    state.pos++;
  }

  if (state.pos >= state.input.size())
    return std::nullopt;

  // Check for start of mapping
  if (state.input[state.pos] == '{') {
    return Node::Load(state.input.substr(state.pos));
  }
  // Check for start of sequence
  else if (state.input[state.pos] == '[') {
    return Node::Load(state.input.substr(state.pos));
  }
  // Parse string value
  else if (state.input[state.pos] == '"') {
    state.pos++;
    state.column++;
    std::string str;

    while (state.pos < state.input.size() && state.input[state.pos] != '"') {
      if (state.input[state.pos] == '\\') {
        state.pos++;
        state.column++;
        if (state.pos >= state.input.size())
          break;
      }
      str += state.input[state.pos];
      state.pos++;
      state.column++;
    }

    if (state.pos < state.input.size() && state.input[state.pos] == '"') {
      state.pos++;
      state.column++;
    }

    return Node(str);
  }
  // Parse number
  else if (isdigit(state.input[state.pos]) || state.input[state.pos] == '-') {
    std::string num_str;
    bool is_float = false;

    while (state.pos < state.input.size() &&
           (isdigit(state.input[state.pos]) || state.input[state.pos] == '.' || state.input[state.pos] == '-')) {
      if (state.input[state.pos] == '.')
        is_float = true;
      num_str += state.input[state.pos];
      state.pos++;
      state.column++;
    }

    if (is_float) {
      try {
        return Node(std::stod(num_str));
      } catch (...) {
        return std::nullopt;
      }
    } else {
      try {
        return Node(static_cast<int64_t>(std::stoll(num_str)));
      } catch (...) {
        return std::nullopt;
      }
    }
  }
  // Parse boolean/null
  else {
    std::string ident;
    while (state.pos < state.input.size() && isalpha(state.input[state.pos])) {
      ident += state.input[state.pos];
      state.pos++;
      state.column++;
    }

    if (ident == "true")
      return Node(true);
    if (ident == "false")
      return Node(false);
    if (ident == "null")
      return Node(std::monostate{});

    return std::nullopt;
  }
}

Node Node::Load(const std::string_view yaml_str) {
  ParseState state{yaml_str};
  while (state.pos < state.input.size()) {
    // Skip whitespace
    while (state.pos < state.input.size() && (state.input[state.pos] == ' ' || state.input[state.pos] == '\t' ||
                                              state.input[state.pos] == '\n' || state.input[state.pos] == '\r')) {
      if (state.input[state.pos] == '\n') {
        state.line++;
        state.column = 1;
      } else {
        state.column++;
      }
      state.pos++;
    }

    if (state.pos >= state.input.size())
      break;

    // Check for start of mapping
    if (state.input[state.pos] == '{') {
      state.pos++;
      state.column++;
      Node node;
      node.value = Map{};
      auto &map = std::get<Map>(node.value);

      while (state.pos < state.input.size() && state.input[state.pos] != '}') {
        // Parse key
        auto key_node = parse_value(state);
        if (!key_node || !std::holds_alternative<std::string>(key_node->value)) {
          throw std::runtime_error("Expected string key in mapping");
        }
        std::string key = std::get<std::string>(key_node->value);

        // Skip colon
        while (state.pos < state.input.size() && (state.input[state.pos] == ':' || state.input[state.pos] == ' ')) {
          state.pos++;
          state.column++;
        }

        // Parse value
        auto value_node = parse_value(state);
        if (!value_node) {
          throw std::runtime_error("Expected value in mapping");
        }

        map.emplace(key, *value_node);

        // Skip comma
        while (state.pos < state.input.size() && (state.input[state.pos] == ',' || state.input[state.pos] == ' ')) {
          state.pos++;
          state.column++;
        }
      }

      if (state.pos < state.input.size() && state.input[state.pos] == '}') {
        state.pos++;
        state.column++;
      }

      return node;
    }
    // Check for start of sequence
    else if (state.input[state.pos] == '[') {
      state.pos++;
      state.column++;
      Node node;
      node.value = Sequence{};
      auto &seq = std::get<Sequence>(node.value);

      while (state.pos < state.input.size() && state.input[state.pos] != ']') {
        auto item_node = parse_value(state);
        if (item_node) {
          seq.push_back(*item_node);
        }

        // Skip comma
        while (state.pos < state.input.size() && (state.input[state.pos] == ',' || state.input[state.pos] == ' ')) {
          state.pos++;
          state.column++;
        }
      }

      if (state.pos < state.input.size() && state.input[state.pos] == ']') {
        state.pos++;
        state.column++;
      }

      return node;
    }
    // Parse string value
    else if (state.input[state.pos] == '"') {
      state.pos++;
      state.column++;
      std::string str;

      while (state.pos < state.input.size() && state.input[state.pos] != '"') {
        if (state.input[state.pos] == '\\') {
          state.pos++;
          state.column++;
          if (state.pos >= state.input.size())
            break;
        }
        str += state.input[state.pos];
        state.pos++;
        state.column++;
      }

      if (state.pos < state.input.size() && state.input[state.pos] == '"') {
        state.pos++;
        state.column++;
      }

      return Node(str);
    }
    // Parse number
    else if (isdigit(state.input[state.pos]) || state.input[state.pos] == '-') {
      std::string num_str;
      bool is_float = false;

      while (state.pos < state.input.size() &&
             (isdigit(state.input[state.pos]) || state.input[state.pos] == '.' || state.input[state.pos] == '-')) {
        if (state.input[state.pos] == '.')
          is_float = true;
        num_str += state.input[state.pos];
        state.pos++;
        state.column++;
      }

      if (is_float) {
        try {
          return Node(std::stod(num_str));
        } catch (...) {
          throw std::runtime_error("Invalid float number");
        }
      } else {
        try {
          return Node(static_cast<int64_t>(std::stoll(num_str)));
        } catch (...) {
          throw std::runtime_error("Invalid integer number");
        }
      }
    }
    // Parse boolean/null
    else {
      std::string ident;
      while (state.pos < state.input.size() && isalpha(state.input[state.pos])) {
        ident += state.input[state.pos];
        state.pos++;
        state.column++;
      }

      if (ident == "true")
        return Node(true);
      if (ident == "false")
        return Node(false);
      if (ident == "null")
        return Node(std::monostate{});

      throw std::runtime_error("Unexpected token: " + ident);
    }
  }

  return Node(std::monostate{});
}

void format_value(FormatState &state, const Node &node) {
  if (std::holds_alternative<std::monostate>(node.value)) {
    state.output << "null";
  } else if (auto *b = std::get_if<bool>(&node.value)) {
    state.output << (*b ? "true" : "false");
  } else if (auto *i = std::get_if<int64_t>(&node.value)) {
    state.output << *i;
  } else if (auto *d = std::get_if<double>(&node.value)) {
    state.output << *d;
  } else if (auto *s = std::get_if<std::string>(&node.value)) {
    state.output << std::quoted(*s);
  } else if (auto *seq = std::get_if<Sequence>(&node.value)) {
    state.output << "[";
    bool first = true;
    for (const auto &item : *seq) {
      if (!first)
        state.output << ", ";
      first = false;
      format_value(state, item);
    }
    state.output << "]";
  } else if (auto *map = std::get_if<Map>(&node.value)) {
    state.output << "{";
    bool first = true;
    for (const auto &[key, value] : *map) {
      if (!first)
        state.output << ", ";
      first = false;
      state.output << std::quoted(key) << ": ";
      format_value(state, value);
    }
    state.output << "}";
  }
}

std::ostream &operator<<(std::ostream &os, const Node &node) {
  FormatState state;
  format_value(state, node);
  os << state.output.str();
  return os;
}

} // namespace yaml
} // namespace shilos
