//===--- lang_frontend.hh - Demand-checked CoD native lowering ------===//
#pragma once

#include "lang_syntax.hh"
#include "lang_region_runtime.hh"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <map>
#include <queue>
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
  std::optional<size_t> region_id = {};
  bool operator==(const Value &) const = default;
};
using Environment = std::map<size_t, Value>;
using Locals = std::map<std::string, Value>;
struct Function {
  Form *form;
  Module *lexical;
  bool operator==(const Function &) const = default;
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
  std::map<std::string, std::vector<Function>> functions;
  std::map<std::string, Type> types;
  std::vector<Function> conversions;
  std::set<Module *> unit_sources;
  std::set<size_t> selected_units;
  Locals globals;
  Environment exports;
  enum class State { registered, executing, complete } state = State::registered;
  bool builtin = false;
};
struct Compilation {
  std::string cpp;
  std::vector<std::string> scanned_modules;
  std::vector<std::string> execution_order;
  std::vector<std::string> warnings;
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
  struct Product {
    Form *form;
    Module *module;
    std::vector<Parameter> fields;
    enum State { unresolved, resolving, complete } state = unresolved;
  };
  std::vector<Product> products_;
  struct Array {
    Form *form;
    Module *module;
    Type element, index;
    bool resolved = false;
  };
  std::vector<Array> arrays_;
  struct Region { std::string owner; bool active = true; };
  std::vector<Region> regions_;
  bool regional_runtime_ = false;
  std::ostringstream type_definitions_;
  size_t next_unit_ = 0;
  bool warnings_as_errors_ = false;
  std::vector<std::string> warnings_;

  static void require_user_type_name(const std::string &name, const Location &at) {
    static const std::set<std::string> reserved{"i64", "u8", "u32", "u64", "f32", "f64", "bool", "void", "region", "ref"};
    if (reserved.contains(name)) fail(at, "reserved scalar type name '" + name + "'");
  }

