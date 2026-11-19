//===--- lang_syntax.hh - CoD source-language prototype syntax ------===//
#pragma once

#include <algorithm>
#include <cctype>
#include <compare>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cod::language {

struct Location {
  std::string file;
  unsigned line = 1;
  unsigned column = 1;
  auto operator<=>(const Location &) const = default;
};

inline std::string location_text(const Location &at) {
  return at.file + ":" + std::to_string(at.line) + ":" + std::to_string(at.column);
}

[[noreturn]] inline void fail(const Location &at, const std::string &message) {
  throw std::runtime_error(location_text(at) + ": " + message);
}

enum class TokenKind { identifier, symbol, integer, real, string, punctuation, end };
struct Token {
  TokenKind kind;
  std::string text;
  Location at;
};

inline std::vector<Token> tokenize(const std::string &source, const std::string &file) {
  std::vector<Token> tokens;
  Location at{file};
  size_t cursor = 0;
  auto advance = [&]() {
    if (source[cursor++] == '\n') {
      ++at.line;
      at.column = 1;
    } else {
      ++at.column;
    }
  };
  auto identifier = [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; };
  while (cursor < source.size()) {
    const char c = source[cursor];
    if (std::isspace(static_cast<unsigned char>(c))) {
      advance();
      continue;
    }
    if (c == '#') {
      while (cursor < source.size() && source[cursor] != '\n')
        advance();
      continue;
    }
    if (c == '/' && cursor + 1 < source.size() && source[cursor + 1] == '/')
      fail(at, "'//' is reserved for fraction construction; use '#' for line comments");
    if (source.compare(cursor, 2, "::") == 0)
      fail(at, "use a single colon for type annotations, or -> for callable results");
    const auto start = at;
    const auto offset = cursor;
    if (c == '@' || std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
      const bool symbol = c == '@';
      if (symbol) {
        advance();
        if (cursor == source.size() ||
            !(std::isalpha(static_cast<unsigned char>(source[cursor])) || source[cursor] == '_'))
          fail(start, "expected a symbol name after '@'");
      }
      const auto name_start = cursor;
      while (cursor < source.size() && identifier(source[cursor]))
        advance();
      tokens.push_back({symbol ? TokenKind::symbol : TokenKind::identifier,
                        source.substr(name_start, cursor - name_start), start});
      continue;
    }
    if (std::isdigit(static_cast<unsigned char>(c))) {
      while (cursor < source.size() && std::isdigit(static_cast<unsigned char>(source[cursor])))
        advance();
      bool real = false;
      if (cursor + 1 < source.size() && source[cursor] == '.' &&
          std::isdigit(static_cast<unsigned char>(source[cursor + 1]))) {
        real = true;
        advance();
        while (cursor < source.size() && std::isdigit(static_cast<unsigned char>(source[cursor]))) advance();
      }
      if (cursor < source.size() && (source[cursor] == 'e' || source[cursor] == 'E')) {
        real = true;
        advance();
        if (cursor < source.size() && (source[cursor] == '+' || source[cursor] == '-')) advance();
        if (cursor == source.size() || !std::isdigit(static_cast<unsigned char>(source[cursor])))
          fail(start, "expected exponent digits");
        while (cursor < source.size() && std::isdigit(static_cast<unsigned char>(source[cursor]))) advance();
      }
      if (source.compare(cursor, 3, "f32") == 0 || source.compare(cursor, 3, "f64") == 0) {
        real = true;
        advance(); advance(); advance();
      }
      if (!real && source.compare(cursor, 2, "u8") == 0) { advance(); advance(); }
      else if (!real && (source.compare(cursor, 3, "u32") == 0 || source.compare(cursor, 3, "u64") == 0)) {
        advance(); advance(); advance();
      }
      tokens.push_back({real ? TokenKind::real : TokenKind::integer, source.substr(offset, cursor - offset), start});
      continue;
    }
    if (c == '"') {
      advance();
      std::string value;
      while (cursor < source.size() && source[cursor] != '"') {
        char next = source[cursor];
        if (next == '\n')
          fail(start, "newline in module path");
        if (next == '\\') {
          advance();
          if (cursor == source.size() || (source[cursor] != '\\' && source[cursor] != '"'))
            fail(start, "module paths support only escaped backslash or quote");
          next = source[cursor];
        }
        value += next;
        advance();
      }
      if (cursor == source.size())
        fail(start, "unterminated module path");
      advance();
      tokens.push_back({TokenKind::string, std::move(value), start});
      continue;
    }
    std::string punctuation(1, c);
    if (source.compare(cursor, 3, "...") == 0 || source.compare(cursor, 3, "<=>") == 0 ||
        source.compare(cursor, 3, "<->") == 0) {
      const auto triple = source.substr(cursor, 3);
      advance(); advance(); advance();
      tokens.push_back({TokenKind::punctuation, triple, start});
      continue;
    }
    if (cursor + 1 < source.size()) {
      const auto pair = source.substr(cursor, 2);
      if (pair == "->" || pair == "==" || pair == "!=" || pair == "<=" || pair == ">=" ||
          pair == "&&" || pair == "||" || pair == "<-" || pair == "<:" || pair == ".." || pair == "=>") {
        punctuation = pair;
        advance();
      }
    }
    if (std::string("(){}[];,:.+-*/%<>=!").find(c) == std::string::npos && punctuation.size() != 2)
      fail(start, "unsupported character in CoD source");
    advance();
    tokens.push_back({TokenKind::punctuation, punctuation, start});
  }
  tokens.push_back({TokenKind::end, "", at});
  return tokens;
}

enum class ArgumentMode { value, borrowed_read, borrowed_ref, borrowed_ptr };

struct Type {
  enum Kind { integer, uint8, uint32, uint64, boolean, void_type, procedure, float32, float64,
              named, unit, quantity, region, regional, array, reference, null_ref, abstract_type, arguments, parameter,
              exception, never };
  Kind kind;
  std::string name;
  size_t identity = 0;
  std::shared_ptr<Type> target;
  std::vector<Type> parameters;
  std::vector<Type> bounds;
  std::vector<Type> owner_bounds;
  std::vector<Type> throws_types;
  std::vector<size_t> owners;
  std::vector<ArgumentMode> argument_modes;
  std::vector<std::string> capture_owners; // Source names anchoring a callable environment's regional dependencies.
  std::vector<std::optional<size_t>> capture_owner_regions; // Bound actual owner for each visible with-name.
  std::vector<size_t> capture_formals; // Nonzero indices refer to the enclosing callable's owner binders.
  std::vector<size_t> capture_regions; // Concrete WPO region identities, never runtime type information.
  std::vector<bool> const_views;
  std::vector<bool> nullable_views;
  bool variadic = false;
  bool owned = false;
  bool pure_callable = false;
  bool value_result = false;
  Type(Kind kind = void_type, std::string name = {}, size_t identity = 0)
      : kind(kind), name(std::move(name)), identity(identity) {}
  bool same_capture_contract(const Type &other) const {
    if (capture_regions != other.capture_regions) return false;
    if (kind != procedure) return true;
    const auto unresolved = [](const Type &type) {
      std::pair<std::set<size_t>, std::set<std::string>> result;
      for (size_t i = 0; i < type.capture_owners.size(); ++i) {
        if (!type.capture_formals.empty() && type.capture_formals[i])
          result.first.insert(type.capture_formals[i]);
        else if (i >= type.capture_owner_regions.size() || !type.capture_owner_regions[i])
          result.second.insert(type.capture_owners[i]);
      }
      return result;
    };
    return unresolved(*this) == unresolved(other);
  }
  bool operator==(const Type &other) const {
    return kind == other.kind && name == other.name && identity == other.identity &&
           parameters == other.parameters && bounds == other.bounds && variadic == other.variadic && owned == other.owned && pure_callable == other.pure_callable && value_result == other.value_result &&
           throws_types == other.throws_types && owner_bounds == other.owner_bounds && owners == other.owners && argument_modes == other.argument_modes && const_views == other.const_views && nullable_views == other.nullable_views &&
           same_capture_contract(other) &&
           ((!target && !other.target) || (target && other.target && *target == *other.target));
  }
};
inline bool floating(Type type) { return type.kind == Type::float32 || type.kind == Type::float64; }
inline bool integral(Type type) {
  return type == Type::integer || type == Type::uint8 || type == Type::uint32 || type == Type::uint64;
}
inline bool numeric(Type type) { return integral(type) || floating(type); }
inline Type representation(Type type) {
  return type.kind == Type::quantity ? *type.target : type;
}
inline Type quantity(Type unit, Type number) {
  unit.kind = Type::quantity;
  unit.target = std::make_shared<Type>(std::move(number));
  return unit;
}
inline bool widens(Type from, Type to) {
  if (from == to) return true;
  if (from.kind == Type::quantity || to.kind == Type::quantity) {
    if (from.kind != Type::quantity || to.kind != Type::quantity || from.identity != to.identity ||
        !from.target || !to.target) return false;
    return widens(*from.target, *to.target);
  }
  switch (from.kind) {
  case Type::uint8:
    return to == Type::uint32 || to == Type::uint64 || to == Type::integer || floating(to);
  case Type::uint32:
    return to == Type::uint64 || to == Type::integer || to == Type::float64;
  case Type::float32: return to == Type::float64;
  default: return false;
  }
}
inline std::string type_name(const Type &type) {
  switch (type.kind) {
  case Type::integer: return "i64";
  case Type::uint8: return "u8";
  case Type::uint32: return "u32";
  case Type::uint64: return "u64";
  case Type::boolean: return "bool";
  case Type::void_type: return "void";
  case Type::procedure: {
    if (!type.target) return "procedure family";
    std::string result = type.pure_callable ? "func" : "proc";
    result += "(";
    for (size_t i = 0; i < type.parameters.size(); ++i)
      result += (i ? ", " : "") + type_name(type.parameters[i]) +
                (type.variadic && i + 1 == type.parameters.size() ? "..." : "");
    result += ") -> " + std::string(type.const_views.size() > type.parameters.size() && type.const_views.back() ? "const " : "");
    result += type.target->kind == Type::procedure ? "(" + type_name(*type.target) + ")" : type_name(*type.target);
    if (!type.capture_owners.empty() || !type.capture_regions.empty()) {
      result += " with {";
      if (type.capture_owners.empty()) result += "region <inferred>";
      for (size_t i = 0; i < type.capture_owners.size(); ++i)
        result += (i ? ", " : "") + std::string("region ") + type.capture_owners[i];
      result += "}";
    }
    return result;
  }
  case Type::float32: return "f32";
  case Type::float64: return "f64";
  case Type::quantity: return type_name(*type.target) + "(" + type.name + ")";
  case Type::unit: return type.name;
  case Type::named: case Type::regional: case Type::array:
  case Type::abstract_type: return type.name;
  case Type::region: return std::string(type.owned ? "owned " : "") + "region" +
                           (type.target ? " :" + type_name(*type.target) : "");
  case Type::reference: return "stored reference to " + type_name(*type.target);
  case Type::null_ref: return "null";
  case Type::arguments: return "variadic arguments";
  case Type::parameter: return "type parameter " + std::to_string(type.identity + 1);
  case Type::exception: return type.name;
  case Type::never: return "non-returning expression";
  }
  return "unknown";
}

