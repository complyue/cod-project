//===--- lang_frontend.hh - Demand-checked CoD native lowering ------===//
#pragma once

#include "lang_syntax.hh"
#include "lang_llvm.hh"

#include <llvm/ADT/ImmutableMap.h>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <deque>
#include <fstream>
#include <iterator>
#include <map>
#include <queue>
#include <set>
#include <string_view>

namespace cod::language {

struct Module;
struct Procedure;
struct Value {
  Type type = Type::void_type;
  llvm::Value *native = nullptr; // Procedure bindings are compile-time metadata.
  std::shared_ptr<Procedure> procedure;
  Module *owner = nullptr;
  std::optional<size_t> region_id = {};
  std::optional<size_t> holding_id = {};
};
using Bindings = llvm::ImmutableMap<size_t, const Value *>;
struct Environment {
  Bindings bindings{nullptr};
  Bindings captured_bindings{nullptr};
  bool captured_context = false;

  const Value *lookup(size_t id) const {
    const auto found = bindings.lookup(id);
    return found ? *found : nullptr;
  }
  bool captured(size_t id, const Value *value) const {
    const auto found = captured_bindings.lookup(id);
    return found && *found == value;
  }
};
struct Locals {
  std::map<std::string, Value> visible;
  std::set<std::string> declared;

  Locals nested() const { return {visible, {}}; }
  bool declare(const std::string &name, Value value) {
    if (!declared.insert(name).second) return false;
    visible.insert_or_assign(name, std::move(value));
    return true;
  }
};
struct Method {
  Form *form;
  Module *lexical;
  bool operator==(const Method &) const = default;
};
struct Procedure {
  Form *form = nullptr;
  Module *lexical = nullptr;
  std::string builtin;
  Environment captures;
};
struct Module {
  std::string path;
  std::vector<std::unique_ptr<Form>> forms;
  std::map<std::string, size_t> symbols;
  std::map<std::string, Form *> symbol_aliases;
  std::set<std::string> active_aliases;
  std::set<std::string> available_symbols;
  std::map<const Form *, Module *> dependencies;
  std::map<std::string, std::vector<Method>> methods;
  std::map<std::string, Type> types;
  std::vector<Method> conversions;
  std::set<Module *> unit_sources;
  std::set<size_t> selected_units;
  std::map<std::string, Value> globals;
  std::map<const Form *, Value> procedures;
  std::map<size_t, Value> exports;
  enum class State { registered, executing, complete } state = State::registered;
  bool builtin = false;
};
struct Compilation {
  std::vector<std::string> scanned_modules;
  std::vector<std::string> execution_order;
  std::vector<std::string> warnings;
  std::string llvm_ir;
  std::string bitcode;
  bool uses_regions = false;
};

class Compiler {
  // Shared compiler snapshots retain binding values until lowering finishes.
  // The factory outlives every map; none of this storage enters the program.
  Bindings::Factory binding_factory_{false};
  std::deque<Value> binding_values_;
  std::map<std::string, std::unique_ptr<Module>> modules_;
  std::vector<std::string> symbol_names_;
  std::map<size_t, Type> symbol_types_;
  std::map<size_t, Method> symbol_declarations_;
  std::vector<std::string> scanned_;
  std::vector<std::string> execution_order_;
  std::set<const Form *> active_calls_;
  struct CallSite { const Form *form; Location at; };
  std::vector<CallSite> call_path_;
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
  enum class Presence { empty, owned, conditional };
  struct Region { llvm::Value *owner; Presence access = Presence::owned; };
  std::vector<Region> regions_;
  struct Holding { size_t region; bool module_lifetime; Presence state = Presence::owned; };
  std::vector<Holding> holdings_;
  bool module_top_level_ = false;
  bool region_initializer_ = false;
  bool regional_runtime_ = false;
  size_t next_unit_ = 0;
  LLVMEmission ir_;
  bool warnings_as_errors_ = false;
  std::vector<std::string> warnings_;
  bool scope_rebound_ = false; // Includes bindings with no native TLS slot.

  static void require_user_type_name(const std::string &name, const Location &at) {
    static const std::set<std::string> reserved{"i64", "u8", "u32", "u64", "f32", "f64", "bool", "void", "region", "ref"};
    if (reserved.contains(name)) fail(at, "reserved scalar type name '" + name + "'");
    if (intrinsic_name(name) || expression_keyword(name) || module_keyword(name))
      fail(at, "reserved keyword '" + name + "' cannot be used as an ordinary name");
  }
  static std::string_view procedure_operation(std::string_view name) {
    if (name == "region") return "create a region resource";
    if (name == "used_bytes" || name == "capacity_bytes") return "inspect a region resource";
    if (name == "store" || name == "setfield") return "modify regional storage";
    return {};
  }

