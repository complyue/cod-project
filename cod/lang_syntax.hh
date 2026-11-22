//===--- lang_syntax.hh - CoD source-language prototype syntax ------===//
#pragma once

#include "lang_source.hh"

#include <cod_utf8.h>
#include <algorithm>
#include <cctype>
#include <charconv>
#include <compare>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cod::language {

[[noreturn]] inline void fail(const Location &at, const std::string &message) {
  throw std::runtime_error(location_text(at) + ": " + message);
}

enum class TokenKind { identifier, symbol, integer, real, string, punctuation, end };
struct Token {
  TokenKind kind;
  std::string text;
  Location at;
};

struct PipelineOperator {
  enum class Kind { composition, source, consumer } kind;
  bool reversed;

  std::string_view canonical_spelling() const {
    switch (kind) {
    case Kind::composition: return "|>";
    case Kind::source: return "!>";
    case Kind::consumer: return "$>";
    }
    throw std::logic_error("unknown pipeline operator kind");
  }
};

inline std::optional<PipelineOperator> pipeline_operator(std::string_view spelling) {
  using K = PipelineOperator::Kind;
  if (spelling == "|>" || spelling == "<|") return PipelineOperator{K::composition, spelling == "<|"};
  if (spelling == "!>" || spelling == "<!") return PipelineOperator{K::source, spelling == "<!"};
  if (spelling == "$>" || spelling == "<$") return PipelineOperator{K::consumer, spelling == "<$"};
  return {};
}

inline std::vector<Token> tokenize(const std::shared_ptr<const IndexedSource> &revision) {
  const auto source = revision->bytes();
  std::vector<Token> tokens;
  size_t cursor = 0;
  const auto emit = [&](TokenKind kind, std::string text, size_t first) {
    tokens.push_back({kind, std::move(text), {{}, revision, {first, cursor}}});
  };
  auto identifier = [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; };
  while (cursor < source.size()) {
    const char c = source[cursor];
    if (std::isspace(static_cast<unsigned char>(c))) {
      ++cursor;
      continue;
    }
    if (c == '#') {
      while (cursor < source.size() && source[cursor] != '\n')
        ++cursor;
      continue;
    }
    if (c == '/' && cursor + 1 < source.size() && source[cursor + 1] == '/')
      fail({{}, revision, {cursor, cursor + 2}}, "'//' is reserved for fraction construction; use '#' for line comments");
    if (source.compare(cursor, 2, "::") == 0)
      fail({{}, revision, {cursor, cursor + 2}}, "use a single colon for type annotations, or -> for callable results");
    const auto offset = cursor;
    const Location start{{}, revision, {offset, offset + 1}};
    if (c == '@' || std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
      const bool symbol = c == '@';
      if (symbol) {
        ++cursor;
        if (cursor == source.size() ||
            !(std::isalpha(static_cast<unsigned char>(source[cursor])) || source[cursor] == '_'))
          fail(start, "expected a symbol name after '@'");
      }
      const auto name_start = cursor;
      while (cursor < source.size() && identifier(source[cursor]))
        ++cursor;
      emit(symbol ? TokenKind::symbol : TokenKind::identifier,
           std::string(source.substr(name_start, cursor - name_start)), offset);
      continue;
    }
    if (std::isdigit(static_cast<unsigned char>(c))) {
      while (cursor < source.size() && std::isdigit(static_cast<unsigned char>(source[cursor])))
        ++cursor;
      bool real = false;
      if (cursor + 1 < source.size() && source[cursor] == '.' &&
          std::isdigit(static_cast<unsigned char>(source[cursor + 1]))) {
        real = true;
        ++cursor;
        while (cursor < source.size() && std::isdigit(static_cast<unsigned char>(source[cursor]))) ++cursor;
      }
      if (cursor < source.size() && (source[cursor] == 'e' || source[cursor] == 'E')) {
        real = true;
        ++cursor;
        if (cursor < source.size() && (source[cursor] == '+' || source[cursor] == '-')) ++cursor;
        if (cursor == source.size() || !std::isdigit(static_cast<unsigned char>(source[cursor])))
          fail(start, "expected exponent digits");
        while (cursor < source.size() && std::isdigit(static_cast<unsigned char>(source[cursor]))) ++cursor;
      }
      if (source.compare(cursor, 3, "f32") == 0 || source.compare(cursor, 3, "f64") == 0) {
        real = true;
        cursor += 3;
      }
      if (!real && source.compare(cursor, 2, "u8") == 0) cursor += 2;
      else if (!real && (source.compare(cursor, 3, "u32") == 0 || source.compare(cursor, 3, "u64") == 0)) {
        cursor += 3;
      }
      emit(real ? TokenKind::real : TokenKind::integer, std::string(source.substr(offset, cursor - offset)), offset);
      continue;
    }
    if (c == '"') {
      ++cursor;
      bool escaped = false;
      while (cursor < source.size()) {
        const auto next = source[cursor];
        if (next == '\n') fail(start, "newline in quoted text");
        if (!escaped && next == '"') break;
        escaped = !escaped && next == '\\';
        ++cursor;
      }
      if (cursor == source.size()) fail(start, "unterminated quoted text");
      const auto raw = source.substr(offset + 1, cursor - offset - 1);
      ++cursor;
      emit(TokenKind::string, std::string(raw), offset);
      continue;
    }
    std::string punctuation(1, c);
    if (source.compare(cursor, 3, "...") == 0 || source.compare(cursor, 3, "<=>") == 0 ||
        source.compare(cursor, 3, "<->") == 0) {
      const auto triple = source.substr(cursor, 3);
      cursor += 3;
      emit(TokenKind::punctuation, std::string(triple), offset);
      continue;
    }
    if (cursor + 1 < source.size()) {
      const auto pair = source.substr(cursor, 2);
      if (pair == "->" || pair == "==" || pair == "!=" || pair == "<=" || pair == ">=" ||
          pair == "&&" || pair == "||" || pair == "<-" || pair == "<:" || pair == ".." || pair == "=>" ||
          pipeline_operator(pair)) {
        punctuation = pair;
        ++cursor;
      }
    }
    if (std::string("(){}[];,:.+-*/%<>=!|").find(c) == std::string::npos && punctuation.size() != 2)
      fail(start, "unsupported character in CoD source");
    ++cursor;
    emit(TokenKind::punctuation, std::move(punctuation), offset);
  }
  emit(TokenKind::end, "", cursor);
  return tokens;
}

enum class ArgumentMode { value, borrowed_read, borrowed_ref, borrowed_ptr, borrowed_handle };