struct Form;
struct ReceivingHeader;
struct Expression {
  enum class Kind { integer, real, boolean, name, symbol, unary, binary, call, value_call, perform, behave, effect,
                    block, grouped, conditional, field, range_for, fold, allocate, index, null_ref,
                    try_scope, handler, raise, return_value, deferred, make_region, touch, effect_group, transfer, procedure_effect,
                    pointer_group, pointer, make_local, rebind, region_binding, callable, alias, capture_owner,
                    here_body, named_argument, break_value, continue_value };
  Kind kind;
  Location at;
  std::string text;
  std::vector<std::unique_ptr<Expression>> children;
  std::optional<size_t> symbol_id;
  std::string binding;
  bool spread = false;
  std::optional<Type> annotation;
  std::shared_ptr<Form> definition = {};
  bool intrinsic = false;
  bool fixed_binding = false;
  bool read_only = false;
  bool scoped = false;
  std::string declaration_kind = {};
  std::string owner_name = {};
  std::shared_ptr<ReceivingHeader> receiving = {};
  // A supplied body is stored after the written operands. It binds slot one.
  bool has_here_body() const { return !children.empty() && children.back()->kind == Kind::here_body; }
};
using Expr = std::unique_ptr<Expression>;

struct HereSignature;
struct Parameter {
  std::string name;
  Type type;
  Location at;
  bool automatic_conversion = false;
  std::vector<std::string> conversion_route;
  bool variadic = false;
  std::string owner = {};
  bool read_only = false;
  std::string declaration_kind = {};
  bool fixed_binding = false;
  std::shared_ptr<HereSignature> here = {};
};
struct HereSignature {
  std::vector<Parameter> parameters;
  // A labelled protocol has no positional field ordering.
  bool labelled = false;
  Type result = Type::void_type;
  std::string result_kind, result_owner;
  bool result_const = false;
  std::optional<std::vector<Parameter>> feedback_fields;
  std::vector<Type> throws_types;
};
struct ReceivingItem {
  std::string name;
  Location at;
  std::string declaration_kind, owner, operation, selector;
  std::optional<Type> annotation;
  bool fixed_binding = false, read_only = false;
};
struct ReceivingHeader {
  enum class Shape { ordered, single, labelled } shape;
  std::vector<ReceivingItem> items;
  bool from_for = false;
};
struct TypeParameter { std::string name; std::optional<Type> bound; Location at; };
struct OwnerParameter { std::string name; std::optional<Type> root; Location at; bool explicit_parameter = false; };
struct SymbolOwner { std::string name; Type root; Location at; };
struct UnitSelection { std::string source, local; Location at; };
struct SymbolMapping {
  std::string local;
  std::string source;
  Location at;
};
struct Capture { std::string name; Location at; };
struct Form {
  enum class Kind { symbol, symbol_from, function, procedure, data, region_definition, array, unit, using_units, global,
                    using_resource, import_resource, mixin_resource, export_resource, expression, abstract_type, conformance };
  Kind kind;
  Location at;
  std::string name;
  std::string path;
  std::string alias;
  bool symbolic = false;
  bool explicit_alias = false;
  bool using_symbol = false;
  bool exporting = false;
  bool conversion = false;
  bool operation = false;
  bool alias_operation = false;
  std::vector<SymbolMapping> mappings;
  std::vector<UnitSelection> units;
  std::vector<Parameter> parameters;
  std::vector<Capture> captures;
  bool explicit_captures = false;
  bool implicit_scope = false;
  bool supplied_scope = false;
  std::vector<TypeParameter> type_parameters;
  std::vector<Type> throws_types;
  std::vector<OwnerParameter> owner_parameters;
  std::shared_ptr<SymbolOwner> symbol_owner;
  std::string result_owner;
  bool result_const = false;
  std::string result_kind;
  std::optional<Type> result_type;
  std::optional<Type> index_type;
  std::optional<Type> parent_type;
  Expr body;
};

inline bool swap_operation(std::string_view op) { return op == "<=>" || op == "<->"; }
inline bool identity_operation(std::string_view op) { return op == "is" || op == "is not"; }

inline int binary_precedence(const std::string &op) {
  if (op == "||") return 1;
  if (op == "&&") return 2;
  if (op == "==" || op == "!=" || identity_operation(op)) return 3;
  if (op == "<" || op == ">" || op == "<=" || op == ">=") return 4;
  if (op == "+" || op == "-") return 5;
  if (op == "*" || op == "/" || op == "%") return 6;
  return -1;
}

inline bool intrinsic_name(std::string_view name) {
  return name == "cause_at" || name == "region" || name == "used_bytes" || name == "capacity_bytes" ||
         name == "deref" || name == "len" || name == "ref_at" || name == "store" ||
         name == "setfield" || name == "scalar" || name == "i64" || name == "u8" ||
         name == "u32" || name == "u64" || name == "f32" || name == "f64";
}

inline bool expression_keyword(std::string_view name) {
  return name == "true" || name == "false" || name == "null" || name == "alloc" || name == "is" || name == "not" ||
         name == "for" || name == "fold" || name == "var" || name == "let" || name == "effect" ||
         name == "perform" || name == "behave" || name == "if" || name == "procedure" ||
         name == "function" || name == "func" || name == "proc" || name == "global" || name == "symbol" ||
         name == "try" || name == "catch" || name == "finally" || name == "throw" || name == "return" || name == "defer" || name == "from" ||
         name == "val" || name == "ref" || name == "throws" || name == "make" || name == "owned" || name == "touch" || name == "ptr" || name == "typeof" || name == "intrinsic" || name == "const";
}

inline bool module_keyword(std::string_view name) {
  return name == "type" || name == "data" || name == "regional" || name == "unit" || name == "array" ||
         name == "using" || name == "import" || name == "mixin" || name == "export" || name == "conversion";
}

class Parser {
  std::vector<Token> tokens_;
  size_t cursor_ = 0;
  static constexpr size_t structure_limit = 128;
  static constexpr size_t recursion_limit = 1024;
  size_t recursion_ = 0;
  std::map<const Expression *, size_t> depths_;
  std::vector<size_t> right_paren_;
  bool parsing_handoff_ = false;
  bool labelled_yield_ = false;

