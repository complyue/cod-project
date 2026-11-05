//===--- lang_frontend.hh - Demand-checked CoD native lowering ------===//
#pragma once

#include "lang_syntax.hh"

#include <charconv>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <map>
#include <set>
#include <sstream>

namespace cod::language {

struct Module;
struct Procedure;
struct Value {
  Type type = Type::void_type;
  std::string code;
  std::shared_ptr<Procedure> procedure;
  Module *owner = nullptr;
  bool operator==(const Value &) const = default;
};
using Environment = std::map<size_t, Value>;
using Locals = std::map<std::string, Value>;
struct Function {
  Form *form;
  Module *lexical;
};
struct Procedure {
  Form *form = nullptr;
  Module *lexical = nullptr;
  std::string builtin;
  Environment captures;
  std::set<size_t> requirements;
};
struct Module {
  std::string path;
  std::vector<std::unique_ptr<Form>> forms;
  std::map<std::string, size_t> symbols;
  std::map<std::string, Form *> symbol_aliases;
  std::set<std::string> active_aliases;
  std::set<std::string> available_symbols;
  std::map<const Form *, Module *> dependencies;
  std::map<std::string, Function> functions;
  Locals globals;
  Environment exports;
  enum class State { registered, executing, complete } state = State::registered;
  bool declarations_only = true;
  bool builtin = false;
};
struct Compilation {
  std::string cpp;
  std::vector<std::string> scanned_modules;
  std::vector<std::string> execution_order;
};

class Compiler {
  std::map<std::string, std::unique_ptr<Module>> modules_;
  std::vector<std::string> symbol_names_;
  std::map<size_t, Type> symbol_types_;
  std::vector<std::string> scanned_;
  std::vector<std::string> execution_order_;
  std::ostringstream storage_;
  size_t serial_ = 0;
  std::set<const Form *> active_calls_;
  std::vector<std::unique_ptr<Form>> imported_forms_;

