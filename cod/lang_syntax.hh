//===--- lang_syntax.hh - CoD source-language prototype syntax ------===//
#pragma once

#include <cctype>
#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
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
    if (c == '#' || (c == '/' && cursor + 1 < source.size() && source[cursor + 1] == '/')) {
      while (cursor < source.size() && source[cursor] != '\n')
        advance();
      continue;
    }
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
    if (cursor + 1 < source.size()) {
      const auto pair = source.substr(cursor, 2);
      if (pair == "->" || pair == "==" || pair == "!=" || pair == "<=" || pair == ">=" ||
          pair == "&&" || pair == "||" || pair == "::" || pair == "<-" || pair == "..") {
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
              named, product, unit, region, regional, array, reference, null_ref };
  Kind kind;
  std::string name;
  size_t identity = 0;
  Kind representation = void_type;
  std::shared_ptr<Type> target;
  Type(Kind kind = void_type, std::string name = {}, size_t identity = 0, Kind representation = void_type)
      : kind(kind), name(std::move(name)), identity(identity), representation(representation) {}
  bool operator==(const Type &other) const {
    return kind == other.kind && name == other.name && identity == other.identity &&
           representation == other.representation &&
           ((!target && !other.target) || (target && other.target && *target == *other.target));
  }
};
inline bool floating(Type type) { return type.kind == Type::float32 || type.kind == Type::float64; }
inline bool integral(Type type) {
  return type == Type::integer || type == Type::uint8 || type == Type::uint32 || type == Type::uint64;
}
inline bool numeric(Type type) { return integral(type) || floating(type); }
inline std::string type_name(const Type &type) {
  switch (type.kind) {
  case Type::integer: return "i64";
  case Type::uint8: return "u8";
  case Type::uint32: return "u32";
  case Type::uint64: return "u64";
  case Type::boolean: return "bool";
  case Type::void_type: return "void";
  case Type::procedure: return "procedure";
  case Type::float32: return "f32";
  case Type::float64: return "f64";
  case Type::named: case Type::product: case Type::unit: case Type::regional: case Type::array: return type.name;
  case Type::region: return "region";
  case Type::reference: return "ref<" + type_name(*type.target) + ">";
  case Type::null_ref: return "null";
  }
  return "unknown";
}

struct Expression {
  enum class Kind { integer, real, boolean, name, symbol, unary, binary, call, perform, behave, effect,
                    block, conditional, field, let, range_for, fold, allocate, index, null_ref };
  Kind kind;
  Location at;
  std::string text;
  std::vector<std::unique_ptr<Expression>> children;
  std::optional<size_t> symbol_id;
  std::string binding;
};
using Expr = std::unique_ptr<Expression>;

struct Parameter {
  std::string name;
  Type type;
  Location at;
  bool automatic_conversion = false;
  std::vector<std::string> conversion_route;
};
struct UnitSelection { std::string source, local; Location at; };
struct SymbolMapping {
  std::string local;
  std::string source;
  Location at;
};
struct Form {
  enum class Kind { symbol, symbol_from, function, procedure, product, array, unit, using_units, global, using_resource, import_resource, expression };
  Kind kind;
  Location at;
  std::string name;
  std::string path;
  std::string alias;
  bool symbolic = false;
  bool explicit_alias = false;
  bool using_symbol = false;
  bool conversion = false;
  bool regional = false;
  std::vector<SymbolMapping> mappings;
  std::vector<UnitSelection> units;
  std::vector<Parameter> parameters;
  std::optional<Type> result_type;
  std::optional<Type> index_type;
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

class Parser {
  std::vector<Token> tokens_;
  size_t cursor_ = 0;