  struct ParseEntry {
    size_t &depth;
    ParseEntry(size_t &depth, const Location &at, std::string_view subject) : depth(depth) {
      if (depth == recursion_limit)
        fail(at, "compiler resource limit: " + std::to_string(recursion_limit) +
            " active recursive parsing entries while parsing " + std::string(subject));
      ++depth;
    }
    ParseEntry(const ParseEntry &) = delete;
    ParseEntry &operator=(const ParseEntry &) = delete;
    ~ParseEntry() { --depth; }
  };
  static void check_depth(size_t depth, const Location &at, std::string_view subject) {
    if (depth > structure_limit)
      fail(at, "compiler resource limit: " + std::to_string(structure_limit) +
          " source structure levels while parsing " + std::string(subject));
  }
  static size_t type_depth(const Type &type, const Location &at) {
    size_t result = 0;
    std::vector<std::pair<const Type *, size_t>> work{{&type, 1}};
    while (!work.empty()) {
      const auto [current, depth] = work.back(); work.pop_back();
      check_depth(depth, at, "a type");
      result = std::max(result, depth);
      if (current->target) work.emplace_back(current->target.get(), depth + 1);
      for (const auto *types : {&current->parameters, &current->bounds, &current->owner_bounds, &current->throws_types})
        for (const auto &nested : *types) work.emplace_back(&nested, depth + 1);
    }
    return result;
  }
  static size_t parameter_depth(const Parameter &parameter) {
    auto depth = type_depth(parameter.type, parameter.at);
    if (const auto &here = parameter.here) {
      depth = std::max(depth, 1 + type_depth(here->result, parameter.at));
      for (const auto &input : here->parameters) depth = std::max(depth, 1 + parameter_depth(input));
      for (const auto &failure : here->throws_types) depth = std::max(depth, 1 + type_depth(failure, parameter.at));
      if (here->feedback_fields) for (const auto &field : *here->feedback_fields)
        depth = std::max(depth, 1 + type_depth(field.type, field.at));
    }
    return depth;
  }
  Expr finish(Expr expression) {
    // Cache only complete nodes. Check incremental left/postfix trees before
    // recursive consumers or destruction can encounter an excessive depth.
    std::vector<std::pair<const Expression *, bool>> work{{expression.get(), false}};
    while (!work.empty()) {
      const auto [node, ready] = work.back(); work.pop_back();
      if (depths_.contains(node)) continue;
      if (!ready) {
        work.emplace_back(node, true);
        for (const auto &child : node->children) work.emplace_back(child.get(), false);
        if (node->definition && node->definition->body) work.emplace_back(node->definition->body.get(), false);
        continue;
      }
      size_t depth = 1;
      for (const auto &child : node->children) depth = std::max(depth, depths_.at(child.get()) + 1);
      const auto include_type = [&](const Type &type) { depth = std::max(depth, type_depth(type, node->at) + 1); };
      if (node->annotation) include_type(*node->annotation);
      if (node->receiving) for (const auto &item : node->receiving->items)
        if (item.annotation) include_type(*item.annotation);
      if (node->definition) {
        const auto &definition = *node->definition;
        if (definition.body) depth = std::max(depth, depths_.at(definition.body.get()) + 1);
        for (const auto &parameter : definition.parameters) depth = std::max(depth, parameter_depth(parameter) + 1);
        for (const auto &parameter : definition.type_parameters) if (parameter.bound) include_type(*parameter.bound);
        for (const auto &parameter : definition.owner_parameters) if (parameter.root) include_type(*parameter.root);
        for (const auto &failure : definition.throws_types) include_type(failure);
        for (const auto *type : {&definition.result_type, &definition.index_type, &definition.parent_type})
          if (*type) include_type(**type);
        if (definition.symbol_owner) include_type(definition.symbol_owner->root);
      }
      check_depth(depth, node->at, "an expression");
      depths_.emplace(node, depth);
    }
    return expression;
  }