  std::string fresh(const char *prefix = "v") { return std::string(prefix) + std::to_string(serial_++); }
  static std::string cpp_type(const Type &type) {
    switch (type.kind) {
    case Type::integer: return "std::int64_t";
    case Type::uint8: return "std::uint8_t";
    case Type::uint32: return "std::uint32_t";
    case Type::uint64: return "std::uint64_t";
    case Type::boolean: return "bool";
    case Type::float32: return "float";
    case Type::float64: return "double";
    case Type::product: return "Record" + std::to_string(type.identity);
    case Type::reference: case Type::array: case Type::regional: return "std::uint64_t";
    case Type::unit: return cpp_type(type.representation);
    case Type::void_type: return "void";
    default: throw std::logic_error("unresolved native value type");
    }
  }
  static Type reference(Type target) {
    Type type{Type::reference};
    type.target = std::make_shared<Type>(std::move(target));
    return type;
  }
  static std::string payload_type(const Type &type) {
    if (type.kind == Type::regional) return "Record" + std::to_string(type.identity);
    if (type.kind == Type::array) return "Array" + std::to_string(type.identity);
    return cpp_type(type);
  }
  std::string region_of(const Value &value, const Location &at) {
    if (!value.region_id || !regions_.at(*value.region_id).active)
      fail(at, "regional value requires a live owning region in this caller scope");
    return "*" + regions_.at(*value.region_id).owner;
  }
  void close_regions(size_t begin, const Value &result, const Location &at) {
    if (result.region_id && *result.region_id >= begin)
      fail(at, "regional result would outlive its owning scope");
    for (size_t i = begin; i < regions_.size(); ++i) regions_[i].active = false;
  }
  Type resolve_name(Type type, Module &module, const Location &at) {
    if (type.kind != Type::named) return type;
    const auto found = module.types.find(type.name);
    if (found == module.types.end()) fail(at, "unavailable type '" + type.name + "'");
    return found->second;
  }
  Type resolve(Type type, Module &module, const Location &at) {
    type = resolve_name(type, module, at);
    if (type.kind == Type::reference) {
      auto target = resolve_name(*type.target, module, at);
      if (target.kind == Type::reference || target == Type::region || target == Type::void_type)
        fail(at, "a regional reference requires a bits or regional target type");
      if (target.kind != Type::regional) target = resolve(target, module, at);
      regional_runtime_ = true;
      return reference(target);
    }
    if (type.kind == Type::array) {
      auto &array = arrays_.at(type.identity);
      if (!array.resolved) {
        array.element = resolve_name(*array.form->result_type, *array.module, array.form->at);
        array.index = resolve_name(*array.form->index_type, *array.module, array.form->at);
        if (!numeric(array.element) && array.element != Type::boolean && array.element.kind != Type::unit)
          fail(at, "initial regional arrays store scalar bits; use parallel arrays for product fields");
        if (!integral(array.index) && !(array.index.kind == Type::unit && integral(array.index.representation)))
          fail(at, "an array index type requires an integer representation");
        type_definitions_ << "using " << payload_type(type) << " = CoDArray<" << cpp_type(array.element) << ">;\n";
        array.resolved = true;
      }
      regional_runtime_ = true;
    }
    if (type.kind == Type::product || type.kind == Type::regional) {
      auto &product = products_.at(type.identity);
      if (product.state == Product::resolving)
        fail(at, "recursive by-value product '" + type.name + "' has no finite layout");
      if (product.state == Product::unresolved) {
        product.state = Product::resolving;
        std::set<std::string> fields;
        for (const auto &field : product.form->parameters) {
          if (!fields.insert(field.name).second) fail(field.at, "duplicate product field '" + field.name + "'");
          const auto field_type = resolve(field.type, *product.module, field.at);
          if (field_type == Type::region || field_type.kind == Type::regional || field_type.kind == Type::array ||
              (field_type.kind == Type::reference && type.kind != Type::regional))
            fail(field.at, "regional fields require an explicit regional type and reference fields");
          product.fields.push_back({field.name, field_type, field.at, false, {}});
        }
        type_definitions_ << "struct " << payload_type(type) << " {\n";
        for (size_t i = 0; i < product.fields.size(); ++i) {
          const auto &field = product.fields[i].type;
          type_definitions_ << "  " << (field.kind == Type::reference ?
              "shilos::regional_ptr<" + payload_type(*field.target) + ">" : cpp_type(field)) << " f" << i << ";\n";
        }
        if (type.kind == Type::regional) {
          regional_runtime_ = true;
          const auto name = payload_type(type);
          type_definitions_ << "  " << name << "(CoDRegion &region";
          for (size_t i = 0; i < product.fields.size(); ++i)
            type_definitions_ << ", " << cpp_type(product.fields[i].type) << " a" << i << " = {}";
          type_definitions_ << ")";
          for (size_t i = 0; i < product.fields.size(); ++i) {
            const auto &field = product.fields[i].type;
            type_definitions_ << (i ? ", " : " : ") << "f" << i << "(";
            if (field.kind == Type::reference)
              type_definitions_ << "region_pointer<" << payload_type(*field.target) << ">(region, a" << i << ")";
            else type_definitions_ << "a" << i;
            type_definitions_ << ")";
          }
          type_definitions_ << " {}\n  " << name << "(const " << name << " &) = delete;\n  "
                            << name << "(" << name << " &&) = delete;\n  " << name << " &operator=(const "
                            << name << " &) = delete;\n  " << name << " &operator=(" << name << " &&) = delete;\n";
        }
        type_definitions_ << "};\n";
        product.state = Product::complete;
      }
    }
    return type;
  }
  std::string store(Type type) {
    const auto name = fresh("saved");
    storage_ << "  " << cpp_type(type) << " " << name << "{};\n";
    return name;
  }
  Value scalar(Type type, const std::string &code, std::ostream &out, std::optional<size_t> region_id = {}) {
    const auto name = fresh();
    out << "const " << cpp_type(type) << " " << name << " = " << code << ";\n";
    return {type, name, {}, nullptr, region_id};
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
    std::set<std::string> lexical_selections;
    for (const auto &owned : result.forms) {
      auto &form = *owned;
      if (form.kind == Form::Kind::symbol) {
        if (result.symbols.contains(form.name) || result.symbol_aliases.contains(form.name))
          fail(form.at, "duplicate symbol name @" + form.name);
        new_symbol(result, form.name);
      } else if (form.kind == Form::Kind::unit) {
        require_user_type_name(form.name, form.at);
        if (!lexical_names.insert(form.name).second) fail(form.at, "duplicate lexical name '" + form.name + "'");
        result.types.emplace(form.name, Type{Type::unit, result.path + "::" + form.name,
                                            next_unit_++, form.result_type->kind});
      } else if (form.kind == Form::Kind::array) {
        require_user_type_name(form.name, form.at);
        if (!lexical_names.insert(form.name).second) fail(form.at, "duplicate lexical name '" + form.name + "'");
        result.types.emplace(form.name, Type{Type::array, result.path + "::" + form.name, arrays_.size()});
        arrays_.push_back({&form, &result, {}, {}, false});
      } else if (form.kind == Form::Kind::product) {
        require_user_type_name(form.name, form.at);
        if (!lexical_names.insert(form.name).second) fail(form.at, "duplicate lexical name '" + form.name + "'");
        const Type type{form.regional ? Type::regional : Type::product, result.path + "::" + form.name, products_.size()};
        result.types.emplace(form.name, type);
        products_.push_back({&form, &result, {}});
      } else if (form.kind == Form::Kind::function || form.kind == Form::Kind::global) {
        if (!lexical_names.insert(form.name).second &&
            (form.kind != Form::Kind::function ||
             (!result.functions.contains(form.name) && !lexical_selections.contains(form.name))))
          fail(form.at, "duplicate lexical name '" + form.name + "'");
        if (form.kind == Form::Kind::function) {
          require_user_type_name(form.name, form.at);
          result.functions[form.name].push_back({&form, &result});
          if (form.conversion) result.conversions.push_back({&form, &result});
        }
      } else if (form.kind == Form::Kind::using_units) {
        for (const auto &selection : form.units) {
          require_user_type_name(selection.local, selection.at);
          if (!lexical_names.insert(selection.local).second)
            fail(selection.at, "duplicate lexical name '" + selection.local + "'");
        }
      } else if (form.kind == Form::Kind::symbol_from) {
        if (result.symbols.contains(form.alias) || !result.symbol_aliases.emplace(form.alias, &form).second)
          fail(form.at, "duplicate symbol alias @" + form.alias);
      } else if (form.kind == Form::Kind::using_resource || form.kind == Form::Kind::import_resource) {
        if (form.using_symbol && !result.symbols.contains(form.alias))
          result.symbol_aliases.try_emplace(form.alias, &form);
        if (!form.symbolic) {
          if (!lexical_names.insert(form.alias).second &&
              !result.functions.contains(form.alias) && !lexical_selections.contains(form.alias))
            fail(form.at, "duplicate lexical alias '" + form.alias + "'");
          lexical_selections.insert(form.alias);
        }
      }
    }
    for (const auto &owned : result.forms) {
      auto &form = *owned;
      if (form.kind != Form::Kind::symbol_from && form.kind != Form::Kind::using_resource &&
          form.kind != Form::Kind::import_resource && form.kind != Form::Kind::using_units) continue;
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
    auto *form = alias->second;
    if (!module.active_aliases.contains(name))
      fail(at, "symbol alias @" + name + " requires its " +
                   (form->using_symbol ? "using symbol" : "symbol from") + " encounter first in this prototype");
    if (!visiting.emplace(&module, name).second) fail(at, "cyclic symbol alias @" + name);
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
        source_expr.kind, source_expr.at, source_expr.text, {}, source_expr.symbol_id, source_expr.binding});
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
    if (found->second.region_id) region_of(found->second, at);
    if (found->second.type == Type::procedure || found->second.type == Type::region) return found->second;
    const auto type = found->second.type;
    return scalar(type, type == Type::boolean ? "(ctx.s" + std::to_string(id) + " != 0)" :
                                              "ctx.s" + std::to_string(id), out, found->second.region_id);
  }
  void bind(size_t id, Value value, Module &owner, Environment &env, const Location &at, std::ostream &out) {
    if (value.type == Type::void_type) fail(at, "a symbol cannot be bound to void");
    if (value.type.kind == Type::regional || value.type.kind == Type::array || value.type == Type::null_ref)
      fail(at, "bind bits or a typed regional reference, not a dereferenced regional object");
    if (value.region_id) region_of(value, at);
    fix_type(id, value.type, at);
    value.owner = &owner;
    if (value.type != Type::procedure && value.type != Type::region)
      out << "Effect " << fresh("effect") << "(ctx.s" << id << ", " << value.code << ");\n";
    env[id] = std::move(value);
  }

  // Capture analysis restores only bindings changed in a nested scope. Copying
  // the entire environment at each call makes deep inline chains needlessly
  // expensive, even when their bodies only read one dynamic value.
  struct CaptureState {
    Environment visible;
    std::set<size_t> provided;
    struct Change {
      size_t id;
      std::optional<Value> previous;
      bool newly_provided;
    };
    std::vector<Change> changes;

    void bind(size_t id, std::optional<Value> value) {
      const auto found = visible.find(id);
      changes.push_back({id, found == visible.end() ? std::nullopt : std::optional<Value>(found->second),
                         provided.insert(id).second});
      if (value) visible[id] = std::move(*value);
      else visible.erase(id);
    }
    void restore(size_t mark) {
      while (changes.size() > mark) {
        auto change = std::move(changes.back());
        changes.pop_back();
        if (change.previous) visible[change.id] = std::move(*change.previous);
        else visible.erase(change.id);
        if (change.newly_provided) provided.erase(change.id);
      }
    }
  };

  // Only used, externally supplied symbolic values become captures. A behave
  // dependency includes the selected callee's needs; perform supplies its own.
  void collect(const Expression &expr, Module &module, CaptureState &state,
               std::set<size_t> &needed, std::set<const Procedure *> &visiting) {
    using K = Expression::Kind;
    if (expr.kind == K::block || expr.kind == K::conditional) {
      if (expr.kind == K::conditional) collect(*expr.children[0], module, state, needed, visiting);
      const size_t begin = expr.kind == K::conditional ? 1 : 0;
      const auto mark = state.changes.size();
      for (size_t i = begin; i < expr.children.size(); ++i) {
        collect(*expr.children[i], module, state, needed, visiting);
        if (expr.kind == K::conditional) state.restore(mark);
      }
      state.restore(mark);
      return;
    }
    for (const auto &child : expr.children) collect(*child, module, state, needed, visiting);
    if (expr.kind == K::symbol || expr.kind == K::perform || expr.kind == K::behave) {
      const auto id = expression_symbol(expr, module);
      if (!state.provided.contains(id)) needed.insert(id);
      if (expr.kind == K::behave) {
        const auto callee = state.visible.find(id);
        if (callee != state.visible.end() && callee->second.procedure) {
          // Own the selected procedure while nested effects can replace its binding.
          const auto proc = callee->second.procedure;
          if (proc->form && visiting.insert(proc.get()).second) {
            const auto mark = state.changes.size();
            collect(*proc->form->body, *proc->lexical, state, needed, visiting);
            state.restore(mark);
            visiting.erase(proc.get());
          }
        }
      }
    } else if (expr.kind == K::effect) {
      const auto id = expression_symbol(expr, module);
      std::optional<Value> value;
      if (expr.children[0]->kind == K::symbol) {
        const auto source = expression_symbol(*expr.children[0], module);
        if (const auto found = state.visible.find(source); found != state.visible.end()) value = found->second;
      }
      state.bind(id, std::move(value));
    }
  }
  Value define_procedure(Form &form, Module &lexical, const Environment &env, std::ostream &out) {
    auto proc = std::make_shared<Procedure>();
    proc->form = &form;
    proc->lexical = &lexical;
    CaptureState state{env, {}, {}};
    std::set<const Procedure *> visiting;
    collect(*form.body, lexical, state, proc->requirements, visiting);
    for (const auto id : proc->requirements) {
      const auto found = env.find(id);
      if (found == env.end()) continue; // A perform path must later prove the capture exists.
      auto value = found->second;
      if (value.type != Type::procedure && value.type != Type::region) {
        value.code = store(value.type);
        out << value.code << " = ctx.s" << id << ";\n";
      }
      proc->captures.emplace(id, std::move(value));
    }
    return {Type::procedure, {}, std::move(proc), &lexical};
  }
  Value finish_scope(const std::string &body, Value value, std::ostream &out) {
    if (value.type == Type::procedure) fail({"<prototype>"}, "procedure-valued scope results are not supported");
    if (value.type == Type::void_type || value.type == Type::region || value.type == Type::null_ref) {
      out << "{\n" << body << "}\n";
      return value;
    }
    const auto result = fresh("result");
    out << cpp_type(value.type) << " " << result << "{};\n{\n" << body
        << result << " = " << value.code << ";\n}\n";
    return {value.type, result, {}, nullptr, value.region_id};
  }
  Value block(const Expression &expr, Module &module, Environment env, Locals locals,
              bool pure, std::ostream &out) {
    const auto region_begin = regions_.size();
    std::ostringstream body;
    Value result;
    for (const auto &child : expr.children) result = expression(*child, module, env, locals, pure, body);
    close_regions(region_begin, result, expr.at);
    // Fresh native names already delimit lexical values; pure scopes have no
    // dynamic resources to restore. Avoid native brace depth proportional to
    // a deeply inlined pure call chain.
    if (pure) { out << body.str(); return result; }
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
      require(args[i].type, resolve(form.parameters[i].type, lexical, form.parameters[i].at), at);
      if (args[i].region_id) region_of(args[i], at);
      if (pure && args[i].type == Type::region) fail(at, "a function reads input snapshots, not region resources");
      if (!locals.emplace(form.parameters[i].name, args[i]).second)
        fail(form.parameters[i].at, "duplicate parameter name");
    }
    auto result = block(*form.body, lexical, std::move(env), std::move(locals), pure, out);
    active_calls_.erase(&form);
    if (form.result_type) require(result.type, resolve(*form.result_type, lexical, form.at), form.at);
    return result;
  }
  struct Conversion {
    Function function;
    Type source, target;
  };
  struct Route {
    std::vector<Function> steps;
    std::vector<Function> alternative;
  };
  struct MethodMatch {
    Function function;
    std::vector<Route> routes;
  };
  Conversion conversion(Function function) {
    auto &form = *function.form;
    if (form.parameters.size() != 1 || !form.result_type || form.parameters[0].automatic_conversion)
      fail(form.at, "a conversion function requires one exact UoM input and an explicit UoM result");
    auto source = resolve(form.parameters[0].type, *function.lexical, form.at);
    auto target = resolve(*form.result_type, *function.lexical, form.at);
    if (source.kind != Type::unit || target.kind != Type::unit)
      fail(form.at, "a conversion function must connect UoM types");
    return {function, source, target};
  }
  std::vector<Conversion> conversion_graph(Module &module) {
    std::vector<Conversion> result;
    for (const auto &function : module.conversions) result.push_back(conversion(function));
    return result;
  }
  static std::optional<std::vector<Function>> shortest_route(
      const std::vector<Conversion> &graph, Type source, Type target,
      const std::optional<Function> &excluded = {}) {
    struct Step { size_t unit; std::vector<Function> path; };
    std::queue<Step> pending;
    std::set<size_t> visited{source.identity};
    pending.push({source.identity, {}});
    while (!pending.empty()) {
      auto step = std::move(pending.front());
      pending.pop();
      for (const auto &edge : graph) {
        if (edge.source.identity != step.unit || (excluded && edge.function == *excluded)) continue;
        if (!visited.insert(edge.target.identity).second) continue;
        auto path = step.path;
        path.push_back(edge.function);
        if (edge.target == target) return path;
        pending.push({edge.target.identity, std::move(path)});
      }
    }
    return {};
  }
  std::optional<Route> route(Type actual, const Parameter &parameter, Module &lexical,
                              Module &caller, const Location &at, bool diagnose_unavailable = true) {
    const auto target = resolve(parameter.type, lexical, parameter.at);
    if (parameter.automatic_conversion && target.kind != Type::unit)
      fail(parameter.at, "a starred parameter requires a UoM type");
    if (actual == target) return Route{};
    if (!parameter.automatic_conversion || actual.kind != Type::unit) return {};
    const auto graph = conversion_graph(caller);
    Route result;
    for (const auto &edge : graph) {
      if (edge.source != actual || edge.target != target) continue;
      if (result.steps.empty()) result.steps = {edge.function};
      else if (result.alternative.empty()) result.alternative = {edge.function};
    }
    if (!result.steps.empty()) return result;
    if (!parameter.conversion_route.empty()) {
      std::vector<Type> declared{target};
      std::set<size_t> visited{target.identity};
      for (const auto &name : parameter.conversion_route) {
        const auto unit = resolve({Type::named, name}, lexical, parameter.at);
        if (unit.kind != Type::unit || !visited.insert(unit.identity).second)
          fail(parameter.at, "declared conversion route requires distinct UoM identities");
        declared.push_back(unit);
      }
      size_t start = declared.size();
      for (size_t i = 1; i < declared.size(); ++i) if (declared[i] == actual) start = i;
      if (start == declared.size()) return {};
      for (size_t i = start; i > 0; --i) {
        std::optional<Function> selected, alternative;
        for (const auto &edge : graph) {
          if (edge.source != declared[i] || edge.target != declared[i-1]) continue;
          if (!selected) selected = edge.function;
          else if (!alternative) alternative = edge.function;
        }
        if (!selected) {
          if (!diagnose_unavailable) return {};
          fail(at, "declared conversion route has an unavailable step from " +
                       type_name(declared[i]) + " to " + type_name(declared[i-1]));
        }
        if (alternative && result.alternative.empty()) {
          result.alternative = result.steps;
          result.alternative.push_back(*alternative);
        } else if (!result.alternative.empty()) result.alternative.push_back(*selected);
        result.steps.push_back(*selected);
      }
      return result;
    }
    const auto chosen = shortest_route(graph, actual, target);
    if (!chosen) return {};
    result.steps = *chosen;
    // Any distinct simple path omits at least one edge of the chosen path.
    // Find a witness without enumerating exponentially many cyclic walks.
    for (const auto &edge : result.steps) {
      if (auto alternative = shortest_route(graph, actual, target, edge)) {
        result.alternative = std::move(*alternative);
        break;
      }
    }
    return result;
  }
  std::optional<MethodMatch> method(const std::string &name, Module &module,
                                    const std::vector<Value> &args, const Location &at) {
    const auto found = module.functions.find(name);
    if (found == module.functions.end()) return {};
    std::vector<MethodMatch> exact, converted;
    for (const auto &candidate : found->second) {
      if (candidate.form->parameters.size() != args.size()) continue;
      bool matches = true;
      for (size_t i = 0; i < args.size(); ++i) {
        const auto &parameter = candidate.form->parameters[i];
        const auto type = resolve(parameter.type, *candidate.lexical, parameter.at);
        if (parameter.automatic_conversion && type.kind != Type::unit)
          fail(parameter.at, "a starred parameter requires a UoM type");
        matches = matches && args[i].type == type;
      }
      if (matches) exact.push_back({candidate, std::vector<Route>(args.size())});
    }
    if (exact.size() > 1) fail(at, "ambiguous method '" + name + "' for supplied argument types");
    if (!exact.empty()) return exact.front();
    for (const auto &candidate : found->second) {
      if (candidate.form->parameters.size() != args.size()) continue;
      MethodMatch match{candidate, {}};
      bool matches = true, needs_conversion = false;
      for (size_t i = 0; i < args.size(); ++i) {
        auto path = route(args[i].type, candidate.form->parameters[i], *candidate.lexical, module, at, false);
        if (!path) { matches = false; break; }
        needs_conversion = needs_conversion || !path->steps.empty();
        match.routes.push_back(std::move(*path));
      }
      if (matches) (needs_conversion ? converted : exact).push_back(std::move(match));
    }
    const auto &candidates = exact.empty() ? converted : exact;
    if (candidates.size() > 1) fail(at, "ambiguous method '" + name + "' for supplied argument types");
    if (candidates.empty()) {
      // A missing edge makes an overload inapplicable. Preserve the detailed
      // diagnostic for an unambiguous declaration after considering alternatives.
      if (found->second.size() == 1) {
        const auto &candidate = found->second.front();
        if (candidate.form->parameters.size() == args.size())
          for (size_t i = 0; i < args.size(); ++i)
            route(args[i].type, candidate.form->parameters[i], *candidate.lexical, module, at);
      }
      return {};
    }
    return candidates.front();
  }
  Value converted(Value value, const Route &route, const Location &at, std::ostream &out) {
    if (!route.alternative.empty()) {
      auto describe = [&](const std::vector<Function> &path) {
        std::string result = type_name(value.type);
        for (const auto &step : path)
          result += " --" + step.form->name + "--> " + type_name(conversion(step).target);
        return result;
      };
      const auto message = "ambiguous unit conversion; selected " + describe(route.steps) +
                           "; alternative " + describe(route.alternative) + " [-Wunit-conversion]";
      if (warnings_as_errors_) fail(at, "warning treated as error: " + message);
      warnings_.push_back(at.file + ":" + std::to_string(at.line) + ":" + std::to_string(at.column) +
                          ": warning: " + message);
    }
    for (const auto &step : route.steps)
      value = call(*step.form, *step.lexical, {}, {value}, true, at, out);
    return value;
  }
  Value call_method(const MethodMatch &match, std::vector<Value> args, const Location &at, std::ostream &out) {
    for (size_t i = 0; i < args.size(); ++i) args[i] = converted(args[i], match.routes[i], at, out);
    return call(*match.function.form, *match.function.lexical, {}, args, true, at, out);
  }
  Type array_type(const Value &value, const Location &at) {
    const auto type = value.type.kind == Type::reference ? *value.type.target : value.type;
    if (type.kind != Type::array) fail(at, "expected a regional array");
    region_of(value, at);
    return type;
  }
  Value array_element(Value array, Value index, const Location &at, std::ostream &out, bool as_reference = false) {
    const auto type = array_type(array, at);
    const auto &layout = arrays_.at(type.identity);
    require(index.type, layout.index, at);
    const auto result_type = as_reference ? reference(layout.element) : layout.element;
    return scalar(result_type, std::string(as_reference ? "array_reference<" : "array_read<") +
        cpp_type(layout.element) + ">(" + region_of(array, at) + ", " + array.code +
        ", checked_cast<std::uint64_t>(" + index.code + "))", out, as_reference ? array.region_id : std::nullopt);
  }
  Value invoke(const Expression &expr, Module &module, Environment &env, Locals &locals,
               bool pure, std::ostream &out) {
    if (pure) fail(expr.at, "a function cannot invoke a procedure");
    const auto id = expression_symbol(expr, module);
    const auto found = env.find(id);
    if (found == env.end()) fail(expr.at, "missing required procedure binding " + symbol_names_[id]);
    if (!found->second.procedure) fail(expr.at, "perform/behave requires a procedure value");
    const auto proc = found->second.procedure; // Resolve before arguments/capture installation.
    auto args = arguments(expr, module, env, locals, pure, out);
    if (!proc->builtin.empty()) {
      if (args.size() != 1) fail(expr.at, "@" + proc->builtin + " expects one argument");
      if (proc->builtin == "assert") {
        require(args[0].type, Type::boolean, expr.at);
        out << "if (!(" << args[0].code << ")) runtime_failure(\"CoD assertion failed\");\n";
      } else if (args[0].type == Type::integer) {
        out << "std::printf(\"%lld\\n\", static_cast<long long>(" << args[0].code << "));\n";
      } else if (integral(args[0].type)) {
        out << "std::printf(\"%llu\\n\", static_cast<unsigned long long>(" << args[0].code << "));\n";
      } else if (args[0].type == Type::boolean) {
        out << "std::puts(" << args[0].code << " ? \"true\" : \"false\");\n";
      } else if (floating(args[0].type)) {
        out << "std::printf(\"" << (args[0].type == Type::float32 ? "%.9g" : "%.17g")
            << "\\n\", static_cast<double>(" << args[0].code << "));\n";
      } else {
        fail(expr.at, "@print expects a numeric scalar or bool");
      }
      return {};
    }
    if (args.size() == proc->form->parameters.size()) {
      for (size_t i = 0; i < args.size(); ++i) {
        if (const auto path = route(args[i].type, proc->form->parameters[i], *proc->lexical, module, expr.at))
          args[i] = converted(args[i], *path, expr.at, out);
      }
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
  Value allocate_expression(const Expression &expr, Module &module, Environment &env, Locals &locals,
                          bool pure, std::ostream &out) {
    if (pure) fail(expr.at, "a function cannot allocate regional storage");
    auto owner = expression(*expr.children[0], module, env, locals, pure, out);
    require(owner.type, Type::region, expr.at);
    const auto region = region_of(owner, expr.at);
    auto type = resolve({Type::named, expr.text}, module, expr.at);
    if (type.kind != Type::array && type.kind != Type::regional)
      fail(expr.at, "alloc requires a regional type or regional array");
    std::vector<Value> args;
    for (size_t i = 1; i < expr.children.size(); ++i)
      args.push_back(expression(*expr.children[i], module, env, locals, pure, out));
    std::string values;
    if (type.kind == Type::array) {
      if (args.size() != 2 || !integral(args[0].type))
        fail(expr.at, "array allocation requires an integer length and initial element value");
      require(args[1].type, arrays_.at(type.identity).element, expr.at);
      values = "checked_cast<std::uint64_t>(" + args[0].code + "), " + args[1].code;
    } else {
      const auto &fields = products_.at(type.identity).fields;
      if (args.size() != fields.size()) fail(expr.at, "wrong field count for regional allocation");
      for (size_t i = 0; i < args.size(); ++i) {
        if (!(args[i].type == Type::null_ref && fields[i].type.kind == Type::reference))
          require(args[i].type, fields[i].type, expr.at);
        if (args[i].region_id && args[i].region_id != owner.region_id)
          fail(expr.at, "cross-region reference fields are outside this prototype");
        if (i) values += ", ";
        values += args[i].code;
      }
    }
    return scalar(reference(type), "region_offset(" + region + ", (" + region + ").create<" +
        payload_type(type) + ">(" + values + ").get())", out, owner.region_id);
  }
  Value loop_expression(const Expression &expr, Module &module, Environment &env, Locals &locals,
                          bool pure, std::ostream &out) {
    using K = Expression::Kind;
    const auto first = expression(*expr.children[0], module, env, locals, pure, out);
    const auto last = expression(*expr.children[1], module, env, locals, pure, out);
    require(last.type, first.type, expr.at);
    if (!integral(first.type) && !(first.type.kind == Type::unit && integral(first.type.representation)))
      fail(expr.at, "a range requires integer endpoints or one integer-backed UoM identity");
    const auto index = fresh("index");
    auto inner_locals = locals;
    inner_locals[expr.text] = {first.type, index, {}, nullptr};
    Value accumulator;
    if (expr.kind == K::fold) {
      accumulator = expression(*expr.children[2], module, env, locals, pure, out);
      if (accumulator.type == Type::void_type || accumulator.type == Type::procedure ||
          accumulator.type == Type::region || accumulator.type == Type::null_ref ||
          accumulator.type.kind == Type::array || accumulator.type.kind == Type::regional)
        fail(expr.at, "a fold accumulator requires a bits value");
      const auto name = fresh("accumulator");
      out << cpp_type(accumulator.type) << " " << name << " = " << accumulator.code << ";\n";
      accumulator.code = name;
      inner_locals[expr.binding] = accumulator;
    }
    std::ostringstream body;
    const auto next = block(*expr.children.back(), module, env, std::move(inner_locals), pure, body);
    out << "for (" << cpp_type(first.type) << " " << index << " = " << first.code << "; "
        << index << " < " << last.code << "; ++" << index << ") {\n" << body.str();
    if (expr.kind == K::fold) {
      require(next.type, accumulator.type, expr.at);
      if (next.region_id != accumulator.region_id)
        fail(expr.at, "a fold reference must retain its owning region");
      out << accumulator.code << " = " << next.code << ";\n";
    }
    out << "}\n";
    return accumulator;
  }
  Value call_expression(const Expression &expr, Module &module, Environment &env, Locals &locals,
                          bool pure, std::ostream &out) {
    using K = Expression::Kind;
    if (expr.text == "region") {
      if (pure) fail(expr.at, "a function cannot create a region resource");
      const auto args = arguments(expr, module, env, locals, pure, out);
      if (args.size() != 1 || !integral(args[0].type)) fail(expr.at, "region requires an integer byte capacity");
      regional_runtime_ = true;
      const auto owner = fresh("region");
      const auto id = regions_.size();
      regions_.push_back({owner, true});
      out << "CoDRegionOwner " << owner << "(region_capacity(checked_cast<std::uint64_t>("
          << args[0].code << ")));\n";
      return {Type::region, owner, {}, nullptr, id};
    }
    if (expr.text == "used_bytes" || expr.text == "capacity_bytes") {
      if (pure) fail(expr.at, "a function cannot inspect a region resource");
      const auto args = arguments(expr, module, env, locals, pure, out);
      if (args.size() != 1) fail(expr.at, "region size requires one resource");
      require(args[0].type, Type::region, expr.at);
      return scalar(Type::uint64, "(" + region_of(args[0], expr.at) + ")." +
          (expr.text == "used_bytes" ? "occupation()" : "capacity()"), out);
    }
    if (expr.text == "deref") {
      const auto args = arguments(expr, module, env, locals, pure, out);
      if (args.size() != 1 || args[0].type.kind != Type::reference)
        fail(expr.at, "deref requires a typed regional reference");
      const auto type = resolve(*args[0].type.target, module, expr.at);
      const auto region = region_of(args[0], expr.at);
      if (type.kind == Type::array || type.kind == Type::regional)
        return {type, args[0].code, {}, nullptr, args[0].region_id};
      return scalar(type, "region_read<" + payload_type(type) + ">(" + region + ", " + args[0].code + ")", out);
    }
    if (expr.text == "len" || expr.text == "ref_at") {
      const auto args = arguments(expr, module, env, locals, pure, out);
      if (args.size() != (expr.text == "len" ? 1u : 2u)) fail(expr.at, "wrong array argument count");
      if (expr.text == "ref_at") return array_element(args[0], args[1], expr.at, out, true);
      const auto type = array_type(args[0], expr.at);
      return scalar(Type::uint64, "region_read<" + payload_type(type) + ">(" + region_of(args[0], expr.at) +
                    ", " + args[0].code + ").size", out);
    }
    if (expr.text == "store") {
      if (pure) fail(expr.at, "a function cannot modify regional storage");
      const auto args = arguments(expr, module, env, locals, pure, out);
      if (args.size() == 2 && args[0].type.kind == Type::reference &&
          (numeric(*args[0].type.target) || *args[0].type.target == Type::boolean || args[0].type.target->kind == Type::unit)) {
        require(args[1].type, *args[0].type.target, expr.at);
        out << "if (!" << args[0].code << ") runtime_failure(\"null regional reference\");\n";
        out << "*region_pointer<" << cpp_type(args[1].type) << ">(" << region_of(args[0], expr.at)
            << ", " << args[0].code << ").get() = " << args[1].code << ";\n";
        return args[1];
      }
      if (args.size() != 3) fail(expr.at, "store requires an array/index/value or scalar reference/value");
      const auto type = array_type(args[0], expr.at);
      const auto &layout = arrays_.at(type.identity);
      require(args[1].type, layout.index, expr.at);
      require(args[2].type, layout.element, expr.at);
      return scalar(layout.element, "array_store<" + cpp_type(layout.element) + ">(" + region_of(args[0], expr.at) +
          ", " + args[0].code + ", checked_cast<std::uint64_t>(" + args[1].code + "), " + args[2].code + ")", out);
    }
    if (expr.text == "setfield") {
      if (pure) fail(expr.at, "a function cannot modify regional storage");
      if (expr.children.size() != 3 || expr.children[1]->kind != K::name)
        fail(expr.at, "setfield requires a reference, field name and value");
      const auto object = expression(*expr.children[0], module, env, locals, pure, out);
      if (object.type.kind != Type::reference || object.type.target->kind != Type::regional)
        fail(expr.at, "setfield requires a reference to a regional record");
      const auto type = resolve(*object.type.target, module, expr.at);
      auto value = expression(*expr.children[2], module, env, locals, pure, out);
      const auto &fields = products_.at(type.identity).fields;
      for (size_t i = 0; i < fields.size(); ++i) {
        if (fields[i].name != expr.children[1]->text) continue;
        const auto field = fields[i].type;
        if (value.type == Type::null_ref && field.kind == Type::reference) value.type = field;
        require(value.type, field, expr.at);
        if (value.region_id && value.region_id != object.region_id)
          fail(expr.at, "cross-region reference fields are outside this prototype");
        const auto region = region_of(object, expr.at);
        out << "if (!" << object.code << ") runtime_failure(\"null regional reference\");\n";
        out << "region_pointer<" << payload_type(type) << ">(" << region << ", " << object.code << ")->f" << i << " = ";
        if (field.kind == Type::reference) {
          out << "region_pointer<" << payload_type(*field.target) << ">(" << region << ", " << value.code << ")";
          value.region_id = object.region_id;
        } else out << value.code;
        out << ";\n";
        return value;
      }
      fail(expr.at, "no such regional field '" + expr.children[1]->text + "'");
    }
    static const std::map<std::string, Type> casts{{"i64", Type::integer}, {"u8", Type::uint8},
        {"u32", Type::uint32}, {"u64", Type::uint64}, {"f32", Type::float32}, {"f64", Type::float64}};
    if (const auto cast = casts.find(expr.text); cast != casts.end()) {
      const auto args = arguments(expr, module, env, locals, pure, out);
      if (args.size() != 1 || !numeric(args[0].type)) fail(expr.at, "numeric conversion requires one scalar value");
      return scalar(cast->second, "checked_cast<" + cpp_type(cast->second) + ">(" + args[0].code + ")", out);
    }
    if (const auto found = module.types.find(expr.text); found != module.types.end()) {
      const auto type = resolve(found->second, module, expr.at);
      if (type.kind == Type::regional || type.kind == Type::array)
        fail(expr.at, "regional objects require alloc in a region resource");
      const auto args = arguments(expr, module, env, locals, pure, out);
      if (type.kind == Type::unit) {
        if (args.size() != 1) fail(expr.at, "a UoM constructor requires one scalar value");
        require(args[0].type, type.representation, expr.at);
        return {type, args[0].code, {}, nullptr};
      }
      const auto &fields = products_.at(type.identity).fields;
      if (args.size() != fields.size()) fail(expr.at, "wrong field count for '" + expr.text + "'");
      std::string values;
      for (size_t i = 0; i < args.size(); ++i) {
        require(args[i].type, fields[i].type, expr.at);
        if (i) values += ", ";
        values += args[i].code;
      }
      return scalar(type, cpp_type(type) + "{" + values + "}", out);
    }
    const auto function = module.functions.find(expr.text);
    if (expr.text == "scalar" && function == module.functions.end()) {
      const auto args = arguments(expr, module, env, locals, pure, out);
      if (args.size() != 1 || args[0].type.kind != Type::unit)
        fail(expr.at, "scalar requires one UoM value");
      return {args[0].type.representation, args[0].code, {}, nullptr};
    }
    if (function == module.functions.end()) fail(expr.at, "unavailable lexical function '" + expr.text + "'");
    const auto args = arguments(expr, module, env, locals, pure, out);
    auto selected = method(expr.text, module, args, expr.at);
    if (!selected && function->second.size() == 1) {
      const auto &candidate = function->second.front();
      return call(*candidate.form, *candidate.lexical, {}, args, true, expr.at, out);
    }
    if (!selected) fail(expr.at, "no matching method for '" + expr.text + "'");
    return call_method(*selected, args, expr.at, out);
  }
  Value binary_expression(const Expression &expr, Module &module, Environment &env, Locals &locals,
                          bool pure, std::ostream &out) {
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
    if (const auto selected = method(expr.text, module, {left, right}, expr.at))
      return call_method(*selected, {left, right}, expr.at, out);
    if ((expr.text == "==" || expr.text == "!=") &&
        (left.type.kind == Type::reference || left.type == Type::null_ref) &&
        (right.type.kind == Type::reference || right.type == Type::null_ref)) {
      if (left.type != Type::null_ref && right.type != Type::null_ref) require(right.type, left.type, expr.at);
      auto equality = "(" + left.code + " == " + right.code + ")";
      if (left.region_id && right.region_id && left.region_id != right.region_id)
        equality = "(" + left.code + " == 0 && " + right.code + " == 0)";
      return scalar(Type::boolean, expr.text == "==" ? equality : "(!" + equality + ")", out);
    }
    require(right.type, left.type, expr.at);
    if (expr.text == "==" || expr.text == "!=") {
      if (!numeric(left.type) && left.type != Type::boolean) fail(expr.at, "equality requires scalar bits");
      return scalar(Type::boolean, "(" + left.code + expr.text + right.code + ")", out);
    }
    if (!numeric(left.type)) fail(expr.at, "arithmetic requires numeric operands");
    if (expr.text == "<" || expr.text == ">" || expr.text == "<=" || expr.text == ">=")
      return scalar(Type::boolean, "(" + left.code + expr.text + right.code + ")", out);
    if (floating(left.type)) {
      if (expr.text == "%") fail(expr.at, "remainder requires integer operands");
      return scalar(left.type, "(" + left.code + expr.text + right.code + ")", out);
    }
    const std::map<std::string, std::string> names{{"+", "add"}, {"-", "sub"}, {"*", "mul"}, {"/", "div"}, {"%", "rem"}};
    return scalar(left.type, "checked_" + names.at(expr.text) + "(" + left.code + ", " + right.code + ")", out);
  }
  Value conditional_expression(const Expression &expr, Module &module, Environment &env, Locals &locals,
                          bool pure, std::ostream &out) {
    const auto condition = expression(*expr.children[0], module, env, locals, pure, out);
    require(condition.type, Type::boolean, expr.at);
    std::ostringstream yes, no;
    auto first = block(*expr.children[1], module, env, locals, pure, yes);
    auto second = block(*expr.children[2], module, env, locals, pure, no);
    if (first.type == Type::null_ref && second.type.kind == Type::reference) {
      first.type = second.type;
      first.region_id = second.region_id;
    } else if (second.type == Type::null_ref && first.type.kind == Type::reference) {
      second.type = first.type;
      second.region_id = first.region_id;
    }
    require(second.type, first.type, expr.at);
    if (first.region_id != second.region_id) fail(expr.at, "conditional references require one owning region");
    if (first.type == Type::void_type || first.type == Type::region || first.type == Type::null_ref) {
      out << "if (" << condition.code << ") {\n" << yes.str() << "} else {\n" << no.str() << "}\n";
      return first;
    }
    const auto result = fresh("choice");
    out << cpp_type(first.type) << " " << result << "{};\nif (" << condition.code << ") {\n"
        << yes.str() << result << " = " << first.code << ";\n} else {\n" << no.str()
        << result << " = " << second.code << ";\n}\n";
    return {first.type, result, {}, nullptr, first.region_id};
  }
  Value expression(const Expression &expr, Module &module, Environment &env, Locals &locals,
                   bool pure, std::ostream &out) {
    using K = Expression::Kind;
    switch (expr.kind) {
    case K::null_ref: return {Type::null_ref, "UINT64_C(0)", {}, nullptr};
    case K::integer: {
      auto spelling = expr.text;
      const auto type = spelling.ends_with("u8") ? Type::uint8 : spelling.ends_with("u32") ? Type::uint32 :
                        spelling.ends_with("u64") ? Type::uint64 : Type::integer;
      if (type != Type::integer) {
        spelling.resize(spelling.size() - (type == Type::uint8 ? 2 : 3));
        std::uint64_t value{};
        const auto parsed = std::from_chars(spelling.data(), spelling.data() + spelling.size(), value);
        if (parsed.ec != std::errc{} || parsed.ptr != spelling.data() + spelling.size() ||
            (type == Type::uint8 && value > UINT8_MAX) || (type == Type::uint32 && value > UINT32_MAX))
          fail(expr.at, "integer literal is outside " + type_name(type) + " range");
        return scalar(type, "static_cast<" + cpp_type(type) + ">(UINT64_C(" + std::to_string(value) + "))", out);
      }
      std::int64_t value{};
      const auto parsed = std::from_chars(expr.text.data(), expr.text.data() + expr.text.size(), value);
      if (parsed.ec != std::errc{} || parsed.ptr != expr.text.data() + expr.text.size())
        fail(expr.at, "integer literal is outside i64 range");
      return scalar(Type::integer, "INT64_C(" + std::to_string(value) + ")", out);
    }
    case K::real: {
      auto spelling = expr.text;
      const auto type = spelling.ends_with("f32") ? Type::float32 : Type::float64;
      if (spelling.ends_with("f32") || spelling.ends_with("f64")) spelling.resize(spelling.size() - 3);
      const auto in_range = [&](auto value) {
        const auto parsed = std::from_chars(spelling.data(), spelling.data() + spelling.size(), value);
        return parsed.ec == std::errc{} && parsed.ptr == spelling.data() + spelling.size() && std::isfinite(value);
      };
      // Validate in the declared precision: a double intermediate can round
      // a finite f32 literal onto an underflow/overflow boundary.
      if (!(type == Type::float32 ? in_range(float{}) : in_range(double{})))
        fail(expr.at, "floating literal is outside " + type_name(type) + " range");
      if (spelling.find_first_of(".eE") == std::string::npos) spelling += ".0";
      return scalar(type, spelling + (type == Type::float32 ? "f" : ""), out);
    }
    case K::boolean: return scalar(Type::boolean, expr.text, out);
    case K::let: {
      auto value = expression(*expr.children[0], module, env, locals, pure, out);
      if (value.type == Type::void_type || value.type == Type::procedure)
        fail(expr.at, "a lexical value requires a bits value");
      if (!locals.emplace(expr.text, value).second) fail(expr.at, "duplicate lexical value '" + expr.text + "'");
      return value;
    }
    case K::field: {
      const auto value = expression(*expr.children[0], module, env, locals, pure, out);
      if (value.type.kind != Type::product && value.type.kind != Type::regional)
        fail(expr.at, "field access requires a product value; dereference regional pointers first");
      const auto &fields = products_.at(value.type.identity).fields;
      for (size_t i = 0; i < fields.size(); ++i) {
        if (fields[i].name != expr.text) continue;
        const auto member = ".f" + std::to_string(i);
        if (value.type.kind == Type::product) return scalar(fields[i].type, value.code + member, out);
        const auto field = "region_read<" + payload_type(value.type) + ">(" + region_of(value, expr.at) +
                           ", " + value.code + ")" + member;
        if (fields[i].type.kind == Type::reference)
          return scalar(fields[i].type, "region_offset(" + region_of(value, expr.at) + ", " + field + ".get())",
                        out, value.region_id);
        return scalar(fields[i].type, field, out);
      }
      fail(expr.at, "no field '" + expr.text + "' in " + type_name(value.type));
    }
    case K::index: {
      auto array = expression(*expr.children[0], module, env, locals, pure, out);
      auto index = expression(*expr.children[1], module, env, locals, pure, out);
      return array_element(array, index, expr.at, out);
    }
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
    case K::allocate: return allocate_expression(expr, module, env, locals, pure, out);
    case K::range_for: case K::fold: return loop_expression(expr, module, env, locals, pure, out);
    case K::block: return block(expr, module, env, locals, pure, out);
    case K::call: return call_expression(expr, module, env, locals, pure, out);
    case K::perform: case K::behave: return invoke(expr, module, env, locals, pure, out);
    case K::unary: {
      if (expr.text == "-" && expr.children[0]->kind == K::integer &&
          expr.children[0]->text == "9223372036854775808")
        return scalar(Type::integer, "std::numeric_limits<std::int64_t>::min()", out);
      auto value = expression(*expr.children[0], module, env, locals, pure, out);
      if (const auto selected = method(expr.text, module, {value}, expr.at))
        return call_method(*selected, {value}, expr.at, out);
      if (expr.text == "-" && floating(value.type)) return scalar(value.type, "(-" + value.code + ")", out);
      require(value.type, expr.text == "!" ? Type::boolean : Type::integer, expr.at);
      return scalar(value.type, expr.text == "!" ? "(!" + value.code + ")" :
                                              "checked_sub(INT64_C(0), " + value.code + ")", out);
    }
    case K::binary: return binary_expression(expr, module, env, locals, pure, out);
    case K::conditional: return conditional_expression(expr, module, env, locals, pure, out);
    }
    fail(expr.at, "unsupported expression");
  }
  void execute_module(Module &module, const Environment &caller, const Location &at, std::ostream &out) {
    // This composes runtime code after the complete module scan. A repeated
    // encounter reuses the incorporated module's resources, never its effects.
    if (module.state == Module::State::complete) return;
    if (module.state == Module::State::executing) fail(at, "cyclic module execution is outside the first prototype");
    const auto region_begin = regions_.size();
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
      case Form::Kind::function: case Form::Kind::product: case Form::Kind::array: case Form::Kind::unit: break;
      case Form::Kind::using_units: {
        auto &source = *module.dependencies.at(&form);
        execute_module(source, env, form.at, body);
        if (!module.unit_sources.insert(&source).second)
          fail(form.at, "UoMs from a source module must be selected in one using bundle");
        std::set<size_t> selected;
        for (const auto &selection : form.units) {
          const auto type = resolve({Type::named, selection.source}, source, selection.at);
          if (type.kind != Type::unit) fail(selection.at, "using units requires UoM definitions");
          if (!module.selected_units.insert(type.identity).second)
            fail(selection.at, "repeated using of UoM identity " + type_name(type));
          selected.insert(type.identity);
          if (!module.types.emplace(selection.local, type).second)
            fail(selection.at, "duplicate lexical alias '" + selection.local + "'");
        }
        for (const auto &function : source.conversions) {
          const auto edge = conversion(function);
          if (!selected.contains(edge.source.identity) || !selected.contains(edge.target.identity)) continue;
          module.conversions.push_back(function);
          auto &methods = module.functions[function.form->name];
          if (std::find(methods.begin(), methods.end(), function) == methods.end()) methods.push_back(function);
        }
        break;
      }
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
        if (value.region_id || value.type.kind == Type::reference || value.type == Type::region)
          fail(form.at, "module globals contain bits values, not regional resources or references");
        require(value.type, resolve(*form.result_type, module, form.at), form.at);
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
        if (form.symbolic && !form.using_symbol && !module.available_symbols.contains(form.alias))
          fail(form.at, "procedure destination @" + form.alias + " must be declared before using/import");
        auto &source = *module.dependencies.at(&form);
        execute_module(source, env, form.at, body);
        if (form.symbolic) {
          const auto source_id = symbol(source, form.name, form.at);
          const auto resource = source.exports.find(source_id);
          if (resource == source.exports.end() || !resource->second.procedure)
            fail(form.at, "source resource @" + form.name + " has no procedure value; use symbol from for identity");
          if (form.using_symbol) {
            const auto alias = module.symbol_aliases.find(form.alias);
            if (alias != module.symbol_aliases.end() && alias->second == &form) {
              module.active_aliases.insert(form.alias);
              module.available_symbols.insert(form.alias);
            }
          }
          const auto destination = symbol(module, form.alias, form.at);
          if (form.kind == Form::Kind::using_resource && (form.using_symbol || !form.explicit_alias) &&
              destination != source_id)
            fail(form.at, "symbol identity mismatch: source @" + form.name + " is " + symbol_names_[source_id] +
                             ", local @" + form.alias + " is " + symbol_names_[destination] +
                             "; write using \"" + form.path + "\" @" + form.name + " as @" + form.alias +
                             "; to select that destination explicitly");
          auto value = resource->second;
          if (form.kind == Form::Kind::import_resource)
            value = define_procedure(import_body(*value.procedure, form, module), module, env, body);
          bind(destination, std::move(value), module, env, form.at, body);
        } else if (const auto type = source.types.find(form.name); type != source.types.end()) {
          require_user_type_name(form.alias, form.at);
          if (type->second.kind == Type::unit)
            fail(form.at, "select UoMs and their conversions together with using units");
          if (form.kind == Form::Kind::import_resource) fail(form.at, "types preserve identity through using");
          if (module.functions.contains(form.alias) || module.globals.contains(form.alias) ||
              !module.types.emplace(form.alias, type->second).second)
            fail(form.at, "duplicate lexical alias '" + form.alias + "'");
        } else if (const auto function = source.functions.find(form.name); function != source.functions.end()) {
          require_user_type_name(form.alias, form.at);
          if (form.kind == Form::Kind::import_resource)
            fail(form.at, "function-body import is outside the prototype; use 'using' for a lexical function");
          if (module.types.contains(form.alias) || module.globals.contains(form.alias))
            fail(form.at, "duplicate lexical alias '" + form.alias + "'");
          auto &methods = module.functions[form.alias];
          for (const auto &candidate : function->second) {
            bool existing = false;
            for (const auto &current : methods)
              if (current.form == candidate.form && current.lexical == candidate.lexical) existing = true;
            if (!existing) methods.push_back(candidate);
          }
        } else if (const auto value = source.globals.find(form.name); value != source.globals.end()) {
          if (module.types.contains(form.alias) || module.functions.contains(form.alias) ||
              !module.globals.emplace(form.alias, value->second).second)
            fail(form.at, "duplicate lexical alias '" + form.alias + "'");
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
      if (value.type != Type::procedure && value.type != Type::region) {
        value.code = store(value.type);
        body << value.code << " = ctx.s" << id << ";\n";
      }
      module.exports.emplace(id, std::move(value));
    }
    out << "{\n" << body.str() << "}\n";
    close_regions(region_begin, {}, at);
    module.state = Module::State::complete;
  }

public:
  explicit Compiler(bool warnings_as_errors = false) : warnings_as_errors_(warnings_as_errors) {}
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
#include <cmath>
#include <type_traits>
#include <utility>

[[noreturn]] static void runtime_failure(const char *message) {
  std::fprintf(stderr, "%s\n", message);
  std::exit(70);
}
template<class T> [[noreturn]] static void integer_failure(const char *operation) {
  const char *type = std::is_same_v<T, std::int64_t> ? "i64" :
                     std::is_same_v<T, std::uint8_t> ? "u8" :
                     std::is_same_v<T, std::uint32_t> ? "u32" : "u64";
  char message[80];
  std::snprintf(message, sizeof(message), "%s %s overflow", type, operation);
  runtime_failure(message);
}
template<class T> static T checked_add(T a, T b) {
  T result;
  if (__builtin_add_overflow(a, b, &result)) integer_failure<T>("addition");
  return result;
}
template<class T> static T checked_sub(T a, T b) {
  T result;
  if (__builtin_sub_overflow(a, b, &result)) integer_failure<T>("subtraction");
  return result;
}
template<class T> static T checked_mul(T a, T b) {
  T result;
  if (__builtin_mul_overflow(a, b, &result)) integer_failure<T>("multiplication");
  return result;
}
template<class T> static T checked_div(T a, T b) {
  if (b == 0) runtime_failure("division by zero");
  if constexpr (std::is_signed_v<T>)
    if (a == std::numeric_limits<T>::min() && b == -1) integer_failure<T>("division");
  return a / b;
}
template<class T> static T checked_rem(T a, T b) {
  if (b == 0) runtime_failure("remainder by zero");
  if constexpr (std::is_signed_v<T>)
    if (a == std::numeric_limits<T>::min() && b == -1) return 0;
  return a % b;
}
template<class To, class From> static To checked_cast(From value) {
  if constexpr (std::is_integral_v<To> && std::is_integral_v<From>) {
    if (!std::in_range<To>(value)) runtime_failure("numeric conversion out of range");
  } else if constexpr (std::is_integral_v<To>) {
    const auto upper = std::ldexp(From(1), std::numeric_limits<To>::digits);
    const auto lower = std::is_signed_v<To> ? -upper : From(0);
    const auto truncated = std::trunc(value);
    if (!std::isfinite(truncated) || truncated < lower || truncated >= upper)
      runtime_failure("numeric conversion out of range");
  }
  const auto result = static_cast<To>(value);
  if constexpr (std::is_floating_point_v<To> && std::is_floating_point_v<From>)
    if (std::isfinite(value) && !std::isfinite(result)) runtime_failure("numeric conversion out of range");
  return result;
}
template<class T> struct Effect {
  T &slot;
  T saved;
  Effect(T &target, T value) : slot(target), saved(target) { slot = value; }
  ~Effect() { slot = saved; }
  Effect(const Effect &) = delete;
  Effect &operator=(const Effect &) = delete;
};
)CPP";
    if (regional_runtime_) cpp << region_runtime;
    for (size_t i = 0; i < products_.size(); ++i) cpp << "struct Record" << i << ";\n";
    cpp << type_definitions_.str() << "struct Context {\n";
    for (const auto &[id, type] : symbol_types_)
      if (type != Type::procedure && type != Type::region) cpp << "  " << cpp_type(type) << " s" << id << "{};\n";
    cpp << "};\nstatic thread_local Context ctx;\nint main() {\n";
    if (regional_runtime_) cpp << "try {\n";
    cpp << storage_.str() << body.str() << "return 0;\n";
    if (regional_runtime_) cpp << "} catch (const std::exception &error) { runtime_failure(error.what()); }\n";
    cpp << "}\n";
    return {cpp.str(), scanned_, execution_order_, warnings_};
  }
};

} // namespace cod::language