  static Type reference(Type target) {
    Type type{Type::reference};
    type.target = std::make_shared<Type>(std::move(target));
    return type;
  }
  llvm::Value *region_pointer(const Value &value, const Location &at) {
    if (value.holding_id && holdings_.at(*value.holding_id).state != Presence::owned)
      fail(at, "whole-region binding is empty or not owned on every path");
    if (!value.region_id || regions_.at(*value.region_id).access != Presence::owned)
      fail(at, "regional value requires a live owning region in this caller scope");
    return regions_.at(*value.region_id).owner;
  }
  void close_holdings(size_t begin, const Value &result, const Location &at) {
    for (size_t i = begin; i < holdings_.size(); ++i) {
      auto &holding = holdings_[i];
      if (holding.module_lifetime || holding.state == Presence::empty) continue;
      if (result.region_id == holding.region)
        fail(at, "regional result would outlive its owning scope");
      regions_[holding.region].access = holding.state == Presence::owned ? Presence::empty : Presence::conditional;
      holding.state = Presence::empty;
    }
  }
  struct Ownership {
    std::vector<Presence> regions, holdings;
  };
  Ownership ownership() const {
    Ownership result;
    for (const auto &region : regions_) result.regions.push_back(region.access);
    for (const auto &holding : holdings_) result.holdings.push_back(holding.state);
    return result;
  }
  void restore_ownership(const Ownership &state) {
    for (size_t i = 0; i < regions_.size(); ++i)
      regions_[i].access = i < state.regions.size() ? state.regions[i] : Presence::empty;
    for (size_t i = 0; i < holdings_.size(); ++i)
      holdings_[i].state = i < state.holdings.size() ? state.holdings[i] : Presence::empty;
  }
  void merge_ownership(const Ownership &other) {
    for (size_t i = 0; i < other.regions.size(); ++i)
      if (regions_[i].access != other.regions[i]) regions_[i].access = Presence::conditional;
    for (size_t i = 0; i < other.holdings.size(); ++i)
      if (holdings_[i].state != other.holdings[i]) holdings_[i].state = Presence::conditional;
  }
  Type resolve_name(Type type, Module &module, const Location &at) {
    if (type.kind != Type::named) return type;
    const auto found = module.types.find(type.name);
    if (found == module.types.end()) fail(at, "unavailable type '" + type.name + "'");
    return found->second;
  }
  Type resolve(Type type, Module &module, const Location &at) {
    type = resolve_name(type, module, at);
    if (type.kind == Type::procedure) {
      if (!type.target) fail(at, "a procedure type requires its parameter and result signature");
      for (auto &parameter : type.parameters) {
        parameter = resolve(parameter, module, at);
        if (parameter == Type::void_type) fail(at, "a procedure parameter must have a value type");
      }
      type.target = std::make_shared<Type>(resolve(*type.target, module, at));
      return type;
    }
    if (type.kind == Type::reference) {
      auto target = resolve_name(*type.target, module, at);
      if (target.kind == Type::reference || target == Type::region || target == Type::void_type || target.kind == Type::procedure)
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
          if (field_type.kind == Type::procedure) fail(field.at, "procedure fields are outside the prototype");
          if (field_type == Type::region || field_type.kind == Type::regional || field_type.kind == Type::array ||
              (field_type.kind == Type::reference && type.kind != Type::regional))
            fail(field.at, "regional fields require an explicit regional type and reference fields");
          product.fields.push_back({field.name, field_type, field.at, false, {}});
        }
        if (type.kind == Type::regional) regional_runtime_ = true;
        std::vector<Type> field_types;
        for (const auto &field : product.fields) field_types.push_back(field.type);
        ir_.record(type, field_types);
        product.state = Product::complete;
      }
    }
    return type;
  }
  Value scalar(Type type, llvm::Value *native, std::optional<size_t> region_id = {}) {
    return {type, native, {}, nullptr, region_id};
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
        symbol_declarations_.emplace(new_symbol(result, form.name), Method{&form, &result});
      } else if (form.kind == Form::Kind::procedure && form.symbolic) {
        if (!result.symbols.contains(form.name) && !result.symbol_aliases.contains(form.name))
          symbol_declarations_.emplace(new_symbol(result, form.name), Method{&form, &result});
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
      } else if (form.kind == Form::Kind::function || (form.kind == Form::Kind::procedure && !form.symbolic) || form.kind == Form::Kind::global) {
        if (!lexical_names.insert(form.name).second &&
            (form.kind == Form::Kind::global ||
             (!result.methods.contains(form.name) && !lexical_selections.contains(form.name))))
          fail(form.at, "duplicate lexical name '" + form.name + "'");
        if (form.kind != Form::Kind::global) {
          if (!form.operation) require_user_type_name(form.name, form.at);
          if (form.operation && form.name == "setfield")
            fail(form.at, "setfield overloads need a compile-time field-selector contract; outside this prototype");
          result.methods[form.name].push_back({&form, &result});
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
              !result.methods.contains(form.alias) && !lexical_selections.contains(form.alias))
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
    form->symbolic = true;
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
  Type procedure_type(const Form &form, Module &module) {
    Type type{Type::procedure};
    for (const auto &parameter : form.parameters)
      type.parameters.push_back(resolve(parameter.type, module, parameter.at));
    type.target = std::make_shared<Type>(resolve(*form.result_type, module, form.at));
    return type;
  }
  void declare_symbol_type(size_t id) {
    if (symbol_types_.contains(id)) return;
    const auto declaration = symbol_declarations_.find(id);
    if (declaration == symbol_declarations_.end()) return; // Compiler-provided intrinsic family.
    const auto &[form, module] = declaration->second;
    auto type = form->kind == Form::Kind::symbol ? resolve(*form->result_type, *module, form->at)
                                               : procedure_type(*form, *module);
    if (type == Type::void_type) fail(form->at, "a symbol cannot have void type");
    symbol_types_.emplace(id, std::move(type));
  }
  void fix_type(size_t id, Type type, const Location &at) {
    declare_symbol_type(id);
    const auto [found, added] = symbol_types_.emplace(id, type);
    if (!added && found->second != type)
      fail(at, "incompatible value types for " + symbol_names_[id] +
                   ": expected " + type_name(found->second) + ", got " + type_name(type));
  }
  Value read_symbol(size_t id, const Environment &env, const Location &at) {
    const auto *found = env.lookup(id);
    if (!found) fail(at, (env.captured_context
        ? "perform requires an unavailable definition-time capture " : "missing required binding ") + symbol_names_[id]);
    if (found->region_id) region_pointer(*found, at);
    if (found->type.kind == Type::procedure || found->type == Type::region) return *found;
    const auto type = found->type;
    return scalar(type, env.captured(id, found) ? found->native : ir_.read_symbol(id, type), found->region_id);
  }
  Value bind(size_t id, Value value, Module &owner, Environment &env, const Location &at) {
    if (value.type == Type::void_type) fail(at, "a symbol cannot be bound to void");
    if (value.type.kind == Type::regional || value.type.kind == Type::array || value.type == Type::null_ref)
      fail(at, "bind bits or a typed regional reference, not a dereferenced regional object");
    if (value.region_id) region_pointer(value, at);
    declare_symbol_type(id);
    if (value.procedure && !value.procedure->builtin.empty()) {
      const auto declared = symbol_types_.find(id);
      if (declared != symbol_types_.end() && declared->second.kind == Type::procedure && declared->second.target) {
        const auto &type = declared->second;
        if (type.parameters.size() != 1 || *type.target != Type::void_type ||
            (value.procedure->builtin == "assert" ? type.parameters[0] != Type::boolean :
             !numeric(type.parameters[0]) && type.parameters[0] != Type::boolean))
          fail(at, "signature is incompatible with intrinsic @" + value.procedure->builtin);
        value.type = type;
      }
    }
    fix_type(id, value.type, at);
    if (value.type == Type::region) {
      if (value.holding_id) {
        fail(at, "cannot transfer ownership out of a dynamic region binding; use a new resource to shadow it");
      }
      value.holding_id = holdings_.size();
      ir_.hold_region(value.native, module_top_level_);
      holdings_.push_back({*value.region_id, module_top_level_});
    }
    ir_.effect(id, value.type, value.native);
    value.owner = &owner;
    binding_values_.push_back(std::move(value));
    env.bindings = binding_factory_.add(env.bindings, id, &binding_values_.back());
    return binding_values_.back();
  }

  void resolve_procedure_symbols(Expression &expr, Module &module) {
    if (uses_symbol(expr.kind) && !expr.symbol_id)
      expr.symbol_id = symbol(module, expr.text, expr.at);
    for (const auto &child : expr.children) resolve_procedure_symbols(*child, module);
  }
  Value define_procedure(Form &form, Module &lexical, const Environment &env) {
    resolve_procedure_symbols(*form.body, lexical);
    auto proc = std::make_shared<Procedure>();
    proc->form = &form;
    proc->lexical = &lexical;
    // This is compiler metadata, not a runtime closure or a retained region.
    // SSA values already represent each binding's value at its definition.
    // Only demanded reads generate native uses, after typed method selection.
    proc->captures = env;
    proc->captures.captured_context = true;
    proc->captures.captured_bindings = env.bindings;
    return {procedure_type(form, lexical), {}, std::move(proc), &lexical};
  }
  Value block(const Expression &expr, Module &module, Environment env, Locals locals,
              bool pure, bool discard_result = false) {
    const auto module_scope = llvm::SaveAndRestore(module_top_level_, false);
    const auto region_initializer = llvm::SaveAndRestore(region_initializer_, false);
    const auto rebound = llvm::SaveAndRestore(scope_rebound_, false);
    const auto holding_begin = holdings_.size();
    const auto native_scope = ir_.scope();

    Value result;
    for (const auto &child : expr.children) result = expression(*child, module, env, locals, pure);
    if (discard_result) result = {};
    close_holdings(holding_begin, result, expr.at);
    ir_.restore(native_scope);
    if (result.type.kind == Type::procedure) fail(expr.at, "procedure-valued scope results are not supported");
    return result;
  }
  std::vector<Value> arguments(const Expression &expr, Module &module, Environment &env,
                               Locals &locals, bool pure,
                               std::vector<LLVMEmission::ConversionPoint> *points = nullptr) {
    const auto region_initializer = llvm::SaveAndRestore(region_initializer_, false);
    std::vector<Value> values;
    for (const auto &arg : expr.children) {
      values.push_back(expression(*arg, module, env, locals, pure));
      if (points) points->push_back(ir_.conversion_point());
    }
    return values;
  }
  static bool converting_parameters(const Form &form) {
    return std::any_of(form.parameters.begin(), form.parameters.end(),
                       [](const Parameter &parameter) { return parameter.automatic_conversion; });
  }
  bool converting_method(const std::string &name, const Module &module, size_t arity) {
    const auto found = module.methods.find(name);
    if (arity < 2 || found == module.methods.end()) return false;
    return std::any_of(found->second.begin(), found->second.end(), [&](const Method &function) {
      return function.form->parameters.size() == arity && converting_parameters(*function.form);
    });
  }
  Value call(Form &form, Module &lexical, Environment env, const std::vector<Value> &args,
             bool pure, const Location &at) {
    call_path_.push_back({&form, at});
    if (form.conversion) conversion({&form, &lexical});
    if (args.size() != form.parameters.size()) fail(at, "wrong argument count for '" + form.name + "'");
    if (!active_calls_.insert(&form).second) fail(at, "recursive call is outside the first prototype");
    Locals locals;
    for (size_t i = 0; i < args.size(); ++i) {
      require(args[i].type, resolve(form.parameters[i].type, lexical, form.parameters[i].at), at);
      if (args[i].region_id) region_pointer(args[i], at);
      if (pure && args[i].type == Type::region) fail(at, "a function reads input snapshots, not region resources");
      if (args[i].type == Type::region)
        fail(at, "whole-region resources require effect bindings, not lexical parameters");
      if (!locals.declare(form.parameters[i].name, args[i]))
        fail(form.parameters[i].at, "duplicate parameter name");
    }
    auto result = block(*form.body, lexical, std::move(env), std::move(locals), pure);
    active_calls_.erase(&form);
    if (form.result_type) require(result.type, resolve(*form.result_type, lexical, form.at), form.at);
    call_path_.pop_back();
    return result;
  }
  struct Conversion {
    Method function;
    Type source, target;
  };
  struct Route {
    std::vector<Method> steps;
    std::vector<Method> alternative;
  };
  struct MethodMatch {
    Method function;
    std::vector<Route> routes;
  };
  Conversion conversion(Method function) {
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
  static std::optional<std::vector<Method>> shortest_route(
      const std::vector<Conversion> &graph, Type source, Type target,
      const std::optional<Method> &excluded = {}) {
    struct Step { size_t unit; std::vector<Method> path; };
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
        std::optional<Method> selected, alternative;
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
  static std::string callable_name(const Form &form) {
    if (form.operation) return "(" + form.name + ")";
    return form.symbolic ? "@" + form.name : form.name;
  }
  [[noreturn]] static void method_failure(const std::string &name, const std::vector<Value> &args,
                                         const std::vector<Method> &candidates, const Location &at,
                                         bool ambiguous) {
    std::string message = (ambiguous ? "ambiguous method '" : "no matching method '") + name +
                          "' for supplied argument types (";
    for (size_t i = 0; i < args.size(); ++i)
      message += (i ? ", " : "") + type_name(args[i].type);
    message += "): " + std::to_string(candidates.size()) + (ambiguous ? " matching" : " available") + " candidates";
    const auto shown = std::min<size_t>(candidates.size(), 4);
    for (size_t i = 0; i < shown; ++i) {
      const auto &form = *candidates[i].form;
      message += "\n" + location_text(form.at) + ": note: candidate " +
                 (form.kind == Form::Kind::procedure ? "procedure '" : "function '") + callable_name(form) + "'";
    }
    if (shown < candidates.size())
      message += "\nnote: " + std::to_string(candidates.size() - shown) + " additional candidates omitted";
    fail(at, message);
  }
  std::optional<MethodMatch> method(const std::string &name, Module &module,
                                    const std::vector<Value> &args, const Location &at) {
    const auto found = module.methods.find(name);
    if (found == module.methods.end()) return {};
    std::vector<MethodMatch> exact, converted;
    const auto reject_ambiguity = [&](const std::vector<MethodMatch> &matches) {
      if (matches.size() < 2) return;
      std::vector<Method> candidates;
      for (const auto &match : matches) candidates.push_back(match.function);
      method_failure(name, args, candidates, at, true);
    };
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
    reject_ambiguity(exact);
    if (!exact.empty()) return exact.front();
    for (const auto &candidate : found->second) {
      if (candidate.form->parameters.size() != args.size()) continue;
      MethodMatch match{candidate, {}};
      bool matches = true;
      for (size_t i = 0; i < args.size(); ++i) {
        auto path = route(args[i].type, candidate.form->parameters[i], *candidate.lexical, module, at, false);
        if (!path) { matches = false; break; }
        match.routes.push_back(std::move(*path));
      }
      if (matches) converted.push_back(std::move(match));
    }
    reject_ambiguity(converted);
    if (converted.empty()) {
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
    return converted.front();
  }
  Value converted(Value value, const Route &route, const Location &at,
                  const LLVMEmission::ConversionPoint *point = nullptr) {
    if (route.steps.empty()) return value;
    if (!route.alternative.empty()) {
      auto describe = [&](const std::vector<Method> &path) {
        std::string result = type_name(value.type);
        for (const auto &step : path)
          result += " --" + step.form->name + "--> " + type_name(conversion(step).target);
        return result;
      };
      const auto message = "ambiguous unit conversion; selected " + describe(route.steps) +
                           "; alternative " + describe(route.alternative) + " [-Wunit-conversion]";
      if (warnings_as_errors_) fail(at, "warning treated as error: " + message);
      warnings_.push_back(location_text(at) + ": warning: " + message);
    }
    // Dispatch needs every argument's type, but a selected conversion executes
    // immediately after its own argument, before any later argument effects.
    const auto resume = point ? std::optional(ir_.begin_conversion(*point)) : std::nullopt;
    for (const auto &step : route.steps)
      value = call(*step.form, *step.lexical, {}, {value}, true, at);
    if (resume) ir_.end_conversion(*point, *resume);
    return value;
  }
  Value call_method(const MethodMatch &match, std::vector<Value> args, const Location &at,
                    const Environment &env, bool pure, const std::vector<LLVMEmission::ConversionPoint> &points = {}) {
    for (size_t i = 0; i < args.size(); ++i)
      args[i] = converted(args[i], match.routes[i], at, points.empty() ? nullptr : &points[i]);
    const bool function = match.function.form->kind == Form::Kind::function;
    if (!function && pure) fail(at, match.function.form->operation
        ? "a function cannot invoke a procedure overload" : "a function cannot invoke a procedure");
    // Lexical procedures and operation overloads use the caller's dynamic
    // environment. Only symbolic perform installs definition-time captures.
    return call(*match.function.form, *match.function.lexical, function ? Environment{} : env, args, function, at);
  }
  Type array_type(const Value &value, const Location &at) {
    const auto type = value.type.kind == Type::reference ? *value.type.target : value.type;
    if (type.kind != Type::array) fail(at, "expected a regional array");
    region_pointer(value, at);
    return type;
  }
  Value array_element(Value array, Value index, const Location &at, bool as_reference = false) {
    const auto type = array_type(array, at);
    const auto &layout = arrays_.at(type.identity);
    require(index.type, layout.index, at);
    const auto result_type = as_reference ? reference(layout.element) : layout.element;
    return scalar(result_type, ir_.array_element(region_pointer(array, at), array.native, index.type, index.native, layout.element, as_reference), as_reference ? array.region_id : std::nullopt);
  }
  Value invoke(const Expression &expr, Module &module, Environment &env, Locals &locals,
               bool pure) {
    if (pure) fail(expr.at, "a function cannot invoke a procedure");
    const auto id = expression_symbol(expr, module);
    const auto *found = env.lookup(id);
    if (!found) fail(expr.at, (env.captured_context
        ? "perform requires an unavailable definition-time capture " : "missing required procedure binding ") + symbol_names_[id]);
    if (!found->procedure) fail(expr.at, "perform/behave requires a procedure value");
    const auto value = *found;
    const auto proc = value.procedure; // Resolve before arguments/capture installation.
    std::vector<LLVMEmission::ConversionPoint> points;
    const bool convert_arguments = expr.children.size() > 1 && proc->form && converting_parameters(*proc->form);
    auto args = arguments(expr, module, env, locals, pure, convert_arguments ? &points : nullptr);
    if (!proc->builtin.empty()) {
      if (args.size() != 1) fail(expr.at, "@" + proc->builtin + " expects one argument");
      if (value.type.target) require(args[0].type, value.type.parameters[0], expr.at);
      if (proc->builtin == "assert") {
        require(args[0].type, Type::boolean, expr.at);
        ir_.assert_value(args[0].native);
      } else {
        if (!numeric(args[0].type) && args[0].type != Type::boolean)
          fail(expr.at, "@print expects a numeric scalar or bool");
        ir_.print(args[0].type, args[0].native);
      }
      return {};
    }
    if (args.size() == proc->form->parameters.size()) {
      for (size_t i = 0; i < args.size(); ++i) {
        if (const auto path = route(args[i].type, proc->form->parameters[i], *proc->lexical, module, expr.at))
          args[i] = converted(args[i], *path, expr.at, points.empty() ? nullptr : &points[i]);
      }
    }
    const auto native_scope = ir_.scope();

    auto call_env = expr.kind == Expression::Kind::perform ? proc->captures : env;
    auto result = call(*proc->form, *proc->lexical, std::move(call_env), args, false, expr.at);
    ir_.restore(native_scope);
    return result;
  }
  Value allocate_expression(const Expression &expr, Module &module, Environment &env, Locals &locals,
                          bool pure) {
    if (pure) fail(expr.at, "a function cannot allocate regional storage");
    auto owner = expression(*expr.children[0], module, env, locals, pure);
    require(owner.type, Type::region, expr.at);
    region_pointer(owner, expr.at);
    auto type = resolve({Type::named, expr.text}, module, expr.at);
    if (type.kind != Type::array && type.kind != Type::regional)
      fail(expr.at, "alloc requires a regional type or regional array");
    std::vector<Value> args;
    for (size_t i = 1; i < expr.children.size(); ++i)
      args.push_back(expression(*expr.children[i], module, env, locals, pure));

    if (type.kind == Type::array) {
      if (args.size() != 2 || !integral(args[0].type))
        fail(expr.at, "array allocation requires an integer length and initial element value");
      require(args[1].type, arrays_.at(type.identity).element, expr.at);

    } else {
      const auto &fields = products_.at(type.identity).fields;
      if (args.size() != fields.size()) fail(expr.at, "wrong field count for regional allocation");
      for (size_t i = 0; i < args.size(); ++i) {
        if (!(args[i].type == Type::null_ref && fields[i].type.kind == Type::reference))
          require(args[i].type, fields[i].type, expr.at);
        if (args[i].region_id && args[i].region_id != owner.region_id)
          fail(expr.at, "cross-region reference fields are outside this prototype");

      }
    }
    std::vector<llvm::Value *> native_args;
    for (const auto &arg : args) native_args.push_back(arg.native);
    auto *native = type.kind == Type::array ?
        ir_.array_new(region_pointer(owner, expr.at), arrays_.at(type.identity).element,
                      args[0].type, args[0].native, args[1].native) :
        ir_.record_new(region_pointer(owner, expr.at), type, native_args);
    return scalar(reference(type), native, owner.region_id);
  }
  Value loop_expression(const Expression &expr, Module &module, Environment &env, Locals &locals,
                          bool pure) {
    using K = Expression::Kind;
    const auto first = expression(*expr.children[0], module, env, locals, pure);
    const auto last = expression(*expr.children[1], module, env, locals, pure);
    require(last.type, first.type, expr.at);
    if (!integral(first.type) && !(first.type.kind == Type::unit && integral(first.type.representation)))
      fail(expr.at, "a range requires integer endpoints or one integer-backed UoM identity");
    Value accumulator;
    if (expr.kind == K::fold) {
      accumulator = expression(*expr.children[2], module, env, locals, pure);
      if (accumulator.type == Type::void_type || accumulator.type.kind == Type::procedure ||
          accumulator.type == Type::region || accumulator.type == Type::null_ref ||
          accumulator.type.kind == Type::array || accumulator.type.kind == Type::regional)
        fail(expr.at, "a fold accumulator requires a bits value");
    }
    const auto before = ownership();
    auto native_loop = ir_.loop(first.type, first.native, last.native, accumulator.native);
    auto inner_locals = locals.nested();
    inner_locals.declare(expr.text, {first.type, native_loop.index, {}, nullptr});
    if (expr.kind == K::fold) {
      accumulator.native = native_loop.accumulator;
      inner_locals.declare(expr.binding, accumulator);
    }

    const auto next = block(*expr.children.back(), module, env, std::move(inner_locals), pure,
                            expr.kind == K::range_for);

    bool empty = false, at_most_once = false;
    const auto *start = llvm::dyn_cast<llvm::ConstantInt>(first.native);
    const auto *end = llvm::dyn_cast<llvm::ConstantInt>(last.native);
    if (start && end) {
      const auto representation = first.type.kind == Type::unit ? first.type.representation : first.type.kind;
      const auto width = start->getBitWidth() + 1;
      const auto distance = representation == Type::integer
          ? end->getValue().sext(width) - start->getValue().sext(width)
          : end->getValue().zext(width) - start->getValue().zext(width);
      empty = distance.sle(0);
      at_most_once = distance.sle(1);
    }
    if (empty) restore_ownership(before);
    else if (!at_most_once) {
      for (size_t i = 0; i < before.holdings.size(); ++i)
        if (holdings_[i].state != before.holdings[i])
          fail(expr.at, "a repeated loop cannot consume an outer region owner; acquire the region within each iteration");
    }

    if (expr.kind == K::fold) {
      require(next.type, accumulator.type, expr.at);
      if (next.region_id != accumulator.region_id)
        fail(expr.at, "a fold reference must retain its owning region");
    }
    ir_.end_loop(native_loop, next.native);
    return accumulator;
  }
  Value call_expression(const Expression &expr, Module &module, Environment &env, Locals &locals,
                          bool pure, bool function_result = false) {
    using K = Expression::Kind;
    if (expr.children.size() == 2 && binary_precedence(expr.text) >= 0)
      return binary_expression(expr, module, env, locals, pure, function_result);
    if (expr.children.size() == 1 && (expr.text == "-" || expr.text == "!"))
      return unary_expression(expr, module, env, locals, pure, function_result);
    std::vector<LLVMEmission::ConversionPoint> points;
    std::optional<std::vector<Value>> evaluated;
    const auto values = [&]() -> const std::vector<Value> & {
      if (!evaluated) evaluated = arguments(expr, module, env, locals, pure,
          converting_method(expr.text, module, expr.children.size()) ? &points : nullptr);
      return *evaluated;
    };
    if (intrinsic_name(expr.text) && expr.text != "setfield") {
      const auto &args = values();
      if (const auto selected = method(expr.text, module, args, expr.at))
        return call_method(*selected, args, expr.at, env, pure || function_result, points);
    }
    const auto operation = procedure_operation(expr.text);
    if (function_result && !operation.empty())
      fail(expr.at, "a bits global initializer requires a pure function, not a procedure-only operation");
    if (pure && !operation.empty()) fail(expr.at, "a function cannot " + std::string(operation));
    if (expr.text == "region") {
      const auto &args = values();
      if (args.size() != 1 || !integral(args[0].type)) fail(expr.at, "region requires an integer byte capacity");
      regional_runtime_ = true;
      if (!region_initializer_) fail(expr.at, "a new region must be acquired by an effect binding");
      auto *owner = ir_.region_new(args[0].type, args[0].native);
      const auto id = regions_.size();
      regions_.push_back({owner});

      return {Type::region, owner, {}, nullptr, id};
    }
    if (expr.text == "used_bytes" || expr.text == "capacity_bytes") {
      const auto &args = values();
      if (args.size() != 1) fail(expr.at, "region size requires one resource");
      require(args[0].type, Type::region, expr.at);
      return scalar(Type::uint64, ir_.region_size(region_pointer(args[0], expr.at), expr.text == "capacity_bytes"));
    }
    if (expr.text == "deref") {
      const auto &args = values();
      if (args.size() != 1 || args[0].type.kind != Type::reference)
        fail(expr.at, "deref requires a typed regional reference");
      const auto type = resolve(*args[0].type.target, module, expr.at);
      auto *native = ir_.dereference(region_pointer(args[0], expr.at), args[0].native, type);
      if (type.kind == Type::array || type.kind == Type::regional)
        return {type, native, {}, nullptr, args[0].region_id};
      return scalar(type, native);
    }
    if (expr.text == "len" || expr.text == "ref_at") {
      const auto &args = values();
      if (args.size() != (expr.text == "len" ? 1u : 2u)) fail(expr.at, "wrong array argument count");
      if (expr.text == "ref_at") return array_element(args[0], args[1], expr.at, true);
      array_type(args[0], expr.at);
      return scalar(Type::uint64, ir_.array_length(region_pointer(args[0], expr.at), args[0].native));
    }
    if (expr.text == "store") {
      const auto &args = values();
      if (args.size() == 2 && args[0].type.kind == Type::reference &&
          (numeric(*args[0].type.target) || *args[0].type.target == Type::boolean || args[0].type.target->kind == Type::unit)) {
        require(args[1].type, *args[0].type.target, expr.at);

        ir_.scalar_store(region_pointer(args[0], expr.at), args[0].native, args[1].native);
        return args[1];
      }
      if (args.size() != 3) fail(expr.at, "store requires an array/index/value or scalar reference/value");
      const auto type = array_type(args[0], expr.at);
      const auto &layout = arrays_.at(type.identity);
      require(args[1].type, layout.index, expr.at);
      require(args[2].type, layout.element, expr.at);
      return scalar(layout.element, ir_.array_store(region_pointer(args[0], expr.at), args[0].native, args[1].type, args[1].native, layout.element, args[2].native));
    }
    if (expr.text == "setfield") {
      if (expr.children.size() != 3 || expr.children[1]->kind != K::name)
        fail(expr.at, "setfield requires a reference, field name and value");
      const auto object = expression(*expr.children[0], module, env, locals, pure);
      if (object.type.kind != Type::reference || object.type.target->kind != Type::regional)
        fail(expr.at, "setfield requires a reference to a regional record");
      const auto type = resolve(*object.type.target, module, expr.at);
      auto value = expression(*expr.children[2], module, env, locals, pure);
      const auto &fields = products_.at(type.identity).fields;
      for (size_t i = 0; i < fields.size(); ++i) {
        if (fields[i].name != expr.children[1]->text) continue;
        const auto field = fields[i].type;
        if (value.type == Type::null_ref && field.kind == Type::reference) value.type = field;
        require(value.type, field, expr.at);
        if (value.region_id && value.region_id != object.region_id)
          fail(expr.at, "cross-region reference fields are outside this prototype");
        region_pointer(object, expr.at);

        if (field.kind == Type::reference) value.region_id = object.region_id;

        ir_.set_field(region_pointer(object, expr.at), object.native, type, i, field, value.native);
        return value;
      }
      fail(expr.at, "no such regional field '" + expr.children[1]->text + "'");
    }
    static const std::map<std::string, Type> casts{{"i64", Type::integer}, {"u8", Type::uint8},
        {"u32", Type::uint32}, {"u64", Type::uint64}, {"f32", Type::float32}, {"f64", Type::float64}};
    if (const auto cast = casts.find(expr.text); cast != casts.end()) {
      const auto &args = values();
      if (args.size() != 1 || !numeric(args[0].type)) fail(expr.at, "numeric conversion requires one scalar value");
      return scalar(cast->second, ir_.cast(args[0].type, cast->second, args[0].native));
    }
    if (const auto found = module.types.find(expr.text); found != module.types.end()) {
      const auto type = resolve(found->second, module, expr.at);
      if (type.kind == Type::regional || type.kind == Type::array)
        fail(expr.at, "regional objects require alloc in a region resource");
      const auto &args = values();
      if (type.kind == Type::unit) {
        if (args.size() != 1) fail(expr.at, "a UoM constructor requires one scalar value");
        require(args[0].type, type.representation, expr.at);
        return {type, args[0].native, {}, nullptr};
      }
      const auto &fields = products_.at(type.identity).fields;
      if (args.size() != fields.size()) fail(expr.at, "wrong field count for '" + expr.text + "'");

      for (size_t i = 0; i < args.size(); ++i) require(args[i].type, fields[i].type, expr.at);
      std::vector<llvm::Value *> native_args;
      for (const auto &arg : args) native_args.push_back(arg.native);
      return scalar(type, ir_.product(type, native_args));
    }
    const auto function = module.methods.find(expr.text);
    if (expr.text == "scalar") {
      const auto &args = values();
      if (args.size() != 1 || args[0].type.kind != Type::unit)
        fail(expr.at, "scalar requires one UoM value");
      return {args[0].type.representation, args[0].native, {}, nullptr};
    }
    if (function == module.methods.end()) {
      if (expr.text == "=" || expr.text == "[]=")
        fail(expr.at, "unavailable operation overload '(" + expr.text + ")'");
      fail(expr.at, "unavailable lexical function '" + expr.text + "'");
    }
    const auto &args = values();
    auto selected = method(expr.text, module, args, expr.at);
    if (!selected && function->second.size() == 1) {
      const auto &candidate = function->second.front();
      return call_method({candidate, std::vector<Route>(args.size())}, args, expr.at, env, pure || function_result, points);
    }
    if (!selected) method_failure(expr.text, args, function->second, expr.at, false);
    return call_method(*selected, args, expr.at, env, pure || function_result, points);
  }
  Value unary_expression(const Expression &expr, Module &module, Environment &env, Locals &locals,
                         bool pure, bool function_result = false) {
    using K = Expression::Kind;
    if (expr.text == "-" && expr.children[0]->kind == K::integer) {
      std::string_view magnitude = expr.children[0]->text;
      while (magnitude.starts_with('0')) magnitude.remove_prefix(1);
      if (magnitude == "9223372036854775808")
        return scalar(Type::integer, ir_.bits(Type::integer, UINT64_C(1) << 63));
    }
    auto value = expression(*expr.children[0], module, env, locals, pure);
    if (const auto selected = method(expr.text, module, {value}, expr.at))
      return call_method(*selected, {value}, expr.at, env, pure || function_result);
    if (expr.text == "-" && floating(value.type))
      return scalar(value.type, ir_.unary(value.type, expr.text, value.native));
    require(value.type, expr.text == "!" ? Type::boolean : Type::integer, expr.at);
    return scalar(value.type, ir_.unary(value.type, expr.text, value.native));
  }
  Value binary_expression(const Expression &expr, Module &module, Environment &env, Locals &locals,
                          bool pure, bool function_result = false) {
    auto left = expression(*expr.children[0], module, env, locals, pure);
    if (expr.text == "&&" || expr.text == "||") {
      require(left.type, Type::boolean, expr.at);
      auto right_env = env;
      auto right_locals = locals.nested();
      const auto rebound = llvm::SaveAndRestore(scope_rebound_, false);

      const auto native_scope = ir_.scope();
      const auto skipped = ownership();
      auto native_branch = ir_.branch(left.native, expr.text == "||");
      const auto right = expression(*expr.children[1], module, right_env, right_locals, pure);
      require(right.type, Type::boolean, expr.at);
      if (scope_rebound_ || ir_.scope() != native_scope)
        fail(expr.at, "conditional rebinding in a logical operand needs an explicit scope");
      merge_ownership(skipped);
      ir_.otherwise(native_branch, right.native);
      return scalar(Type::boolean, ir_.merge(native_branch, Type::boolean, left.native));
    }
    std::vector<LLVMEmission::ConversionPoint> points;
    if (converting_method(expr.text, module, 2)) points.push_back(ir_.conversion_point());
    auto right = expression(*expr.children[1], module, env, locals, pure);
    if (!points.empty()) points.push_back(ir_.conversion_point());
    if (const auto selected = method(expr.text, module, {left, right}, expr.at))
      return call_method(*selected, {left, right}, expr.at, env, pure || function_result, points);
    if ((expr.text == "==" || expr.text == "!=") &&
        (left.type.kind == Type::reference || left.type == Type::null_ref) &&
        (right.type.kind == Type::reference || right.type == Type::null_ref)) {
      if (left.type != Type::null_ref && right.type != Type::null_ref) require(right.type, left.type, expr.at);
      return scalar(Type::boolean, ir_.reference_equal(expr.text, left.native, right.native,
                      left.region_id && right.region_id && left.region_id != right.region_id));
    }
    require(right.type, left.type, expr.at);
    if (expr.text == "==" || expr.text == "!=") {
      if (!numeric(left.type) && left.type != Type::boolean) fail(expr.at, "equality requires scalar bits");
      return scalar(Type::boolean, ir_.binary(left.type, expr.text, left.native, right.native));
    }
    if (!numeric(left.type)) fail(expr.at, "arithmetic requires numeric operands");
    if (expr.text == "<" || expr.text == ">" || expr.text == "<=" || expr.text == ">=")
      return scalar(Type::boolean, ir_.binary(left.type, expr.text, left.native, right.native));
    if (floating(left.type) && expr.text == "%") fail(expr.at, "remainder requires integer operands");
    return scalar(left.type, ir_.binary(left.type, expr.text, left.native, right.native));
  }
  Value conditional_expression(const Expression &expr, Module &module, Environment &env, Locals &locals,
                          bool pure) {
    const auto condition = expression(*expr.children[0], module, env, locals, pure);
    require(condition.type, Type::boolean, expr.at);

    const auto before = ownership();
    auto native_branch = ir_.branch(condition.native);
    auto first = block(*expr.children[1], module, env, locals.nested(), pure);
    const auto first_ownership = ownership();
    restore_ownership(before);
    ir_.otherwise(native_branch, first.native);
    auto second = block(*expr.children[2], module, env, locals.nested(), pure);
    merge_ownership(first_ownership);
    if (first.type == Type::null_ref && second.type.kind == Type::reference) {
      first.type = second.type;
      first.region_id = second.region_id;
    } else if (second.type == Type::null_ref && first.type.kind == Type::reference) {
      second.type = first.type;
      second.region_id = first.region_id;
    }
    require(second.type, first.type, expr.at);
    if (first.region_id != second.region_id) fail(expr.at, "conditional references require one owning region");
    auto *native = ir_.merge(native_branch, first.type, second.native);
    if (first.type == Type::void_type || first.type == Type::region || first.type == Type::null_ref)
      return first;
    return scalar(first.type, native, first.region_id);
  }
  Value expression(const Expression &expr, Module &module, Environment &env, Locals &locals,
                   bool pure) {
    const auto location = ir_.source_location(expr.at);
    using K = Expression::Kind;
    switch (expr.kind) {
    case K::null_ref: return scalar(Type::null_ref, ir_.bits(Type::null_ref, 0));
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
        return scalar(type, ir_.bits(type, value));
      }
      std::int64_t value{};
      const auto parsed = std::from_chars(expr.text.data(), expr.text.data() + expr.text.size(), value);
      if (parsed.ec != std::errc{} || parsed.ptr != expr.text.data() + expr.text.size())
        fail(expr.at, "integer literal is outside i64 range");
      return scalar(Type::integer, ir_.bits(Type::integer, value));
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
      return scalar(type, ir_.floating_literal(type, spelling));
    }
    case K::boolean: return scalar(Type::boolean, ir_.bits(Type::boolean, expr.text == "true"));
    case K::let: {
      auto value = expression(*expr.children[0], module, env, locals, pure);
      if (value.type == Type::region)
        fail(expr.at, "whole-region resources require effect bindings, not lexical values");
      if (value.type == Type::void_type || value.type.kind == Type::procedure)
        fail(expr.at, "a lexical value requires a bits value");
      if (!locals.declare(expr.text, value)) fail(expr.at, "duplicate lexical value '" + expr.text + "'");
      return value;
    }
    case K::field: {
      const auto value = expression(*expr.children[0], module, env, locals, pure);
      if (value.type.kind != Type::product && value.type.kind != Type::regional)
        fail(expr.at, "field access requires a product value; dereference regional pointers first");
      const auto &fields = products_.at(value.type.identity).fields;
      for (size_t i = 0; i < fields.size(); ++i) {
        if (fields[i].name != expr.text) continue;
        if (value.type.kind == Type::product)
          return scalar(fields[i].type, ir_.field(value.native, i));
        if (fields[i].type.kind == Type::reference)
          return scalar(fields[i].type, ir_.regional_field(region_pointer(value, expr.at), value.native, value.type, i, fields[i].type), value.region_id);
        return scalar(fields[i].type, ir_.regional_field(region_pointer(value, expr.at), value.native, value.type, i, fields[i].type));
      }
      fail(expr.at, "no field '" + expr.text + "' in " + type_name(value.type));
    }
    case K::index: {
      auto array = expression(*expr.children[0], module, env, locals, pure);
      auto index = expression(*expr.children[1], module, env, locals, pure);
      return array_element(array, index, expr.at);
    }
    case K::name: {
      if (const auto local = locals.visible.find(expr.text); local != locals.visible.end()) return local->second;
      if (const auto global = module.globals.find(expr.text); global != module.globals.end()) return global->second;
      if (const auto methods = module.methods.find(expr.text); methods != module.methods.end()) {
        if (methods->second.size() != 1) fail(expr.at, "a procedure value requires an unambiguous lexical declaration");
        const auto &method = methods->second.front();
        if (method.form->kind == Form::Kind::procedure && !method.form->operation) {
          const auto value = method.lexical->procedures.find(method.form);
          if (value == method.lexical->procedures.end()) fail(expr.at, "lexical procedure used before its definition");
          return value->second;
        }
      }
      fail(expr.at, "unavailable lexical value '" + expr.text + "'");
    }
    case K::symbol:
      if (pure) fail(expr.at, "a function cannot read symbolic effects");
      return read_symbol(expression_symbol(expr, module), env, expr.at);
    case K::effect: {
      if (pure) fail(expr.at, "a function cannot establish effects");
      const auto region_initializer = llvm::SaveAndRestore(region_initializer_,
          expr.children[0]->kind == K::call && expr.children[0]->text == "region");
      auto value = expression(*expr.children[0], module, env, locals, pure);
      value = bind(expression_symbol(expr, module), value, module, env, expr.at);
      scope_rebound_ = true;
      return value;
    }
    case K::allocate: return allocate_expression(expr, module, env, locals, pure);
    case K::range_for: case K::fold: return loop_expression(expr, module, env, locals, pure);
    case K::block: return block(expr, module, env, locals.nested(), pure);
    case K::call: return call_expression(expr, module, env, locals, pure);
    case K::perform: case K::behave: return invoke(expr, module, env, locals, pure);
    case K::unary: return unary_expression(expr, module, env, locals, pure);
    case K::binary: return binary_expression(expr, module, env, locals, pure);
    case K::conditional: return conditional_expression(expr, module, env, locals, pure);
    }
    fail(expr.at, "unsupported expression");
  }
  static void select_method(Module &module, Method function, const std::string &alias, const Location &at) {
    if (alias == "setfield")
      fail(at, "setfield overloads need a compile-time field-selector contract; outside this prototype");
    if (function.form->kind == Form::Kind::procedure && function.form->operation && !intrinsic_name(alias) &&
        binary_precedence(alias) < 0 && alias != "!" && alias != "=" && alias != "[]=")
      fail(at, "a procedure overload requires an operator or intrinsic destination");
    if (!intrinsic_name(alias)) require_user_type_name(alias, at);
    if (module.types.contains(alias) || module.globals.contains(alias))
      fail(at, "duplicate lexical alias '" + alias + "'");
    auto &methods = module.methods[alias];
    if (std::find(methods.begin(), methods.end(), function) == methods.end()) methods.push_back(function);
  }
  static void select_type(Module &module, Type type, const std::string &alias, const Location &at) {
    require_user_type_name(alias, at);
    if (module.methods.contains(alias) || module.globals.contains(alias) ||
        !module.types.emplace(alias, std::move(type)).second)
      fail(at, "duplicate lexical alias '" + alias + "'");
  }
  void execute_module(Module &module, const Environment &caller, const Location &at) {
    // This composes runtime code after the complete module scan. A repeated
    // encounter reuses the incorporated module's resources, never its effects.
    if (module.state == Module::State::complete) return;
    if (module.state == Module::State::executing) fail(at, "cyclic module execution is outside the first prototype");
    const auto module_scope = llvm::SaveAndRestore(module_top_level_, true);
    const auto holding_begin = holdings_.size();
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
    const auto native_scope = ir_.scope();
    auto env = caller;
    Locals locals;

    for (const auto &owned : module.forms) {
      auto &form = *owned;
      switch (form.kind) {
      case Form::Kind::symbol:
        declare_symbol_type(symbol(module, form.name, form.at));
        module.available_symbols.insert(form.name);
        break;
      case Form::Kind::function: case Form::Kind::product: case Form::Kind::array: case Form::Kind::unit: break;
      case Form::Kind::using_units: {
        auto &source = *module.dependencies.at(&form);
        execute_module(source, env, form.at);
        if (!module.unit_sources.insert(&source).second)
          fail(form.at, "UoMs from a source module must be selected in one using bundle");
        std::set<size_t> selected;
        for (const auto &selection : form.units) {
          const auto type = resolve({Type::named, selection.source}, source, selection.at);
          if (type.kind != Type::unit) fail(selection.at, "using units requires UoM definitions");
          if (!module.selected_units.insert(type.identity).second)
            fail(selection.at, "repeated using of UoM identity " + type_name(type));
          selected.insert(type.identity);
          select_type(module, type, selection.local, selection.at);
        }
        for (const auto &function : source.conversions) {
          const auto edge = conversion(function);
          if (!selected.contains(edge.source.identity) || !selected.contains(edge.target.identity)) continue;
          module.conversions.push_back(function);
          select_method(module, function, function.form->name, form.at);
        }
        break;
      }
      case Form::Kind::symbol_from: {
        auto &source = *module.dependencies.at(&form);
        execute_module(source, env, form.at);
        symbol(source, form.name, form.at);
        module.active_aliases.insert(form.alias);
        module.available_symbols.insert(form.alias);
        break;
      }
      case Form::Kind::procedure: {
        if (form.operation) break;
        auto value = define_procedure(form, module, env);
        if (form.symbolic) {
          const auto id = symbol(module, form.name, form.at);
          module.available_symbols.insert(form.name);
          bind(id, std::move(value), module, env, form.at);
        } else {
          module.procedures.emplace(&form, std::move(value));
        }
        break;
      }
      case Form::Kind::global: {
        if (module.methods.contains(form.name)) fail(form.at, "duplicate lexical name '" + form.name + "'");
        auto value = form.body->kind == Expression::Kind::call
            ? call_expression(*form.body, module, env, locals, false, true)
            : form.body->kind == Expression::Kind::unary
            ? unary_expression(*form.body, module, env, locals, false, true)
            : expression(*form.body, module, env, locals, false);
        if (value.region_id || value.type.kind == Type::reference || value.type == Type::region)
          fail(form.at, "module globals contain bits values, not regional resources or references");
        require(value.type, resolve(*form.result_type, module, form.at), form.at);
        module.globals.emplace(form.name, std::move(value));
        break;
      }
      case Form::Kind::expression:
        expression(*form.body, module, env, locals, false);
        break;
      case Form::Kind::using_resource: case Form::Kind::import_resource: {
        if (form.symbolic && !form.using_symbol && !module.available_symbols.contains(form.alias))
          fail(form.at, "procedure destination @" + form.alias + " must be declared before using/import");
        auto &source = *module.dependencies.at(&form);
        execute_module(source, env, form.at);
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
            value = define_procedure(import_body(*value.procedure, form, module), module, env);
          bind(destination, std::move(value), module, env, form.at);
        } else if (const auto type = source.types.find(form.name); type != source.types.end()) {
          if (form.alias_operation) fail(form.at, "an operation name requires a callable overload");
          if (type->second.kind == Type::unit)
            fail(form.at, "select UoMs and their conversions together with using units");
          if (form.kind == Form::Kind::import_resource) fail(form.at, "types preserve identity through using");
          select_type(module, type->second, form.alias, form.at);
        } else if (const auto function = source.methods.find(form.name); function != source.methods.end()) {
          if (!form.alias_operation) require_user_type_name(form.alias, form.at);
          if (form.kind == Form::Kind::import_resource)
            fail(form.at, "function-body import is outside the prototype; use 'using' for a lexical function");
          for (const auto &candidate : function->second) select_method(module, candidate, form.alias, form.at);
        } else if (const auto value = source.globals.find(form.name); value != source.globals.end()) {
          if (form.alias_operation) fail(form.at, "an operation name requires a callable overload");
          if (module.types.contains(form.alias) || module.methods.contains(form.alias) ||
              !module.globals.emplace(form.alias, value->second).second)
            fail(form.at, "duplicate lexical alias '" + form.alias + "'");
        } else {
          fail(form.at, "unavailable module resource '" + form.name + "'");
        }
        break;
      }
      }
    }
    for (const auto &[id, current] : env.bindings) {
      if (current->owner != &module) continue;
      auto value = *current;
      if (value.type.kind != Type::procedure && value.type != Type::region) {
        value.native = ir_.read_symbol(id, value.type);
      }
      module.exports.emplace(id, std::move(value));
    }

    close_holdings(holding_begin, {}, at);
    ir_.restore(native_scope);
    module.state = Module::State::complete;
  }

public:
  explicit Compiler(bool warnings_as_errors = false) : warnings_as_errors_(warnings_as_errors) {}
  Compilation compile(const std::filesystem::path &entry) {
    auto &module = load(entry.string(), {entry.string()});
    // All source files have been parsed before any execution semantics are lowered.

    try {
      execute_module(module, {}, {module.path});
    } catch (const std::runtime_error &error) {
      // Successful calls pop their sites. An error leaves the demanded path
      // intact, so format it once without copying a growing message on unwind.
      std::string message = error.what();
      for (size_t depth = 0; depth < call_path_.size(); ++depth) {
        if (depth == 4 && call_path_.size() > 8) {
          message += "\nnote: " + std::to_string(call_path_.size() - 8) + " intervening calls omitted";
          depth = call_path_.size() - 4;
        }
        const auto &site = call_path_[call_path_.size() - 1 - depth];
        const bool procedure = site.form->kind == Form::Kind::procedure;
        message += "\n" + location_text(site.at) + ": note: in call to " +
            (procedure ? "procedure '" : "function '") + callable_name(*site.form) +
            "' defined at " + location_text(site.form->at);
      }
      throw std::runtime_error(message);
    }
    auto [llvm_ir, bitcode] = ir_.finish();
    return {scanned_, execution_order_, warnings_, std::move(llvm_ir), std::move(bitcode), regional_runtime_};
  }
};

} // namespace cod::language