  const Token &peek() const { return tokens_[cursor_]; }
  bool is(const std::string &text, size_t ahead = 0) const {
    if (cursor_ + ahead >= tokens_.size()) return false;
    const auto &token = tokens_[cursor_ + ahead];
    return (token.kind == TokenKind::identifier || token.kind == TokenKind::punctuation) &&
           token.text == text;
  }
  bool take(const std::string &text) {
    if (!is(text)) return false;
    ++cursor_;
    return true;
  }
  Token expect(const std::string &text) {
    if (!is(text)) fail(peek().at, "expected '" + text + "'");
    return tokens_[cursor_++];
  }
  bool here_header(size_t ahead = 0) const {
    if (!is("(", ahead)) return false;
    const auto close = right_paren_[cursor_ + ahead];
    return close < tokens_.size() && is("=>", close + 1 - cursor_);
  }
  static bool accepts_supplied_body(const Expression &expr) {
    using K = Expression::Kind;
    return !expr.intrinsic && (expr.kind == K::call || expr.kind == K::value_call ||
        expr.kind == K::perform || expr.kind == K::behave);
  }
  Token name(TokenKind kind) {
    if (peek().kind != kind)
      fail(peek().at, kind == TokenKind::symbol ? "expected a symbol name" : "expected a lexical name");
    return tokens_[cursor_++];
  }
  Token ordinary_name() {
    auto token = name(TokenKind::identifier);
    if (expression_keyword(token.text) || module_keyword(token.text) || intrinsic_name(token.text) ||
        token.text == "bool" || token.text == "void" || token.text == "ref")
      fail(token.at, "reserved keyword '" + token.text + "' cannot be used as an ordinary name");
    return token;
  }
  Token value_name() {
    if (expression_keyword(peek().text))
      fail(peek().at, "reserved expression name '" + peek().text + "' cannot declare a lexical value");
    return ordinary_name();
  }
  static bool operation_token(const Token &token) {
    return (token.kind == TokenKind::identifier && (intrinsic_name(token.text) || token.text == "not" || identity_operation(token.text))) ||
           (token.kind == TokenKind::punctuation &&
            (binary_precedence(token.text) >= 0 || token.text == "!" || token.text == "=" || token.text == "<-" ||
             swap_operation(token.text)));
  }
  Token lexical_name(bool *operation = nullptr) {
    const bool parenthesized = take("(");
    if (operation) *operation = parenthesized;
    if (!parenthesized) return ordinary_name();
    auto op = tokens_[cursor_++];
    if (op.kind == TokenKind::identifier && op.text == "not") { op.text = "!"; op.kind = TokenKind::punctuation; }
    if (op.kind == TokenKind::identifier && op.text == "is" && take("not")) op.text = "is not";
    if (op.kind == TokenKind::punctuation && op.text == "[") {
      expect("]");
      if (take("<-")) op.text = "[]<-";
      else { expect("="); op.text = "[]="; }
    } else if (!operation_token(op))
      fail(op.at, "expected an operator or intrinsic name");
    if (op.text == "&&" || op.text == "||") fail(op.at, "short-circuit operators are control expressions");
    expect(")");
    return op;
  }
  void resource(Form &form, bool symbolic) {
    if (peek().kind != TokenKind::string) fail(peek().at, "expected a quoted module path");
    form.path = tokens_[cursor_++].text;
    form.symbolic = symbolic || peek().kind == TokenKind::symbol;
    const auto kind = form.symbolic ? TokenKind::symbol : TokenKind::identifier;
    form.name = form.symbolic ? name(kind).text : lexical_name(&form.operation).text;
    form.explicit_alias = take("as");
    form.alias = form.explicit_alias ? (form.symbolic ? name(kind).text : lexical_name(&form.alias_operation).text) : form.name;
    if (!form.explicit_alias) form.alias_operation = form.operation;
    if (form.alias_operation && identity_operation(form.alias))
      fail(form.at, "reference identity is an intrinsic operation and cannot be an imported user operation");
  }
  Type parse_type(Parameter *parameter = nullptr) {
    const auto at = peek().at;
    const ParseEntry entry(recursion_, at, "a type");
    auto type = parse_type_impl(parameter);
    type_depth(type, at);
    return type;
  }
  Type parse_type_impl(Parameter *parameter) {
    if (take("(")) {
      auto type = parse_type(parameter);
      expect(")");
      return type;
    }
    auto token = name(TokenKind::identifier);
    if (token.text == "owned" || token.text == "region")
      fail(token.at, "region is a declaration keyword; use 'region name:Root' or 'owned region name:Root', not a value-type annotation");
    if (token.text == "proc" || token.text == "procedure" || token.text == "func" || token.text == "function") {
      Type type{Type::procedure};
      type.pure_callable = token.text == "func" || token.text == "function";
      if (is("[")) fail(peek().at, "callable types associate owners with 'with {region name}'; brackets list body captures");
      expect("(");
      if (!take(")")) {
        do {
          if (type.variadic) fail(peek().at, "only the last procedure parameter can be variadic");
          bool read_only = false;
          std::string mode;
          type.parameters.push_back(signature_type(read_only, mode));
          type.const_views.push_back(read_only);
          type.nullable_views.push_back(mode == "ptr");
          type.variadic = take("...");
        } while (take(","));
        expect(")");
      }
      expect("->");
      bool result_const = false;
      std::string result_mode;
      type.target = std::make_shared<Type>(signature_type(result_const, result_mode));
      if (result_mode == "val" && !numeric(*type.target) && *type.target != Type::boolean)
        fail(token.at, "micro-dispatch implementation gap: explicit val callable results require a concrete scalar type; this is not a language restriction");
      type.value_result = result_mode != "ref" && result_mode != "ptr" &&
          (result_const || result_mode == "val" || numeric(*type.target) || *type.target == Type::boolean ||
           *type.target == Type::void_type || type.target->kind == Type::procedure);
      type.const_views.push_back(result_const);
      type.nullable_views.push_back(result_mode == "ptr");
      if (std::none_of(type.const_views.begin(), type.const_views.end(), [](bool value) { return value; })) type.const_views.clear();
      if (std::none_of(type.nullable_views.begin(), type.nullable_views.end(), [](bool value) { return value; })) type.nullable_views.clear();
      if (take("with")) {
        expect("{");
        do {
          expect("region");
          const auto owner = ordinary_name();
          if (std::find(type.capture_owners.begin(), type.capture_owners.end(), owner.text) != type.capture_owners.end())
            fail(owner.at, "duplicate callable environment owner '" + owner.text + "'");
          type.capture_owners.push_back(owner.text);
        } while (take(","));
        expect("}");
      }
      if (take("throws")) type.throws_types.push_back(parse_type());
      return type;
    }
    if (token.text == "ref" || token.text == "ptr" || token.text == "val")
      fail(token.at, "ref/ptr/val are declaration modifiers, not type constructors; name the target type directly");
    Type type{Type::named, token.text};
    if (token.text == "i64") type = Type::integer;
    else if (token.text == "u8") type = Type::uint8;
    else if (token.text == "u32") type = Type::uint32;
    else if (token.text == "u64") type = Type::uint64;
    else if (token.text == "bool") type = Type::boolean;
    else if (token.text == "void") type = Type::void_type;
    else if (token.text == "f32") type = Type::float32;
    else if (token.text == "f64") type = Type::float64;
    if (take("(")) {
      auto unit = name(TokenKind::identifier);
      if (parameter) {
        parameter->automatic_conversion = take("*");
        if (is("<-") && !parameter->automatic_conversion)
          fail(peek().at, "a conversion route requires a starred parameter");
        while (take("<-")) parameter->conversion_route.push_back(name(TokenKind::identifier).text);
      }
      expect(")");
      Type annotated{Type::named, unit.text};
      annotated.target = std::make_shared<Type>(std::move(type));
      return annotated;
    }
    return type;
  }
  void type_separator() { expect(":"); }
  Expr make(Expression::Kind kind, const Token &token) {
    auto expression = std::make_unique<Expression>(Expression{kind, token.at, token.text, {}, {}, {}, false, {}});
    depths_.erase(expression.get());
    return expression;
  }
  Token field_name() {
    if (!take("(")) return name(TokenKind::identifier);
    auto field = field_name();
    expect(")");
    return field;
  }
  void arguments(Expression &expr) {
    const auto close = right_paren_[cursor_];
    const bool follows_call = close < tokens_.size() &&
        (is("{", close + 1 - cursor_) || here_header(close + 1 - cursor_));
    const bool labelled_yield = labelled_yield_ && expr.kind == Expression::Kind::call && expr.text == "yield";
    const bool named = (accepts_supplied_body(expr) && follows_call) || labelled_yield;
    bool saw_label = false;
    expect("(");
    if (!take(")")) {
      do {
        if (expr.kind == Expression::Kind::call && expr.text == "setfield" && expr.children.size() == 1)
          expr.children.push_back(make(Expression::Kind::name, field_name()));
        else {
          Expr argument;
          if (!named || !is("->")) {
            argument = expression(0, true, !named);
            argument->spread = take("...");
          }
          if (named && take("->")) {
            saw_label = true;
            auto selected = make(Expression::Kind::named_argument, ordinary_name());
            if (argument) selected->children.push_back(std::move(argument));
            else if (labelled_yield) fail(selected->at, "a labelled yield operand cannot request a default");
            argument = std::move(selected);
          } else if (saw_label || labelled_yield) {
            fail(argument->at, labelled_yield ? "a labelled yield requires a label for every operand" :
                                                "positional arguments must precede named arguments");
          }
          expr.children.push_back(std::move(argument));
        }
      } while (take(","));
      expect(")");
    }
  }
  Type region_annotation(bool owned, bool *read_only = nullptr) {
    Type type{Type::region};
    type.owned = owned;
    if (is("<")) fail(peek().at, "region root annotations use ':Root', not '<Root>'");
    if (is(":")) {
      type_separator();
      if (read_only) *read_only = take("const");
      type.target = std::make_shared<Type>(parse_type());
    }
    return type;
  }
  Type signature_type(bool &read_only, std::string &mode) {
    if (is("ptr") || is("ref") || is("val")) mode = tokens_[cursor_++].text;
    read_only = take("const");
    if (is("region") || is("owned")) {
      if (read_only) fail(peek().at, "region target const belongs after the root annotation colon");
      const bool owned = take("owned");
      expect("region");
      return region_annotation(owned, &read_only);
    }
    return parse_type();
  }
  Parameter parameter_declaration() {
    const bool fixed_binding = take("const");
    const bool owned_region = take("owned");
    const bool regional = take("region");
    if (owned_region && !regional) fail(peek().at, "owned requires a region declaration");
    std::string mode;
    if (!regional && (is("ptr") || is("ref") || is("val"))) mode = tokens_[cursor_++].text;
    auto param = value_name();
    Parameter parameter{param.text, Type::void_type, param.at, false, {}};
    parameter.fixed_binding = fixed_binding;
    if (regional) parameter.type = region_annotation(owned_region, &parameter.read_only);
    else {
      type_separator();
      parameter.read_only = take("const");
      parameter.type = parse_type(&parameter);
    }
    parameter.declaration_kind = mode;
    if (take("in")) parameter.owner = ordinary_name().text;
    if (mode == "val" && !parameter.owner.empty()) fail(param.at, "a val parameter has callee-local bits storage, not an external owner");
    if (parameter.type == Type::void_type) fail(param.at, "a parameter must have a value type");
    parameter.variadic = take("...");
    return parameter;
  }
  static void result_kind(const std::string &mode, const std::string &owner, const Location &at) {
    if (mode == "val" && !owner.empty())
      fail(at, "a val result has no owner association; use ref T in O for a borrowed result");
    if ((mode == "ref" || mode == "ptr") && owner.empty())
      fail(at, "a ref/ptr result requires an explicit external owner: use in O");
  }
  std::vector<Parameter> protocol_fields() {
    expect("{");
    std::vector<Parameter> fields;
    std::set<std::string> labels;
    while (!take("}")) {
      auto field = parameter_declaration();
      if (field.variadic) fail(field.at, "a protocol field cannot be variadic");
      if (!labels.insert(field.name).second) fail(field.at, "duplicate protocol label '" + field.name + "'");
      fields.push_back(std::move(field));
      if (!is("}")) expect(";");
      else take(";");
    }
    return fields;
  }
  void here_result(HereSignature &signature, const Location &at) {
    if (is("{")) signature.feedback_fields = protocol_fields();
    else {
      signature.result = signature_type(signature.result_const, signature.result_kind);
      if (take("in")) signature.result_owner = ordinary_name().text;
      result_kind(signature.result_kind, signature.result_owner, at);
    }
  }
  Parameter here_parameter() {
    const ParseEntry entry(recursion_, peek().at, "a here signature");
    expect("here");
    const auto param = value_name();
    Parameter parameter{param.text, Type::void_type, param.at, false, {}};
    auto &signature = *(parameter.here = std::make_shared<HereSignature>());
    expect("(");
    if (!take(")")) {
      do {
        if (is("here") && !is(":", 1)) {
          if (!signature.parameters.empty()) fail(peek().at, "a here callable must be the first parameter");
          signature.parameters.push_back(here_parameter());
        } else signature.parameters.push_back(parameter_declaration());
      } while (take(","));
      expect(")");
    }
    expect("->");
    here_result(signature, param.at);
    if (take("throws")) signature.throws_types.push_back(parse_type());
    if (is("...")) fail(peek().at, "a here parameter cannot be variadic");
    return parameter;
  }
  void generator_contract(Form &form) {
    const auto token = expect("yields");
    if (std::ranges::any_of(form.parameters, [](const auto &parameter) { return parameter.name == "yield"; }))
      fail(token.at, "yields introduces the first parameter 'yield', which is already declared");
    if (!form.parameters.empty() && form.parameters.front().here)
      fail(token.at, "yields supplies the first here parameter");
    Parameter yield{"yield", Type::void_type, token.at, false, {}};
    auto &signature = *(yield.here = std::make_shared<HereSignature>());
    signature.labelled = is("{");
    if (signature.labelled) {
      signature.parameters = protocol_fields();
      if (signature.parameters.empty()) fail(token.at, "a yields clause requires a value component");
    } else {
      Parameter output{"", Type::void_type, peek().at, false, {}};
      output.type = signature_type(output.read_only, output.declaration_kind);
      if (output.type == Type::void_type) fail(output.at, "a yields clause requires a value component");
      if (take("in")) output.owner = ordinary_name().text;
      result_kind(output.declaration_kind, output.owner, output.at);
      signature.parameters.push_back(std::move(output));
    }
    if (take("->")) here_result(signature, token.at);
    form.parameters.insert(form.parameters.begin(), std::move(yield));
    form.result_type = Type::void_type;
  }
  void generic_clause(Form &form) {
    const bool grouped = take("{");
    do {
      if (is("region") || is("owner")) {
        const bool regional = take("region");
        if (!regional) expect("owner");
        auto owner = ordinary_name();
        OwnerParameter parameter{owner.text, {}, owner.at};
        if (regional) { type_separator(); parameter.root = parse_type(); }
        form.owner_parameters.push_back(std::move(parameter));
      } else {
        auto parameter = ordinary_name();
        for (const auto &previous : form.type_parameters)
          if (previous.name == parameter.text) fail(parameter.at, "duplicate type parameter '" + parameter.text + "'");
        TypeParameter type_parameter{parameter.text, {}, parameter.at};
        if (take("<:")) type_parameter.bound = parse_type();
        form.type_parameters.push_back(std::move(type_parameter));
      }
    } while (grouped && take(","));
    if (grouped) expect("}");
  }
  void callable(Form &form, bool inline_effect = false, bool expression_form = false) {
    form.conversion = take("conversion");
    if (form.conversion && !is("function")) fail(peek().at, "conversion requires a pure function");
    const bool function = take("function") || take("func");
    if (!function && !(inline_effect && peek().kind == TokenKind::symbol) && !take("proc")) expect("procedure");
    form.kind = function ? Form::Kind::function : Form::Kind::procedure;
    form.symbolic = !function && peek().kind == TokenKind::symbol;
    if (!(expression_form && (is("(") || is("["))))
      form.name = form.symbolic ? name(TokenKind::symbol).text : lexical_name(&form.operation).text;
    if (take("[")) {
      form.explicit_captures = true;
      if (!take("]")) {
        do {
          const auto binding = value_name();
          if (is("=") || is("<-")) fail(peek().at, "capture lists contain variable names; declare a var alias before the callable");
          form.captures.push_back({binding.text, binding.at});
        } while (take(","));
        expect("]");
      }
      if (!expression_form && function)
        fail(form.at, "captured functions are lexical callable expressions, not module method declarations");
    }
    const bool parameter_list = take("(");
    if (parameter_list) {
      if (!take(")")) {
        do {
          if (is("here") && !is(":", 1)) {
            if (!form.parameters.empty()) fail(peek().at, "a here callable must be the first parameter");
            form.parameters.push_back(here_parameter());
          } else form.parameters.push_back(parameter_declaration());
        } while (take(","));
        expect(")");
      }
    } else if (function) {
      fail(peek().at, "a function requires a parameter list");
    }
    const bool preceding_where = take("where");
    if (preceding_where) generic_clause(form);
    if (is("yields")) generator_contract(form);
    else if (take("->")) {
      form.result_type = signature_type(form.result_const, form.result_kind);
      if (take("in")) form.result_owner = ordinary_name().text;
    }
    result_kind(form.result_kind, form.result_owner, form.at);
    if (take("throws")) form.throws_types.push_back(parse_type());
    if (take("where")) {
      if (preceding_where) fail(form.at, "a callable declares its generic parameters in one where clause");
      generic_clause(form);
    }
    if (take("throws")) {
      if (!form.throws_types.empty()) fail(peek().at, "duplicate throws clause");
      form.throws_types.push_back(parse_type());
    }
    for (auto &parameter : form.parameters) {
      if (parameter.type.kind != Type::region) continue;
      const auto mentions = [&](auto &&self, const Type &type) -> bool {
        return std::find(type.capture_owners.begin(), type.capture_owners.end(), parameter.name) != type.capture_owners.end() ||
               (type.target && self(self, *type.target)) || std::any_of(type.parameters.begin(), type.parameters.end(),
                   [&](const Type &child) { return self(self, child); });
      };
      const bool used = form.result_owner == parameter.name || (form.result_type && mentions(mentions, *form.result_type)) ||
          std::any_of(form.parameters.begin(), form.parameters.end(),
              [&](const Parameter &other) { return other.owner == parameter.name || mentions(mentions, other.type); });
      if (!used) continue;
      if (!parameter.owner.empty() && parameter.owner != parameter.name)
        fail(parameter.at, "a named region owner cannot also name a different generic owner");
      parameter.owner = parameter.name;
      form.owner_parameters.push_back({parameter.name, parameter.type.target ? std::optional<Type>(*parameter.type.target) : std::nullopt,
                                       parameter.at, true});
    }
    std::set<std::string> generic_names;
    for (const auto &parameter : form.type_parameters) generic_names.insert(parameter.name);
    for (const auto &owner : form.owner_parameters)
      if (!generic_names.insert(owner.name).second)
        fail(owner.at, "duplicate generic parameter '" + owner.name + "'");
    for (size_t i = 0; i + 1 < form.parameters.size(); ++i)
      if (form.parameters[i].variadic) fail(form.parameters[i].at, "only the last parameter can be variadic");
    const bool declaration = take(";");
    if (declaration && !parameter_list) fail(form.at, "a callable declaration requires a parameter list");
    if (inline_effect && !form.result_type) form.result_type = Type::void_type;
    if (declaration && !form.result_type)
      fail(form.at, "a callable declaration requires an explicit result type");
    if (declaration && form.conversion)
      fail(form.at, "a conversion requires an implemented function method");
    if (!declaration) {
      const auto previous = std::exchange(labelled_yield_, !form.parameters.empty() && form.parameters.front().here &&
          form.parameters.front().name == "yield" && form.parameters.front().here->labelled);
      form.body = block();
      labelled_yield_ = previous;
      if (!inline_effect && !expression_form) take(";");
    }
    if (inline_effect && (!form.symbolic || declaration || function))
      fail(form.at, "effect proc requires a symbolic procedure definition");
  }
  bool callable_binding() const {
    const size_t prefix = is("const") ? 1 : 0;
    return (is("func", prefix) || is("proc", prefix)) && tokens_[cursor_ + prefix + 1].kind == TokenKind::identifier &&
           (is("=", prefix + 2) || is(":", prefix + 2));
  }
  ReceivingItem receiving_item(bool operation_required, bool labelled) {
    const bool fixed = take("const");
    std::string kind;
    if (is("val") || is("ref") || is("ptr") || is("func")) kind = tokens_[cursor_++].text;
    const auto name = value_name();
    ReceivingItem item{name.text, name.at, std::move(kind), {}, {}, {}, {}, fixed};
    if (take(":")) {
      item.read_only = take("const");
      if (!item.read_only || (!is("=") && !is("<-") && !is(",") && !is(")") && !is("in")))
        item.annotation = parse_type();
    }
    if (take("in")) item.owner = ordinary_name().text;
    if (is("=") || is("<-")) item.operation = tokens_[cursor_++].text;
    else if (operation_required) fail(peek().at, "a receiving destination requires '=' or '<-'");
    if (!item.operation.empty()) {
      if (item.declaration_kind == "val" && item.operation != "<-")
        fail(item.at, "val initialization requires '<-', not '='");
      if (item.declaration_kind == "ref" && item.operation != "=")
        fail(item.at, "ref initialization requires '='");
      if (item.declaration_kind.empty() && (item.fixed_binding || item.annotation || item.read_only || !item.owner.empty()))
        fail(item.at, "an existing receiving destination cannot have a declaration annotation");
    }
    if (labelled) item.selector = is(";") || is("}") ? item.name : ordinary_name().text;
    return item;
  }
  void labelled_receiving(ReceivingHeader &header) {
    header.shape = ReceivingHeader::Shape::labelled;
    expect("{");
    while (!take("}")) {
      header.items.push_back(receiving_item(true, true));
      expect(";");
    }
  }
  static void validate_receivers(const ReceivingHeader &header) {
    std::map<std::string, bool> names;
    for (const auto &item : header.items) {
      const bool fresh = item.operation.empty() || !item.declaration_kind.empty();
      if (const auto [found, inserted] = names.emplace(item.name, fresh); !inserted && (fresh || found->second))
        fail(item.at, "conflicting receiving destination '" + item.name + "'");
    }
  }
  void attach_body(Expression &call, ReceivingHeader header) {
    using K = Expression::Kind;
    if (call.has_here_body()) fail(peek().at, "a call can supply only one trailing here body");
    validate_receivers(header);
    auto supplied = make(K::here_body, peek());
    supplied->receiving = std::make_shared<ReceivingHeader>(std::move(header));
    if (is("=>", 1) || (is("{", 1) && is("=>", 2)))
      fail(peek().at, "write the receiving header as '(...) => ' before the here body");
    const bool previous = std::exchange(parsing_handoff_, true);
    auto body = block();
    parsing_handoff_ = previous;
    if (!supplied->receiving->from_for) supplied->scoped = std::exchange(body->scoped, false);
    supplied->children.push_back(std::move(body));
    call.children.push_back(finish(std::move(supplied)));
    // The call was complete before its trailing operand was added.
    depths_.erase(&call);
  }
  void trailing_body(Expression &call) {
    ReceivingHeader header{ReceivingHeader::Shape::ordered, {}};
    if (take("(")) {
      if (is("{")) labelled_receiving(header);
      else if (!is(")")) {
        header.items.push_back(receiving_item(false, false));
        if (!header.items.front().operation.empty()) header.shape = ReceivingHeader::Shape::single;
        else while (take(",")) {
          auto item = receiving_item(false, false);
          if (!item.operation.empty()) fail(item.at, "an explicit receiving operation requires a single destination or a labelled spec");
          header.items.push_back(std::move(item));
        }
      }
      expect(")");
      expect("=>");
    }
    attach_body(call, std::move(header));
  }
  Expr generator_for(const Location &at) {
    ReceivingHeader header{ReceivingHeader::Shape::single, {}, true};
    if (is("{")) {
      labelled_receiving(header);
      expect(":");
      expect("=");
    } else header.items.push_back(receiving_item(true, false));
    auto producer = expression(0, false);
    if (!accepts_supplied_body(*producer)) fail(at, "a generator for loop requires a producer call");
    attach_body(*producer, std::move(header));
    return producer;
  }
  Expr primary(bool attach, bool owner_projection) {
    auto token = peek();
    if (callable_binding()) {
      const bool fixed_binding = take("const");
      const auto mode = tokens_[cursor_++].text;
      auto group = make(Expression::Kind::pointer_group, token);
      group->declaration_kind = mode;
      group->fixed_binding = fixed_binding;
      auto binding = make(Expression::Kind::pointer, value_name());
      binding->declaration_kind = mode;
      if (take(":")) { binding->read_only = take("const"); binding->annotation = parse_type(); }
      expect("=");
      binding->binding = "=";
      binding->children.push_back(expression());
      group->children.push_back(std::move(binding));
      return group;
    }
    if (is("proc") || is("procedure") || is("func") || is("function")) {
      auto definition = std::make_shared<Form>();
      definition->at = token.at;
      callable(*definition, false, true);
      if (definition->symbolic || definition->operation || !definition->body)
        fail(token.at, "a lexical callable expression requires a body and a lexical name or no name");
      auto expr = make(Expression::Kind::callable, token);
      expr->definition = std::move(definition);
      return expr;
    }
    if (is("owned") || (is("region") && !is("(", 1)) ||
        (is("const") && (is("owned", 1) || is("region", 1)))) {
      const bool fixed_binding = take("const");
      const bool owned = take("owned");
      expect("region");
      auto expr = make(Expression::Kind::region_binding, value_name());
      expr->fixed_binding = fixed_binding;
      expr->annotation = region_annotation(owned, &expr->read_only);
      expr->binding = owned ? "<-" : "=";
      expect(expr->binding);
      expr->children.push_back(expression());
      return expr;
    }
    if (take("make")) {
      if (take("local")) {
        auto expr = make(Expression::Kind::make_local, token);
        if (take("typeof")) {
          expect("(");
          expr->children.push_back(expression());
          expect(")");
        } else expr->annotation = parse_type();
        if (is("{")) expr->children.push_back(block());
        return expr;
      }
      auto expr = make(Expression::Kind::make_region, name(TokenKind::identifier));
      expect("(");
      expect("capacity");
      expect("=");
      expr->children.push_back(expression());
      expect(")");
      expect("{");
      while (!take("}")) {
        auto field = make(Expression::Kind::field, field_name());
        if (take("=")) field->binding = "=";
        else { expect("<-"); field->binding = "<-"; }
        field->children.push_back(expression());
        expr->children.push_back(std::move(field));
        if (!is("}")) expect(";");
        else take(";");
      }
      return expr;
    }
    if (is("try") || is("defer")) {
      const bool deferred = take("defer");
      if (!deferred) expect("try");
      auto expr = make(deferred ? Expression::Kind::deferred : Expression::Kind::try_scope, token);
      if (deferred && !is("{")) {
        if (is("catch") || is("finally")) {
          expr->binding = "fallback";
          expr->children.push_back(make(Expression::Kind::block, token));
        } else {
          auto call = expression();
          const auto kind = call->kind;
          if (kind != Expression::Kind::call && kind != Expression::Kind::value_call &&
              kind != Expression::Kind::perform && kind != Expression::Kind::behave)
            fail(token.at, "defer requires a callable invocation or an explicit delayed block");
          expr->children.push_back(std::move(call));
          return expr;
        }
      } else expr->children.push_back(block());
      bool catch_all = false;
      while (take("catch")) {
        if (catch_all) fail(peek().at, "catch-all must be the final catch clause");
        expect("(");
        auto handler = make(Expression::Kind::handler, value_name());
        if (take(":")) {
          expect("const");
          handler->annotation = parse_type();
        } else catch_all = true;
        expect(")");
        handler->children.push_back(block());
        expr->children.push_back(std::move(handler));
      }
      if (take("finally")) expr->children.push_back(block());
      if (!deferred && expr->children.size() == 1) fail(token.at, "try requires catch, finally, or both");
      return expr;
    }
    if (take("throw")) {
      auto expr = make(Expression::Kind::raise, token);
      if (!is(";") && !is("}")) {
        expr->children.push_back(expression());
        if (take("from")) expr->children.push_back(expression());
      }
      return expr;
    }
    if (take("return")) {
      auto expr = make(Expression::Kind::return_value, token);
      if (!is(";") && !is("}")) expr->children.push_back(expression());
      return expr;
    }
    if (parsing_handoff_ && (is("break") || is("continue"))) {
      ++cursor_;
      auto expr = make(token.text == "break" ? Expression::Kind::break_value : Expression::Kind::continue_value, token);
      if (!is(";") && !is("}")) expr->children.push_back(expression());
      return expr;
    }
    if (take("alloc")) {
      const bool symbolic = peek().kind == TokenKind::symbol;
      auto owner = make(symbolic ? Expression::Kind::symbol : Expression::Kind::name,
                        symbolic ? name(TokenKind::symbol) : value_name());
      auto expr = make(Expression::Kind::allocate, name(TokenKind::identifier));
      expr->children.push_back(std::move(owner));
      arguments(*expr);
      return expr;
    }
    if (is("for") || is("fold")) {
      const bool fold = take("fold");
      if (!fold) expect("for");
      if (!fold && !is("(")) return generator_for(token.at);
      expect("(");
      auto expr = make(fold ? Expression::Kind::fold : Expression::Kind::range_for,
                       value_name());
      expect("in");
      expr->children.push_back(expression());
      expect("..");
      expr->children.push_back(expression());
      if (fold) {
        expect(",");
        expr->binding = value_name().text;
        if (expr->text == expr->binding) fail(expr->at, "range and accumulator names must differ");
        expect("=");
        expr->children.push_back(expression());
      }
      expect(")");
      expr->children.push_back(block());
      if (take("else")) {
        if (fold) fail(token.at, "else belongs to a range for; fold already has an initial accumulator");
        expr->children.push_back(block());
      }
      return expr;
    }
    const size_t qualifier = is("const") ? 1 : 0;
    if (is("var", qualifier) || ((is("val", qualifier) || is("ref", qualifier) || is("ptr", qualifier)) && is("var", qualifier + 1))) {
      const bool fixed_binding = take("const");
      std::string mode;
      if (!is("var")) mode = tokens_[cursor_++].text;
      expect("var");
      auto expr = make(Expression::Kind::alias, value_name());
      expr->declaration_kind = mode;
      expr->fixed_binding = fixed_binding;
      if (take(":")) {
        expr->read_only = take("const");
        if (!is("->") && !is("in")) expr->annotation = parse_type();
        else if (!expr->read_only) fail(peek().at, "an alias colon requires a type or const qualifier");
      }
      if (take("in")) expr->owner_name = ordinary_name().text;
      if (!take("->")) fail(peek().at, "var aliases require '-> lvalue'; use ref/ptr for reference bindings or val for independent bits storage");
      expr->children.push_back(expression());
      return expr;
    }
    if (is("ptr") || is("ref") || is("val") || (is("const") && (is("ptr", 1) || is("ref", 1) || is("val", 1)))) {
      const bool fixed = take("const");
      const auto mode = tokens_[cursor_++].text;
      auto group = make(Expression::Kind::pointer_group, token);
      group->fixed_binding = fixed;
      group->declaration_kind = mode;
      do {
        if (is("{")) { group->children.push_back(block()); break; }
        auto expr = make(Expression::Kind::pointer, value_name());
        expr->declaration_kind = mode;
        if (take(":")) { expr->read_only = take("const"); expr->annotation = parse_type(); }
        if (take("in")) expr->owner_name = ordinary_name().text;
        if (take("<-")) expr->binding = "<-";
        else { expect("="); expr->binding = "="; }
        if (mode == "val" && expr->binding != "<-") fail(expr->at, "val initialization requires '<-', not '='");
        if (mode == "ref" && expr->binding != "=") fail(expr->at, "ref initialization requires '='; use val for an independent copy");
        expr->children.push_back(expression());
        group->children.push_back(std::move(expr));
      } while (take(","));
      return group;
    }
    if (is("let") || is("var"))
      fail(token.at, "use ref/ptr for reference bindings or val for independent bits storage");

    if (take("effect")) {
      if (is("proc") || is("procedure") || (peek().kind == TokenKind::symbol && is("(", 1))) {
        auto definition = std::make_shared<Form>();
        definition->at = peek().at;
        callable(*definition, true);
        auto expr = make(Expression::Kind::procedure_effect, token);
        expr->text = definition->name;
        expr->definition = std::move(definition);
        return expr;
      }
      auto group = make(Expression::Kind::effect_group, token);
      do {
        auto expr = make(Expression::Kind::effect, name(TokenKind::symbol));
        if (group->children.empty() && (is(";") || is("}"))) return expr;
        if (take("=")) expr->binding = "=";
        else { expect("<-"); expr->binding = "<-"; }
        expr->children.push_back(expression());
        group->children.push_back(std::move(expr));
        if (!is(",") || tokens_[cursor_ + 1].kind != TokenKind::symbol ||
            (!is("=", 2) && !is("<-", 2))) break;
        expect(",");
      } while (true);
      if (group->children.size() == 1) return std::move(group->children.front());
      for (const auto &binding : group->children)
        if (binding->binding != "=") fail(binding->at, "a grouped effect currently requires object-reference '=' bindings");
      return group;
    }
    if (take("touch")) return make(Expression::Kind::touch, name(TokenKind::symbol));
    if (is("perform") || is("behave")) {
      const bool perform = take("perform");
      if (!perform) expect("behave");
      auto expr = make(perform ? Expression::Kind::perform : Expression::Kind::behave,
                       name(TokenKind::symbol));
      if (is("(")) arguments(*expr);
      return expr;
    }
    if (take("if")) {
      auto expr = make(Expression::Kind::conditional, token);
      expect("(");
      expr->children.push_back(expression());
      expect(")");
      expr->children.push_back(block());
      expect("else");
      expr->children.push_back(block());
      return expr;
    }
    if (is("{")) return block();
    if (take("intrinsic")) {
      if (!is("(")) fail(peek().at, "intrinsic requires a parenthesized operator or intrinsic name");
      auto expr = make(Expression::Kind::call, lexical_name());
      expr->intrinsic = true;
      arguments(*expr);
      return expr;
    }
    if (is("(") && ((is(")", 2) && operation_token(tokens_[cursor_ + 1])) ||
        (is("is", 1) && is("not", 2) && is(")", 3)) ||
        (is("[", 1) && is("]", 2) && (is("=", 3) || is("<-", 3)) && is(")", 4)))) {
      auto expr = make(Expression::Kind::call, lexical_name());
      arguments(*expr);
      return expr;
    }
    if (take("(")) {
      auto expr = make(Expression::Kind::grouped, token);
      expr->children.push_back(expression());
      expect(")");
      return expr;
    }
    if (take("-") || take("!") || take("not")) {
      auto expr = make(Expression::Kind::unary, token);
      if (expr->text == "not") expr->text = "!";
      expr->children.push_back(expression(7, attach, owner_projection));
      return expr;
    }
    if (token.kind == TokenKind::integer) {
      ++cursor_;
      return make(Expression::Kind::integer, token);
    }
    if (token.kind == TokenKind::real) {
      ++cursor_;
      return make(Expression::Kind::real, token);
    }
    if (token.kind == TokenKind::symbol) {
      ++cursor_;
      return make(Expression::Kind::symbol, token);
    }
    if (token.kind == TokenKind::identifier) {
      ++cursor_;
      if (token.text == "null") {
        auto expr = make(Expression::Kind::null_ref, token);
        if (take("in")) expr->owner_name = ordinary_name().text;
        return expr;
      }
      if (token.text == "true" || token.text == "false") return make(Expression::Kind::boolean, token);
      if (token.text == "procedure" || token.text == "proc" || token.text == "function" || token.text == "global" || token.text == "symbol")
        fail(token.at, "prototype declarations belong at module top level");
      auto expr = make(is("(") ? Expression::Kind::call : Expression::Kind::name, token);
      if (is("(")) arguments(*expr);
      return expr;
    }
    fail(token.at, "expected an expression; symbolic writes require 'effect'");
  }
  Expr expression(int minimum = 0, bool attach = true, bool owner_projection = true) {
    const ParseEntry entry(recursion_, peek().at, "an expression");
    return finish(expression_impl(minimum, attach, owner_projection));
  }
  Expr expression_impl(int minimum, bool attach, bool owner_projection) {
    auto left = primary(attach, owner_projection);
    while (true) {
      if (attach && accepts_supplied_body(*left) && (is("{") || here_header())) {
        trailing_body(*left);
        left = finish(std::move(left));
      } else if (is("(")) {
        auto call = make(Expression::Kind::value_call, peek());
        call->children.push_back(std::move(left));
        arguments(*call);
        left = finish(std::move(call));
      } else if (take(".")) {
        auto member = make(Expression::Kind::field, name(TokenKind::identifier));
        member->children.push_back(std::move(left));
        left = finish(std::move(member));
      } else if (owner_projection && take("->")) {
        auto owner = make(Expression::Kind::capture_owner, ordinary_name());
        owner->children.push_back(std::move(left));
        left = finish(std::move(owner));
      } else if (is("[")) {
        auto index = make(Expression::Kind::index, expect("["));
        index->children.push_back(std::move(left));
        index->children.push_back(expression());
        expect("]");
        left = finish(std::move(index));
      } else break;
    }
    while ((peek().kind == TokenKind::punctuation || is("is")) && binary_precedence(peek().text) >= minimum) {
      auto op = tokens_[cursor_++];
      if (op.text == "is" && take("not")) op.text = "is not";
      auto binary = make(Expression::Kind::binary, op);
      binary->children.push_back(std::move(left));
      binary->children.push_back(expression(binary_precedence(op.text) + 1, attach, owner_projection));
      left = finish(std::move(binary));
    }
    if (minimum == 0 && swap_operation(peek().text)) {
      auto swap = make(Expression::Kind::call, tokens_[cursor_++]);
      swap->children.push_back(std::move(left));
      swap->children.push_back(expression(1, attach, owner_projection));
      if (swap_operation(peek().text)) fail(peek().at, "swap operators cannot be chained");
      return swap;
    }
    if (minimum == 0 && left->kind == Expression::Kind::index && (is("=") || is("<-"))) {
      auto assignment = make(Expression::Kind::call, tokens_[cursor_++]);
      assignment->text = "[]" + assignment->text;
      assignment->children = std::move(left->children);
      assignment->children.push_back(expression(0, attach, owner_projection));
      return assignment;
    }
    if (minimum == 0 && is("<-")) {
      auto transfer = make(Expression::Kind::transfer, expect("<-"));
      transfer->children.push_back(std::move(left));
      transfer->children.push_back(expression(0, attach, owner_projection));
      return transfer;
    }
    if (minimum == 0 && is("=")) {
      auto rebind = make(Expression::Kind::rebind, expect("="));
      rebind->children.push_back(std::move(left));
      rebind->children.push_back(expression(0, attach, owner_projection));
      return rebind;
    }
    return left;
  }
  Expr block() {
    const ParseEntry entry(recursion_, peek().at, "a block");
    return finish(block_impl());
  }
  Expr block_impl() {
    auto result = make(Expression::Kind::block, expect("{"));
    result->scoped = is("{") && peek().at.line == result->at.line &&
        peek().at.column == result->at.column + 1;
    if (result->scoped) expect("{");
    while (!take("}")) {
      if (peek().kind == TokenKind::end) fail(peek().at, "unterminated scope");
      result->children.push_back(expression());
      if (!is("}")) expect(";");
      else take(";");
    }
    if (result->scoped) expect("}");
    return result;
  }

