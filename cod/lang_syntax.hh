//===--- lang_syntax.hh - CoD source-language prototype syntax ------===//
#pragma once

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
    if (source.compare(cursor, 3, "...") == 0) {
      advance(); advance(); advance();
      tokens.push_back({TokenKind::punctuation, "...", start});
      continue;
    }
    if (cursor + 1 < source.size()) {
      const auto pair = source.substr(cursor, 2);
      if (pair == "->" || pair == "==" || pair == "!=" || pair == "<=" || pair == ">=" ||
          pair == "&&" || pair == "||" || pair == "::" || pair == "<-" || pair == "<:" || pair == "..") {
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
  std::vector<size_t> owners;
  std::vector<bool> const_views;
  bool variadic = false;
  bool owned = false;
  Type(Kind kind = void_type, std::string name = {}, size_t identity = 0)
      : kind(kind), name(std::move(name)), identity(identity) {}
  bool operator==(const Type &other) const {
    return kind == other.kind && name == other.name && identity == other.identity &&
           parameters == other.parameters && bounds == other.bounds && variadic == other.variadic && owned == other.owned &&
           owner_bounds == other.owner_bounds && owners == other.owners && const_views == other.const_views &&
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
    std::string result = "proc(";
    for (size_t i = 0; i < type.parameters.size(); ++i)
      result += (i ? ", " : "") + type_name(type.parameters[i]) +
                (type.variadic && i + 1 == type.parameters.size() ? "..." : "");
    return result + ") -> " + type_name(*type.target);
  }
  case Type::float32: return "f32";
  case Type::float64: return "f64";
  case Type::quantity: return type_name(*type.target) + "(" + type.name + ")";
  case Type::unit: return type.name;
  case Type::named: case Type::regional: case Type::array:
  case Type::abstract_type: return type.name;
  case Type::region: return std::string(type.owned ? "owned " : "") + "region" +
                           (type.target ? "<" + type_name(*type.target) + ">" : "");
  case Type::reference: return "ref<" + type_name(*type.target) + ">";
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
  enum class Kind { integer, real, boolean, name, symbol, unary, binary, call, perform, behave, effect,
                    block, conditional, field, let, range_for, fold, allocate, index, null_ref,
                    try_scope, handler, raise, make_region, touch, effect_group, transfer, procedure_effect };
  Kind kind;
  Location at;
  std::string text;
  std::vector<std::unique_ptr<Expression>> children;
  std::optional<size_t> symbol_id;
  std::string binding;
  bool spread = false;
  std::optional<Type> annotation;
  std::shared_ptr<Form> definition = {};
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
};
struct TypeParameter { std::string name; std::optional<Type> bound; Location at; };
struct OwnerParameter { std::string name; std::optional<Type> root; Location at; };
struct SymbolOwner { std::string name; Type root; Location at; };
struct UnitSelection { std::string source, local; Location at; };
struct SymbolMapping {
  std::string local;
  std::string source;
  Location at;
};
struct Form {
  enum class Kind { symbol, symbol_from, function, procedure, data, array, unit, using_units, global,
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
  std::vector<TypeParameter> type_parameters;
  std::vector<OwnerParameter> owner_parameters;
  std::shared_ptr<SymbolOwner> symbol_owner;
  std::string result_owner;
  bool result_const = false;
  std::optional<Type> result_type;
  std::optional<Type> index_type;
  std::optional<Type> parent_type;
  Expr body;
};

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
  return name == "cause_at" || name == "region" || name == "root" || name == "used_bytes" || name == "capacity_bytes" ||
         name == "deref" || name == "len" || name == "ref_at" || name == "store" ||
         name == "setfield" || name == "scalar" || name == "i64" || name == "u8" ||
         name == "u32" || name == "u64" || name == "f32" || name == "f64";
}

inline bool expression_keyword(std::string_view name) {
  return name == "true" || name == "false" || name == "null" || name == "alloc" ||
         name == "for" || name == "fold" || name == "let" || name == "effect" ||
         name == "perform" || name == "behave" || name == "if" || name == "procedure" ||
         name == "function" || name == "proc" || name == "global" || name == "symbol" ||
         name == "try" || name == "catch" || name == "finally" || name == "throw" || name == "from" ||
         name == "make" || name == "owned" || name == "touch";
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
            (binary_precedence(token.text) >= 0 || token.text == "!" || token.text == "=" || token.text == "<-"));
  }
  Token lexical_name(bool *operation = nullptr) {
    const bool parenthesized = take("(");
    if (operation) *operation = parenthesized;
    if (!parenthesized) return ordinary_name();
    auto op = tokens_[cursor_++];
    if (op.kind == TokenKind::punctuation && op.text == "[") {
      expect("]");
      expect("=");
      op.text = "[]=";
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
    auto token = name(TokenKind::identifier);
    if (token.text == "owned") {
      auto type = parse_type(parameter);
      if (type.kind != Type::region || type.owned) fail(token.at, "owned requires a whole-region type");
      type.owned = true;
      return type;
    }
    if (token.text == "proc" || token.text == "procedure") {
      Type type{Type::procedure};
      expect("(");
      if (!take(")")) {
        do {
          if (type.variadic) fail(peek().at, "only the last procedure parameter can be variadic");
          type.parameters.push_back(parse_type());
          type.variadic = take("...");
        } while (take(","));
        expect(")");
      }
      expect("->");
      type.target = std::make_shared<Type>(parse_type());
      return type;
    }
    if (token.text == "ref") {
      expect("<");
      Type type{Type::reference};
      type.target = std::make_shared<Type>(parse_type());
      expect(">");
      return type;
    }
    Type type{Type::named, token.text};
    if (token.text == "region") {
      type = Type::region;
      if (take("<")) {
        type.target = std::make_shared<Type>(parse_type());
        expect(">");
      }
    }
    else if (token.text == "i64") type = Type::integer;
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
  void type_separator() { if (!take("::")) expect(":"); }
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
  void callable(Form &form, bool inline_effect = false) {
    form.conversion = take("conversion");
    if (form.conversion && !is("function")) fail(peek().at, "conversion requires a pure function");
    const bool function = take("function");
    if (!function && !take("proc")) expect("procedure");
    form.kind = function ? Form::Kind::function : Form::Kind::procedure;
    form.symbolic = !function && peek().kind == TokenKind::symbol;
    form.name = form.symbolic ? name(TokenKind::symbol).text : lexical_name(&form.operation).text;
    const bool parameter_list = take("(");
    if (parameter_list) {
      if (!take(")")) {
        do {
          take("ptr");
          auto param = value_name();
          Parameter parameter{param.text, Type::void_type, param.at, false, {}};
          type_separator();
          parameter.read_only = take("const");
          parameter.type = parse_type(&parameter);
          if (take("in")) parameter.owner = ordinary_name().text;
          if (parameter.type == Type::void_type) fail(param.at, "a parameter must have a value type");
          parameter.variadic = take("...");
          form.parameters.push_back(std::move(parameter));
        } while (take(","));
        expect(")");
      }
    } else if (function) {
      fail(peek().at, "a function requires a parameter list");
    }
    if (take("->") || take("::")) {
      form.result_const = take("const");
      form.result_type = parse_type();
      if (take("in")) form.result_owner = ordinary_name().text;
    }
    if (take("where")) {
      const bool grouped = take("{");
      do {
        if (is("region") || is("owner")) {
          const bool regional = take("region");
          if (!regional) expect("owner");
          auto owner = ordinary_name();
          OwnerParameter parameter{owner.text, {}, owner.at};
          if (regional) { type_separator(); parameter.root = parse_type(); }
          form.owner_parameters.push_back(std::move(parameter));
          continue;
        }
        auto parameter = ordinary_name();
        for (const auto &previous : form.type_parameters)
          if (previous.name == parameter.text) fail(parameter.at, "duplicate type parameter '" + parameter.text + "'");
        TypeParameter type_parameter{parameter.text, {}, parameter.at};
        if (take("<:")) type_parameter.bound = parse_type();
        form.type_parameters.push_back(std::move(type_parameter));
      } while (grouped && take(","));
      if (grouped) expect("}");
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
    if ((!function || declaration) && !form.result_type)
      fail(form.at, "a callable declaration requires an explicit result type");
    if (declaration && form.conversion)
      fail(form.at, "a conversion requires an implemented function method");
    if (!declaration) { form.body = block(); if (!inline_effect) take(";"); }
    if (inline_effect && (!form.symbolic || declaration || function))
      fail(form.at, "effect proc requires a symbolic procedure definition");
  }
  Expr primary() {
    auto token = peek();
    if (take("make")) {
      auto expr = make(Expression::Kind::make_region, name(TokenKind::identifier));
      expect("(");
      expect("capacity");
      expect("=");
      expr->children.push_back(expression());
      expect(")");
      expect("{");
      while (!take("}")) {
        auto field = make(Expression::Kind::field, field_name());
        expect("<-");
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
        if (take("::")) {
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
      return expr;
    }
    if (take("let")) {
      auto expr = make(Expression::Kind::let, value_name());
      if (take("::")) expr->annotation = parse_type();
      if (take("<-")) expr->binding = "<-";
      else { expect("="); expr->binding = "="; }
      expr->children.push_back(expression());
      return expr;
    }
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
    if (is("(") && ((is(")", 2) && operation_token(tokens_[cursor_ + 1])) ||
        (is("[", 1) && is("]", 2) && is("=", 3) && is(")", 4)))) {
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
      if (take(".")) {
        auto member = make(Expression::Kind::field, name(TokenKind::identifier));
        member->children.push_back(std::move(left));
        left = std::move(member);
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
    if (minimum == 0 && left->kind == Expression::Kind::index && is("=")) {
      auto assignment = make(Expression::Kind::call, expect("="));
      assignment->text = "[]=";
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
    return left;
  }
  Expr block() {
    auto result = make(Expression::Kind::block, expect("{"));
    while (!take("}")) {
      if (peek().kind == TokenKind::end) fail(peek().at, "unterminated scope");
      result->children.push_back(expression());
      if (!is("}")) expect(";");
      else take(";");
    }
    return result;
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
            if (!is("::") && !is(":")) fail(peek().at, "a symbol declaration requires an explicit type");
            type_separator();
            declaration->result_type = parse_type();
            if (region_symbol) {
              Type region{Type::region};
              region.target = std::make_shared<Type>(*declaration->result_type);
              declaration->result_type = std::move(region);
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
      } else if (is("type") || is("data") || is("regional")) {
        if (is("regional")) fail(start.at, "regional type is not a declaration; use data for a regional representation");
        const bool data = take("data");
        if (!data) expect("type");
        form->kind = Form::Kind::data;
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
          auto field = name(TokenKind::identifier);
          type_separator();
          auto type = parse_type();
          if (type == Type::void_type) fail(field.at, "a product field must have a value type");
          form->parameters.push_back({field.text, type, field.at, false, {}});
          if (!is("}")) expect(";");
        }
        take(";");
      } else if (is("function") || is("procedure") || is("proc") || is("conversion")) {
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
    return forms;
  }
};

} // namespace cod::language