struct Type {
  enum Kind { integer, uint8, uint32, uint64, boolean, void_type, procedure, float32, float64,
              named, unit, quantity, product, region_schema, region, regional, array, reference, null_ref, abstract_type, arguments, parameter,
              exception, never, c_type_marker, native_handle, exception_holder, enumeration, string, c_string };
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
inline bool text_type(const Type &type) { return type.kind == Type::string || type.kind == Type::c_string; }
inline bool widens_text(const Type &from, const Type &to) { return from.kind == Type::c_string && to.kind == Type::string; }
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
  case Type::string: return "String";
  case Type::c_string: return "CString";
  case Type::exception_holder: return "exception :" + type_name(*type.target);
  case Type::native_handle: return (type.owned ? "own handle " : "handle ") + type.name;
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
  case Type::named: case Type::product: case Type::region_schema: case Type::regional: case Type::array:
  case Type::abstract_type: case Type::c_type_marker: case Type::enumeration: return type.name;
  case Type::region: return std::string(type.owned ? "own " : "") + "region" +
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
struct PipelineEndpoints {
  struct Connection {
    Token spelling;
    size_t operand;
  };
  // All indices refer to operands in written order, before any supplied body.
  size_t middle;
  std::optional<Connection> source, consumer;
};

struct Expression {
  enum class Kind { integer, real, boolean, name, symbol, unary, binary, call, value_call, perform, behave, effect,
                    block, grouped, conditional, return_test, field, range_for, index, null_ref,
                    try_scope, handler, raise, return_value, deferred, make_region, touch, effect_group, transfer, procedure_effect,
                    pointer_group, pointer, rebind, region_binding, callable, alias, capture_owner,
                    here_body, named_argument, break_value, continue_value, typed_expression, return_tail,
                    processor_composition, pipeline, here_binding, make_object, native_handle_binding, make_owned_handle, make_borrowed_handle,
                    exception_binding, make_exception, rethrow_from, case_expression, case_arm, string_literal,
                    case_break, case_fallthrough, case_failure };
  enum class ExceptionPlacement { default_region, capacity, cause };
  Kind kind;
  Location at;
  std::string text;
  std::vector<std::unique_ptr<Expression>> children;
  std::optional<size_t> symbol_id;
  std::string binding;
  bool spread = false;
  std::optional<Location> checked_ref_argument;
  std::optional<Type> annotation;
  std::shared_ptr<Form> definition = {};
  bool intrinsic = false;
  bool fixed_binding = false;
  bool read_only = false;
  bool scoped = false;
  bool case_resumes = false;
  bool case_breaks = false;
  bool grouped_deferred_call = false;
  bool repeated_defer_registration = true;
  std::string declaration_kind = {};
  std::string owner_name = {};
  std::shared_ptr<ReceivingHeader> receiving = {};
  std::unique_ptr<PipelineEndpoints> endpoints = {};
  bool initializer_block = false;
  SourceSpan span = {};
  bool constructor_arguments = false;
  bool construction_type_query = false;
  bool construction_label = false;
  // An explicit placement operand follows the initializer operands in children.
  ExceptionPlacement exception_placement = ExceptionPlacement::default_region;
  const Expression *construction_initializer() const {
    const auto index = construction_type_query ? 1u : 0u;
    return initializer_block && children.size() > index && children[index]->kind == Kind::block
        ? children[index].get() : nullptr;
  }
  std::span<const std::unique_ptr<Expression>> initializer_items() const {
    if (kind != Kind::make_object) return children;
    const auto *block = construction_initializer();
    if (!block) return {};
    return block->children;
  }
  const Expression *initializer_field_label() const {
    if (kind == Kind::field && !binding.empty()) return this;
    if (kind != Kind::transfer && kind != Kind::rebind) return nullptr;
    const auto *label = children.front().get();
    while (label->kind == Kind::grouped) label = label->children.front().get();
    return label->construction_label ? label : nullptr;
  }
  // A supplied body is stored after the written operands. It binds slot one.
  bool has_here_body() const { return !children.empty() && children.back()->kind == Kind::here_body; }
  bool inspects_return(bool binds_payload = false) const;
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
struct OwnerParameter {
  std::string name;
  std::optional<Type> root;
  Location at;
  bool explicit_parameter = false, read_only = false;
};
struct SymbolOwner { std::string name; Type root; Location at; };
struct SymbolReference {
  bool nullable;
  bool read_only;
};
struct UnitSelection { std::string source, local; Location at; };
struct SymbolMapping {
  std::string local;
  std::string source;
  Location at;
};
struct Capture { std::string name; Location at; bool typed_name_only = false; };
struct EnumMember {
  Token name;
  std::optional<Token> initializer;
  bool negative = false;
};
struct Form {
  enum class Kind { symbol, symbol_from, function, procedure, data, region_definition, array, unit, using_units, global,
                    using_resource, import_resource, mixin_resource, export_resource, export_from_resource,
                    expression, abstract_type, conformance, enumeration, native_handle_type };
  Kind kind;
  Location at;
  std::string name;
  std::string path;
  std::string alias;
  bool symbolic = false;
  bool explicit_alias = false;
  bool using_symbol = false;
  bool exporting = false;
  bool owning_handle_symbol = false;
  bool conversion = false;
  bool operation = false;
  bool alias_operation = false;
  bool c_data = false;
  bool c_linkage = false;
  std::optional<Type> native_owner_type;
  std::vector<SymbolMapping> mappings;
  std::vector<UnitSelection> units;
  std::vector<EnumMember> enum_members;
  std::optional<std::vector<Token>> selected_members; // An empty list selects every member spelling.
  std::vector<Parameter> parameters;
  std::vector<Capture> captures;
  bool explicit_captures = false;
  bool implicit_scope = false;
  bool case_scan_scope = false;
  bool supplied_scope = false;
  bool supplied_group = false;
  std::vector<TypeParameter> type_parameters;
  std::vector<Type> throws_types;
  std::vector<OwnerParameter> owner_parameters;
  std::shared_ptr<SymbolOwner> symbol_owner;
  std::optional<SymbolReference> symbol_reference;
  std::string result_owner;
  bool result_const = false;
  std::string result_kind;
  std::optional<Type> result_type;
  std::optional<Type> index_type;
  std::optional<Type> parent_type;
  Expr body;
  SourceSpan span = {};