  static void normalize_assignment_calls(Expression &expr) {
    using K = Expression::Kind;
    if (expr.kind == K::call && !expr.has_here_body() && (expr.text == "=" || expr.text == "<-")) {
      if (expr.children.size() != 2) fail(expr.at, "assignment requires a destination and a source");
      expr.kind = expr.text == "=" ? K::rebind : K::transfer;
    }
    for (const auto &child : expr.children) normalize_assignment_calls(*child);
    if (expr.definition && expr.definition->body) normalize_assignment_calls(*expr.definition->body);
  }
  static void normalize_full_scopes(Expression &expr, Form::Kind callable_kind) {
    using K = Expression::Kind;
    const bool delayed_scope = expr.kind == K::deferred && expr.children.front()->kind == K::block &&
        expr.children.front()->scoped;
    if (expr.kind == K::here_body && expr.scoped && expr.receiving->shape == ReceivingHeader::Shape::ordered) {
      auto definition = std::make_shared<Form>();
      definition->kind = callable_kind == Form::Kind::procedure ? Form::Kind::procedure : Form::Kind::function;
      definition->at = expr.at;
      definition->supplied_scope = true;
      for (const auto &item : expr.receiving->items) {
        Parameter parameter{item.name, item.annotation.value_or(Type{Type::void_type}), item.at, false, {}};
        parameter.declaration_kind = item.declaration_kind;
        parameter.owner = item.owner;
        parameter.read_only = item.read_only;
        parameter.fixed_binding = item.fixed_binding;
        definition->parameters.push_back(std::move(parameter));
      }
      definition->body = std::move(expr.children.front());
      expr.children.clear();
      expr.definition = std::move(definition);
    }
    const auto nested_kind = expr.kind == K::block && expr.scoped && callable_kind != Form::Kind::function
        ? Form::Kind::procedure : callable_kind;
    for (const auto &child : expr.children) normalize_full_scopes(*child, nested_kind);
    if (expr.definition && expr.definition->body)
      normalize_full_scopes(*expr.definition->body, expr.definition->kind);
    if (delayed_scope) {
      // Keep block-defer preparation delayed after its full scope becomes a call.
      auto body = std::make_unique<Expression>();
      body->kind = K::block; body->at = expr.children.front()->at;
      body->children.push_back(std::move(expr.children.front()));
      expr.children.front() = std::move(body);
    }
    if (expr.kind != K::block || !expr.scoped) return;
    auto definition = std::make_shared<Form>();
    definition->kind = callable_kind == Form::Kind::function ? Form::Kind::function : Form::Kind::procedure;
    definition->at = expr.at;
    definition->implicit_scope = true;
    definition->body = std::make_unique<Expression>(std::move(expr));
    definition->body->scoped = false;
    expr = Expression{};
    expr.kind = K::value_call; expr.at = definition->at;
    auto callable = std::make_unique<Expression>();
    callable->kind = K::callable; callable->at = definition->at;
    callable->definition = std::move(definition);
    expr.children.push_back(std::move(callable));
  }
  static void validate_explicit_scopes(const Expression &expr, bool iteration = false, bool deferred = false,
                                       const Location *draining = nullptr, const ReceivingHeader *handoff = nullptr) {
    using K = Expression::Kind;
    if ((expr.kind == K::block || expr.kind == K::here_body) && expr.scoped) {
      iteration = deferred = false;
      draining = nullptr;
      handoff = nullptr;
    }
    if (expr.kind == K::here_body && !expr.scoped) handoff = expr.receiving.get();
    if (expr.kind == K::break_value || expr.kind == K::continue_value) {
      if (!handoff) fail(expr.at, "'" + expr.text + "' requires an active single-brace here body");
      if (handoff->from_for && expr.kind == K::break_value && !expr.children.empty())
        fail(expr.at, "a generator for loop requires bare 'break'");
    }
    if (expr.kind == K::deferred && draining)
      fail(expr.at, "defer cannot register work into its currently draining scope\n" +
          location_text(*draining) + ": note: enclosing deferred execution begins here");
    const bool declaration = expr.kind == K::pointer_group || expr.kind == K::region_binding ||
        expr.kind == K::alias || (expr.kind == K::callable && !expr.definition->name.empty());
    if (deferred && declaration)
      fail(expr.at, "a deferred-block variable declaration requires a nested explicit {{ ... }} scope");
    if (deferred && (expr.kind == K::effect || expr.kind == K::effect_group || expr.kind == K::procedure_effect))
      fail(expr.at, "effect installation in a deferred block requires a nested explicit {{ ... }} scope");
    if (iteration && declaration)
      fail(expr.at, "a loop-body variable declaration requires a nested per-iteration {{ ... }} scope");
    if (handoff && declaration)
      fail(expr.at, "a here-body variable declaration requires a nested explicit {{ ... }} scope");
    for (size_t i = 0; i < expr.children.size(); ++i) {
      const bool loop_body = (expr.kind == K::range_for && i >= 2) ||
                            (expr.kind == K::fold && i == 3);
      const bool deferred_body = expr.kind == K::deferred && expr.children.front()->kind == K::block;
      validate_explicit_scopes(*expr.children[i], loop_body || iteration, deferred_body || deferred,
          deferred_body ? &expr.at : draining, loop_body || deferred_body ? nullptr : handoff);
    }
    // A callable body executes in its own call scope, independently of where
    // its value is constructed. Loops inside it still need explicit scopes.
    if (expr.definition && expr.definition->body)
      validate_explicit_scopes(*expr.definition->body);
  }

public:
  Parser(const std::string &source, const std::string &file)
      : tokens_(tokenize(source, file)), right_paren_(tokens_.size(), tokens_.size()) {
    std::vector<size_t> open;
    for (size_t i = 0; i < tokens_.size(); ++i) if (tokens_[i].kind == TokenKind::punctuation) {
      if (tokens_[i].text == "(") open.push_back(i);
      if (tokens_[i].text == ")" && !open.empty()) {
        right_paren_[open.back()] = i;
        open.pop_back();
      }
    }
  }