  const Token &peek() const { return tokens_[cursor_]; }
  bool is(const std::string &text) const {
    return (peek().kind == TokenKind::identifier || peek().kind == TokenKind::punctuation) &&
           peek().text == text;
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
  Token lexical_name() {
    if (!take("(")) return name(TokenKind::identifier);
    auto op = tokens_[cursor_++];
    if (op.kind != TokenKind::punctuation || (binary_precedence(op.text) < 0 && op.text != "!"))
      fail(op.at, "expected an operator name");
    if (op.text == "&&" || op.text == "||") fail(op.at, "short-circuit operators are control expressions");
    expect(")");
    return op;
  }
  void resource(Form &form, bool symbolic) {
    if (peek().kind != TokenKind::string) fail(peek().at, "expected a quoted module path");
    form.path = tokens_[cursor_++].text;
    form.symbolic = symbolic || peek().kind == TokenKind::symbol;
    const auto kind = form.symbolic ? TokenKind::symbol : TokenKind::identifier;
    form.name = form.symbolic ? name(kind).text : lexical_name().text;
    form.explicit_alias = take("as");
    form.alias = form.explicit_alias ? (form.symbolic ? name(kind).text : lexical_name().text) : form.name;
  }
  Type parse_type() {
    auto token = name(TokenKind::identifier);
    if (token.text == "ref") {
      expect("<");
      Type type{Type::reference};
      type.target = std::make_shared<Type>(parse_type());
      expect(">");
      return type;
    }
    if (token.text == "region") return Type::region;
    if (token.text == "i64") return Type::integer;
    if (token.text == "u8") return Type::uint8;
    if (token.text == "u32") return Type::uint32;
    if (token.text == "u64") return Type::uint64;
    if (token.text == "bool") return Type::boolean;
    if (token.text == "void") return Type::void_type;
    if (token.text == "f32") return Type::float32;
    if (token.text == "f64") return Type::float64;
    return {Type::named, token.text};
  }
  void type_separator() { if (!take("::")) expect(":"); }
  Expr make(Expression::Kind kind, const Token &token) {
    return std::make_unique<Expression>(Expression{kind, token.at, token.text, {}, {}, {}});
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
        else expr.children.push_back(expression());
      } while (take(","));
      expect(")");
    }
  }
  Expr primary() {
    auto token = peek();
    if (take("alloc")) {
      auto owner = make(Expression::Kind::symbol, name(TokenKind::symbol));
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
                       name(TokenKind::identifier));
      expect("in");
      expr->children.push_back(expression());
      expect("..");
      expr->children.push_back(expression());
      if (fold) {
        expect(",");
        expr->binding = name(TokenKind::identifier).text;
        if (expr->text == expr->binding) fail(expr->at, "range and accumulator names must differ");
        expect("=");
        expr->children.push_back(expression());
      }
      expect(")");
      expr->children.push_back(block());
      return expr;
    }
    if (take("let")) {
      auto expr = make(Expression::Kind::let, name(TokenKind::identifier));
      expect("=");
      expr->children.push_back(expression());
      return expr;
    }
    if (take("effect")) {
      auto symbol = name(TokenKind::symbol);
      auto expr = make(Expression::Kind::effect, symbol);
      expect("=");
      expr->children.push_back(expression());
      return expr;
    }
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
    if (is("(") && cursor_ + 2 < tokens_.size() && tokens_[cursor_ + 2].text == ")" &&
        (binary_precedence(tokens_[cursor_ + 1].text) >= 0 || tokens_[cursor_ + 1].text == "!")) {
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
      if (token.text == "procedure" || token.text == "function" || token.text == "global" || token.text == "symbol")
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
      if (take("symbol")) {
        if (take("from")) {
          form->kind = Form::Kind::symbol_from;
          resource(*form, true);
        } else {
          form->kind = Form::Kind::symbol;
          form->name = name(TokenKind::symbol).text;
        }
        expect(";");
      } else if (take("unit")) {
        form->kind = Form::Kind::unit;
        form->name = name(TokenKind::identifier).text;
        type_separator();
        form->result_type = parse_type();
        if (!numeric(*form->result_type)) fail(start.at, "a unit requires a scalar numeric representation");
        expect(";");
      } else if (take("array")) {
        form->kind = Form::Kind::array;
        form->name = name(TokenKind::identifier).text;
        expect("[");
        form->index_type = parse_type();
        expect("]");
        type_separator();
        form->result_type = parse_type();
        expect(";");
      } else if (is("type") || is("regional")) {
        form->regional = take("regional");
        expect("type");
        form->kind = Form::Kind::product;
        form->name = name(TokenKind::identifier).text;
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
      } else if (is("function") || is("procedure") || is("conversion")) {
        form->conversion = take("conversion");
        if (form->conversion && !is("function")) fail(peek().at, "conversion requires a pure function");
        const bool function = take("function");
        if (!function) expect("procedure");
        form->kind = function ? Form::Kind::function : Form::Kind::procedure;
        form->name = function ? lexical_name().text : name(TokenKind::symbol).text;
        if (take("(")) {
          if (!take(")")) {
            do {
              auto param = name(TokenKind::identifier);
              Parameter parameter{param.text, Type::void_type, param.at, false, {}};
              if (take("(")) {
                parameter.type = parse_type();
                parameter.automatic_conversion = take("*");
                if (is("<-") && !parameter.automatic_conversion)
                  fail(peek().at, "a conversion route requires a starred parameter");
                while (take("<-")) parameter.conversion_route.push_back(name(TokenKind::identifier).text);
                expect(")");
              } else {
                type_separator();
                parameter.type = parse_type();
              }
              if (parameter.type == Type::void_type) fail(param.at, "a parameter must have a value type");
              form->parameters.push_back(std::move(parameter));
            } while (take(","));
            expect(")");
          }
        } else if (function) {
          fail(peek().at, "a function requires a parameter list");
        }
        if (take("->") || take("::")) form->result_type = parse_type();
        form->body = block();
        take(";");
      } else if (take("global")) {
        form->kind = Form::Kind::global;
        form->name = name(TokenKind::identifier).text;
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
      } else if (is("using") || is("import")) {
        const bool using_resource = take("using");
        if (!using_resource) expect("import");
        form->kind = using_resource ? Form::Kind::using_resource : Form::Kind::import_resource;
        if (take("units")) {
          if (!using_resource) fail(start.at, "UoMs support fresh definitions and using, not import");
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
          if (!using_resource) fail(start.at, "import symbol is outside the prototype; declare its destination first");
          form->using_symbol = true;
        }
        resource(*form, form->using_symbol);
        if (take("with")) {
          if (using_resource || !form->symbolic)
            fail(start.at, "a symbol map requires an imported procedure");
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