  std::string fresh(const char *prefix = "v") { return std::string(prefix) + std::to_string(serial_++); }
  static std::string cpp_type(Type type) { return type == Type::boolean ? "bool" : "std::int64_t"; }
  std::string store(Type type) {
    const auto name = fresh("saved");
    storage_ << "  " << cpp_type(type) << " " << name << "{};\n";
    return name;
  }
  Value scalar(Type type, const std::string &code, std::ostream &out) {
    const auto name = fresh();
    out << "const " << cpp_type(type) << " " << name << " = " << code << ";\n";
    return {type, name, {}, nullptr};
  }
  static void require(Type actual, Type expected, const Location &at) {
    if (actual != expected)
      fail(at, "expected " + type_name(expected) + ", got " + type_name(actual));
  }
  size_t new_symbol(Module &module, const std::string &name) {
    const auto id = symbol_names_.size();
    symbol_names_.push_back(module.path + "::@" + name);
    module.symbols.emplace(name, id);
    return id;
  }
  Module &load(const std::string &path, const Location &at) {
    const bool builtin = path == "std:io";
    std::error_code error;
    const auto canonical = builtin ? path : std::filesystem::canonical(path, error).string();
    if (error) fail(at, "cannot load module '" + path + "': " + error.message());
    if (const auto found = modules_.find(canonical); found != modules_.end()) return *found->second;
    auto module = std::make_unique<Module>();
    module->path = canonical;
    module->builtin = builtin;
    auto &result = *module;
    modules_.emplace(canonical, std::move(module));
    scanned_.push_back(canonical);
    if (builtin) {
      new_symbol(result, "print");
      new_symbol(result, "assert");
      return result;
    }
    std::ifstream input(canonical);
    if (!input) fail(at, "cannot read module '" + canonical + "'");
    const std::string source((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    result.forms = Parser(source, canonical).parse();
    std::set<std::string> lexical_names;
    for (const auto &owned : result.forms) {
      auto &form = *owned;
      if (form.kind == Form::Kind::symbol) {
        if (result.symbols.contains(form.name) || result.symbol_aliases.contains(form.name))
          fail(form.at, "duplicate symbol name @" + form.name);
        new_symbol(result, form.name);
      } else if (form.kind == Form::Kind::function || form.kind == Form::Kind::global) {
        if (!lexical_names.insert(form.name).second) fail(form.at, "duplicate lexical name '" + form.name + "'");
        if (form.kind == Form::Kind::function) result.functions.emplace(form.name, Function{&form, &result});
        else result.declarations_only = false;
      } else if (form.kind == Form::Kind::symbol_from) {
        result.declarations_only = false;
        if (result.symbols.contains(form.alias) || !result.symbol_aliases.emplace(form.alias, &form).second)
          fail(form.at, "duplicate symbol alias @" + form.alias);
      } else if (form.kind == Form::Kind::using_resource || form.kind == Form::Kind::import_resource) {
        result.declarations_only = false;
        if (!form.symbolic && !lexical_names.insert(form.alias).second) {
          fail(form.at, "duplicate lexical alias '" + form.alias + "'");
        }
      } else {
        result.declarations_only = false;
      }
    }
    for (const auto &owned : result.forms) {
      auto &form = *owned;
      if (form.kind != Form::Kind::symbol_from && form.kind != Form::Kind::using_resource &&
          form.kind != Form::Kind::import_resource) continue;
      const auto dependency = form.path == "std:io" ? form.path :
          (std::filesystem::path(canonical).parent_path() / form.path).string();
      result.dependencies.emplace(&form, &load(dependency, form.at));
    }
    return result;
  }
  size_t symbol(Module &module, const std::string &name, const Location &at,
                std::set<std::pair<Module *, std::string>> &visiting) {
    if (const auto found = module.symbols.find(name); found != module.symbols.end()) return found->second;
    const auto alias = module.symbol_aliases.find(name);
    if (alias == module.symbol_aliases.end()) fail(at, "undeclared symbol @" + name);
    if (!module.active_aliases.contains(name))
      fail(at, "symbol alias @" + name + " requires its symbol from encounter first in this prototype");
    if (!visiting.emplace(&module, name).second) fail(at, "cyclic symbol alias @" + name);
    auto *form = alias->second;
    const auto id = symbol(*module.dependencies.at(form), form->name, at, visiting);
    visiting.erase({&module, name});
    return id;
  }
  size_t symbol(Module &module, const std::string &name, const Location &at) {
    std::set<std::pair<Module *, std::string>> visiting;
    return symbol(module, name, at, visiting);
  }
  size_t expression_symbol(const Expression &expr, Module &module) {
    if (expr.symbol_id) return *expr.symbol_id;
    return symbol(module, expr.text, expr.at);
  }
  static bool uses_symbol(Expression::Kind kind) {
    using K = Expression::Kind;
    return kind == K::symbol || kind == K::effect || kind == K::perform || kind == K::behave;
  }
  Expr import_expression(const Expression &source_expr, Module &source, Module &receiver,
                         const std::map<std::string, std::string> &names,
                         std::map<size_t, size_t> &identities, std::set<std::string> &used) {
    auto expr = std::make_unique<Expression>(Expression{
        source_expr.kind, source_expr.at, source_expr.text, {}, source_expr.symbol_id});
    if (uses_symbol(expr->kind)) {
      used.insert(source_expr.text);
      const auto renamed = names.find(source_expr.text);
      expr->text = renamed == names.end() ? source_expr.text : renamed->second;
      expr->symbol_id = symbol(receiver, expr->text, expr->at);
      const auto source_id = expression_symbol(source_expr, source);
      const auto [found, added] = identities.emplace(source_id, *expr->symbol_id);
      if (!added && found->second != *expr->symbol_id)
        fail(expr->at, "splitting one source symbol identity across destinations is outside the prototype");
    }
    for (const auto &child : source_expr.children)
      expr->children.push_back(import_expression(*child, source, receiver, names, identities, used));
    return expr;
  }
  Form &import_body(const Procedure &source, const Form &selection, Module &receiver) {
    if (!source.form) fail(selection.at, "a built-in procedure has no importable body; use using");
    std::map<std::string, std::string> names;
    for (const auto &mapping : selection.mappings) {
      if (!names.emplace(mapping.source, mapping.local).second)
        fail(mapping.at, "duplicate source role @" + mapping.source + " in symbol map");
      symbol(receiver, mapping.local, mapping.at);
    }
    auto form = std::make_unique<Form>();
    form->kind = Form::Kind::procedure;
    form->at = selection.at;
    form->name = selection.alias;
    form->parameters = source.form->parameters;
    form->result_type = source.form->result_type;
    std::map<size_t, size_t> identities;
    std::set<std::string> used;
    form->body = import_expression(*source.form->body, *source.lexical, receiver, names, identities, used);
    for (const auto &mapping : selection.mappings)
      if (!used.contains(mapping.source))
        fail(mapping.at, "unknown source-body role @" + mapping.source + " in symbol map");
    auto &result = *form;
    imported_forms_.push_back(std::move(form));
    return result;
  }
  void fix_type(size_t id, Type type, const Location &at) {
    const auto [found, added] = symbol_types_.emplace(id, type);
    if (!added && found->second != type)
      fail(at, "incompatible value types for " + symbol_names_[id]);
  }
  Value read_symbol(size_t id, const Environment &env, const Location &at, std::ostream &out) {
    const auto found = env.find(id);
    if (found == env.end()) fail(at, "missing required binding " + symbol_names_[id]);
    if (found->second.type == Type::procedure) return found->second;
    const auto type = found->second.type;
    return scalar(type, type == Type::boolean ? "(ctx.s" + std::to_string(id) + " != 0)" :
                                              "ctx.s" + std::to_string(id), out);
  }
  void bind(size_t id, Value value, Module &owner, Environment &env, const Location &at, std::ostream &out) {
    if (value.type == Type::void_type) fail(at, "a symbol cannot be bound to void");
    fix_type(id, value.type, at);
    value.owner = &owner;
    if (value.type != Type::procedure)
      out << "Effect " << fresh("effect") << "(ctx.s" << id << ", " << value.code << ");\n";
    env[id] = std::move(value);
  }

  // Only used, externally supplied symbolic values become captures. A behave
  // dependency includes the selected callee's needs; perform supplies its own.
  void collect(const Expression &expr, Module &module, Environment &visible,
               std::set<size_t> &provided, std::set<size_t> &needed,
               std::set<const Procedure *> &visiting) {
    using K = Expression::Kind;
    if (expr.kind == K::block || expr.kind == K::conditional) {
      if (expr.kind == K::conditional) collect(*expr.children[0], module, visible, provided, needed, visiting);
      const size_t begin = expr.kind == K::conditional ? 1 : 0;
      auto inner_visible = visible;
      auto inner_provided = provided;
      for (size_t i = begin; i < expr.children.size(); ++i) {
        if (expr.kind == K::conditional) {
          inner_visible = visible;
          inner_provided = provided;
        }
        collect(*expr.children[i], module, inner_visible, inner_provided, needed, visiting);
      }
      return;
    }
    for (const auto &child : expr.children) collect(*child, module, visible, provided, needed, visiting);
    if (expr.kind == K::symbol || expr.kind == K::perform || expr.kind == K::behave) {
      const auto id = expression_symbol(expr, module);
      if (!provided.contains(id)) needed.insert(id);
      if (expr.kind == K::behave) {
        const auto callee = visible.find(id);
        if (callee != visible.end() && callee->second.procedure) {
          auto &proc = *callee->second.procedure;
          if (proc.form && visiting.insert(&proc).second) {
            auto next_visible = visible;
            auto next_provided = provided;
            collect(*proc.form->body, *proc.lexical, next_visible, next_provided, needed, visiting);
            visiting.erase(&proc);
          }
        }
      }
    } else if (expr.kind == K::effect) {
      const auto id = expression_symbol(expr, module);
      provided.insert(id);
      if (expr.children[0]->kind == K::symbol) {
        const auto source = expression_symbol(*expr.children[0], module);
        if (const auto found = visible.find(source); found != visible.end()) visible[id] = found->second;
      } else {
        visible.erase(id);
      }
    }
  }
  Value define_procedure(Form &form, Module &lexical, const Environment &env, std::ostream &out) {
    auto proc = std::make_shared<Procedure>();
    proc->form = &form;
    proc->lexical = &lexical;
    auto visible = env;
    std::set<size_t> provided;
    std::set<const Procedure *> visiting;
    collect(*form.body, lexical, visible, provided, proc->requirements, visiting);
    for (const auto id : proc->requirements) {
      const auto found = env.find(id);
      if (found == env.end()) continue; // A perform path must later prove the capture exists.
      auto value = found->second;
      if (value.type != Type::procedure) {
        value.code = store(value.type);
        out << value.code << " = ctx.s" << id << ";\n";
      }
      proc->captures.emplace(id, std::move(value));
    }
    return {Type::procedure, {}, std::move(proc), &lexical};
  }
  Value finish_scope(const std::string &body, Value value, std::ostream &out) {
    if (value.type == Type::procedure) fail({"<prototype>"}, "procedure-valued scope results are not supported");
    if (value.type == Type::void_type) {
      out << "{\n" << body << "}\n";
      return {};
    }
    const auto result = fresh("result");
    out << cpp_type(value.type) << " " << result << "{};\n{\n" << body
        << result << " = " << value.code << ";\n}\n";
    return {value.type, result, {}, nullptr};
  }
  Value block(const Expression &expr, Module &module, Environment env, Locals locals,
              bool pure, std::ostream &out) {
    std::ostringstream body;
    Value result;
    for (const auto &child : expr.children) result = expression(*child, module, env, locals, pure, body);
    return finish_scope(body.str(), result, out);
  }
  std::vector<Value> arguments(const Expression &expr, Module &module, Environment &env,
                               Locals &locals, bool pure, std::ostream &out) {
    std::vector<Value> values;
    for (const auto &arg : expr.children) values.push_back(expression(*arg, module, env, locals, pure, out));
    return values;
  }
  Value call(Form &form, Module &lexical, Environment env, const std::vector<Value> &args,
             bool pure, const Location &at, std::ostream &out) {
    if (args.size() != form.parameters.size()) fail(at, "wrong argument count for '" + form.name + "'");
    if (!active_calls_.insert(&form).second) fail(at, "recursive call is outside the first prototype");
    Locals locals;
    for (size_t i = 0; i < args.size(); ++i) {
      require(args[i].type, form.parameters[i].type, at);
      if (!locals.emplace(form.parameters[i].name, args[i]).second)
        fail(form.parameters[i].at, "duplicate parameter name");
    }
    auto result = block(*form.body, lexical, std::move(env), std::move(locals), pure, out);
    active_calls_.erase(&form);
    if (form.result_type) require(result.type, *form.result_type, form.at);
    return result;
  }
  Value invoke(const Expression &expr, Module &module, Environment &env, Locals &locals,
               bool pure, std::ostream &out) {
    if (pure) fail(expr.at, "a function cannot invoke a procedure");
    const auto id = expression_symbol(expr, module);
    const auto found = env.find(id);
    if (found == env.end()) fail(expr.at, "missing required procedure binding " + symbol_names_[id]);
    if (!found->second.procedure) fail(expr.at, "perform/behave requires a procedure value");
    const auto proc = found->second.procedure; // Resolve before arguments/capture installation.
    const auto args = arguments(expr, module, env, locals, pure, out);
    if (!proc->builtin.empty()) {
      if (args.size() != 1) fail(expr.at, "@" + proc->builtin + " expects one argument");
      if (proc->builtin == "assert") {
        require(args[0].type, Type::boolean, expr.at);
        out << "if (!(" << args[0].code << ")) runtime_failure(\"CoD assertion failed\");\n";
      } else if (args[0].type == Type::integer) {
        out << "std::printf(\"%lld\\n\", static_cast<long long>(" << args[0].code << "));\n";
      } else if (args[0].type == Type::boolean) {
        out << "std::puts(" << args[0].code << " ? \"true\" : \"false\");\n";
      } else {
        fail(expr.at, "@print expects i64 or bool");
      }
      return {};
    }
    std::ostringstream body;
    auto call_env = env;
    if (expr.kind == Expression::Kind::perform) {
      for (const auto required : proc->requirements)
        if (!proc->captures.contains(required))
          fail(expr.at, "perform requires an unavailable definition-time capture " + symbol_names_[required]);
      for (const auto &[capture, value] : proc->captures) bind(capture, value, *proc->lexical, call_env, expr.at, body);
    }
    auto result = call(*proc->form, *proc->lexical, std::move(call_env), args, false, expr.at, body);
    return finish_scope(body.str(), result, out);
  }
  Value expression(const Expression &expr, Module &module, Environment &env, Locals &locals,
                   bool pure, std::ostream &out) {
    using K = Expression::Kind;
    switch (expr.kind) {
    case K::integer: {
      std::int64_t value{};
      const auto parsed = std::from_chars(expr.text.data(), expr.text.data() + expr.text.size(), value);
      if (parsed.ec != std::errc{}) fail(expr.at, "integer literal is outside i64 range");
      return scalar(Type::integer, "INT64_C(" + std::to_string(value) + ")", out);
    }
    case K::boolean: return scalar(Type::boolean, expr.text, out);
    case K::name: {
      if (const auto local = locals.find(expr.text); local != locals.end()) return local->second;
      if (const auto global = module.globals.find(expr.text); global != module.globals.end()) return global->second;
      fail(expr.at, "unavailable lexical value '" + expr.text + "'");
    }
    case K::symbol:
      if (pure) fail(expr.at, "a function cannot read symbolic effects");
      return read_symbol(expression_symbol(expr, module), env, expr.at, out);
    case K::effect: {
      if (pure) fail(expr.at, "a function cannot establish effects");
      auto value = expression(*expr.children[0], module, env, locals, pure, out);
      bind(expression_symbol(expr, module), value, module, env, expr.at, out);
      return value;
    }
    case K::block: return block(expr, module, env, locals, pure, out);
    case K::call: {
      const auto function = module.functions.find(expr.text);
      if (function == module.functions.end()) fail(expr.at, "unavailable lexical function '" + expr.text + "'");
      const auto args = arguments(expr, module, env, locals, pure, out);
      return call(*function->second.form, *function->second.lexical, {}, args, true, expr.at, out);
    }
    case K::perform: case K::behave: return invoke(expr, module, env, locals, pure, out);
    case K::unary: {
      if (expr.text == "-" && expr.children[0]->kind == K::integer &&
          expr.children[0]->text == "9223372036854775808")
        return scalar(Type::integer, "std::numeric_limits<std::int64_t>::min()", out);
      auto value = expression(*expr.children[0], module, env, locals, pure, out);
      require(value.type, expr.text == "!" ? Type::boolean : Type::integer, expr.at);
      return scalar(value.type, expr.text == "!" ? "(!" + value.code + ")" :
                                              "checked_sub(0, " + value.code + ")", out);
    }
    case K::binary: {
      auto left = expression(*expr.children[0], module, env, locals, pure, out);
      if (expr.text == "&&" || expr.text == "||") {
        require(left.type, Type::boolean, expr.at);
        auto right_env = env;
        auto right_locals = locals;
        std::ostringstream right_body;
        const auto right = expression(*expr.children[1], module, right_env, right_locals, pure, right_body);
        require(right.type, Type::boolean, expr.at);
        if (right_env != env) fail(expr.at, "conditional rebinding in a logical operand needs an explicit scope");
        const auto result = fresh("logical");
        out << "bool " << result << " = " << left.code << ";\nif ("
            << (expr.text == "||" ? "!" : "") << result << ") {\n" << right_body.str()
            << result << " = " << right.code << ";\n}\n";
        return {Type::boolean, result, {}, nullptr};
      }
      auto right = expression(*expr.children[1], module, env, locals, pure, out);
      require(right.type, left.type, expr.at);
      if (expr.text == "==" || expr.text == "!=") {
        if (left.type != Type::integer && left.type != Type::boolean) fail(expr.at, "equality requires scalar bits");
        return scalar(Type::boolean, "(" + left.code + expr.text + right.code + ")", out);
      }
      require(left.type, Type::integer, expr.at);
      if (expr.text == "<" || expr.text == ">" || expr.text == "<=" || expr.text == ">=")
        return scalar(Type::boolean, "(" + left.code + expr.text + right.code + ")", out);
      const std::map<std::string, std::string> names{{"+", "add"}, {"-", "sub"}, {"*", "mul"}, {"/", "div"}, {"%", "rem"}};
      return scalar(Type::integer, "checked_" + names.at(expr.text) + "(" + left.code + ", " + right.code + ")", out);
    }
    case K::conditional: {
      const auto condition = expression(*expr.children[0], module, env, locals, pure, out);
      require(condition.type, Type::boolean, expr.at);
      std::ostringstream yes, no;
      const auto first = block(*expr.children[1], module, env, locals, pure, yes);
      const auto second = block(*expr.children[2], module, env, locals, pure, no);
      require(second.type, first.type, expr.at);
      if (first.type == Type::void_type) {
        out << "if (" << condition.code << ") {\n" << yes.str() << "} else {\n" << no.str() << "}\n";
        return {};
      }
      const auto result = fresh("choice");
      out << cpp_type(first.type) << " " << result << "{};\nif (" << condition.code << ") {\n"
          << yes.str() << result << " = " << first.code << ";\n} else {\n" << no.str()
          << result << " = " << second.code << ";\n}\n";
      return {first.type, result, {}, nullptr};
    }
    }
    fail(expr.at, "unsupported expression");
  }
  void execute_module(Module &module, const Environment &caller, const Location &at, std::ostream &out) {
    if (module.state == Module::State::complete) {
      if (!module.declarations_only) fail(at, "repeated execution encounter for '" + module.path +
                                             "' is outside the first prototype; use its locally bound resources");
      return;
    }
    if (module.state == Module::State::executing) fail(at, "cyclic module execution is outside the first prototype");
    module.state = Module::State::executing;
    execution_order_.push_back(module.path);
    if (module.builtin) {
      for (const auto &[name, id] : module.symbols) {
        auto proc = std::make_shared<Procedure>();
        proc->builtin = name;
        proc->lexical = &module;
        module.exports.emplace(id, Value{Type::procedure, {}, std::move(proc), &module});
      }
      module.state = Module::State::complete;
      return;
    }
    auto env = caller;
    Locals locals;
    std::ostringstream body;
    for (const auto &owned : module.forms) {
      auto &form = *owned;
      switch (form.kind) {
      case Form::Kind::symbol:
        module.available_symbols.insert(form.name);
        break;
      case Form::Kind::function: break;
      case Form::Kind::symbol_from: {
        auto &source = *module.dependencies.at(&form);
        execute_module(source, env, form.at, body);
        symbol(source, form.name, form.at);
        module.active_aliases.insert(form.alias);
        module.available_symbols.insert(form.alias);
        break;
      }
      case Form::Kind::procedure: {
        const auto id = symbol(module, form.name, form.at);
        bind(id, define_procedure(form, module, env, body), module, env, form.at, body);
        break;
      }
      case Form::Kind::global: {
        auto value = expression(*form.body, module, env, locals, false, body);
        require(value.type, *form.result_type, form.at);
        const auto saved = store(value.type);
        body << saved << " = " << value.code << ";\n";
        value.code = saved;
        module.globals.emplace(form.name, std::move(value));
        break;
      }
      case Form::Kind::expression:
        expression(*form.body, module, env, locals, false, body);
        break;
      case Form::Kind::using_resource: case Form::Kind::import_resource: {
        if (form.symbolic && !module.available_symbols.contains(form.alias))
          fail(form.at, "procedure destination @" + form.alias + " must be declared before using/import");
        auto &source = *module.dependencies.at(&form);
        execute_module(source, env, form.at, body);
        if (form.symbolic) {
          const auto source_id = symbol(source, form.name, form.at);
          const auto resource = source.exports.find(source_id);
          if (resource == source.exports.end() || !resource->second.procedure)
            fail(form.at, "source resource @" + form.name + " has no procedure value; use symbol from for identity");
          auto value = resource->second;
          if (form.kind == Form::Kind::import_resource)
            value = define_procedure(import_body(*value.procedure, form, module), module, env, body);
          const auto destination = symbol(module, form.alias, form.at);
          bind(destination, std::move(value), module, env, form.at, body);
        } else if (const auto function = source.functions.find(form.name); function != source.functions.end()) {
          if (form.kind == Form::Kind::import_resource)
            fail(form.at, "function-body import is outside the prototype; use 'using' for a lexical function");
          module.functions.emplace(form.alias, function->second);
        } else if (const auto value = source.globals.find(form.name); value != source.globals.end()) {
          module.globals.emplace(form.alias, value->second);
        } else {
          fail(form.at, "unavailable module resource '" + form.name + "'");
        }
        break;
      }
      }
    }
    for (const auto &[id, current] : env) {
      if (current.owner != &module) continue;
      auto value = current;
      if (value.type != Type::procedure) {
        value.code = store(value.type);
        body << value.code << " = ctx.s" << id << ";\n";
      }
      module.exports.emplace(id, std::move(value));
    }
    out << "{\n" << body.str() << "}\n";
    module.state = Module::State::complete;
  }

public:
  Compilation compile(const std::filesystem::path &entry) {
    auto &module = load(entry.string(), {entry.string()});
    // All source files have been parsed before any execution semantics are lowered.
    std::ostringstream body;
    execute_module(module, {}, {module.path}, body);
    std::ostringstream cpp;
    cpp << R"CPP(#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>

[[noreturn]] static void runtime_failure(const char *message) {
  std::fprintf(stderr, "%s\n", message);
  std::exit(70);
}
static std::int64_t checked_add(std::int64_t a, std::int64_t b) {
  std::int64_t result;
  if (__builtin_add_overflow(a, b, &result)) runtime_failure("i64 addition overflow");
  return result;
}
static std::int64_t checked_sub(std::int64_t a, std::int64_t b) {
  std::int64_t result;
  if (__builtin_sub_overflow(a, b, &result)) runtime_failure("i64 subtraction overflow");
  return result;
}
static std::int64_t checked_mul(std::int64_t a, std::int64_t b) {
  std::int64_t result;
  if (__builtin_mul_overflow(a, b, &result)) runtime_failure("i64 multiplication overflow");
  return result;
}
static std::int64_t checked_div(std::int64_t a, std::int64_t b) {
  if (b == 0) runtime_failure("division by zero");
  if (a == std::numeric_limits<std::int64_t>::min() && b == -1) runtime_failure("i64 division overflow");
  return a / b;
}
static std::int64_t checked_rem(std::int64_t a, std::int64_t b) {
  if (b == 0) runtime_failure("remainder by zero");
  if (a == std::numeric_limits<std::int64_t>::min() && b == -1) return 0;
  return a % b;
}
struct Effect {
  std::int64_t &slot;
  std::int64_t saved;
  Effect(std::int64_t &target, std::int64_t value) : slot(target), saved(target) { slot = value; }
  ~Effect() { slot = saved; }
  Effect(const Effect &) = delete;
  Effect &operator=(const Effect &) = delete;
};
struct Context {
)CPP";
    for (const auto &[id, type] : symbol_types_)
      if (type != Type::procedure) cpp << "  std::int64_t s" << id << "{};\n";
    cpp << "};\nstatic thread_local Context ctx;\nint main() {\n" << storage_.str() << body.str()
        << "return 0;\n}\n";
    return {cpp.str(), scanned_, execution_order_};
  }
};

} // namespace cod::language
