
#include "shilos.hh"

#include <cstring>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace shilos {
namespace yaml {

struct ParseState {
  std::string_view input;
  size_t pos = 0;
  size_t line = 1;
  size_t column = 1;

  explicit ParseState(std::string_view str) : input(str) {}
};

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

Node Node::Load(std::string_view yaml_str) {
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

void format_yaml(std::ostream &os, const Node &node, int indent) {
  std::visit(
      [&os, indent](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
          os << std::string(indent, ' ') << "null";
        } else if constexpr (std::is_same_v<T, bool>) {
          os << std::string(indent, ' ') << (arg ? "true" : "false");
        } else if constexpr (std::is_same_v<T, int64_t>) {
          os << std::string(indent, ' ') << arg;
        } else if constexpr (std::is_same_v<T, double>) {
          os << std::string(indent, ' ') << arg;
        } else if constexpr (std::is_same_v<T, std::string>) {
          os << std::string(indent, ' ') << "\"" << arg << "\"";
        } else if constexpr (std::is_same_v<T, Sequence>) {
          os << std::string(indent, ' ') << "[\n";
          for (const auto &item : arg) {
            format_yaml(os, item, indent + 2);
            os << ",\n";
          }
          if (!arg.empty()) {
            os.seekp(-2, std::ios_base::cur); // Remove last comma
          }
          os << "\n" << std::string(indent, ' ') << "]";
        } else if constexpr (std::is_same_v<T, Map>) {
          os << std::string(indent, ' ') << "{\n";
          for (const auto &[key, value] : arg) {
            os << std::string(indent + 2, ' ') << "\"" << key << "\": ";
            format_yaml(os, value, 0);
            os << ",\n";
          }
          if (!arg.empty()) {
            os.seekp(-2, std::ios_base::cur); // Remove last comma
          }
          os << "\n" << std::string(indent, ' ') << "}";
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
