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

[[noreturn]] inline void fail(const Location &at, const std::string &message) {
  throw std::runtime_error(at.file + ":" + std::to_string(at.line) + ":" +
                           std::to_string(at.column) + ": " + message);
}

enum class TokenKind { identifier, symbol, integer, string, punctuation, end };
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
      tokens.push_back({TokenKind::integer, source.substr(offset, cursor - offset), start});
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
          pair == "&&" || pair == "||") {
        punctuation = pair;
        advance();
      }
    }
    if (std::string("(){};,:+-*/%<>=!").find(c) == std::string::npos && punctuation.size() != 2)
      fail(start, "unsupported character in CoD source");
    advance();
    tokens.push_back({TokenKind::punctuation, punctuation, start});
  }
  tokens.push_back({TokenKind::end, "", at});
  return tokens;
}

enum class Type { integer, boolean, void_type, procedure };
inline std::string type_name(Type type) {
  switch (type) {
  case Type::integer: return "i64";
  case Type::boolean: return "bool";
  case Type::void_type: return "void";
  case Type::procedure: return "procedure";
  }
  return "unknown";
}

struct Expression {
  enum class Kind { integer, boolean, name, symbol, unary, binary, call, perform, behave, effect, block, conditional };
  Kind kind;
  Location at;
  std::string text;
  std::vector<std::unique_ptr<Expression>> children;
  std::optional<size_t> symbol_id;
};
using Expr = std::unique_ptr<Expression>;

struct Parameter {
  std::string name;
  Type type;
  Location at;
};
struct SymbolMapping {
  std::string local;
  std::string source;
  Location at;
};
struct Form {
  enum class Kind { symbol, symbol_from, function, procedure, global, using_resource, import_resource, expression };
  Kind kind;
  Location at;
  std::string name;
  std::string path;
  std::string alias;
  bool symbolic = false;
  bool explicit_alias = false;
  bool using_symbol = false;
  std::vector<SymbolMapping> mappings;
  std::vector<Parameter> parameters;
  std::optional<Type> result_type;
  Expr body;
};

class Parser {
  std::vector<Token> tokens_;
  size_t cursor_ = 0;

  const Token &peek() const { return tokens_[cursor_]; }
  bool is(const std::string &text) const { return peek().text == text; }
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
  void resource(Form &form, bool symbolic) {
    if (peek().kind != TokenKind::string) fail(peek().at, "expected a quoted module path");
    form.path = tokens_[cursor_++].text;
    form.symbolic = symbolic || peek().kind == TokenKind::symbol;
    const auto kind = form.symbolic ? TokenKind::symbol : TokenKind::identifier;
    form.name = name(kind).text;
    form.explicit_alias = take("as");
    form.alias = form.explicit_alias ? name(kind).text : form.name;
  }
  Type parse_type() {
    auto token = name(TokenKind::identifier);
    if (token.text == "i64") return Type::integer;
    if (token.text == "bool") return Type::boolean;
    if (token.text == "void") return Type::void_type;
    fail(token.at, "prototype supports i64, bool and void types");
  }
  static int precedence(const std::string &op) {
    if (op == "||") return 1;
    if (op == "&&") return 2;
    if (op == "==" || op == "!=") return 3;
    if (op == "<" || op == ">" || op == "<=" || op == ">=") return 4;
    if (op == "+" || op == "-") return 5;
    if (op == "*" || op == "/" || op == "%") return 6;
    return -1;
  }
  Expr make(Expression::Kind kind, const Token &token) {
    return std::make_unique<Expression>(Expression{kind, token.at, token.text, {}, {}});
  }
  void arguments(Expression &expr) {
    expect("(");
    if (!take(")")) {
      do { expr.children.push_back(expression()); } while (take(","));
      expect(")");
    }
  }
  Expr primary() {
    auto token = peek();
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
    if (take("(")) {
      auto expr = expression();
      expect(")");
      return expr;
    }
    if (take("-") || take("!")) {
      auto expr = make(Expression::Kind::unary, token);
      expr->children.push_back(primary());
      return expr;
    }
    if (token.kind == TokenKind::integer) {
      ++cursor_;
      return make(Expression::Kind::integer, token);
    }
    if (token.kind == TokenKind::symbol) {
      ++cursor_;
      return make(Expression::Kind::symbol, token);
    }
    if (token.kind == TokenKind::identifier) {
      ++cursor_;
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
    while (precedence(peek().text) >= minimum) {
      auto op = tokens_[cursor_++];
      auto binary = make(Expression::Kind::binary, op);
      binary->children.push_back(std::move(left));
      binary->children.push_back(expression(precedence(op.text) + 1));
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
      } else if (is("function") || is("procedure")) {
        const bool function = take("function");
        if (!function) expect("procedure");
        form->kind = function ? Form::Kind::function : Form::Kind::procedure;
        form->name = name(function ? TokenKind::identifier : TokenKind::symbol).text;
        if (take("(")) {
          if (!take(")")) {
            do {
              auto param = name(TokenKind::identifier);
              expect(":");
              const auto type = parse_type();
              if (type == Type::void_type) fail(param.at, "a parameter must have a value type");
              form->parameters.push_back({param.text, type, param.at});
            } while (take(","));
            expect(")");
          }
        } else if (function) {
          fail(peek().at, "a function requires a parameter list");
        }
        if (take("->")) form->result_type = parse_type();
        form->body = block();
        take(";");
      } else if (take("global")) {
        form->kind = Form::Kind::global;
        form->name = name(TokenKind::identifier).text;
        expect(":");
        form->result_type = parse_type();
        if (*form->result_type == Type::void_type) fail(start.at, "a bits global cannot be void");
        expect("=");
        form->body = expression();
        const auto kind = form->body->kind;
        const bool negative_literal = kind == Expression::Kind::unary && form->body->text == "-" &&
                                      form->body->children.front()->kind == Expression::Kind::integer;
        if (kind != Expression::Kind::integer && kind != Expression::Kind::boolean &&
            kind != Expression::Kind::call && !negative_literal)
          fail(start.at, "a bits global initializer must be a bits literal or function call");
        expect(";");
      } else if (is("using") || is("import")) {
        const bool using_resource = take("using");
        if (!using_resource) expect("import");
        form->kind = using_resource ? Form::Kind::using_resource : Form::Kind::import_resource;
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