  std::vector<std::unique_ptr<Form>> parse() {
    std::vector<std::unique_ptr<Form>> forms;
    while (peek().kind != TokenKind::end) {
      auto start = peek();
      auto form = std::make_unique<Form>();
      form->at = start.at;
      if (take("export")) {
        if (is("import")) {
          form->exporting = true;
        } else {
          form->kind = Form::Kind::export_resource;
          form->symbolic = peek().kind == TokenKind::symbol;
          form->name = form->symbolic ? name(TokenKind::symbol).text : lexical_name(&form->operation).text;
          expect(";");
          forms.push_back(std::move(form));
          continue;
        }
      }
      const bool region_symbol = is("region") && is("symbol", 1);
      if (region_symbol) expect("region");
      if (take("symbol")) {
        if (take("from")) {
          if (region_symbol) fail(start.at, "use 'symbol from' to select a symbol identity; 'region symbol' declares one");
          form->kind = Form::Kind::symbol_from;
          resource(*form, true);
        } else {
          std::vector<std::pair<Form *, std::string>> declarations;
          do {
            auto declaration = std::make_unique<Form>();
            declaration->kind = Form::Kind::symbol;
            const auto token = name(peek().kind == TokenKind::symbol ? TokenKind::symbol : TokenKind::identifier);
            declaration->name = token.text;
            declaration->at = token.at;
            if (region_symbol) {
              declaration->result_type = region_annotation(false);
            } else {
              if (!is("::") && !is(":")) fail(peek().at, "a symbol declaration requires an explicit type");
              type_separator();
              declaration->result_type = parse_type();
            }
            const auto owner = take("in") ? ordinary_name().text : std::string{};
            declarations.emplace_back(declaration.get(), owner);
            forms.push_back(std::move(declaration));
          } while (take(","));
          if (take("where")) {
            expect("{"); expect("region");
            auto owner = ordinary_name();
            type_separator();
            auto constraint = std::make_shared<SymbolOwner>(SymbolOwner{owner.text, parse_type(), owner.at});
            expect("}");
            for (auto &[declaration, name] : declarations) {
              if (region_symbol || name != owner.text)
                fail(declaration->at, "each object symbol must name the declared region owner");
              declaration->symbol_owner = constraint;
            }
          } else {
            for (const auto &[declaration, owner] : declarations)
              if (!owner.empty()) fail(declaration->at, "symbol owner requires a where {region ...} declaration");
          }
          expect(";");
          continue;
        }
        expect(";");
      } else if (take("unit")) {
        form->kind = Form::Kind::unit;
        form->name = ordinary_name().text;
        expect(";");
      } else if (take("array")) {
        form->kind = Form::Kind::array;
        form->name = ordinary_name().text;
        expect("[");
        form->index_type = parse_type();
        expect("]");
        type_separator();
        form->result_type = parse_type();
        expect(";");
      } else if (is("type") || is("data") || is("regional") ||
                 (is("region") && tokens_[cursor_ + 1].kind == TokenKind::identifier && is("{", 2))) {
        if (is("regional")) fail(start.at, "regional type is not a declaration; use data for a regional representation");
        const bool region = take("region");
        const bool data = region || take("data");
        if (!data) expect("type");
        form->kind = region ? Form::Kind::region_definition : Form::Kind::data;
        form->name = ordinary_name().text;
        if (take("<:")) form->parent_type = parse_type();
        if (!is("{")) {
          if (data && !form->parent_type)
            fail(start.at, "a head-only data declaration requires an abstract parent");
          form->kind = data ? Form::Kind::conformance : Form::Kind::abstract_type;
          expect(";");
          forms.push_back(std::move(form));
          continue;
        }
        if (!data)
          fail(start.at, "abstract type declarations have no body; use data for a representation");
        expect("{");
        while (!take("}")) {
          const bool fixed_binding = take("const");
          if (is("region") || (is("owned") && is("region", 1)))
            fail(peek().at, "region handles cannot be stored in user data fields");
          std::string mode;
          if (is("ref") || is("ptr") || is("val")) mode = tokens_[cursor_++].text;
          auto field = name(TokenKind::identifier);
          type_separator();
          const bool read_only = take("const");
          auto type = parse_type();
          if (type == Type::void_type) fail(field.at, "a product field must have a value type");
          form->parameters.push_back({field.text, type, field.at, false, {}});
          form->parameters.back().declaration_kind = mode;
          form->parameters.back().fixed_binding = fixed_binding;
          form->parameters.back().read_only = read_only;
          if (!is("}")) expect(";");
        }
        take(";");
      } else if (!callable_binding() && (is("function") || is("func") || is("procedure") || is("proc") || is("conversion"))) {
        callable(*form);
      } else if (take("global")) {
        form->kind = Form::Kind::global;
        form->name = ordinary_name().text;
        type_separator();
        form->result_type = parse_type();
        if (*form->result_type == Type::void_type) fail(start.at, "a bits global cannot be void");
        expect("=");
        form->body = expression();
        const auto *root = form->body.get();
        while (root->kind == Expression::Kind::grouped) root = root->children.front().get();
        const auto kind = root->kind;
        const bool negative_literal = kind == Expression::Kind::unary && root->text == "-" &&
                                      (root->children.front()->kind == Expression::Kind::integer ||
                                       root->children.front()->kind == Expression::Kind::real);
        if (kind != Expression::Kind::integer && kind != Expression::Kind::real && kind != Expression::Kind::boolean &&
            kind != Expression::Kind::call && !negative_literal)
          fail(start.at, "a bits global initializer must be a bits literal or function call");
        expect(";");
      } else if (is("using") || is("import") || is("mixin")) {
        const bool using_resource = take("using");
        const bool importing = !using_resource && take("import");
        if (!using_resource && !importing) expect("mixin");
        form->kind = using_resource ? Form::Kind::using_resource :
                     importing ? Form::Kind::import_resource : Form::Kind::mixin_resource;
        if (take("units")) {
          if (!using_resource) fail(start.at, "UoMs support fresh definitions and using, not import or mixin");
          form->kind = Form::Kind::using_units;
          form->path = name(TokenKind::string).text;
          expect("{");
          do {
            auto source = name(TokenKind::identifier);
            auto local = take("as") ? name(TokenKind::identifier).text : source.text;
            form->units.push_back({source.text, local, source.at});
          } while (take(","));
          expect("}");
          expect(";");
          forms.push_back(std::move(form));
          continue;
        }
        if (take("symbol")) {
          if (!using_resource) fail(start.at, "combined symbol selection requires using; declare its destination first");
          form->using_symbol = true;
        }
        resource(*form, form->using_symbol);
        if (take("with")) {
          if (form->kind != Form::Kind::mixin_resource)
            fail(start.at, "a symbol map requires a mixed-in procedure");
          expect("{");
          while (!take("}")) {
            auto local = name(TokenKind::symbol);
            expect("as");
            auto source = name(TokenKind::symbol);
            form->mappings.push_back({local.text, source.text, local.at});
            expect(";");
          }
        }
        expect(";");
      } else {
        if (is("let")) fail(peek().at, "module values require an explicit bits global declaration");
        form->kind = Form::Kind::expression;
        form->body = expression();
        expect(";");
      }
      forms.push_back(std::move(form));
      depths_.clear();
    }
    for (const auto &form : forms) if (form->body) {
      validate_explicit_scopes(*form->body);
      normalize_assignment_calls(*form->body);
      normalize_full_scopes(*form->body, form->kind);
    }
    return forms;
  }
};

} // namespace cod::language