  bool publishes_definition() const {
    return exporting && (kind == Kind::symbol || kind == Kind::data || kind == Kind::abstract_type || kind == Kind::native_handle_type || kind == Kind::array || kind == Kind::enumeration ||
                         kind == Kind::function || kind == Kind::procedure);
  }
  bool requires_explicit_return() const { return kind == Kind::procedure && !implicit_scope && !supplied_scope; }
  bool returns_tail_value() const { return !supplied_group && (kind == Kind::function || implicit_scope || supplied_scope); }
  bool completion_requires_return(const Type &result) const {
    return !supplied_group && (requires_explicit_return() || result != Type::void_type);
  }
};

inline bool Expression::inspects_return(bool binds_payload) const {
  if (kind == Kind::return_test) return !binds_payload || !binding.empty();
  if (definition && definition->supplied_group && definition->body->inspects_return(binds_payload)) return true;
  return std::ranges::any_of(children, [&](const auto &child) { return child->inspects_return(binds_payload); });
}

inline bool swap_operation(std::string_view op) { return op == "<=>" || op == "<->"; }
inline bool identity_operation(std::string_view op) { return op == "is" || op == "is not"; }

inline constexpr int endpoint_precedence = 1;
inline constexpr int composition_precedence = 2;
inline constexpr int unary_precedence = 9;

inline int binary_precedence(const std::string &op) {
  if (op == "||") return 3;
  if (op == "&&") return 4;
  if (op == "==" || op == "!=" || identity_operation(op)) return 5;
  if (op == "<" || op == ">" || op == "<=" || op == ">=") return 6;
  if (op == "+" || op == "-") return 7;
  if (op == "*" || op == "/" || op == "%") return 8;
  return -1;
}

inline bool intrinsic_name(std::string_view name) {
  return name == "region" || name == "deref" || name == "scalar" || name == "i64" || name == "u8" ||
         name == "u32" || name == "u64" || name == "f32" || name == "f64";
}

inline bool explicit_query_intrinsic_name(std::string_view name) {
  return name == "len" || name == "used_bytes" || name == "capacity_bytes";
}

inline bool expression_keyword(std::string_view name) {
  return name == "true" || name == "false" || name == "null" || name == "is" || name == "not" ||
         name == "for" || name == "case" || name == "var" || name == "effect" ||
         name == "perform" || name == "behave" || name == "if" || name == "procedure" ||
         name == "function" || name == "func" || name == "proc" || name == "global" || name == "symbol" ||
         name == "try" || name == "catch" || name == "finally" || name == "throw" || name == "return" || name == "defer" ||
         name == "val" || name == "ref" || name == "make" || name == "touch" || name == "ptr" || name == "typeof" || name == "intrinsic" || name == "const";
}

inline bool lexical_module_name(std::string_view name) {
  return name == "type" || name == "data" || name == "unit" || name == "array";
}

inline bool module_keyword(std::string_view name) {
  return lexical_module_name(name) ||
         name == "using" || name == "import" || name == "mixin" || name == "export" || name == "conversion";
}

class Parser {
  enum class HereAttachment { immediate, for_body, pipeline_body, initializer_operand };
  std::vector<Token> tokens_;
  size_t cursor_ = 0;
  static constexpr size_t structure_limit = 128;
  static constexpr size_t recursion_limit = 1024;
  size_t recursion_ = 0;
  std::map<const Expression *, size_t> depths_;
  std::vector<size_t> right_paren_;
  bool parsing_handoff_ = false;
  bool parsing_case_ = false;
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
    if (!depths_.contains(expression.get()))
      expression->span.past = std::max(expression->span.past, tokens_[cursor_ - 1].at.span.past);
    std::vector<std::pair<Expression *, bool>> work{{expression.get(), false}};
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
      for (const auto &child : node->children) {
        depth = std::max(depth, depths_.at(child.get()) + 1);
        node->span.first = std::min(node->span.first, child->span.first);
        node->span.past = std::max(node->span.past, child->span.past);
      }
      const auto include_type = [&](const Type &type) { depth = std::max(depth, type_depth(type, node->at) + 1); };
      if (node->annotation) include_type(*node->annotation);
      if (node->receiving) for (const auto &item : node->receiving->items)
        if (item.annotation) include_type(*item.annotation);
      if (node->definition) {
        auto &definition = *node->definition;
        definition.span = node->span;
        if (definition.body) {
          depth = std::max(depth, depths_.at(definition.body.get()) + 1);
          definition.span.past = std::max(definition.span.past, definition.body->span.past);
          node->span.past = definition.span.past;
        }
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
  bool ownership_qualifier(const std::string &resource, size_t ahead = 0) const {
    return is("own", ahead) && is(resource, ahead + 1);
  }
  bool exception_binding_head(size_t ahead = 0) const {
    return ownership_qualifier("exception", ahead) ||
           (is("exception", ahead) && tokens_[cursor_ + ahead + 1].kind == TokenKind::identifier && is(":", ahead + 2));
  }
  void expect_exception_binder() {
    take("own");
    expect("exception");
  }
  static Type exception_holder(Type object) {
    Type holder{Type::exception_holder};
    holder.target = std::make_shared<Type>(std::move(object));
    return holder;
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
  std::optional<PipelineOperator> next_pipeline_operator() const {
    return peek().kind == TokenKind::punctuation ? pipeline_operator(peek().text) : std::nullopt;
  }
  static bool accepts_supplied_body(const Expression &expr) {
    using K = Expression::Kind;
    return !expr.intrinsic && (expr.kind == K::call || expr.kind == K::value_call ||
        expr.kind == K::perform || expr.kind == K::behave ||
        expr.kind == K::processor_composition || expr.kind == K::pipeline);
  }
  Token name(TokenKind kind) {
    if (peek().kind != kind)
      fail(peek().at, kind == TokenKind::symbol ? "expected a symbol name" : "expected a lexical name");
    return tokens_[cursor_++];
  }
  Token ordinary_name(bool lexical_value = false) {
    auto token = name(TokenKind::identifier);
    if (expression_keyword(token.text) || (module_keyword(token.text) && !(lexical_value && lexical_module_name(token.text))) ||
        intrinsic_name(token.text) ||
        token.text == "bool" || token.text == "void" || token.text == "ref")
      fail(token.at, "reserved keyword '" + token.text + "' cannot be used as an ordinary name");
    return token;
  }
  Token value_name() {
    if (expression_keyword(peek().text))
      fail(peek().at, "reserved expression name '" + peek().text + "' cannot declare a lexical value");
    return ordinary_name(true);
  }
  static bool operation_token(const Token &token) {
    return (token.kind == TokenKind::identifier && (intrinsic_name(token.text) || token.text == "not" || identity_operation(token.text))) ||
           (token.kind == TokenKind::punctuation &&
            (binary_precedence(token.text) >= 0 || token.text == "!" || token.text == "=" || token.text == "<-" ||
             swap_operation(token.text) || pipeline_operator(token.text)));
  }
  Token lexical_name(bool *operation = nullptr, bool explicit_intrinsic = false) {
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
    } else if (!operation_token(op) &&
               !(explicit_intrinsic && op.kind == TokenKind::identifier && explicit_query_intrinsic_name(op.text)))
      fail(op.at, "expected an operator or intrinsic name");
    if (op.text == "&&" || op.text == "||") fail(op.at, "short-circuit operators are control expressions");
    expect(")");
    return op;
  }
  void resource(Form &form, bool symbolic) {
    form.path = module_path();
    resource_selection(form, symbolic);
  }
  std::string module_path() {
    if (peek().kind != TokenKind::string) fail(peek().at, "expected a quoted module path");
    const auto token = name(TokenKind::string);
    std::string result;
    for (size_t i = 0; i < token.text.size(); ++i) {
      auto byte = token.text[i];
      if (byte == '\\') {
        byte = token.text[++i];
        if (byte != '\\' && byte != '"') fail(token.at, "module paths support only escaped backslash or quote");
      }
      result.push_back(byte);
    }
    return result;
  }
  static std::string text_literal(const Token &token) {
    std::string result;
    const auto raw = std::string_view(token.text);
    for (size_t i = 0; i < raw.size(); ++i) {
      const auto byte = raw[i];
      if (byte == '\r') fail(token.at, "newline in text literal");
      if (byte != '\\') { result.push_back(byte); continue; }
      switch (raw[++i]) {
      case '"': case '\\': result.push_back(raw[i]); break;
      case 'n': result.push_back('\n'); break;
      case 'r': result.push_back('\r'); break;
      case 't': result.push_back('\t'); break;
      case '0': result.push_back('\0'); break;
      case 'u': {
        if (i + 1 >= raw.size() || raw[i + 1] != '{') fail(token.at, "expected '{' in Unicode scalar escape");
        const auto first = i + 2;
        const auto close = raw.find('}', first);
        if (close == raw.npos) fail(token.at, "unterminated Unicode scalar escape");
        uint32_t value = 0;
        const auto [end, status] = std::from_chars(raw.data() + first, raw.data() + close, value, 16);
        if (status != std::errc{} || end != raw.data() + close || !cod_unicode_scalar(value))
          fail(token.at, "invalid Unicode scalar escape");
        uint8_t bytes[4];
        const auto width = cod_utf8_encode(value, bytes);
        result.append(reinterpret_cast<const char *>(bytes), width);
        i = close;
        break;
      }
      default: fail(token.at, "unknown text literal escape");
      }
    }
    if (!cod_utf8_valid(reinterpret_cast<const uint8_t *>(result.data()), result.size()))
      fail(token.at, "invalid UTF-8 in text literal");
    return result;
  }
  void resource_selection(Form &form, bool symbolic) {
    form.symbolic = symbolic || peek().kind == TokenKind::symbol;
    const auto kind = form.symbolic ? TokenKind::symbol : TokenKind::identifier;
    form.name = form.symbolic ? name(kind).text : lexical_name(&form.operation).text;
    form.explicit_alias = take("as");
    form.alias = form.explicit_alias ? (form.symbolic ? name(kind).text : lexical_name(&form.alias_operation).text) : form.name;
    if (!form.explicit_alias) form.alias_operation = form.operation;
    if (form.alias_operation && identity_operation(form.alias))
      fail(form.at, "reference identity is an intrinsic operation and cannot be an imported user operation");
  }
  void resource_item(Form &form) {
    resource_selection(form, form.using_symbol);
    if (!form.symbolic && (form.kind == Form::Kind::using_resource || form.kind == Form::Kind::import_resource) && take("{")) {
      auto &members = form.selected_members.emplace();
      if (!take("..")) {
        do { members.push_back(ordinary_name()); } while (take(",") && !is("}"));
      }
      expect("}");
    }
    if (!take("with")) return;
    if (form.kind != Form::Kind::mixin_resource)
      fail(form.at, "a symbol map requires a mixed-in procedure");
    expect("{");
    while (!take("}")) {
      auto local = name(TokenKind::symbol);
      expect("as");
      auto source = name(TokenKind::symbol);
      form.mappings.push_back({local.text, source.text, local.at});
      expect(";");
    }
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
    if (token.text == "region" || (token.text == "own" && is("region")))
      fail(token.at, "region is a declaration keyword; use 'region name:Root' or 'own region name:Root'");
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
    auto type = simple_type(token);
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
  static Type simple_type(const Token &token) {
    Type type{Type::named, token.text};
    if (token.text == "i64") type = Type::integer;
    else if (token.text == "u8") type = Type::uint8;
    else if (token.text == "u32") type = Type::uint32;
    else if (token.text == "u64") type = Type::uint64;
    else if (token.text == "bool") type = Type::boolean;
    else if (token.text == "void") type = Type::void_type;
    else if (token.text == "f32") type = Type::float32;
    else if (token.text == "f64") type = Type::float64;
    else if (token.text == "String") type = Type::string;
    else if (token.text == "CString") type = Type::c_string;
    return type;
  }
  void type_separator() { expect(":"); }
  Expr make(Expression::Kind kind, const Token &token) {
    auto expression = std::make_unique<Expression>();
    expression->kind = kind;
    expression->at = token.at;
    expression->text = token.text;
    expression->span = token.at.span;
    depths_.erase(expression.get());
    return expression;
  }
  Token field_name() {
    if (!take("(")) return name(TokenKind::identifier);
    auto field = field_name();
    expect(")");
    return field;
  }
  bool starts_checked_ref_argument() const {
    if (!is("not") || !is("null", 1) || is("in", 2) || is("is", 2)) return false;
    const auto kind = tokens_[cursor_ + 2].kind;
    return kind == TokenKind::identifier || kind == TokenKind::integer || kind == TokenKind::real ||
           kind == TokenKind::symbol || is("(", 2) || is("{", 2) || is("!", 2);
  }
  void arguments(Expression &expr, bool allow_supplied_body = true) {
    const auto close = right_paren_[cursor_];
    const bool follows_call = allow_supplied_body && close < tokens_.size() &&
        (is("{", close + 1 - cursor_) || here_header(close + 1 - cursor_));
    const bool labelled_yield = labelled_yield_ && expr.kind == Expression::Kind::call && expr.text == "yield";
    const bool named = (expr.kind == Expression::Kind::call && !expr.intrinsic) ||
        expr.kind == Expression::Kind::make_exception || expr.kind == Expression::Kind::make_object ||
        (accepts_supplied_body(expr) && follows_call) || labelled_yield;
    const bool accepts_ref_argument = expr.kind == Expression::Kind::call || expr.kind == Expression::Kind::value_call ||
        expr.kind == Expression::Kind::perform || expr.kind == Expression::Kind::behave;
    expect("(");
    if (!take(")")) {
      do {
        Expr argument;
        if (!named || !is("->")) {
          std::optional<Location> checked_ref;
          if (accepts_ref_argument && starts_checked_ref_argument()) {
            checked_ref = expect("not").at;
            expect("null");
          }
          argument = expression(0, HereAttachment::immediate, !named);
          argument->checked_ref_argument = std::move(checked_ref);
          argument->spread = take("...");
          if (argument->checked_ref_argument && argument->spread)
            fail(*argument->checked_ref_argument, "a checked ref selection supplies one ordinary argument");
        }
        if (named && take("->")) {
          const bool construction = expr.kind == Expression::Kind::make_object || expr.kind == Expression::Kind::make_exception;
          auto selected = make(Expression::Kind::named_argument, construction ? field_name() : value_name());
          if (argument) selected->children.push_back(std::move(argument));
          else if (labelled_yield) fail(selected->at, "a labelled yield operand cannot request a default");
          argument = std::move(selected);
        } else if (labelled_yield) {
          fail(argument->at, "a labelled yield requires a label for every operand");
        }
        expr.children.push_back(std::move(argument));
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
  Type signature_type(bool &read_only, std::string &mode, bool native_handles = false) {
    if (is("ptr") || is("ref") || is("val") || (native_handles && is("handle") && !is("in", 1) && !is("where", 1) &&
        (!is("throws", 1) || is("in", 2)) &&
        tokens_[cursor_ + 1].kind == TokenKind::identifier)) mode = tokens_[cursor_++].text;
    read_only = take("const");
    if ((is("exception") && is(":", 1)) || ownership_qualifier("exception")) {
      if (!mode.empty() || read_only) fail(peek().at, "expected an unqualified exception holder");
      expect_exception_binder();
      expect(":");
      read_only = take("const");
      return exception_holder(parse_type());
    }
    if (is("region") || ownership_qualifier("region")) {
      if (read_only) fail(peek().at, "region target const belongs after the root annotation colon");
      const bool owned = take("own");
      expect("region");
      return region_annotation(owned, &read_only);
    }
    return parse_type();
  }
  Parameter parameter_declaration(bool native_handles = false) {
    const bool fixed_binding = take("const");
    const bool exception = exception_binding_head();
    if (exception) expect_exception_binder();
    const bool owned_region = ownership_qualifier("region");
    if (owned_region) expect("own");
    const bool regional = !exception && take("region");
    std::string mode;
    if (!regional && !exception && (is("ptr") || is("ref") || is("val") || (native_handles && is("handle") && !is(":", 1))))
      mode = tokens_[cursor_++].text;
    auto param = value_name();
    Parameter parameter{param.text, Type::void_type, param.at, false, {}};
    parameter.fixed_binding = fixed_binding;
    if (regional) parameter.type = region_annotation(owned_region, &parameter.read_only);
    else {
      type_separator();
      parameter.read_only = take("const");
      auto object = parse_type(&parameter);
      parameter.type = exception ? exception_holder(std::move(object)) : std::move(object);
    }
    parameter.declaration_kind = mode;
    if (take("in")) parameter.owner = ordinary_name().text;
    if (exception && !parameter.owner.empty()) fail(param.at, "an exception holder establishes its own owner");
    if (mode == "val" && !parameter.owner.empty()) fail(param.at, "a val parameter has callee-local bits storage, not an external owner");
    if (!regional && !exception && mode.empty() && !parameter.owner.empty()) parameter.declaration_kind = "ref";
    if (parameter.type == Type::void_type) fail(param.at, "a parameter must have a value type");
    parameter.variadic = take("...");
    return parameter;
  }
  static void result_kind(const std::string &mode, const std::string &owner, const Location &at) {
    if (mode == "val" && !owner.empty())
      fail(at, "a val result has no owner association; use ref T in O for a borrowed result");
    if ((mode == "ref" || mode == "ptr") && owner.empty())
      fail(at, "a ref/ptr result requires an explicit external owner: use in O");
    if (mode == "handle" && owner.empty())
      fail(at, "a handle result requires an explicit input owner: use in O");
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
          if (!signature.parameters.empty() && !signature.parameters.back().here)
            fail(peek().at, "here callable parameters must precede ordinary parameters");
          signature.parameters.push_back(here_parameter());
        } else signature.parameters.push_back(parameter_declaration());
      } while (take(","));
      expect(")");
    }
    if (is("yields")) add_yield_parameter(signature.parameters, true);
    else {
      expect("->");
      here_result(signature, param.at);
    }
    if (take("throws")) signature.throws_types.push_back(parse_type());
    if (is("...")) fail(peek().at, "a here parameter cannot be variadic");
    return parameter;
  }
  void add_yield_parameter(std::vector<Parameter> &parameters, bool allow_upstream_here) {
    const auto token = expect("yields");
    if (std::ranges::any_of(parameters, [](const auto &parameter) { return parameter.name == "yield"; }))
      fail(token.at, "yields introduces the first parameter 'yield', which is already declared");
    if (!allow_upstream_here && !parameters.empty() && parameters.front().here)
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
    parameters.insert(parameters.begin(), std::move(yield));
  }
  void generator_contract(Form &form) {
    const bool upstream = !form.parameters.empty() && form.parameters.front().here &&
        !form.parameters.front().here->parameters.empty() && form.parameters.front().here->parameters.front().here;
    add_yield_parameter(form.parameters, upstream);
    form.result_type = Type::void_type;
  }
  void generic_clause(Form &form) {
    const bool grouped = take("{");
    do {
      if (is("region") || is("owner") || (is("const") && is("owner", 1))) {
        const bool read_only = take("const");
        const bool regional = take("region");
        if (!regional) expect("owner");
        auto owner = ordinary_name();
        OwnerParameter parameter{owner.text, {}, owner.at, false, read_only};
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
    if (peek().kind == TokenKind::string) {
      const auto linkage = name(TokenKind::string);
      if (linkage.text != "C" || function || inline_effect || expression_form)
        fail(linkage.at, "C linkage requires a module-level proc \"C\" declaration or definition");
      form.c_linkage = true;
    }
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
    const auto pipeline = form.operation ? pipeline_operator(form.name) : std::nullopt;
    if (parameter_list) {
      if (!take(")")) {
        do {
          if (is("here") && !is(":", 1)) {
            if (!pipeline && !form.parameters.empty() && !form.parameters.back().here)
              fail(peek().at, "here callable parameters must precede ordinary parameters");
            form.parameters.push_back(here_parameter());
          } else form.parameters.push_back(parameter_declaration(true));
        } while (take(","));
        expect(")");
      }
    } else if (function) {
      fail(peek().at, "a function requires a parameter list");
    }
    if (pipeline) {
      if (form.parameters.size() != 2) fail(form.at, "a pipeline method requires two explicit operands");
      if (pipeline->reversed) std::swap(form.parameters[0], form.parameters[1]);
      form.name = pipeline->canonical_spelling();
    }
    const bool preceding_where = take("where");
    if (preceding_where) generic_clause(form);
    if (is("yields")) generator_contract(form);
    else if (take("->")) {
      form.result_type = signature_type(form.result_const, form.result_kind, true);
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
    if (declaration && !form.result_type && !form.c_linkage)
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
    if (call.endpoints && call.endpoints->consumer)
      fail(peek().at, "a pipeline can have only one consumer");
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
    auto producer = expression(0, HereAttachment::for_body);
    if (!accepts_supplied_body(*producer)) fail(at, "a generator for loop requires a producer call");
    attach_body(*producer, std::move(header));
    return producer;
  }
  Expr return_expression(const Token &token, bool guard_arm = false) {
    auto expr = make(Expression::Kind::return_value, token);
    if (!is(";") && !is("}") && !(guard_arm && is("else")))
      expr->children.push_back(expression());
    return finish(std::move(expr));
  }

  Expr return_conditional(const Token &opening_if) {
    using K = Expression::Kind;
    const ParseEntry entry(recursion_, opening_if.at, "a return-state conditional");
    auto condition = make(K::return_test, expect("returning"));
    expect("(");
    if (!is(")")) {
      if (take("in")) condition->owner_name = ordinary_name().text;
      else {
        condition->fixed_binding = take("const");
        if (is("val") || is("ref") || is("ptr")) condition->declaration_kind = tokens_[cursor_++].text;
        condition->binding = value_name().text;
        if (take(":")) {
          condition->read_only = take("const");
          condition->annotation = parse_type();
        }
        if (take("in")) condition->owner_name = ordinary_name().text;
      }
    }
    expect(")");
    auto expr = make(K::conditional, opening_if);
    expr->children.push_back(std::move(condition));
    expr->children.push_back(block());
    if (!take("else")) expr->children.push_back(make(K::block, opening_if));
    else if (is("if", 0) && is("returning", 1)) expr->children.push_back(return_conditional(expect("if")));
    else expr->children.push_back(block());
    return finish(std::move(expr));
  }

  void field_initializers(Expression &expr) {
    expect("{");
    while (!take("}")) {
      auto field = make(Expression::Kind::field, field_name());
      if (take("=")) field->binding = "=";
      else { expect("<-"); field->binding = "<-"; }
      field->children.push_back(expression());
      expr.children.push_back(std::move(field));
      if (!is("}")) expect(";");
      else take(";");
    }
  }

  bool starts_field_initializer() const {
    size_t field = 0;
    while (is("(", field)) ++field;
    if (tokens_[cursor_ + field].kind != TokenKind::identifier) return false;
    for (size_t close = 1; close <= field; ++close)
      if (!is(")", field + close)) return false;
    return is("<-", field * 2 + 1) || is("=", field * 2 + 1);
  }
  Expr construction_label() {
    const ParseEntry entry(recursion_, peek().at, "an initializer label");
    if (is("(")) {
      auto group = make(Expression::Kind::grouped, expect("("));
      group->children.push_back(construction_label());
      expect(")");
      return finish(std::move(group));
    }
    auto label = make(Expression::Kind::name, name(TokenKind::identifier));
    label->construction_label = true;
    return finish(std::move(label));
  }

  Expr exception_construction(const Token &token) {
    auto expr = make(Expression::Kind::make_exception, token);
    const auto object = ordinary_name();
    expr->annotation = Type{Type::named, object.text};
    expr->constructor_arguments = is("(");
    if (expr->constructor_arguments) arguments(*expr, false);

    Expr placement;
    if (take("in")) {
      expr->exception_placement = Expression::ExceptionPlacement::capacity;
      expect("region");
      expect("(");
      placement = expression(0, HereAttachment::initializer_operand, false);
      expect("->");
      expect("capacity");
      expect(")");
    } else if (take("from")) {
      expr->exception_placement = Expression::ExceptionPlacement::cause;
      placement = expression(0, HereAttachment::initializer_operand);
    }
    if (is("in") || is("from")) fail(peek().at, "an exception construction has at most one placement clause");
    if (!expr->constructor_arguments) {
      expr->initializer_block = true;
      field_initializers(*expr);
    }
    if (placement) expr->children.push_back(std::move(placement));
    return expr;
  }

  Expr catch_handler(bool &catch_all) {
    expect("(");
    auto handler = make(Expression::Kind::handler, value_name());
    if (take(":")) {
      handler->read_only = take("const");
      handler->annotation = parse_type();
    } else catch_all = true;
    expect(")");
    handler->children.push_back(block());
    return handler;
  }

  bool starts_case_arm() const {
    if (!is("(")) return false;
    const auto close = right_paren_[cursor_] - cursor_;
    return is("|", close + 1) || (is("->", close + 1) && (is("{", close + 2) || is("throw", close + 2)));
  }
  Expr case_expression(const Token &token) {
    using K = Expression::Kind;
    auto result = make(K::case_expression, token);
    result->children.push_back(expression());
    expect("of");
    expect("{");
    const bool previous = std::exchange(parsing_case_, true);
    while (!take("}")) {
      if (peek().kind == TokenKind::end) fail(peek().at, "unterminated case body");
      if (!starts_case_arm()) {
        result->children.push_back(expression());
      } else {
        auto arm = make(K::case_arm, expect("("));
        if (is(")")) arm->text = "_";
        else {
          const auto pattern = peek();
          if (pattern.kind != TokenKind::identifier || !is(")", 1))
            fail(pattern.at, "case pattern is outside the supported seed profile");
          arm->text = tokens_[cursor_++].text;
        }
        expect(")");
        if (take("|") && !is("->")) arm->children.push_back(expression(0, HereAttachment::immediate, false));
        expect("->");
        if (is("throw")) {
          if (arm->text != "_" || !arm->children.empty())
            fail(peek().at, "the unbraced throwing arm requires an unguarded catch-all");
          arm->children.push_back(make(K::case_failure, expect("throw")));
        } else arm->children.push_back(block());
        result->children.push_back(finish(std::move(arm)));
      }
      if (!is("}")) expect(";");
      else take(";");
    }
    parsing_case_ = previous;
    return result;
  }

  Expr primary(HereAttachment attachment, bool owner_projection) {
    const bool pipeline_operand = attachment == HereAttachment::pipeline_body;
    auto token = peek();
    if (token.kind == TokenKind::string) {
      ++cursor_;
      auto literal = make(Expression::Kind::string_literal, token);
      literal->text = text_literal(token);
      return literal;
    }
    if (is("here") && tokens_[cursor_ + 1].kind == TokenKind::identifier) {
      expect("here");
      auto binding = make(Expression::Kind::here_binding, value_name());
      expect("=");
      binding->children.push_back(expression());
      return binding;
    }
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
    if ((is("handle") && tokens_[cursor_ + 1].kind == TokenKind::identifier) ||
        ownership_qualifier("handle") || (is("const") && ownership_qualifier("handle", 1))) {
      const bool fixed = take("const");
      const bool owned = take("own");
      expect("handle");
      auto expr = make(Expression::Kind::native_handle_binding, value_name());
      expr->fixed_binding = fixed;
      type_separator();
      expr->annotation = parse_type();
      if (!owned && take("in")) expr->owner_name = ordinary_name().text;
      expr->binding = owned ? "<-" : "=";
      expect(expr->binding);
      expr->children.push_back(expression());
      return expr;
    }
    if (ownership_qualifier("region") || (is("region") && !is("(", 1)) ||
        (is("const") && (ownership_qualifier("region", 1) || is("region", 1)))) {
      const bool fixed_binding = take("const");
      const bool owned = take("own");
      expect("region");
      auto expr = make(Expression::Kind::region_binding, value_name());
      expr->fixed_binding = fixed_binding;
      expr->annotation = region_annotation(owned, &expr->read_only);
      expr->binding = owned ? "<-" : "=";
      expect(expr->binding);
      expr->children.push_back(expression());
      return expr;
    }
    if (exception_binding_head()) {
      expect_exception_binder();
      auto expr = make(Expression::Kind::exception_binding, value_name());
      expect(":");
      expr->read_only = take("const");
      expr->annotation = parse_type();
      expect("<-");
      expr->children.push_back(expression());
      return expr;
    }
    if (take("make")) {
      if (is("exception") && tokens_[cursor_ + 1].kind == TokenKind::identifier && !is("in", 1)) {
        expect("exception");
        return exception_construction(token);
      }
      const bool owns_handle = ownership_qualifier("handle");
      if (owns_handle || (is("handle") && is("(", 2))) {
        if (owns_handle) expect("own");
        expect("handle");
        auto expr = make(owns_handle ? Expression::Kind::make_owned_handle : Expression::Kind::make_borrowed_handle, token);
        expr->annotation = Type{Type::named, ordinary_name().text};
        arguments(*expr, false);
        if (!owns_handle) {
          expect("in");
          expr->owner_name = ordinary_name().text;
        }
        return expr;
      }
      const bool legacy_region = is("(", 1) && is("capacity", 2) && is("=", 3);
      const auto constructed = peek();
      auto expr = make(legacy_region ? Expression::Kind::make_region : Expression::Kind::make_object,
                       name(TokenKind::identifier));
      if (legacy_region) {
        expect("(");
        expect("capacity");
        expect("=");
        expr->children.push_back(expression());
        expect(")");
      } else {
        if (expr->text == "typeof" && take("(")) {
          expr->construction_type_query = true;
          expr->children.push_back(expression());
          expect(")");
        } else expr->annotation = simple_type(constructed);
        if (!expr->construction_type_query && is("(")) {
          expr->constructor_arguments = true;
          arguments(*expr, false);
        }
        if (take("in")) expr->owner_name = ordinary_name().text;
        if (expr->constructor_arguments) return expr;
        if (!is("{")) return expr;
        expr->initializer_block = true;
        expr->children.push_back(block(true));
        return expr;
      }
      field_initializers(*expr);
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
          expr->grouped_deferred_call = call->kind == Expression::Kind::grouped;
          while (call->kind == Expression::Kind::grouped) {
            auto inner = std::move(call->children.front());
            call = std::move(inner);
          }
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
        expr->children.push_back(catch_handler(catch_all));
      }
      if (take("finally")) expr->children.push_back(block());
      if (!deferred && expr->children.size() == 1) fail(token.at, "try requires catch, finally, or both");
      return expr;
    }
    if (is("rethrow") && is("from", 1)) {
      expect("rethrow");
      expect("from");
      auto expr = make(Expression::Kind::rethrow_from, token);
      expr->children.push_back(expression());
      return expr;
    }
    if (take("throw")) {
      auto expr = make(Expression::Kind::raise, token);
      if (peek().kind == TokenKind::identifier && is("{", 1)) {
        expr->annotation = parse_type();
        field_initializers(*expr);
      } else if (!is(";") && !is("}")) {
        auto value = expression();
        if (take("from")) {
          auto cause = expression(0, value->kind == Expression::Kind::name ? HereAttachment::initializer_operand : HereAttachment::immediate);
          if (value->kind == Expression::Kind::name && is("{")) {
            auto constructed = make(Expression::Kind::make_exception, token);
            constructed->annotation = Type{Type::named, value->text};
            constructed->initializer_block = true;
            constructed->exception_placement = Expression::ExceptionPlacement::cause;
            field_initializers(*constructed);
            constructed->children.push_back(std::move(cause));
            expr->children.push_back(std::move(constructed));
            return expr;
          }
          expr->children.push_back(std::move(value));
          expr->children.push_back(std::move(cause));
        } else {
          expr->children.push_back(std::move(value));
        }
      }
      return expr;
    }
    if (take("return")) return return_expression(token);
    if (take("case")) return case_expression(token);
    if (parsing_case_ && is("fallthrough") && (is(";", 1) || is("}", 1)))
      return make(Expression::Kind::case_fallthrough, expect("fallthrough"));
    if ((parsing_handoff_ || parsing_case_) && (is("break") || is("continue"))) {
      ++cursor_;
      auto expr = make(token.text == "break" ? Expression::Kind::break_value : Expression::Kind::continue_value, token);
      if (!is(";") && !is("}")) expr->children.push_back(expression());
      return expr;
    }
    if (take("for")) {
      if (!is("(")) return generator_for(token.at);
      expect("(");
      auto expr = make(Expression::Kind::range_for, value_name());
      expect("in");
      expr->children.push_back(expression());
      expect("..");
      expr->children.push_back(expression());
      expect(")");
      expr->children.push_back(block());
      if (take("else")) expr->children.push_back(block());
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
    if (is("var"))
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
      if (is("(") && !(pipeline_operand && here_header())) arguments(*expr, !pipeline_operand);
      return expr;
    }
    if (take("if")) {
      if (is("returning")) return return_conditional(token);
      using K = Expression::Kind;
      auto expr = make(K::conditional, token);
      expect("(");
      expr->children.push_back(expression());
      expect(")");
      const bool single_return = is("return");
      expr->children.push_back(single_return ? return_expression(expect("return"), true) : block());
      if (single_return && is(";") && is("else", 1)) expect(";");
      if (take("else")) expr->children.push_back(block());
      else {
        auto empty = make(K::block, token);
        const auto end = expr->children.back()->span.past;
        empty->at.span = empty->span = {end, end};
        expr->children.push_back(std::move(empty));
      }
      return expr;
    }
    if (is("{")) return block();
    if (take("intrinsic")) {
      if (!is("(")) fail(peek().at, "intrinsic requires a parenthesized operator or intrinsic name");
      auto expr = make(Expression::Kind::call, lexical_name(nullptr, true));
      expr->intrinsic = true;
      arguments(*expr);
      return expr;
    }
    if (is("(") && ((is(")", 2) && operation_token(tokens_[cursor_ + 1])) ||
        (is("is", 1) && is("not", 2) && is(")", 3)) ||
        (is("[", 1) && is("]", 2) && (is("=", 3) || is("<-", 3)) && is(")", 4)))) {
      auto expr = make(Expression::Kind::call, lexical_name());
      arguments(*expr, !pipeline_operand);
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
      expr->children.push_back(expression(unary_precedence, attachment, owner_projection));
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
      const bool call = is("(") && !(pipeline_operand && here_header());
      auto expr = make(call ? Expression::Kind::call : Expression::Kind::name, token);
      if (call) arguments(*expr, !pipeline_operand);
      return expr;
    }
    fail(token.at, "expected an expression; symbolic writes require 'effect'");
  }
  Expr expression(int minimum = 0, HereAttachment attachment = HereAttachment::immediate,
                  bool owner_projection = true, bool type_suffix = true, Expr prefix = {}) {
    const ParseEntry entry(recursion_, peek().at, "an expression");
    auto operand = expression_impl(minimum, attachment, owner_projection, std::move(prefix));
    if (minimum != 0 || !type_suffix || !is(":")) return finish(std::move(operand));
    operand = finish(std::move(operand));
    auto typed = make(Expression::Kind::typed_expression, expect(":"));
    typed->children.push_back(std::move(operand));
    typed->annotation = parse_type();
    return finish(std::move(typed));
  }
  Expr compose_processors(Expr first, bool owner_projection, bool reversed) {
    using K = Expression::Kind;
    std::vector<Expr> operands;
    std::vector<Token> operators;
    operands.push_back(std::move(first));
    while (const auto op = next_pipeline_operator()) {
      if (op->kind != PipelineOperator::Kind::composition) break;
      if (op->reversed != reversed)
        fail(peek().at, "mixed processor composition directions require parentheses");
      // A run of N binary operators needs at least N + 1 expression levels.
      check_depth(operators.size() + 2, peek().at, "an expression");
      operators.push_back(tokens_[cursor_++]);
      operands.push_back(expression(composition_precedence + 1, HereAttachment::pipeline_body, owner_projection));
    }
    auto compose = [&](size_t index, Expr left, Expr right) {
      auto node = make(K::processor_composition, operators[index]);
      node->children.push_back(std::move(left));
      node->children.push_back(std::move(right));
      return finish(std::move(node));
    };
    if (reversed) {
      auto result = std::move(operands.back());
      for (size_t i = operators.size(); i > 0; --i)
        result = compose(i - 1, std::move(operands[i - 1]), std::move(result));
      return result;
    }
    auto result = std::move(operands.front());
    for (size_t i = 0; i < operators.size(); ++i)
      result = compose(i, std::move(result), std::move(operands[i + 1]));
    return result;
  }
  Expr connect_pipeline(Expr first, bool owner_projection) {
    using K = PipelineOperator::Kind;
    auto node = make(Expression::Kind::pipeline, peek());
    node->endpoints = std::make_unique<PipelineEndpoints>();
    auto &endpoints = *node->endpoints;
    node->children.push_back(std::move(first));
    while (const auto op = next_pipeline_operator()) {
      if (op->kind == K::composition) break;
      const auto spelling = tokens_[cursor_++];
      const auto right = node->children.size();
      const bool source = op->kind == K::source;
      auto &connection = source ? endpoints.source : endpoints.consumer;
      if (connection)
        fail(spelling.at, source ? "a pipeline can have only one source" : "a pipeline can have only one consumer");
      const bool endpoint_on_right = source == op->reversed;
      const auto middle = endpoint_on_right ? right - 1 : right;
      if (right != 1 && middle != endpoints.middle)
        fail(spelling.at, "source and consumer must attach to the same pipeline middle");
      endpoints.middle = middle;
      connection = PipelineEndpoints::Connection{spelling, endpoint_on_right ? right : right - 1};
      node->children.push_back(expression(composition_precedence, HereAttachment::pipeline_body, owner_projection));
    }
    return node;
  }
  Expr expression_impl(int minimum, HereAttachment attachment, bool owner_projection, Expr prefix) {
    const bool attach = attachment == HereAttachment::immediate;
    const bool pipeline_operand = attachment == HereAttachment::pipeline_body;
    auto left = prefix ? std::move(prefix) : primary(attachment, owner_projection);
    while (true) {
      if (attach && accepts_supplied_body(*left) && (is("{") || here_header())) {
        trailing_body(*left);
        left = finish(std::move(left));
      } else if (is("(") && !(pipeline_operand && here_header())) {
        auto call = make(Expression::Kind::value_call, peek());
        call->children.push_back(std::move(left));
        arguments(*call, !pipeline_operand);
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
      binary->children.push_back(expression(binary_precedence(op.text) + 1, attachment, owner_projection));
      left = finish(std::move(binary));
    }
    if (minimum <= composition_precedence) {
      if (const auto op = next_pipeline_operator(); op && op->kind == PipelineOperator::Kind::composition)
        left = compose_processors(std::move(left), owner_projection, op->reversed);
    }
    if (minimum <= endpoint_precedence) {
      if (const auto endpoint = next_pipeline_operator(); endpoint && endpoint->kind != PipelineOperator::Kind::composition)
        left = connect_pipeline(std::move(left), owner_projection);
      using K = Expression::Kind;
      if (attach && (left->kind == K::pipeline || left->kind == K::processor_composition) && here_header())
        trailing_body(*left);
    }
    if (minimum == 0 && swap_operation(peek().text)) {
      auto swap = make(Expression::Kind::call, tokens_[cursor_++]);
      swap->children.push_back(std::move(left));
      swap->children.push_back(expression(endpoint_precedence, attachment, owner_projection));
      if (swap_operation(peek().text)) fail(peek().at, "swap operators cannot be chained");
      return swap;
    }
    if (minimum == 0 && left->kind == Expression::Kind::index && (is("=") || is("<-"))) {
      auto assignment = make(Expression::Kind::call, tokens_[cursor_++]);
      assignment->text = "[]" + assignment->text;
      assignment->children = std::move(left->children);
      assignment->children.push_back(expression(0, attachment, owner_projection, false));
      return assignment;
    }
    if (minimum == 0 && is("<-")) {
      auto transfer = make(Expression::Kind::transfer, expect("<-"));
      transfer->children.push_back(std::move(left));
      transfer->children.push_back(expression(0, attachment, owner_projection, false));
      return transfer;
    }
    if (minimum == 0 && is("=")) {
      auto rebind = make(Expression::Kind::rebind, expect("="));
      rebind->children.push_back(std::move(left));
      rebind->children.push_back(expression(0, attachment, owner_projection, false));
      return rebind;
    }
    return left;
  }
  Expr block(bool initializer = false) {
    const ParseEntry entry(recursion_, peek().at, "a block");
    return finish(block_impl(initializer));
  }
  Expr block_impl(bool initializer) {
    auto result = make(Expression::Kind::block, expect("{"));
    result->scoped = is("{") && peek().at.span.first == result->at.span.past;
    if (initializer && result->scoped) fail(result->at, "use an ordinary initializer block after make");
    if (result->scoped) expect("{");
    while (!take("}")) {
      if (peek().kind == TokenKind::end) fail(peek().at, "unterminated scope");
      auto label = initializer && starts_field_initializer() ? construction_label() : Expr{};
      result->children.push_back(expression(0, HereAttachment::immediate, true, true, std::move(label)));
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
  static void normalize_return_tail(Form &form) {
    using K = Expression::Kind;
    if (!form.returns_tail_value()) return;
    auto &body = *form.body;
    if (body.kind == K::block && body.children.empty()) return;
    auto &result = body.kind == K::block ? body.children.back() : form.body;
    if (result->kind == K::return_value || result->kind == K::case_fallthrough) return;
    auto tail = std::make_unique<Expression>();
    tail->kind = K::return_tail;
    tail->at = result->at;
    tail->span = result->span;
    tail->children.push_back(std::move(result));
    result = std::move(tail);
  }
  static void normalize_full_scopes(Expression &expr, Form::Kind callable_kind) {
    using K = Expression::Kind;
    const bool delayed_scope = expr.kind == K::deferred && expr.children.front()->kind == K::block &&
        expr.children.front()->scoped;
    const auto *single = expr.kind == K::here_body && !expr.scoped && expr.receiving->shape == ReceivingHeader::Shape::single
        ? &expr.receiving->items.front() : nullptr;
    const bool receives_bits = single && single->operation == "<-" &&
        (single->declaration_kind.empty() || single->declaration_kind == "val");
    const bool receives_ref = single && single->operation == "=" && single->declaration_kind == "ref";
    const bool receives_fields = expr.kind == K::here_body && expr.receiving->shape == ReceivingHeader::Shape::labelled &&
        std::ranges::all_of(expr.receiving->items, [](const auto &item) {
          return item.declaration_kind == "val" || item.declaration_kind == "ref";
        });
    if (expr.kind == K::here_body && (expr.receiving->shape == ReceivingHeader::Shape::ordered || receives_bits || receives_ref || receives_fields)) {
      auto definition = std::make_shared<Form>();
      definition->kind = callable_kind == Form::Kind::function ? Form::Kind::function : Form::Kind::procedure;
      definition->at = expr.at;
      definition->span = expr.span;
      definition->supplied_scope = true;
      definition->supplied_group = !expr.scoped;
      for (const auto &item : expr.receiving->items) {
        Parameter parameter{item.name, item.annotation.value_or(Type{Type::void_type}), item.at, false, {}};
        parameter.declaration_kind = item.declaration_kind;
        parameter.owner = item.owner;
        parameter.read_only = item.read_only;
        parameter.fixed_binding = item.fixed_binding;
        definition->parameters.push_back(std::move(parameter));
      }
      definition->body = std::move(expr.children.front());
      if (receives_bits && single->declaration_kind.empty()) {
        auto &parameter = definition->parameters.front();
        parameter.name = "$here.received";
        // Prepare the selected existing destination on each actual handoff.
        auto delivery = std::make_unique<Expression>();
        delivery->kind = K::transfer;
        delivery->at = single->at;
        delivery->span = single->at.span;
        for (const auto &name : {single->name, parameter.name}) {
          auto operand = std::make_unique<Expression>();
          operand->kind = K::name;
          operand->at = single->at;
          operand->span = single->at.span;
          operand->text = name;
          delivery->children.push_back(std::move(operand));
        }
        definition->body->children.insert(definition->body->children.begin(), std::move(delivery));
      }
      expr.children.clear();
      expr.definition = std::move(definition);
    }
    const auto nested_kind = expr.kind == K::block && expr.scoped && callable_kind != Form::Kind::function
        ? Form::Kind::procedure : callable_kind;
    for (const auto &child : expr.children) normalize_full_scopes(*child, nested_kind);
    if (expr.definition && expr.definition->body) {
      normalize_full_scopes(*expr.definition->body, expr.definition->kind);
      normalize_return_tail(*expr.definition);
    }
    if (delayed_scope) {
      // Keep block-defer preparation delayed after its full scope becomes a call.
      auto body = std::make_unique<Expression>();
      body->kind = K::block; body->at = expr.children.front()->at;
      body->span = expr.children.front()->span;
      body->children.push_back(std::move(expr.children.front()));
      expr.children.front() = std::move(body);
    }
    if (expr.kind != K::block || !expr.scoped) return;
    auto definition = std::make_shared<Form>();
    definition->kind = callable_kind == Form::Kind::function ? Form::Kind::function : Form::Kind::procedure;
    definition->at = expr.at;
    definition->span = expr.span;
    definition->implicit_scope = true;
    definition->case_scan_scope = expr.case_resumes;
    definition->body = std::make_unique<Expression>(std::move(expr));
    definition->body->scoped = false;
    normalize_return_tail(*definition);
    expr = Expression{};
    expr.kind = K::value_call; expr.at = definition->at;
    expr.span = definition->span;
    auto callable = std::make_unique<Expression>();
    callable->kind = K::callable; callable->at = definition->at;
    callable->span = definition->span;
    callable->definition = std::move(definition);
    expr.children.push_back(std::move(callable));
  }
  enum class ReturnContext { module, callable, deferred };
  static void validate_explicit_scopes(Expression &expr, ReturnContext returning,
                                       bool iteration = false, bool deferred = false,
                                       const Location *draining = nullptr, const ReceivingHeader *handoff = nullptr,
                                       Expression *case_body = nullptr, Expression *arm_body = nullptr, bool attached_arm = false) {
    using K = Expression::Kind;
    if ((expr.kind == K::block || expr.kind == K::here_body) && expr.scoped) {
      iteration = deferred = false;
      draining = nullptr;
      handoff = nullptr;
      case_body = nullptr;
      if (!attached_arm) arm_body = nullptr;
      returning = ReturnContext::callable;
    }
    if (expr.kind == K::return_test && returning != ReturnContext::deferred)
      fail(expr.at, "'if returning' requires deferred execution in a lexical return destination");
    if (expr.kind == K::return_test && !expr.binding.empty() && expr.declaration_kind.empty() && !expr.annotation)
      fail(expr.at, "a return pattern binding requires val, ref, ptr or a complete type annotation");
    if (expr.kind == K::here_body && !expr.scoped) {
      handoff = expr.receiving.get();
      case_body = nullptr;
      arm_body = nullptr;
    }
    if (expr.kind == K::case_fallthrough) {
      if (!arm_body) fail(expr.at, "'fallthrough' requires an active case arm");
      arm_body->case_resumes = true;
    }
    if (expr.kind == K::break_value || expr.kind == K::continue_value) {
      if (case_body) {
        if (expr.kind == K::continue_value) fail(expr.at, "'continue' cannot cross a case body");
        expr.kind = K::case_break;
        case_body->case_breaks = true;
      } else {
        if (!handoff) fail(expr.at, expr.kind == K::break_value ? "'break' requires an active case or single-brace here body"
                                                             : "'continue' requires an active single-brace here body");
        if (handoff->from_for && expr.kind == K::break_value && !expr.children.empty())
          fail(expr.at, "a generator for loop requires bare 'break'");
      }
    }
    if (expr.kind == K::deferred) {
      if (draining)
        fail(expr.at, "defer cannot register work into its currently draining scope\n" +
            location_text(*draining) + ": note: enclosing deferred execution begins here");
      // Ordinary iterations share the surrounding cleanup scope. Full scopes
      // and callable invocations start a fresh registration sequence.
      expr.repeated_defer_registration = iteration || handoff;
    }
    const bool declaration = expr.kind == K::pointer_group || expr.kind == K::region_binding ||
        expr.kind == K::native_handle_binding || expr.kind == K::here_binding ||
        expr.kind == K::alias || (expr.kind == K::callable && !expr.definition->name.empty());
    if (deferred && declaration)
      fail(expr.at, "a deferred-block variable declaration requires a nested explicit {{ ... }} scope");
    if (deferred && (expr.kind == K::effect || expr.kind == K::effect_group || expr.kind == K::procedure_effect))
      fail(expr.at, "effect installation in a deferred block requires a nested explicit {{ ... }} scope");
    if (iteration && declaration)
      fail(expr.at, "a loop-body variable declaration requires a nested per-iteration {{ ... }} scope");
    if (arm_body && !arm_body->scoped && declaration)
      fail(expr.at, "a case-arm variable declaration requires a nested explicit {{ ... }} scope");
    if (handoff && declaration)
      fail(expr.at, "a here-body variable declaration requires a nested explicit {{ ... }} scope");
    for (size_t i = 0; i < expr.children.size(); ++i) {
      const bool loop_body = expr.kind == K::range_for && i >= 2;
      const bool deferred_body = expr.kind == K::deferred && expr.children.front()->kind == K::block;
      const auto child_returning = deferred_body && returning != ReturnContext::module ? ReturnContext::deferred : returning;
      const bool case_item = expr.kind == K::case_expression && i != 0;
      const bool selected_arm = expr.kind == K::case_arm && i + 1 == expr.children.size();
      auto &child = *expr.children[i];
      const bool delayed_entry = expr.kind == K::deferred && (i != 0 || deferred_body);
      validate_explicit_scopes(child, child_returning, loop_body || iteration, deferred_body || deferred,
          deferred_body ? &expr.at : draining, delayed_entry || loop_body || case_item ? nullptr : handoff,
          delayed_entry || loop_body ? nullptr : case_item ? &expr : case_body,
          delayed_entry || case_item ? nullptr : selected_arm ? &child : arm_body, selected_arm);
      if (selected_arm) expr.case_resumes = child.case_resumes;
    }
    // A callable body executes in its own call scope, independently of where
    // its value is constructed. Loops inside it still need explicit scopes.
    if (expr.definition && expr.definition->body)
      validate_explicit_scopes(*expr.definition->body, ReturnContext::callable);
  }

public:
  explicit Parser(const std::shared_ptr<const IndexedSource> &source)
      : tokens_(tokenize(source)), right_paren_(tokens_.size(), tokens_.size()) {
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
    const auto append = [&](std::unique_ptr<Form> form) {
      form->span = {form->at.span.first, tokens_[cursor_ - 1].at.span.past};
      forms.push_back(std::move(form));
    };
    while (peek().kind != TokenKind::end) {
      auto start = peek();
      auto form = std::make_unique<Form>();
      form->at = start.at;
      const auto native_handle_head = [&] { return is("handle") && is("type", 1); };
      const auto reference_symbol_head = [&] { return (is("ref") || is("ptr")) && is("symbol", 1); };
      if (take("export")) {
        if (is("from") && tokens_[cursor_ + 1].kind == TokenKind::string) {
          expect("from");
          form->kind = Form::Kind::export_from_resource;
          resource(*form, false);
          expect(";");
          append(std::move(form));
          continue;
        }
        const auto next = peek().kind == TokenKind::end ? TokenKind::end : tokens_[cursor_ + 1].kind;
        const bool named_type = native_handle_head() || (is("data") && (next == TokenKind::identifier || next == TokenKind::string)) ||
            ((is("type") || is("array") || is("enum")) && next == TokenKind::identifier);
        const bool named_callable = (is("func") || is("function") || is("proc") || is("procedure")) && !callable_binding();
        if (is("import") || (is("symbol") && next == TokenKind::symbol) || reference_symbol_head() || named_type || named_callable) {
          form->exporting = true;
        } else {
          form->kind = Form::Kind::export_resource;
          form->symbolic = peek().kind == TokenKind::symbol;
          form->name = form->symbolic ? name(TokenKind::symbol).text : lexical_name(&form->operation).text;
          expect(";");
          append(std::move(form));
          continue;
        }
      }
      if (reference_symbol_head()) {
        const bool nullable = take("ptr");
        if (!nullable) expect("ref");
        expect("symbol");
        form->kind = Form::Kind::symbol;
        form->name = name(TokenKind::symbol).text;
        form->symbolic = true;
        type_separator();
        const bool read_only = take("const");
        form->result_type = parse_type();
        expect("in");
        const auto selected = ordinary_name();
        expect("where"); expect("{");
        const bool const_owner = take("const");
        expect("owner");
        const auto owner = ordinary_name();
        if (owner.text != selected.text) fail(selected.at, "reference symbol must name its declared owner parameter");
        expect("}"); expect(";");
        form->symbol_reference = SymbolReference{nullable, read_only || const_owner};
        append(std::move(form));
        continue;
      }
      const bool region_symbol = is("region") && is("symbol", 1);
      if (region_symbol) expect("region");
      if (take("symbol")) {
        if (is("from") && tokens_[cursor_ + 1].kind == TokenKind::string) {
          expect("from");
          if (region_symbol) fail(start.at, "use 'symbol from' to select a symbol identity; 'region symbol' declares one");
          form->kind = Form::Kind::symbol_from;
          resource(*form, true);
        } else {
          std::vector<std::pair<Form *, std::string>> declarations;
          do {
            auto declaration = std::make_unique<Form>();
            declaration->kind = Form::Kind::symbol;
            const auto token = peek().kind == TokenKind::symbol ? name(TokenKind::symbol) : ordinary_name();
            declaration->name = token.text;
            declaration->at = token.at;
            declaration->symbolic = true;
            declaration->exporting = form->exporting;
            if (region_symbol) {
              declaration->result_type = region_annotation(false);
            } else {
              if (!is("::") && !is(":")) fail(peek().at, "a symbol declaration requires an explicit type");
              type_separator();
              declaration->owning_handle_symbol = ownership_qualifier("handle");
              if (declaration->owning_handle_symbol) { expect("own"); expect("handle"); }
              declaration->result_type = parse_type();
            }
            const auto owner = take("in") ? ordinary_name().text : std::string{};
            declarations.emplace_back(declaration.get(), owner);
            append(std::move(declaration));
            if (form->exporting && is(",")) fail(peek().at, "export requires a single named symbol declaration");
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
          for (const auto &[declaration, name] : declarations)
            declaration->span.past = tokens_[cursor_ - 1].at.span.past;
          continue;
        }
        expect(";");
      } else if (is("unit") && tokens_[cursor_ + 1].kind == TokenKind::identifier) {
        expect("unit");
        form->kind = Form::Kind::unit;
        form->name = ordinary_name().text;
        expect(";");
      } else if (is("enum") && tokens_[cursor_ + 1].kind == TokenKind::identifier) {
        expect("enum");
        form->kind = Form::Kind::enumeration;
        form->name = ordinary_name().text;
        type_separator();
        form->result_type = parse_type();
        expect("{");
        while (!take("}")) {
          EnumMember member{ordinary_name(), {}};
          if (take("=")) {
            member.negative = take("-");
            member.initializer = peek().kind == TokenKind::identifier && !member.negative
                ? ordinary_name() : name(TokenKind::integer);
          }
          expect(";");
          form->enum_members.push_back(std::move(member));
        }
        if (form->enum_members.empty()) fail(start.at, "an enum requires at least one member");
        take(";");
      } else if (is("array") && tokens_[cursor_ + 1].kind == TokenKind::identifier) {
        expect("array");
        form->kind = Form::Kind::array;
        form->name = ordinary_name().text;
        expect("[");
        form->index_type = parse_type();
        expect("]");
        type_separator();
        form->result_type = parse_type();
        expect(";");
      } else if (native_handle_head()) {
        expect("handle"); expect("type");
        form->kind = Form::Kind::native_handle_type;
        form->name = ordinary_name().text;
        if (take("in")) form->native_owner_type = parse_type();
        expect(";");
      } else if (((is("type") || is("data")) && tokens_[cursor_ + 1].kind == TokenKind::identifier) ||
                 (is("data") && tokens_[cursor_ + 1].kind == TokenKind::string) ||
                 (is("region") && tokens_[cursor_ + 1].kind == TokenKind::identifier && is("{", 2))) {
        const bool region = take("region");
        const bool data = region || take("data");
        if (!data) expect("type");
        form->kind = region ? Form::Kind::region_definition : Form::Kind::data;
        if (!region && data && peek().kind == TokenKind::string) {
          const auto linkage = name(TokenKind::string);
          if (linkage.text != "C") fail(linkage.at, "expected \"C\" as the data linkage");
          form->c_data = true;
        }
        form->name = ordinary_name().text;
        if (take("<:")) form->parent_type = parse_type();
        if (!is("{")) {
          if (form->c_data) fail(peek().at, "expected fields for a data \"C\" record; declare native handles with 'handle type'");
          if (data && !form->parent_type)
            fail(start.at, "a head-only data declaration requires an abstract parent");
          if (form->exporting && data) fail(start.at, "export requires a locally defined identity");
          form->kind = data ? Form::Kind::conformance : Form::Kind::abstract_type;
          expect(";");
          append(std::move(form));
          continue;
        }
        if (!data)
          fail(start.at, "abstract type declarations have no body; use data for a representation");
        expect("{");
        while (!take("}")) {
          const bool fixed_binding = take("const");
          if (is("region") || ownership_qualifier("region"))
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
        if (form->c_data && form->parameters.empty())
          fail(start.at, "a data \"C\" record requires at least one field; declare native handles with 'handle type'");
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
          form->path = module_path();
          expect("{");
          do {
            auto source = ordinary_name();
            auto local = take("as") ? ordinary_name().text : source.text;
            form->units.push_back({source.text, local, source.at});
          } while (take(","));
          expect("}");
          expect(";");
          append(std::move(form));
          continue;
        }
        if (take("symbol")) {
          if (!using_resource) fail(start.at, "combined symbol selection requires using; declare its destination first");
          form->using_symbol = true;
        }
        form->path = module_path();
        if (take("{")) {
          if (is("}")) fail(peek().at, "expected a selection in the module group");
          while (true) {
            auto item = std::make_unique<Form>();
            item->at = peek().at;
            item->kind = form->kind;
            item->path = form->path;
            item->using_symbol = form->using_symbol;
            item->exporting = form->exporting;
            resource_item(*item);
            append(std::move(item));
            if (!take(",") || is("}")) break;
          }
          expect("}");
          expect(";");
          continue;
        }
        resource_item(*form);
        expect(";");
      } else {
        form->kind = Form::Kind::expression;
        form->body = expression();
        expect(";");
      }
      append(std::move(form));
      depths_.clear();
    }
    for (const auto &form : forms) if (form->body) {
      const auto context = form->kind == Form::Kind::function || form->kind == Form::Kind::procedure
          ? ReturnContext::callable : ReturnContext::module;
      validate_explicit_scopes(*form->body, context);
      normalize_assignment_calls(*form->body);
      normalize_full_scopes(*form->body, form->kind);
      normalize_return_tail(*form);
    }
    return forms;
  }
};

} // namespace cod::language
