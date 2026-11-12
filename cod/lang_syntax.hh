//===--- lang_syntax.hh - CoD source-language prototype syntax ------===//
#pragma once

#include <algorithm>
#include <cctype>
#include <filesystem>
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
          pair == "&&" || pair == "||" || pair == "<-" || pair == "<:" || pair == "..") {
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
  std::vector<std::string> capture_owners; // Source names anchoring a callable environment's regional dependencies.
  std::vector<std::optional<size_t>> capture_owner_regions; // Bound actual owner for each visible with-name.
  std::vector<size_t> capture_formals; // Nonzero indices refer to the enclosing callable's owner binders.
  std::vector<size_t> capture_regions; // Concrete WPO region identities, never runtime type information.
  std::vector<bool> const_views;
  std::vector<bool> nullable_views;
  bool variadic = false;
  bool owned = false;
  bool pure_callable = false;
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
           parameters == other.parameters && bounds == other.bounds && variadic == other.variadic && owned == other.owned && pure_callable == other.pure_callable &&
           throws_types == other.throws_types && owner_bounds == other.owner_bounds && owners == other.owners && const_views == other.const_views && nullable_views == other.nullable_views &&
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
struct Expression {
  enum class Kind { integer, real, boolean, name, symbol, unary, binary, call, value_call, perform, behave, effect,
                    block, conditional, field, range_for, fold, allocate, index, null_ref,
                    try_scope, handler, raise, return_value, make_region, touch, effect_group, transfer, procedure_effect,
                    pointer_group, pointer, make_local, rebind, region_binding, callable, alias, capture_owner };
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
};
using Expr = std::unique_ptr<Expression>;

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

inline int binary_precedence(const std::string &op) {
  if (op == "||") return 1;
  if (op == "&&") return 2;
  if (op == "==" || op == "!=") return 3;
  if (op == "<" || op == ">" || op == "<=" || op == ">=") return 4;
  if (op == "+" || op == "-") return 5;
  if (op == "*" || op == "/" || op == "%") return 6;
  return -1;
}

inline bool intrinsic_name(std::string_view name) {
  return name == "isnull" || name == "cause_at" || name == "region" || name == "root" || name == "used_bytes" || name == "capacity_bytes" ||
         name == "deref" || name == "len" || name == "ref_at" || name == "store" ||
         name == "setfield" || name == "scalar" || name == "i64" || name == "u8" ||
         name == "u32" || name == "u64" || name == "f32" || name == "f64";
}

inline bool expression_keyword(std::string_view name) {
  return name == "true" || name == "false" || name == "null" || name == "alloc" ||
         name == "for" || name == "fold" || name == "var" || name == "let" || name == "effect" ||
         name == "perform" || name == "behave" || name == "if" || name == "procedure" ||
         name == "function" || name == "func" || name == "proc" || name == "global" || name == "symbol" ||
         name == "try" || name == "catch" || name == "finally" || name == "throw" || name == "return" || name == "from" ||
         name == "val" || name == "ref" || name == "throws" || name == "make" || name == "owned" || name == "touch" || name == "ptr" || name == "typeof" || name == "intrinsic" || name == "const";
}

inline bool module_keyword(std::string_view name) {
  return name == "type" || name == "data" || name == "regional" || name == "unit" || name == "array" ||
         name == "using" || name == "import" || name == "mixin" || name == "export" || name == "conversion";
}

class Parser {
  std::vector<Token> tokens_;
  size_t cursor_ = 0;

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
    return (token.kind == TokenKind::identifier && intrinsic_name(token.text)) ||
           (token.kind == TokenKind::punctuation &&
            (binary_precedence(token.text) >= 0 || token.text == "!" || token.text == "=" || token.text == "<-" ||
             swap_operation(token.text)));
  }
  Token lexical_name(bool *operation = nullptr) {
    const bool parenthesized = take("(");
    if (operation) *operation = parenthesized;
    if (!parenthesized) return ordinary_name();
    auto op = tokens_[cursor_++];
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
  }
  Type parse_type(Parameter *parameter = nullptr) {
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
      if (result_mode == "val") fail(token.at, "val declares storage or a parameter, not a result binding");
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
    return std::make_unique<Expression>(Expression{kind, token.at, token.text, {}, {}, {}, false, {}});
  }
  Token field_name() {
    if (!take("(")) return name(TokenKind::identifier);
    auto field = field_name();
    expect(")");
    return field;
  }
  void arguments(Expression &expr) {
    expect("(");
    if (!take(")")) {
      do {
        if (expr.kind == Expression::Kind::call && expr.text == "setfield" && expr.children.size() == 1)
          expr.children.push_back(make(Expression::Kind::name, field_name()));
        else {
          auto argument = expression();
          argument->spread = take("...");
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
    if (!function && !take("proc")) expect("procedure");
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
          const bool fixed_binding = take("const");
          const bool owned_region = take("owned");
          const bool regional = take("region");
          if (owned_region && !regional) fail(peek().at, "owned requires a region declaration");
          std::string mode;
          if (!regional && (is("ptr") || is("ref") || is("val"))) mode = tokens_[cursor_++].text;
          auto param = value_name();
          Parameter parameter{param.text, Type::void_type, param.at, false, {}};
          parameter.fixed_binding = fixed_binding;
          if (regional) {
            parameter.type = region_annotation(owned_region, &parameter.read_only);
          } else {
            type_separator();
            parameter.read_only = take("const");
            parameter.type = parse_type(&parameter);
          }
          parameter.declaration_kind = mode;
          if (take("in")) parameter.owner = ordinary_name().text;
          if (mode == "val" && !parameter.owner.empty()) fail(param.at, "a val parameter has callee-local bits storage, not an external owner");
          if (parameter.type == Type::void_type) fail(param.at, "a parameter must have a value type");
          parameter.variadic = take("...");
          form.parameters.push_back(std::move(parameter));
        } while (take(","));
        expect(")");
      }
    } else if (function) {
      fail(peek().at, "a function requires a parameter list");
    }
    const bool preceding_where = take("where");
    if (preceding_where) generic_clause(form);
    if (take("->")) {
      form.result_type = signature_type(form.result_const, form.result_kind);
      if (form.result_kind == "val") fail(form.at, "val declares storage or a parameter, not a result binding");
      if (take("in")) form.result_owner = ordinary_name().text;
    }
    if (!form.result_kind.empty() && form.result_owner.empty())
      fail(form.at, "a ref/ptr result requires an explicit external owner: use in O");
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
    if (expression_form && !form.result_type) form.result_type = Type::void_type;
    if ((!function || declaration) && !form.result_type)
      fail(form.at, "a callable declaration requires an explicit result type");
    if (declaration && form.conversion)
      fail(form.at, "a conversion requires an implemented function method");
    if (!declaration) { form.body = block(); if (!inline_effect && !expression_form) take(";"); }
    if (inline_effect && (!form.symbolic || declaration || function))
      fail(form.at, "effect proc requires a symbolic procedure definition");
  }
  bool callable_binding() const {
    const size_t prefix = is("const") ? 1 : 0;
    return (is("func", prefix) || is("proc", prefix)) && tokens_[cursor_ + prefix + 1].kind == TokenKind::identifier &&
           (is("=", prefix + 2) || is(":", prefix + 2));
  }
  Expr primary() {
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
    if (take("try")) {
      auto expr = make(Expression::Kind::try_scope, token);
      expr->children.push_back(block());
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
      if (expr->children.size() == 1) fail(token.at, "try requires catch, finally, or both");
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
        expr->children.push_back(expression());
        group->children.push_back(std::move(expr));
      } while (take(","));
      return group;
    }
    if (is("let") || is("var"))
      fail(token.at, "use ref/ptr for reference bindings or val for independent bits storage");

    if (take("effect")) {
      if (is("proc") || is("procedure")) {
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
        (is("[", 1) && is("]", 2) && (is("=", 3) || is("<-", 3)) && is(")", 4)))) {
      auto expr = make(Expression::Kind::call, lexical_name());
      arguments(*expr);
      return expr;
    }
    if (take("(")) {
      auto expr = expression();
      expect(")");
      return expr;
    }
    if (take("-") || take("!")) {
      auto expr = make(Expression::Kind::unary, token);
      expr->children.push_back(expression(7));
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
      if (token.text == "null") return make(Expression::Kind::null_ref, token);
      if (token.text == "true" || token.text == "false") return make(Expression::Kind::boolean, token);
      if (token.text == "procedure" || token.text == "proc" || token.text == "function" || token.text == "global" || token.text == "symbol")
        fail(token.at, "prototype declarations belong at module top level");
      auto expr = make(is("(") ? Expression::Kind::call : Expression::Kind::name, token);
      if (is("(")) arguments(*expr);
      return expr;
    }
    fail(token.at, "expected an expression; symbolic writes require 'effect'");
  }
  Expr expression(int minimum = 0) {
    auto left = primary();
    while (true) {
      if (is("(")) {
        auto call = make(Expression::Kind::value_call, peek());
        call->children.push_back(std::move(left));
        arguments(*call);
        left = std::move(call);
      } else if (take(".")) {
        auto member = make(Expression::Kind::field, name(TokenKind::identifier));
        member->children.push_back(std::move(left));
        left = std::move(member);
      } else if (take("->")) {
        auto owner = make(Expression::Kind::capture_owner, ordinary_name());
        owner->children.push_back(std::move(left));
        left = std::move(owner);
      } else if (is("[")) {
        auto index = make(Expression::Kind::index, expect("["));
        index->children.push_back(std::move(left));
        index->children.push_back(expression());
        expect("]");
        left = std::move(index);
      } else break;
    }
    while (peek().kind == TokenKind::punctuation && binary_precedence(peek().text) >= minimum) {
      auto op = tokens_[cursor_++];
      auto binary = make(Expression::Kind::binary, op);
      binary->children.push_back(std::move(left));
      binary->children.push_back(expression(binary_precedence(op.text) + 1));
      left = std::move(binary);
    }
    if (minimum == 0 && swap_operation(peek().text)) {
      auto swap = make(Expression::Kind::call, tokens_[cursor_++]);
      swap->children.push_back(std::move(left));
      swap->children.push_back(expression(1));
      if (swap_operation(peek().text)) fail(peek().at, "swap operators cannot be chained");
      return swap;
    }
    if (minimum == 0 && left->kind == Expression::Kind::index && (is("=") || is("<-"))) {
      auto assignment = make(Expression::Kind::call, tokens_[cursor_++]);
      assignment->text = "[]" + assignment->text;
      assignment->children = std::move(left->children);
      assignment->children.push_back(expression());
      return assignment;
    }
    if (minimum == 0 && is("<-")) {
      auto transfer = make(Expression::Kind::transfer, expect("<-"));
      transfer->children.push_back(std::move(left));
      transfer->children.push_back(expression());
      return transfer;
    }
    if (minimum == 0 && is("=")) {
      auto rebind = make(Expression::Kind::rebind, expect("="));
      rebind->children.push_back(std::move(left));
      rebind->children.push_back(expression());
      return rebind;
    }
    return left;
  }
  Expr block() {
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

  static void validate_iteration_scopes(const Expression &expr, bool needs_scope = false) {
    using K = Expression::Kind;
    if (expr.kind == K::block && expr.scoped) needs_scope = false;
    if (needs_scope && (expr.kind == K::pointer_group || expr.kind == K::region_binding ||
                        expr.kind == K::alias ||
                        (expr.kind == K::callable && !expr.definition->name.empty())))
      fail(expr.at, "a loop-body variable declaration requires a nested per-iteration {{ ... }} scope");
    for (size_t i = 0; i < expr.children.size(); ++i) {
      const bool iteration = (expr.kind == K::range_for && i >= 2) ||
                             (expr.kind == K::fold && i == 3);
      validate_iteration_scopes(*expr.children[i], iteration || needs_scope);
    }
    // A callable body executes in its own call scope, independently of where
    // its value is constructed. Loops inside it still need explicit scopes.
    if (expr.definition && expr.definition->body)
      validate_iteration_scopes(*expr.definition->body);
  }

public:
  Parser(const std::string &source, const std::string &file) : tokens_(tokenize(source, file)) {}

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
        const auto kind = form->body->kind;
        const bool negative_literal = kind == Expression::Kind::unary && form->body->text == "-" &&
                                      (form->body->children.front()->kind == Expression::Kind::integer ||
                                       form->body->children.front()->kind == Expression::Kind::real);
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
    }
    for (const auto &form : forms)
      if (form->body) validate_iteration_scopes(*form->body);
    return forms;
  }
};

} // namespace cod::language
