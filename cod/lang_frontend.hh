//===--- lang_frontend.hh - Demand-checked CoD native lowering ------===//
#pragma once

#include "lang_syntax.hh"
#include "lang_llvm.hh"

#include <llvm/ADT/ImmutableMap.h>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <compare>
#include <cstdint>
#include <deque>
#include <fstream>
#include <iterator>
#include <map>
#include <queue>
#include <set>
#include <string_view>
#include <variant>

namespace cod::language {

struct Module;
struct Procedure;
struct Conformance {
  Type data, interface;
  bool operator==(const Conformance &) const = default;
};
struct Value {
  Type type = Type::void_type;
  llvm::Value *native = nullptr; // Procedure bindings are compile-time metadata.
  std::shared_ptr<const Procedure> procedure;
  const Module *owner = nullptr;
  std::optional<size_t> region_id = {};
  std::optional<size_t> holding_id = {};
  std::vector<Conformance> conformances = {}; // Selected type facts carried by a borrowed view, never runtime metadata.
  std::shared_ptr<const std::vector<Value>> arguments = {}; // WPO argument pack; no user allocation or runtime container.
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
  std::map<std::string, Location> locations;

  Locals nested() const { return {visible, {}, locations}; }
  bool declare(const std::string &name, Value value, const Location &at) {
    if (!declared.insert(name).second) return false;
    visible.insert_or_assign(name, std::move(value));
    locations.insert_or_assign(name, at);
    return true;
  }
};
using ImplementationContext = std::vector<const Module *>;
struct Method {
  Form *form;
  const Module *lexical;
  ImplementationContext implementation{}; // Source-to-receiver interpretation, entirely compile-time.
  bool operator==(const Method &) const = default;
};
struct CallableDeclaration {
  const Form *form;
  const Module *lexical;
  bool operator==(const CallableDeclaration &) const = default;
};
struct Procedure {
  Form *form = nullptr;
  const Module *lexical = nullptr;
  std::string builtin;
  Environment captures;
  ImplementationContext implementation{};
  std::vector<CallableDeclaration> declarations;
};
using MethodOrigin = std::pair<const Module *, std::string>;
struct CallableIdentity {
  const Module *module;
  // Declarations/explicit unions use their defining name. Imported identities
  // use a receiver-local ordinal, independent of every alias exposing them.
  std::variant<std::string, size_t> definition;
  auto operator<=>(const CallableIdentity &) const = default;
};
struct CallableInformation {
  std::vector<Method> methods;
  std::set<MethodOrigin> origins;
  bool implicit = false;
  std::set<CallableIdentity> identities;
  std::vector<CallableDeclaration> declarations;
};
struct Callable {
  std::vector<Method> methods;
  std::set<MethodOrigin> origins;
  std::set<CallableIdentity> members; // Receiver-local virtual union, by identity.
  std::vector<CallableDeclaration> declarations; // Signatures promise behavior; only methods have bodies.
};
struct GlobalIdentity {
  MethodOrigin origin;
  bool operator==(const GlobalIdentity &) const = default;
};
using PublishedIdentity = std::variant<size_t, Type, CallableIdentity, GlobalIdentity>;
// Each module owns its selected semantic information. Declaration identities
// can be shared with other worlds; the selection containers are never shared.
// Native values here describe the demanded program, not a runtime world ABI.
struct ModuleWorld {
  std::map<std::string, size_t> symbols;
  std::map<std::string, Form *> symbol_aliases;
  std::set<std::string> available_symbols;
  std::map<CallableIdentity, Callable> callables;
  std::map<CallableIdentity, CallableIdentity> imported_callables;
  std::map<std::string, CallableIdentity> callable_aliases;
  std::map<std::string, CallableIdentity> associated_aliases;
  std::map<std::string, CallableIdentity> local_callables;
  std::map<std::string, bool> explicit_method_imports;
  std::set<std::string> lexical_names;
  std::map<std::string, PublishedIdentity> exports;
  std::map<std::string, Type> types;
  std::set<size_t> imported_data;
  std::vector<Conformance> conformances;
  std::vector<Method> conversions;
  std::set<const Module *> unit_sources;
  std::set<size_t> selected_units;
  std::map<MethodOrigin, Value> global_values;
  std::map<std::string, MethodOrigin> global_aliases;
  std::map<const Form *, Value> procedures;
  std::map<size_t, Value> symbolic_resources;
};
struct Module {
  std::string path;
  std::vector<std::unique_ptr<Form>> forms;
  std::map<const Form *, Module *> dependencies;
  ModuleWorld world;
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
enum class Warning { unit_conversion, multi_alias_extension, shadow };
struct WarningOptions {
  bool as_errors = false;
  std::set<Warning> errors;
};

class Compiler {
  // Shared compiler snapshots retain binding values until lowering finishes.
  // The factory outlives every map; none of this storage enters the program.
  Bindings::Factory binding_factory_{false};
  std::deque<Value> binding_values_;
  std::map<std::string, std::unique_ptr<Module>> modules_;
  std::vector<std::string> symbol_names_;
  std::map<size_t, Type> symbol_types_;
  std::map<size_t, CallableDeclaration> symbol_declarations_;
  std::vector<std::string> scanned_;
  std::vector<std::string> execution_order_;
  std::set<const Form *> active_calls_;
  struct CallSite { const Form *form; Location at; };
  std::vector<CallSite> call_path_;
  ImplementationContext implementation_context_;
  std::vector<Conformance> caller_conformances_;
  struct TypeContext {
    const Module *lexical = nullptr;
    std::map<std::string, Type> bindings;
  } type_context_;
  struct Signature {
    std::map<std::string, Type> bindings;
    std::vector<Type> parameters;
    std::optional<Type> result;
  };
  std::vector<std::unique_ptr<Form>> mixed_forms_;
  struct Product {
    Form *form;
    Module *module;
    std::vector<Parameter> fields;
    bool resolved = false;
  };
  std::vector<Product> products_;
  struct Abstract {
    Form *form;
    Module *module;
    std::optional<Type> parent;
    enum State { unresolved, resolving, complete } state = unresolved;
  };
  std::vector<Abstract> abstracts_;
  struct Array {
    Form *form;
    Module *module;
    Type element, index;
    bool resolved = false;
  };
  std::vector<Array> arrays_;
  // Dynamic resources cannot be extracted. Branch and iteration scopes close
  // their own acquisitions without changing an enclosing resource's lifetime.
  struct Region { llvm::Value *owner; Location acquired; bool live = true; };
  std::vector<Region> regions_;
  struct Holding { size_t region; bool module_lifetime; };
  std::vector<Holding> holdings_;
  bool module_top_level_ = false;
  bool region_initializer_ = false;
  bool regional_runtime_ = false;
  size_t next_unit_ = 0;
  LLVMEmission ir_;
  WarningOptions warning_options_;
  std::vector<std::string> warnings_;
  bool scope_rebound_ = false; // Includes bindings with no native TLS slot.

  void warn(Warning kind, const Location &at, std::string message) {
    const auto category = kind == Warning::unit_conversion ? "unit-conversion" :
                          kind == Warning::shadow ? "shadow" : "multi-alias-extension";
    const auto newline = message.find('\n');
    message.insert(newline == std::string::npos ? message.size() : newline,
                   " [-W" + std::string(category) + "]");
    if (warning_options_.as_errors || warning_options_.errors.contains(kind))
      fail(at, "warning treated as error: " + message);
    auto diagnostic = location_text(at) + ": warning: " + message;
    if (kind != Warning::shadow || std::find(warnings_.begin(), warnings_.end(), diagnostic) == warnings_.end())
      warnings_.push_back(std::move(diagnostic));
  }

  void declare_local(Locals &locals, const std::string &name, Value value,
                     const Module &module, const Location &at) {
    if (locals.declared.contains(name)) fail(at, "duplicate lexical value '" + name + "'");
    if (const auto previous = locals.locations.find(name); previous != locals.locations.end()) {
      warn(Warning::shadow, at, "local '" + name + "' shadows an outer binding\n" +
           location_text(previous->second) + ": note: shadowed binding declared here");
    } else if (module.world.callable_aliases.contains(name) || module.world.associated_aliases.contains(name)) {
      warn(Warning::shadow, at, "local '" + name + "' shadows a callable in module '" + module.path + "'");
    } else if (module.world.global_aliases.contains(name) || module.world.types.contains(name)) {
      warn(Warning::shadow, at, "local '" + name + "' shadows a module binding in '" + module.path + "'");
    }
    locals.declare(name, std::move(value), at);
  }

  static void require_user_type_name(const std::string &name, const Location &at) {
    static const std::set<std::string> reserved{"i64", "u8", "u32", "u64", "f32", "f64", "bool", "void", "region", "ref"};
    if (reserved.contains(name)) fail(at, "reserved scalar type name '" + name + "'");
    if (intrinsic_name(name) || expression_keyword(name) || module_keyword(name))
      fail(at, "reserved keyword '" + name + "' cannot be used as an ordinary name");
  }

  static Type reference(Type target) {
    Type type{Type::reference};
    type.target = std::make_shared<Type>(std::move(target));
    return type;
  }
  std::string region_note(std::optional<size_t> region, std::string_view message) const {
    if (!region) return {};
    return "\n" + location_text(regions_.at(*region).acquired) + ": note: " + std::string(message);
  }
  llvm::Value *region_pointer(const Value &value, const Location &at) {
    if (!value.region_id || !regions_.at(*value.region_id).live)
      fail(at, "regional value requires a live owning region in this caller scope" +
                   region_note(value.region_id, "the referenced region was acquired here"));
    return regions_.at(*value.region_id).owner;
  }
  void close_holdings(size_t begin, const Value &result, const Location &at) {
    for (size_t i = begin; i < holdings_.size(); ++i) {
      const auto &holding = holdings_[i];
      if (holding.module_lifetime || !regions_[holding.region].live) continue;
      if (result.region_id == holding.region)
        fail(at, "regional result would outlive its owning scope" +
                     region_note(holding.region, "the escaping value depends on this region acquisition"));
      regions_[holding.region].live = false;
    }
  }
  Type resolve_name(Type type, const Module &module, const Location &at) {
    if (type.kind != Type::named) return type;
    if (!type.target && type_context_.lexical == &module)
      if (const auto found = type_context_.bindings.find(type.name); found != type_context_.bindings.end())
        return found->second;
    const auto found = module.world.types.find(type.name);
    if (found == module.world.types.end()) fail(at, "unavailable type '" + type.name + "'");
    if (type.target) {
      if (found->second.kind != Type::unit) fail(at, "a quantity annotation requires a UoM identity");
      auto number = resolve_name(*type.target, module, at);
      if (!numeric(number) && number.kind != Type::parameter)
        fail(at, "a quantity requires a numeric representation");
      return quantity(found->second, std::move(number));
    }
    return found->second;
  }
  void resolve_abstract(Type type, const Location &at) {
    auto &definition = abstracts_.at(type.identity);
    if (definition.state == Abstract::complete) return;
    if (definition.state == Abstract::resolving) fail(at, "cyclic abstract type inheritance");
    definition.state = Abstract::resolving;
    if (definition.form->parent_type) {
      auto parent = resolve_name(*definition.form->parent_type, *definition.module, definition.form->at);
      if (parent.kind != Type::abstract_type) fail(definition.form->at, "an abstract parent must be an abstract type");
      resolve_abstract(parent, definition.form->at);
      definition.parent = parent;
    }
    definition.state = Abstract::complete;
  }
  bool nominal_subtype(Type actual, Type expected, const Location &at) {
    if (actual == expected) return true;
    if (actual.kind != Type::abstract_type || expected.kind != Type::abstract_type) return false;
    resolve_abstract(actual, at);
    const auto &parent = abstracts_.at(actual.identity).parent;
    return parent && nominal_subtype(*parent, expected, at);
  }
  bool subtype(Type actual, Type expected, const Module &visible, const Location &at) {
    if (nominal_subtype(actual, expected, at)) return true;
    if (actual.kind != Type::regional || expected.kind != Type::abstract_type) return false;
    const auto matches = [&](const Conformance &relation) {
      return relation.data == actual && nominal_subtype(relation.interface, expected, at);
    };
    return std::any_of(visible.world.conformances.begin(), visible.world.conformances.end(), matches) ||
           std::any_of(caller_conformances_.begin(), caller_conformances_.end(), matches);
  }
  static void append_conformance(std::vector<Conformance> &relations, Conformance relation) {
    if (std::find(relations.begin(), relations.end(), relation) == relations.end()) relations.push_back(relation);
  }
  static void merge_conformances(std::vector<Conformance> &into, const std::vector<Conformance> &from) {
    for (const auto &relation : from) append_conformance(into, relation);
  }
  bool referenced_data(Type type, std::set<size_t> &reachable) const {
    if (type.kind == Type::reference) return referenced_data(*type.target, reachable);
    if (type.kind != Type::regional || !reachable.insert(type.identity).second) return true;
    const auto &product = products_.at(type.identity);
    // Unknown layouts retain their facts; checking a fold must not demand them.
    if (!product.resolved) return false;
    return std::all_of(product.fields.begin(), product.fields.end(), [&](const auto &field) {
      return referenced_data(field.type, reachable);
    });
  }
  Value contextual(Value value, const Module &module) {
    if (value.type.kind == Type::regional || value.type.kind == Type::reference || value.type.kind == Type::array) {
      merge_conformances(value.conformances, module.world.conformances);
      merge_conformances(value.conformances, caller_conformances_);
    }
    return value;
  }
  bool establishes(const Value &value, Conformance required, const Location &at) {
    return std::any_of(value.conformances.begin(), value.conformances.end(), [&](const Conformance &relation) {
      return relation.data == required.data && nominal_subtype(relation.interface, required.interface, at);
    });
  }
  bool accepts(const Value &actual, Type expected, const Module &visible, const Location &at) {
    return subtype(actual.type, expected, visible, at) ||
           (actual.type.kind == Type::regional && establishes(actual, {actual.type, expected}, at));
  }
  Value admit(Value actual, Type expected, const Module &visible, const Location &at) {
    actual = contextual(std::move(actual), visible);
    if (!accepts(actual, expected, visible, at)) {
      auto message = "expected " + type_name(expected) + ", got " + type_name(actual.type);
      if (expected.kind == Type::abstract_type) {
        message += "\n" + location_text(at) + ": note: required interface is not established in module '" + visible.path + "'";
        for (const auto &relation : actual.conformances)
          if (relation.data == actual.type)
            message += "\n" + location_text(at) + ": note: available value guarantee is " + type_name(relation.interface);
      }
      fail(at, message);
    }
    return actual;
  }
  void declare_conformance(Module &module, const Form &form) {
    const auto data = resolve_name({Type::named, form.name}, module, form.at);
    if (data.kind != Type::regional) fail(form.at, "a data conformance requires a concrete regional data type");
    if (products_.at(data.identity).module != &module && !module.world.imported_data.contains(data.identity))
      fail(form.at, "declaring external data conformance requires import of the data type");
    const auto interface = resolve_name(*form.parent_type, module, form.at);
    if (interface.kind != Type::abstract_type) fail(form.at, "a data conformance requires an abstract interface");
    resolve_abstract(interface, form.at);
    const Conformance relation{data, interface};
    if (std::find(module.world.conformances.begin(), module.world.conformances.end(), relation) != module.world.conformances.end())
      fail(form.at, "repeated data conformance declaration is outside this prototype");
    module.world.conformances.push_back(relation);
  }
  Type resolve(Type type, const Module &module, const Location &at) {
    type = resolve_name(type, module, at);
    if (type.kind == Type::unit) fail(at, "a UoM identity requires a numeric representation, such as f64(" + type.name + ")");
    if (type.kind == Type::abstract_type) resolve_abstract(type, at);
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
      if (target.kind == Type::reference || target == Type::region || target == Type::void_type ||
          target.kind == Type::procedure || target.kind == Type::abstract_type)
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
        if (!numeric(array.element) && array.element != Type::boolean && array.element.kind != Type::quantity)
          fail(at, "initial regional arrays store scalar bits; use parallel arrays for product fields");
        if (!integral(array.index) && !(array.index.kind == Type::quantity && integral(representation(array.index))))
          fail(at, "an array index type requires an integer representation");
        array.resolved = true;
      }
      regional_runtime_ = true;
    }
    if (type.kind == Type::regional) {
      auto &product = products_.at(type.identity);
      if (!product.resolved) {
        std::set<std::string> fields;
        for (const auto &field : product.form->parameters) {
          if (!fields.insert(field.name).second) fail(field.at, "duplicate product field '" + field.name + "'");
          auto field_type = resolve_name(field.type, *product.module, field.at);
          if (field_type.kind == Type::procedure) fail(field.at, "procedure fields are outside the prototype");
          if (field_type == Type::region || field_type.kind == Type::regional || field_type.kind == Type::array ||
              field_type.kind == Type::abstract_type)
            fail(field.at, "data object fields currently require explicit ref<T> references");
          field_type = resolve(field_type, *product.module, field.at);
          product.fields.push_back({field.name, field_type, field.at, false, {}});
        }
        regional_runtime_ = true;
        std::vector<Type> field_types;
        for (const auto &field : product.fields) field_types.push_back(field.type);
        ir_.record(type, field_types);
        product.resolved = true;
      }
    }
    return type;
  }
  Value scalar(Type type, llvm::Value *native, std::optional<size_t> region_id = {}) {
    return {type, native, {}, nullptr, region_id};
  }
  Value widened(Value value, Type target, const Location &at) {
    if (value.type == target) return value;
    if (!widens(value.type, target))
      fail(at, "no lossless numeric widening from " + type_name(value.type) + " to " + type_name(target));
    value.native = ir_.cast(value.type, target, value.native);
    value.type = std::move(target);
    return value;
  }
  static bool variadic(const Form &form) {
    return !form.parameters.empty() && form.parameters.back().variadic;
  }
  static const Parameter &parameter_at(const Form &form, size_t index) {
    return form.parameters[std::min(index, form.parameters.size() - 1)];
  }
  std::optional<Signature> signature(const Form &form, const Module &lexical, const std::vector<Value> &args,
                                      const Module &caller, const Location &at, bool required = false) {
    const auto reject = [&](const std::string &reason) -> std::optional<Signature> {
      if (required) fail(at, reason + " for '" + form.name + "'\n" + location_text(form.at) + ": note: signature declared here");
      return {};
    };
    const auto minimum = form.parameters.size() - (variadic(form) ? 1 : 0);
    if (args.size() < minimum || (!variadic(form) && args.size() != minimum)) return reject("wrong argument count");
    const auto context = llvm::SaveAndRestore(type_context_, TypeContext{&lexical, {}});
    const auto infer = [&](auto &&self, const Type &pattern, const Type &actual) -> bool {
      if (pattern.kind == Type::named && !pattern.target && std::any_of(form.type_parameters.begin(), form.type_parameters.end(),
          [&](const TypeParameter &p) { return p.name == pattern.name; })) {
        const auto [found, added] = type_context_.bindings.emplace(pattern.name, actual);
        return added || found->second == actual;
      }
      if (pattern.target && actual.target && !self(self, *pattern.target, *actual.target)) return false;
      if (pattern.parameters.size() == actual.parameters.size())
        for (size_t i = 0; i < pattern.parameters.size(); ++i)
          if (!self(self, pattern.parameters[i], actual.parameters[i])) return false;
      return true;
    };
    for (size_t i = 0; i < args.size(); ++i)
      if (!infer(infer, parameter_at(form, i).type, args[i].type)) return reject("inconsistent type-parameter arguments");
    for (const auto &parameter : form.type_parameters) {
      const auto found = type_context_.bindings.find(parameter.name);
      if (found == type_context_.bindings.end()) return reject("cannot infer type parameter '" + parameter.name + "'");
      if (parameter.bound) {
        Value value; value.type = found->second;
        for (const auto &arg : args) merge_conformances(value.conformances, arg.conformances);
        if (!accepts(value, resolve(*parameter.bound, lexical, parameter.at), caller, at))
          return reject("type argument does not satisfy bound for '" + parameter.name + "'");
      }
    }
    Signature result{type_context_.bindings, {}, {}};
    for (size_t i = 0; i < args.size(); ++i) {
      const auto &parameter = parameter_at(form, i);
      result.parameters.push_back(resolve(parameter.type, lexical, parameter.at));
    }
    if (form.result_type) result.result = resolve(*form.result_type, lexical, form.at);
    return result;
  }
  static void require(Type actual, Type expected, const Location &at) {
    if (actual != expected)
      fail(at, "expected " + type_name(expected) + ", got " + type_name(actual));
  }
  size_t new_symbol(Module &module, const std::string &name) {
    const auto id = symbol_names_.size();
    symbol_names_.push_back(module.path + "::@" + name);
    module.world.symbols.emplace(name, id);
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
      result.world.exports.emplace("@print", result.world.symbols.at("print"));
      result.world.exports.emplace("@assert", result.world.symbols.at("assert"));
      return result;
    }
    std::ifstream input(canonical);
    if (!input) fail(at, "cannot read module '" + canonical + "'");
    const std::string source((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    result.forms = Parser(source, canonical).parse();
    auto &lexical_names = result.world.lexical_names;
    std::set<std::string> lexical_selections;
    for (const auto &owned : result.forms) {
      auto &form = *owned;
      if (form.kind == Form::Kind::export_resource) {
        continue; // Publication resolves identities after this module is constructed.
      } else if (form.kind == Form::Kind::symbol) {
        if (result.world.symbols.contains(form.name) || result.world.symbol_aliases.contains(form.name))
          fail(form.at, "duplicate symbol name @" + form.name);
        symbol_declarations_.emplace(new_symbol(result, form.name), CallableDeclaration{&form, &result});
      } else if (form.kind == Form::Kind::procedure && form.symbolic) {
        if (!result.world.symbols.contains(form.name) && !result.world.symbol_aliases.contains(form.name))
          symbol_declarations_.emplace(new_symbol(result, form.name), CallableDeclaration{&form, &result});
      } else if (form.kind == Form::Kind::unit) {
        require_user_type_name(form.name, form.at);
        if (!lexical_names.insert(form.name).second) fail(form.at, "duplicate lexical name '" + form.name + "'");
        result.world.types.emplace(form.name, Type{Type::unit, result.path + "::" + form.name,
                                            next_unit_++});
      } else if (form.kind == Form::Kind::array) {
        require_user_type_name(form.name, form.at);
        if (!lexical_names.insert(form.name).second) fail(form.at, "duplicate lexical name '" + form.name + "'");
        result.world.types.emplace(form.name, Type{Type::array, result.path + "::" + form.name, arrays_.size()});
        arrays_.push_back({&form, &result, {}, {}, false});
      } else if (form.kind == Form::Kind::abstract_type) {
        require_user_type_name(form.name, form.at);
        if (!lexical_names.insert(form.name).second) fail(form.at, "duplicate lexical name '" + form.name + "'");
        result.world.types.emplace(form.name, Type{Type::abstract_type, result.path + "::" + form.name, abstracts_.size()});
        abstracts_.push_back({&form, &result, {}});
      } else if (form.kind == Form::Kind::data) {
        require_user_type_name(form.name, form.at);
        if (!lexical_names.insert(form.name).second) fail(form.at, "duplicate lexical name '" + form.name + "'");
        const Type type{Type::regional, result.path + "::" + form.name, products_.size()};
        result.world.types.emplace(form.name, type);
        products_.push_back({&form, &result, {}});
      } else if (form.kind == Form::Kind::function || (form.kind == Form::Kind::procedure && !form.symbolic) || form.kind == Form::Kind::global) {
        if (!lexical_names.insert(form.name).second &&
            (form.kind == Form::Kind::global ||
             (!result.world.callable_aliases.contains(form.name) && !lexical_selections.contains(form.name))))
          fail(form.at, "duplicate lexical name '" + form.name + "'");
        if (form.kind != Form::Kind::global) {
          if (!form.operation) require_user_type_name(form.name, form.at);
          if (form.operation && form.name == "setfield")
            fail(form.at, "setfield overloads need a compile-time field-selector contract; outside this prototype");
          declare_callable(result, form);
          if (form.conversion) result.world.conversions.push_back({&form, &result});
        }
      } else if (form.kind == Form::Kind::using_units) {
        for (const auto &selection : form.units) {
          require_user_type_name(selection.local, selection.at);
          if (!lexical_names.insert(selection.local).second)
            fail(selection.at, "duplicate lexical name '" + selection.local + "'");
        }
      } else if (form.kind == Form::Kind::symbol_from) {
        if (result.world.symbols.contains(form.alias) || !result.world.symbol_aliases.emplace(form.alias, &form).second)
          fail(form.at, "duplicate symbol alias @" + form.alias);
      } else if (form.kind == Form::Kind::using_resource || form.kind == Form::Kind::import_resource ||
                 form.kind == Form::Kind::mixin_resource) {
        if (form.using_symbol && !result.world.symbols.contains(form.alias))
          result.world.symbol_aliases.try_emplace(form.alias, &form);
        if (!form.symbolic) {
          if (!lexical_names.insert(form.alias).second &&
              !result.world.callable_aliases.contains(form.alias) && !lexical_selections.contains(form.alias))
            fail(form.at, "duplicate lexical alias '" + form.alias + "'");
          lexical_selections.insert(form.alias);
        }
      }
    }
    for (const auto &owned : result.forms) {
      auto &form = *owned;
      if (form.kind != Form::Kind::symbol_from && form.kind != Form::Kind::using_resource &&
          form.kind != Form::Kind::import_resource && form.kind != Form::Kind::mixin_resource &&
          form.kind != Form::Kind::using_units) continue;
      const auto dependency = form.path == "std:io" ? form.path :
          (std::filesystem::path(canonical).parent_path() / form.path).string();
      result.dependencies.emplace(&form, &load(dependency, form.at));
    }
    return result;
  }
  size_t symbol(const Module &module, const std::string &name, const Location &at) {
    if (const auto found = module.world.symbols.find(name); found != module.world.symbols.end()) return found->second;
    if (const auto alias = module.world.symbol_aliases.find(name); alias != module.world.symbol_aliases.end())
      fail(at, "symbol alias @" + name + " requires its " +
                   (alias->second->using_symbol ? "using symbol" : "symbol from") + " encounter first in this prototype");
    fail(at, "undeclared symbol @" + name);
  }
  size_t expression_symbol(const Expression &expr, const Module &module) {
    if (expr.symbol_id) return *expr.symbol_id;
    return symbol(module, expr.text, expr.at);
  }
  static bool uses_symbol(Expression::Kind kind) {
    using K = Expression::Kind;
    return kind == K::symbol || kind == K::effect || kind == K::perform || kind == K::behave;
  }
  Expr mixin_expression(const Expression &source_expr, const Module &source, const Module &receiver,
                         const std::map<std::string, std::string> &names,
                         std::map<size_t, size_t> &identities, std::set<std::string> &used) {
    auto expr = std::make_unique<Expression>(Expression{
        source_expr.kind, source_expr.at, source_expr.text, {}, source_expr.symbol_id, source_expr.binding, source_expr.spread});
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
      expr->children.push_back(mixin_expression(*child, source, receiver, names, identities, used));
    return expr;
  }
  Form &mixin_body(const Procedure &source, const Form &selection, const Module &receiver) {
    if (!source.form) fail(selection.at, "a built-in procedure has no mixin body; use using or import");
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
    form->symbolic = selection.symbolic;
    form->parameters = source.form->parameters;
    form->type_parameters = source.form->type_parameters;
    form->result_type = source.form->result_type;
    std::map<size_t, size_t> identities;
    std::set<std::string> used;
    form->body = mixin_expression(*source.form->body, *source.lexical, receiver, names, identities, used);
    for (const auto &mapping : selection.mappings)
      if (!used.contains(mapping.source))
        fail(mapping.at, "unknown source-body role @" + mapping.source + " in symbol map");
    auto &result = *form;
    mixed_forms_.push_back(std::move(form));
    return result;
  }
  Type procedure_type(const Form &form, const Module &module) {
    Type type{Type::procedure};
    const auto context = llvm::SaveAndRestore(type_context_, TypeContext{&module, {}});
    for (size_t i = 0; i < form.type_parameters.size(); ++i)
      type_context_.bindings.emplace(form.type_parameters[i].name, Type{Type::parameter, {}, i});
    for (const auto &parameter : form.type_parameters)
      type.bounds.push_back(parameter.bound ? resolve(*parameter.bound, module, parameter.at) : Type::void_type);
    type.variadic = variadic(form);
    for (const auto &parameter : form.parameters)
      type.parameters.push_back(resolve(parameter.type, module, parameter.at));
    type.target = std::make_shared<Type>(form.result_type ? resolve(*form.result_type, module, form.at) : Type::void_type);
    return type;
  }
  bool signature_subset(const Method &left, const Method &right, const Module &caller,
                        const Location &at, const std::vector<Value> &args, bool widening = false) {
    const auto l = procedure_type(*left.form, *left.lexical);
    const auto r = procedure_type(*right.form, *right.lexical);
    if (l.parameters.size() - l.variadic < r.parameters.size() - r.variadic) return false;
    std::map<size_t, Type> substitutions;
    const auto covers = [&](auto &&self, Type a, Type b, bool allow_widening) -> bool {
      if (b.kind == Type::parameter) {
        const auto [found, inserted] = substitutions.emplace(b.identity, a);
        if (!inserted && found->second != a) return false;
        const auto bound = r.bounds.at(b.identity);
        if (bound == Type::void_type) return true;
        if (bound.kind == Type::parameter) return false;
        return self(self, a, bound, allow_widening);
      }
      if (a.kind == Type::parameter) {
        const auto bound = l.bounds.at(a.identity);
        if (bound == Type::void_type || bound.kind == Type::parameter) return false;
        return self(self, bound, b, allow_widening);
      }
      if (a.kind == b.kind && a.target && b.target) {
        if (a.kind == Type::quantity && a.identity != b.identity) return false;
        if (a.parameters.size() != b.parameters.size() || a.variadic != b.variadic) return false;
        for (size_t i = 0; i < a.parameters.size(); ++i)
          if (!self(self, a.parameters[i], b.parameters[i], false)) return false;
        return self(self, *a.target, *b.target, allow_widening && a.kind == Type::quantity);
      }
      return (allow_widening && widens(a, b)) || subtype(a, b, caller, at) || std::any_of(args.begin(), args.end(), [&](const Value &value) {
        return value.type == a && accepts(value, b, caller, at);
      });
    };
    for (size_t i = 0; i < args.size(); ++i)
      if (!covers(covers, l.parameters[std::min(i, l.parameters.size() - 1)],
                          r.parameters[std::min(i, r.parameters.size() - 1)], widening)) return false;
    // A fixed arity is a subset of a matching variadic signature, not vice versa.
    if (l.variadic != r.variadic) return !l.variadic;
    return true;
  }
  void declare_symbol_type(size_t id) {
    if (symbol_types_.contains(id)) return;
    const auto declaration = symbol_declarations_.find(id);
    if (declaration == symbol_declarations_.end()) return; // Compiler-provided intrinsic family.
    const auto *form = declaration->second.form;
    auto *module = declaration->second.lexical;
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
    auto result = scalar(type, env.captured(id, found) ? found->native : ir_.read_symbol(id, type), found->region_id);
    result.conformances = found->conformances;
    return result;
  }
  Value bind(size_t id, Value value, const Module &owner, Environment &env, const Location &at) {
    if (value.type == Type::void_type || value.arguments) fail(at, "a symbol cannot be bound to void or a variadic argument pack");
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
        fail(at, "cannot transfer ownership out of a dynamic region binding; use a new resource to shadow it" +
                     region_note(value.region_id, "the non-extractable region was acquired here"));
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

  void resolve_procedure_symbols(Expression &expr, const Module &module) {
    if (uses_symbol(expr.kind) && !expr.symbol_id)
      expr.symbol_id = symbol(module, expr.text, expr.at);
    for (const auto &child : expr.children) resolve_procedure_symbols(*child, module);
  }
  Value define_procedure(Form &form, const Module &lexical, const Environment &env) {
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
  Value block(const Expression &expr, const Module &module, Environment env, Locals locals,
              bool function_context, bool discard_result = false) {
    const auto module_scope = llvm::SaveAndRestore(module_top_level_, false);
    const auto region_initializer = llvm::SaveAndRestore(region_initializer_, false);
    const auto rebound = llvm::SaveAndRestore(scope_rebound_, false);
    const auto holding_begin = holdings_.size();
    const auto native_scope = ir_.scope();

    Value result;
    for (const auto &child : expr.children) result = expression(*child, module, env, locals, function_context);
    if (discard_result) result = {};
    close_holdings(holding_begin, result, expr.at);
    ir_.restore(native_scope);
    if (result.type.kind == Type::procedure) fail(expr.at, "procedure-valued scope results are not supported");
    if (result.arguments) fail(expr.at, "a variadic argument pack cannot escape its call scope");
    return result;
  }
  std::vector<Value> arguments(const Expression &expr, const Module &module, Environment &env,
                               Locals &locals, bool function_context,
                               std::vector<LLVMEmission::ConversionPoint> *points = nullptr) {
    const auto region_initializer = llvm::SaveAndRestore(region_initializer_, false);
    std::vector<Value> values;
    for (const auto &arg : expr.children) {
      auto value = expression(*arg, module, env, locals, function_context);
      if (arg->spread) {
        if (!value.arguments) fail(arg->at, "argument expansion requires a variadic parameter pack");
        for (const auto &element : *value.arguments) {
          values.push_back(element);
          if (points) points->push_back(ir_.conversion_point());
        }
      } else {
        values.push_back(std::move(value));
        if (points) points->push_back(ir_.conversion_point());
      }
    }
    return values;
  }
  static bool converting_parameters(const Form &form) {
    return std::any_of(form.parameters.begin(), form.parameters.end(),
                       [](const Parameter &parameter) { return parameter.automatic_conversion; });
  }
  bool converting_method(const std::string &name, const Module &module,
                         const Location &at) {
    const auto methods = lookup_callable(module, name, at).methods;
    return std::any_of(methods.begin(), methods.end(), [&](const Method &function) {
      return converting_parameters(*function.form);
    });
  }
  Value call(Form &form, const Module &lexical, Environment env, const std::vector<Value> &args,
             bool function_context, const Location &at, const Module &caller, ImplementationContext implementation = {}) {
    const auto instantiated = *signature(form, lexical, args, caller, at, true);
    const auto types = llvm::SaveAndRestore(type_context_, TypeContext{&lexical, instantiated.bindings});
    if (!form.body) fail(at, "callable '" + form.name + "' has no method implementation");
    // A using selection retains this context; import selects the receiving module.
    // The source body still resolves types, globals and symbols in lexical.
    const auto saved_context = llvm::SaveAndRestore(implementation_context_,
        implementation.empty() ? ImplementationContext{&lexical} : std::move(implementation));
    const auto saved_conformances = llvm::SaveAndRestore(caller_conformances_);
    for (const auto &arg : args) {
      const auto value = contextual(arg, caller);
      const auto type = value.type.kind == Type::reference ? *value.type.target : value.type;
      for (const auto &relation : value.conformances)
        if (relation.data == type) append_conformance(caller_conformances_, relation);
    }
    call_path_.push_back({&form, at});
    if (form.conversion) conversion({&form, &lexical});
    if (!active_calls_.insert(&form).second) fail(at, "recursive call is outside the first prototype");
    Locals locals;
    const auto fixed = form.parameters.size() - (variadic(form) ? 1 : 0);
    for (size_t i = 0; i < fixed; ++i) {
      auto argument = admit(args[i], instantiated.parameters[i], caller, at);
      if (args[i].region_id) region_pointer(args[i], at);
      declare_local(locals, form.parameters[i].name, std::move(argument), lexical, form.parameters[i].at);
    }
    if (variadic(form)) {
      std::vector<Value> tail;
      for (size_t i = fixed; i < args.size(); ++i) {
        if (args[i].region_id) region_pointer(args[i], at);
        tail.push_back(admit(args[i], instantiated.parameters[i], caller, at));
      }
      Value pack; pack.type = Type::arguments;
      pack.type.target = std::make_shared<Type>(resolve(form.parameters.back().type, lexical, form.at));
      pack.arguments = std::make_shared<const std::vector<Value>>(std::move(tail));
      declare_local(locals, form.parameters.back().name, std::move(pack), lexical, form.parameters.back().at);
    }
    auto result = block(*form.body, lexical, std::move(env), std::move(locals), function_context);
    active_calls_.erase(&form);
    if (instantiated.result) result = admit(std::move(result), *instantiated.result, lexical, form.at);
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
    std::optional<Type> target = {};
  };
  struct MethodMatch {
    Method function;
    std::vector<Route> routes;
    std::vector<CallableDeclaration> declarations;
  };
  Conversion conversion(Method function) {
    auto &form = *function.form;
    if (form.parameters.size() != 1 || !form.result_type || form.parameters[0].automatic_conversion)
      fail(form.at, "a conversion function requires one exact UoM input and an explicit UoM result");
    auto source = resolve(form.parameters[0].type, *function.lexical, form.at);
    auto target = resolve(*form.result_type, *function.lexical, form.at);
    if (source.kind != Type::quantity || target.kind != Type::quantity)
      fail(form.at, "a conversion function must connect UoM types");
    return {function, source, target};
  }
  std::vector<Conversion> conversion_graph(const Module &module) {
    std::vector<Conversion> result;
    for (const auto &function : module.world.conversions) result.push_back(conversion(function));
    return result;
  }
  static bool conversion_applicable(const Conversion &edge, const std::vector<Conversion> &graph, Type actual) {
    if (!widens(actual, edge.source)) return false;
    // Overloads of one conversion retain argument-directed specificity. Distinct
    // conversion functions remain alternative routes, including equal inputs.
    return !std::any_of(graph.begin(), graph.end(), [&](const Conversion &other) {
      return other.function.lexical == edge.function.lexical &&
             other.function.form->name == edge.function.form->name &&
             other.function.implementation == edge.function.implementation &&
             widens(actual, other.source) && widens(other.source, edge.source) &&
             !widens(edge.source, other.source);
    });
  }
  static std::optional<std::vector<Method>> shortest_route(
      const std::vector<Conversion> &graph, Type source, Type target,
      const std::vector<size_t> &itinerary = {}, const std::optional<Method> &excluded = {}) {
    struct Step { Type type; std::vector<Method> path; };
    std::queue<Step> pending;
    std::vector<Type> visited{source};
    pending.push({source, {}});
    while (!pending.empty()) {
      auto step = std::move(pending.front());
      pending.pop();
      for (const auto &edge : graph) {
        if (!conversion_applicable(edge, graph, step.type) || (excluded && edge.function == *excluded)) continue;
        const auto depth = step.path.size() + 1;
        if (!itinerary.empty() && (depth >= itinerary.size() || edge.target.identity != itinerary[depth])) continue;
        auto path = step.path;
        path.push_back(edge.function);
        if (widens(edge.target, target) && (itinerary.empty() || depth + 1 == itinerary.size())) return path;
        if (std::find(visited.begin(), visited.end(), edge.target) != visited.end()) continue;
        visited.push_back(edge.target);
        pending.push({edge.target, std::move(path)});
      }
    }
    return {};
  }
  std::optional<Route> route(Type actual, const Parameter &parameter, const Module &lexical,
                              const Module &caller, const Location &at, bool diagnose_unavailable = true) {
    const auto target = resolve(parameter.type, lexical, parameter.at);
    if (parameter.automatic_conversion && target.kind != Type::quantity)
      fail(parameter.at, "a starred parameter requires a UoM quantity type");
    if (widens(actual, target)) return Route{{}, {}, target};
    if (!parameter.automatic_conversion || actual.kind != Type::quantity) return {};
    const auto graph = conversion_graph(caller);
    Route result{{}, {}, target};
    for (const auto &edge : graph) {
      if (!conversion_applicable(edge, graph, actual) || !widens(edge.target, target)) continue;
      if (result.steps.empty()) result.steps = {edge.function};
      else if (result.alternative.empty()) result.alternative = {edge.function};
    }
    if (!result.steps.empty()) return result;
    std::vector<size_t> itinerary;
    if (!parameter.conversion_route.empty()) {
      std::vector<size_t> declared{target.identity};
      std::set<size_t> visited{target.identity};
      for (const auto &name : parameter.conversion_route) {
        const auto unit = resolve_name({Type::named, name}, lexical, parameter.at);
        if (unit.kind != Type::unit || !visited.insert(unit.identity).second)
          fail(parameter.at, "declared conversion route requires distinct UoM identities");
        declared.push_back(unit.identity);
      }
      const auto start = std::find(declared.begin() + 1, declared.end(), actual.identity);
      if (start == declared.end()) return {};
      for (auto i = start;; --i) {
        itinerary.push_back(*i);
        if (i == declared.begin()) break;
      }
    }
    const auto chosen = shortest_route(graph, actual, target, itinerary);
    if (!chosen) {
      if (!itinerary.empty() && diagnose_unavailable)
        fail(at, "declared conversion route has an unavailable typed step from " +
                     type_name(actual) + " to " + type_name(target));
      return {};
    }
    result.steps = *chosen;
    // A distinct simple route omits an edge from the selected route. Numeric
    // widening is compatibility, not extra graph edges or ambiguous paths.
    for (const auto &edge : result.steps) {
      if (auto alternative = shortest_route(graph, actual, target, itinerary, edge)) {
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
                                         bool ambiguous, const Module &visible) {
    std::string message = (ambiguous ? "ambiguous method '" : "no matching method '") + name +
                          "' for supplied argument types (";
    for (size_t i = 0; i < args.size(); ++i)
      message += (i ? ", " : "") + type_name(args[i].type);
    message += "): " + std::to_string(candidates.size()) + (ambiguous ? " matching" : " available") + " candidates";
    message += "\n" + location_text(at) + ": note: method lookup uses module '" + visible.path + "'";
    for (size_t i = 0; i < args.size(); ++i)
      for (const auto &relation : args[i].conformances)
        if (relation.data == args[i].type)
          message += "\n" + location_text(at) + ": note: argument " + std::to_string(i + 1) +
                     " guarantees " + type_name(relation.interface);
    const auto shown = std::min<size_t>(candidates.size(), 4);
    for (size_t i = 0; i < shown; ++i) {
      const auto &form = *candidates[i].form;
      message += "\n" + location_text(form.at) + ": note: candidate " +
                 (form.kind == Form::Kind::procedure ? "procedure '" : "function '") + callable_name(form) + "' (";
      for (size_t parameter = 0; parameter < form.parameters.size(); ++parameter) {
        const auto &declared = form.parameters[parameter];
        message += (parameter ? ", " : "") + type_name(declared.type);
        if (declared.automatic_conversion) message += "*";
        for (const auto &step : declared.conversion_route) message += "<-" + step;
      }
      message += ")";
      if (form.result_type) message += " -> " + type_name(*form.result_type);
      if (!candidates[i].implementation.empty() && candidates[i].implementation.back() != candidates[i].lexical)
        message += "; implementation context '" + candidates[i].implementation.back()->path + "'";
    }
    if (shown < candidates.size())
      message += "\nnote: " + std::to_string(candidates.size() - shown) + " additional candidates omitted";
    fail(at, message);
  }
  std::optional<MethodMatch> method(const std::string &name, const Module &module,
                                    const std::vector<Value> &args, const Location &at) {
    const auto information = lookup_callable(module, name, at);
    const auto &candidates = information.methods;
    if (candidates.empty()) return {};
    std::vector<MethodMatch> exact, converted;
    const auto choose_specific = [&](std::vector<MethodMatch> &matches, bool widening) {
      if (matches.size() < 2) return;
      const auto more_specific = [&](const MethodMatch &left, const MethodMatch &right) {
        return signature_subset(left.function, right.function, module, at, args, widening) &&
               !signature_subset(right.function, left.function, module, at, args, widening);
      };
      for (const auto &candidate : matches) {
        if (std::all_of(matches.begin(), matches.end(), [&](const MethodMatch &other) {
              return &candidate == &other || more_specific(candidate, other);
            })) {
          auto selected = candidate;
          matches = {std::move(selected)};
          return;
        }
      }
      std::vector<Method> candidates;
      for (const auto &match : matches) candidates.push_back(match.function);
      method_failure(name, args, candidates, at, true, module);
    };
    for (const auto &candidate : candidates) {
      const auto instantiated = signature(*candidate.form, *candidate.lexical, args, module, at);
      if (!instantiated) continue;
      bool matches = true;
      for (size_t i = 0; i < args.size(); ++i) {
        const auto &parameter = parameter_at(*candidate.form, i);
        const auto type = instantiated->parameters[i];
        if (parameter.automatic_conversion && type.kind != Type::quantity)
          fail(parameter.at, "a starred parameter requires a UoM type");
        matches = matches && accepts(args[i], type, module, at);
      }
      if (matches) exact.push_back({candidate, std::vector<Route>(args.size()), information.declarations});
    }
    choose_specific(exact, false);
    if (!exact.empty()) return exact.front();
    for (const auto &candidate : candidates) {
      const auto instantiated = signature(*candidate.form, *candidate.lexical, args, module, at);
      if (!instantiated) continue;
      const auto context = llvm::SaveAndRestore(type_context_, TypeContext{candidate.lexical, instantiated->bindings});
      MethodMatch match{candidate, {}, information.declarations};
      bool matches = true;
      for (size_t i = 0; i < args.size(); ++i) {
        const auto &parameter = parameter_at(*candidate.form, i);
        if (accepts(args[i], instantiated->parameters[i], module, at)) {
          match.routes.push_back({});
          continue;
        }
        auto path = route(args[i].type, parameter, *candidate.lexical, module, at, false);
        if (!path) { matches = false; break; }
        match.routes.push_back(std::move(*path));
      }
      if (matches) converted.push_back(std::move(match));
    }
    choose_specific(converted, true);
    if (converted.empty()) {
      // A missing edge makes an overload inapplicable. Preserve the detailed
      // diagnostic for an unambiguous declaration after considering alternatives.
      if (candidates.size() == 1) {
        const auto &candidate = candidates.front();
        const auto instantiated = signature(*candidate.form, *candidate.lexical, args, module, at);
        if (instantiated) {
          const auto context = llvm::SaveAndRestore(type_context_, TypeContext{candidate.lexical, instantiated->bindings});
          for (size_t i = 0; i < args.size(); ++i)
            route(args[i].type, parameter_at(*candidate.form, i), *candidate.lexical, module, at);
        }
      }
      return {};
    }
    return converted.front();
  }
  Value converted(Value value, const Route &route, const Location &at, const Module &caller,
                  const LLVMEmission::ConversionPoint *point = nullptr) {
    if (route.steps.empty() && !route.target) return value;
    if (!route.alternative.empty()) {
      auto describe = [&](const std::vector<Method> &path) {
        std::string result = type_name(value.type);
        for (const auto &step : path)
          result += " --" + step.form->name + "--> " + type_name(conversion(step).target);
        return result;
      };
      const auto message = "ambiguous unit conversion; selected " + describe(route.steps) +
                           "; alternative " + describe(route.alternative);
      warn(Warning::unit_conversion, at, message);
    }
    // Dispatch needs every argument's type, but a selected conversion executes
    // immediately after its own argument, before any later argument effects.
    const auto resume = point ? std::optional(ir_.begin_conversion(*point)) : std::nullopt;
    for (const auto &step : route.steps) {
      value = widened(value, conversion(step).source, at);
      value = call(*step.form, *step.lexical, {}, {value}, true, at, caller, step.implementation);
    }
    if (route.target) value = widened(value, *route.target, at);
    if (resume) ir_.end_conversion(*point, *resume);
    return value;
  }
  void check_declarations(const std::vector<CallableDeclaration> &declarations, Form::Kind kind,
                           const std::vector<Value> &args, const Value &result,
                           const Module &caller, const Location &at) {
    for (const auto &declaration : declarations) {
      const auto contract = signature(*declaration.form, *declaration.lexical, args, caller, at);
      if (!contract) continue;
      bool applicable = true;
      for (size_t i = 0; i < args.size(); ++i)
        applicable = applicable && accepts(args[i], contract->parameters[i], caller, at);
      if (!applicable) continue;
      if (declaration.form->kind != kind)
        fail(at, "method callable kind violates declaration\n" + location_text(declaration.form->at) + ": note: callable signature declared here");
      if (contract->result && !accepts(result, *contract->result, caller, at))
        fail(at, "method result violates callable declaration: expected " + type_name(*contract->result) +
                     ", got " + type_name(result.type) + "\n" + location_text(declaration.form->at) + ": note: callable signature declared here");
    }
  }
  Value call_method(const MethodMatch &match, std::vector<Value> args, const Location &at,
                    const Environment &env, bool function_context, const Module &caller,
                    const std::vector<LLVMEmission::ConversionPoint> &points = {}) {
    for (size_t i = 0; i < args.size(); ++i)
      args[i] = converted(args[i], match.routes[i], at, caller, points.empty() ? nullptr : &points[i]);
    const bool function = match.function.form->kind == Form::Kind::function;
    if (!function && function_context) {
      const auto &form = *match.function.form;
      const std::string message = form.operation ? "a function cannot invoke a procedure overload"
                                                 : "a function cannot invoke a procedure";
      fail(at, message + "\n" + location_text(form.at) + ": note: selected procedure '" +
                   callable_name(form) + "' declared here");
    }
    // Lexical procedure calls use current bindings; perform uses captured values.
    auto result = call(*match.function.form, *match.function.lexical, function ? Environment{} : env, args, function, at, caller,
                       match.function.implementation);
    check_declarations(match.declarations, match.function.form->kind, args, result, caller, at);
    return result;
  }
  Type array_type(const Value &value, const Location &at) {
    const auto type = value.type.kind == Type::reference ? *value.type.target : value.type;
    if (type.kind != Type::array) fail(at, "expected a regional array");
    region_pointer(value, at);
    return type;
  }
  Value array_element(Value array, Value index, const Location &at, bool as_reference = false) {
    if (array.arguments) {
      if (as_reference) fail(at, "a variadic argument pack has no regional element address");
      require(index.type, Type::uint64, at);
      const auto &elements = *array.arguments;
      if (const auto *constant = llvm::dyn_cast<llvm::ConstantInt>(index.native)) {
        if (constant->getZExtValue() >= elements.size()) fail(at, "variadic argument index out of bounds");
        const auto result = elements[constant->getZExtValue()];
        if (result.region_id) region_pointer(result, at);
        return result;
      }
      if (elements.empty()) {
        const auto type = *array.type.target;
        if (!numeric(type) && type != Type::boolean && type.kind != Type::quantity)
          fail(at, "an empty argument pack has no regional owner to borrow");
        return scalar(type, ir_.argument_element(type, index.native, {}));
      }
      auto result = elements.front();
      if (result.procedure) fail(at, "a dynamic argument index cannot select a procedure");
      std::vector<llvm::Value *> native;
      for (const auto &element : elements) {
        require(element.type, result.type, at);
        if (element.region_id != result.region_id) fail(at, "a dynamic argument index requires one owning region");
        if (element.region_id) region_pointer(element, at);
        std::erase_if(result.conformances, [&](const Conformance &relation) {
          return !establishes(element, relation, at);
        });
        native.push_back(element.native);
      }
      result.native = ir_.argument_element(result.type, index.native, native);
      return result;
    }
    const auto type = array_type(array, at);
    const auto &layout = arrays_.at(type.identity);
    require(index.type, layout.index, at);
    const auto result_type = as_reference ? reference(layout.element) : layout.element;
    const auto place = ir_.element(region_pointer(array, at), array.native, index.type, index.native, layout.element);
    return scalar(result_type, as_reference ? ir_.reference(place) : ir_.read(place), as_reference ? array.region_id : std::nullopt);
  }
  Value invoke_procedure(const Value &value, const Expression &expr, const Module &module,
                         Environment &env, Locals &locals, bool function_context, bool captured) {
    if (function_context) fail(expr.at, "a function cannot invoke a procedure");
    if (!value.procedure) fail(expr.at, "binding '" + expr.text + "' is not callable");
    const auto proc = value.procedure; // Resolve before arguments/capture installation.
    std::vector<LLVMEmission::ConversionPoint> points;
    const bool convert_arguments = proc->form && converting_parameters(*proc->form);
    auto args = arguments(expr, module, env, locals, function_context, convert_arguments ? &points : nullptr);
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
    {
      const auto instantiated = *signature(*proc->form, *proc->lexical, args, module, expr.at, true);
      const auto types = llvm::SaveAndRestore(type_context_, TypeContext{proc->lexical, instantiated.bindings});
      for (size_t i = 0; i < args.size(); ++i) {
        if (const auto path = route(args[i].type, parameter_at(*proc->form, i), *proc->lexical, module, expr.at))
          args[i] = converted(args[i], *path, expr.at, module, points.empty() ? nullptr : &points[i]);
      }
    }
    const auto native_scope = ir_.scope();

    auto call_env = captured ? proc->captures : env;
    auto result = call(*proc->form, *proc->lexical, std::move(call_env), args, false, expr.at, module, proc->implementation);
    ir_.restore(native_scope);
    check_declarations(proc->declarations, Form::Kind::procedure, args, result, module, expr.at);
    return result;
  }
  Value invoke(const Expression &expr, const Module &module, Environment &env, Locals &locals,
               bool function_context) {
    const auto id = expression_symbol(expr, module);
    const auto *found = env.lookup(id);
    if (!found) fail(expr.at, (env.captured_context
        ? "perform requires an unavailable definition-time capture " : "missing required procedure binding ") + symbol_names_[id]);
    return invoke_procedure(*found, expr, module, env, locals, function_context,
                            expr.kind == Expression::Kind::perform);
  }
  Value allocate_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                          bool function_context) {
    auto owner = expression(*expr.children[0], module, env, locals, function_context);
    require(owner.type, Type::region, expr.at);
    region_pointer(owner, expr.at);
    auto type = resolve({Type::named, expr.text}, module, expr.at);
    if (type.kind != Type::array && type.kind != Type::regional)
      fail(expr.at, "alloc requires a regional type or regional array");
    std::vector<Value> args;
    for (size_t i = 1; i < expr.children.size(); ++i)
      args.push_back(expression(*expr.children[i], module, env, locals, function_context));

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
          fail(expr.at, "cross-region reference fields are outside this prototype" +
                           region_note(owner.region_id, "destination object's region acquired here") +
                           region_note(args[i].region_id, "referenced target's region acquired here"));

      }
    }
    std::vector<llvm::Value *> native_args;
    for (const auto &arg : args) native_args.push_back(arg.native);
    auto *native = type.kind == Type::array ?
        ir_.array_new(region_pointer(owner, expr.at), arrays_.at(type.identity).element,
                      args[0].type, args[0].native, args[1].native) :
        ir_.record_new(region_pointer(owner, expr.at), type, native_args);
    auto result = scalar(reference(type), native, owner.region_id);
    for (const auto &arg : args) merge_conformances(result.conformances, arg.conformances);
    return contextual(std::move(result), module);
  }
  Value loop_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                          bool function_context) {
    using K = Expression::Kind;
    const auto first = expression(*expr.children[0], module, env, locals, function_context);
    const auto last = expression(*expr.children[1], module, env, locals, function_context);
    require(last.type, first.type, expr.at);
    if (!integral(first.type) && !(first.type.kind == Type::quantity && integral(representation(first.type))))
      fail(expr.at, "a range requires integer endpoints or one integer-backed UoM identity");
    Value accumulator;
    if (expr.kind == K::fold) {
      accumulator = contextual(expression(*expr.children[2], module, env, locals, function_context), module);
      if (accumulator.type == Type::void_type || accumulator.type.kind == Type::procedure || accumulator.arguments ||
          accumulator.type == Type::null_ref)
        fail(expr.at, "a fold accumulator requires bits or a borrowed regional value");
      std::set<size_t> reachable;
      if (referenced_data(accumulator.type, reachable))
        std::erase_if(accumulator.conformances, [&](const Conformance &relation) {
          return !reachable.contains(relation.data.identity);
        });
    }
    auto native_loop = ir_.loop(first.type, first.native, last.native, accumulator.native);
    auto inner_locals = locals.nested();
    declare_local(inner_locals, expr.text, {first.type, native_loop.index, {}, nullptr}, module, expr.at);
    if (expr.kind == K::fold) {
      accumulator.native = native_loop.accumulator;
      declare_local(inner_locals, expr.binding, accumulator, module, expr.at);
    }

    const auto next = block(*expr.children.back(), module, env, std::move(inner_locals), function_context,
                            expr.kind == K::range_for);

    if (expr.kind == K::fold) {
      require(next.type, accumulator.type, expr.at);
      if (next.region_id != accumulator.region_id)
        fail(expr.at, "a fold reference must retain its owning region" +
                         region_note(accumulator.region_id, "initial accumulator's region acquired here") +
                         region_note(next.region_id, "iteration result's region acquired here"));
      for (const auto &relation : accumulator.conformances)
        if (!establishes(next, relation, expr.at) && !subtype(relation.data, relation.interface, module, expr.at))
          fail(expr.at, "a fold result must retain established conformance " + type_name(relation.data) +
                           " <: " + type_name(relation.interface));
    }
    ir_.end_loop(native_loop, next.native);
    return accumulator;
  }
  Value call_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                          bool function_context, bool function_result = false) {
    using K = Expression::Kind;
    if (const auto local = locals.visible.find(expr.text); local != locals.visible.end()) {
      const auto callee = local->second;
      return invoke_procedure(callee, expr, module, env, locals, function_context || function_result, false);
    }
    if (expr.children.size() == 2 && binary_precedence(expr.text) >= 0)
      return binary_expression(expr, module, env, locals, function_context, function_result);
    if (expr.children.size() == 1 && (expr.text == "-" || expr.text == "!"))
      return unary_expression(expr, module, env, locals, function_context, function_result);
    std::vector<LLVMEmission::ConversionPoint> points;
    std::optional<std::vector<Value>> evaluated;
    const auto values = [&]() -> const std::vector<Value> & {
      if (!evaluated) evaluated = arguments(expr, module, env, locals, function_context,
          converting_method(expr.text, module, expr.at) ? &points : nullptr);
      return *evaluated;
    };
    if (intrinsic_name(expr.text) && expr.text != "setfield") {
      const auto &args = values();
      if (const auto selected = method(expr.text, module, args, expr.at))
        return call_method(*selected, args, expr.at, env, function_context || function_result, module, points);
    }
    if (expr.text == "region") {
      const auto &args = values();
      if (args.size() != 1 || !integral(args[0].type)) fail(expr.at, "region requires an integer byte capacity");
      regional_runtime_ = true;
      if (!region_initializer_) fail(expr.at, "a new region must be acquired by an effect binding");
      auto *owner = ir_.region_new(args[0].type, args[0].native);
      const auto id = regions_.size();
      regions_.push_back({owner, expr.at});

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
      auto *native = ir_.read(ir_.referent(region_pointer(args[0], expr.at), args[0].native, type));
      if (type.kind == Type::array || type.kind == Type::regional) {
        auto result = scalar(type, native, args[0].region_id);
        result.conformances = args[0].conformances;
        return contextual(std::move(result), module);
      }
      return scalar(type, native);
    }
    if (expr.text == "len" || expr.text == "ref_at") {
      const auto &args = values();
      if (args.size() != (expr.text == "len" ? 1u : 2u)) fail(expr.at, "wrong array argument count");
      if (expr.text == "ref_at") return array_element(args[0], args[1], expr.at, true);
      if (args[0].arguments) return scalar(Type::uint64, ir_.bits(Type::uint64, args[0].arguments->size()));
      array_type(args[0], expr.at);
      return scalar(Type::uint64, ir_.array_length(region_pointer(args[0], expr.at), args[0].native));
    }
    if (expr.text == "store") {
      const auto &args = values();
      if (args.size() == 2 && args[0].type.kind == Type::reference &&
          (numeric(*args[0].type.target) || *args[0].type.target == Type::boolean || args[0].type.target->kind == Type::quantity)) {
        require(args[1].type, *args[0].type.target, expr.at);

        ir_.write(ir_.referent(region_pointer(args[0], expr.at), args[0].native, *args[0].type.target), args[1].native);
        return args[1];
      }
      if (args.size() != 3) fail(expr.at, "store requires an array/index/value or scalar reference/value");
      const auto type = array_type(args[0], expr.at);
      const auto &layout = arrays_.at(type.identity);
      require(args[1].type, layout.index, expr.at);
      require(args[2].type, layout.element, expr.at);
      ir_.write(ir_.element(region_pointer(args[0], expr.at), args[0].native, args[1].type, args[1].native, layout.element), args[2].native);
      return args[2];
    }
    if (expr.text == "setfield") {
      if (expr.children.size() != 3 || expr.children[1]->kind != K::name)
        fail(expr.at, "setfield requires a reference, field name and value");
      const auto object = expression(*expr.children[0], module, env, locals, function_context);
      const auto target = object.type.kind == Type::reference ? *object.type.target : object.type;
      if (target.kind != Type::regional)
        fail(expr.at, "setfield requires a regional record view or reference");
      const auto type = resolve(target, module, expr.at);
      auto value = expression(*expr.children[2], module, env, locals, function_context);
      const auto &fields = products_.at(type.identity).fields;
      for (size_t i = 0; i < fields.size(); ++i) {
        if (fields[i].name != expr.children[1]->text) continue;
        const auto field = fields[i].type;
        if (value.type == Type::null_ref && field.kind == Type::reference) value.type = field;
        require(value.type, field, expr.at);
        if (value.region_id && value.region_id != object.region_id)
          fail(expr.at, "cross-region reference fields are outside this prototype" +
                           region_note(object.region_id, "destination object's region acquired here") +
                           region_note(value.region_id, "referenced target's region acquired here"));
        region_pointer(object, expr.at);

        if (field.kind == Type::reference) {
          value.region_id = object.region_id;
          merge_conformances(value.conformances, object.conformances);
          value = contextual(std::move(value), module);
        }

        ir_.write(ir_.field(region_pointer(object, expr.at), object.native, type, i, field), value.native);
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
    if (module.world.types.contains(expr.text) ||
        (type_context_.lexical == &module && type_context_.bindings.contains(expr.text))) {
      const auto type = resolve_name({Type::named, expr.text}, module, expr.at);
      if (type.kind == Type::abstract_type) fail(expr.at, "an abstract type has no constructor or storage layout");
      if (type.kind == Type::regional || type.kind == Type::array)
        fail(expr.at, "regional objects require alloc in a region resource");
      const auto &args = values();
      if (numeric(type)) {
        if (args.size() != 1 || !numeric(args[0].type)) fail(expr.at, "numeric conversion requires one scalar value");
        return scalar(type, ir_.cast(args[0].type, type, args[0].native));
      }
      if (type.kind == Type::unit) {
        if (args.size() != 1) fail(expr.at, "a UoM constructor requires one scalar value");
        if (!numeric(args[0].type)) fail(expr.at, "a UoM constructor requires one scalar value");
        return scalar(quantity(type, args[0].type), args[0].native);
      }
      fail(expr.at, "this type has no value constructor");
    }
    const auto information = lookup_callable(module, expr.text, expr.at);
    const auto &functions = information.methods;
    if (expr.text == "scalar") {
      const auto &args = values();
      if (args.size() != 1 || args[0].type.kind != Type::quantity)
        fail(expr.at, "scalar requires one UoM value");
      return {representation(args[0].type), args[0].native, {}, nullptr};
    }
    if (functions.empty()) {
      if (!information.identities.empty())
        fail(expr.at, "callable '" + expr.text + "' has no method implementation" +
                     (information.declarations.empty() ? std::string{} :
                      "\n" + location_text(information.declarations.front().form->at) + ": note: callable signature declared here"));
      if (expr.text == "=" || expr.text == "[]=")
        fail(expr.at, "unavailable operation overload '(" + expr.text + ")'");
      fail(expr.at, "unavailable lexical function '" + expr.text + "'");
    }
    const auto &args = values();
    auto selected = method(expr.text, module, args, expr.at);
    if (!selected && functions.size() == 1) {
      const auto &candidate = functions.front();
      return call_method({candidate, std::vector<Route>(args.size()), information.declarations}, args, expr.at, env, function_context || function_result, module, points);
    }
    if (!selected) method_failure(expr.text, args, functions, expr.at, false, module);
    return call_method(*selected, args, expr.at, env, function_context || function_result, module, points);
  }
  Value unary_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                         bool function_context, bool function_result = false) {
    using K = Expression::Kind;
    if (expr.text == "-" && expr.children[0]->kind == K::integer) {
      std::string_view magnitude = expr.children[0]->text;
      while (magnitude.starts_with('0')) magnitude.remove_prefix(1);
      if (magnitude == "9223372036854775808")
        return scalar(Type::integer, ir_.bits(Type::integer, UINT64_C(1) << 63));
    }
    auto value = expression(*expr.children[0], module, env, locals, function_context);
    if (const auto selected = method(expr.text, module, {value}, expr.at))
      return call_method(*selected, {value}, expr.at, env, function_context || function_result, module);
    if (expr.text == "-" && floating(value.type))
      return scalar(value.type, ir_.unary(value.type, expr.text, value.native));
    require(value.type, expr.text == "!" ? Type::boolean : Type::integer, expr.at);
    return scalar(value.type, ir_.unary(value.type, expr.text, value.native));
  }
  Value binary_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                          bool function_context, bool function_result = false) {
    auto left = expression(*expr.children[0], module, env, locals, function_context);
    if (expr.text == "&&" || expr.text == "||") {
      require(left.type, Type::boolean, expr.at);
      auto right_env = env;
      auto right_locals = locals.nested();
      const auto rebound = llvm::SaveAndRestore(scope_rebound_, false);

      const auto native_scope = ir_.scope();
      auto native_branch = ir_.branch(left.native, expr.text == "||");
      const auto right = expression(*expr.children[1], module, right_env, right_locals, function_context);
      require(right.type, Type::boolean, expr.at);
      if (scope_rebound_ || ir_.scope() != native_scope)
        fail(expr.at, "conditional rebinding in a logical operand needs an explicit scope");
      ir_.otherwise(native_branch, right.native);
      return scalar(Type::boolean, ir_.merge(native_branch, Type::boolean, left.native));
    }
    std::vector<LLVMEmission::ConversionPoint> points;
    if (converting_method(expr.text, module, expr.at)) points.push_back(ir_.conversion_point());
    auto right = expression(*expr.children[1], module, env, locals, function_context);
    if (!points.empty()) points.push_back(ir_.conversion_point());
    if (const auto selected = method(expr.text, module, {left, right}, expr.at))
      return call_method(*selected, {left, right}, expr.at, env, function_context || function_result, module, points);
    if ((expr.text == "==" || expr.text == "!=") &&
        (left.type.kind == Type::reference || left.type == Type::null_ref) &&
        (right.type.kind == Type::reference || right.type == Type::null_ref)) {
      if (left.type != Type::null_ref && right.type != Type::null_ref) require(right.type, left.type, expr.at);
      return scalar(Type::boolean, ir_.reference_equal(expr.text, left.native, right.native,
                      left.region_id && right.region_id && left.region_id != right.region_id));
    }
    if (left.type != right.type && numeric(left.type) && numeric(right.type)) {
      std::vector<Type> common;
      for (auto candidate : {Type::uint8, Type::uint32, Type::uint64, Type::integer, Type::float32, Type::float64})
        if (widens(left.type, candidate) && widens(right.type, candidate)) common.push_back(candidate);
      const auto least = std::find_if(common.begin(), common.end(), [&](Type candidate) {
        return std::all_of(common.begin(), common.end(), [&](Type other) { return widens(candidate, other); });
      });
      if (least == common.end())
        fail(expr.at, "numeric operands have no unique lossless common representation; use explicit conversion");
      left = widened(left, *least, expr.at);
      right = widened(right, *least, expr.at);
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
  Value conditional_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                          bool function_context) {
    const auto condition = expression(*expr.children[0], module, env, locals, function_context);
    require(condition.type, Type::boolean, expr.at);

    auto native_branch = ir_.branch(condition.native);
    auto first = block(*expr.children[1], module, env, locals.nested(), function_context);
    ir_.otherwise(native_branch, first.native);
    auto second = block(*expr.children[2], module, env, locals.nested(), function_context);
    if (first.type == Type::null_ref && second.type.kind == Type::reference) {
      first.type = second.type;
      first.region_id = second.region_id;
      first.conformances = second.conformances;
    } else if (second.type == Type::null_ref && first.type.kind == Type::reference) {
      second.type = first.type;
      second.region_id = first.region_id;
      second.conformances = first.conformances;
    }
    require(second.type, first.type, expr.at);
    if (first.region_id != second.region_id)
      fail(expr.at, "conditional references require one owning region" +
                       region_note(first.region_id, "first branch result's region acquired here") +
                       region_note(second.region_id, "second branch result's region acquired here"));
    auto *native = ir_.merge(native_branch, first.type, second.native);
    if (first.type == Type::void_type || first.type == Type::region || first.type == Type::null_ref)
      return first;
    auto result = scalar(first.type, native, first.region_id);
    first = contextual(std::move(first), module);
    second = contextual(std::move(second), module);
    for (const auto &relation : first.conformances) {
      auto common = std::optional(relation.interface);
      while (common && !establishes(second, {relation.data, *common}, expr.at)) {
        resolve_abstract(*common, expr.at);
        common = abstracts_.at(common->identity).parent;
      }
      if (common) append_conformance(result.conformances, {relation.data, *common});
    }
    return result;
  }
  Value expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                   bool function_context) {
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
      auto value = expression(*expr.children[0], module, env, locals, function_context);
      if (value.type == Type::void_type) fail(expr.at, "a lexical value cannot be void");
      declare_local(locals, expr.text, value, module, expr.at);
      return value;
    }
    case K::field: {
      const auto value = expression(*expr.children[0], module, env, locals, function_context);
      if (value.type.kind != Type::regional)
        fail(expr.at, "field access requires a product value; dereference regional pointers first");
      const auto &fields = products_.at(value.type.identity).fields;
      for (size_t i = 0; i < fields.size(); ++i) {
        if (fields[i].name != expr.text) continue;
        if (fields[i].type.kind == Type::reference) {
          auto result = scalar(fields[i].type, ir_.read(ir_.field(region_pointer(value, expr.at), value.native, value.type, i, fields[i].type)), value.region_id);
          result.conformances = value.conformances;
          return contextual(std::move(result), module);
        }
        return scalar(fields[i].type, ir_.read(ir_.field(region_pointer(value, expr.at), value.native, value.type, i, fields[i].type)));
      }
      fail(expr.at, "no field '" + expr.text + "' in " + type_name(value.type));
    }
    case K::index: {
      auto array = expression(*expr.children[0], module, env, locals, function_context);
      auto index = expression(*expr.children[1], module, env, locals, function_context);
      return array_element(array, index, expr.at);
    }
    case K::name: {
      if (const auto local = locals.visible.find(expr.text); local != locals.visible.end()) return local->second;
      if (const auto global = module.world.global_aliases.find(expr.text); global != module.world.global_aliases.end())
        return module.world.global_values.at(global->second);
      if (const auto information = lookup_callable(module, expr.text, expr.at); !information.methods.empty()) {
        const auto &methods = information.methods;
        if (methods.size() != 1) fail(expr.at, "a procedure value requires an unambiguous lexical declaration");
        const auto &method = methods.front();
        if (method.form->kind == Form::Kind::procedure && !method.form->operation) {
          const auto value = method.lexical->world.procedures.find(method.form);
          if (value == method.lexical->world.procedures.end()) fail(expr.at, "lexical procedure used before its definition");
          auto selected = value->second;
          auto procedure = std::make_shared<Procedure>(*selected.procedure);
          procedure->implementation = method.implementation;
          procedure->declarations = information.declarations;
          selected.procedure = std::move(procedure);
          return selected;
        }
      }
      fail(expr.at, "unavailable lexical value '" + expr.text + "'");
    }
    case K::symbol:
      if (function_context) fail(expr.at, "a function cannot read symbolic effects");
      return read_symbol(expression_symbol(expr, module), env, expr.at);
    case K::effect: {
      if (function_context) fail(expr.at, "a function cannot establish effects");
      const auto region_initializer = llvm::SaveAndRestore(region_initializer_,
          expr.children[0]->kind == K::call && expr.children[0]->text == "region");
      auto value = expression(*expr.children[0], module, env, locals, function_context);
      value = bind(expression_symbol(expr, module), value, module, env, expr.at);
      scope_rebound_ = true;
      return value;
    }
    case K::allocate: return allocate_expression(expr, module, env, locals, function_context);
    case K::range_for: case K::fold: return loop_expression(expr, module, env, locals, function_context);
    case K::block: return block(expr, module, env, locals.nested(), function_context);
    case K::call: return call_expression(expr, module, env, locals, function_context);
    case K::perform: case K::behave: return invoke(expr, module, env, locals, function_context);
    case K::unary: return unary_expression(expr, module, env, locals, function_context);
    case K::binary: return binary_expression(expr, module, env, locals, function_context);
    case K::conditional: return conditional_expression(expr, module, env, locals, function_context);
    }
    fail(expr.at, "unsupported expression");
  }
  static void append_method(std::vector<Method> &methods, Method method) {
    if (std::find(methods.begin(), methods.end(), method) == methods.end()) methods.push_back(method);
  }
  static void append_declaration(std::vector<CallableDeclaration> &declarations, CallableDeclaration declaration) {
    if (std::find(declarations.begin(), declarations.end(), declaration) == declarations.end())
      declarations.push_back(declaration);
  }
  static bool operation_name(const std::string &name) {
    return intrinsic_name(name) || binary_precedence(name) >= 0 || name == "!" || name == "=" || name == "[]=";
  }
  static CallableIdentity local_callable(const Module &module, const std::string &name) {
    return {operation_name(name) ? nullptr : &module, name};
  }
  static void declare_callable(Module &module, Form &form) {
    const auto identity = module.world.local_callables.emplace(form.name, local_callable(module, form.name)).first->second;
    auto &family = module.world.callables[identity];
    if (form.body) append_method(family.methods, {&form, &module});
    else append_declaration(family.declarations, {&form, &module});
    family.origins.insert({identity.module, form.name});
    module.world.callable_aliases[form.name] = identity;
  }
  static CallableInformation callable_information(const Module &module, const CallableIdentity &identity) {
    const auto &family = module.world.callables.at(identity);
    CallableInformation result{family.methods, family.origins, false, {identity}, family.declarations};
    for (const auto &member : family.members) {
      const auto contribution = callable_information(module, member);
      result.origins.insert(contribution.origins.begin(), contribution.origins.end());
      for (const auto &method : contribution.methods) append_method(result.methods, method);
      for (const auto &declaration : contribution.declarations) append_declaration(result.declarations, declaration);
    }
    return result;
  }
  // Integrating information changes only the receiving world's identity record.
  static void merge_callable(Module &module, const CallableIdentity &identity, const CallableInformation &information) {
    auto &family = module.world.callables[identity];
    family.origins.insert(information.origins.begin(), information.origins.end());
    for (const auto &method : information.methods) append_method(family.methods, method);
    for (const auto &declaration : information.declarations) append_declaration(family.declarations, declaration);
  }
  static void extend_imported_callable(Module &module, const std::string &alias, const CallableIdentity &identity) {
    const auto local = module.world.local_callables.find(alias);
    if (local == module.world.local_callables.end() || local->second == identity) return;
    const auto declaration = local->second;
    const auto &family = module.world.callables.at(declaration);
    // Scanning registers local overloads before their import is encountered.
    // Attach that declaration group to the selected identity, without merging
    // independently selected families or extending a different source world.
    if (!family.members.empty() || family.origins != std::set<MethodOrigin>{{&module, alias}}) return;
    merge_callable(module, identity, callable_information(module, declaration));
    for (auto *aliases : {&module.world.callable_aliases, &module.world.associated_aliases,
                         &module.world.local_callables})
      for (auto &[name, selected] : *aliases)
        if (selected == declaration) selected = identity;
    for (auto &[source, selected] : module.world.imported_callables)
      if (selected == declaration) selected = identity;
    for (auto &[name, published] : module.world.exports)
      if (auto *selected = std::get_if<CallableIdentity>(&published); selected && *selected == declaration)
        *selected = identity;
    for (auto &[owner, information] : module.world.callables)
      if (information.members.erase(declaration) && owner != identity) information.members.insert(identity);
    module.world.callables.erase(declaration);
  }
  static CallableIdentity combine_alias(Module &module, const std::string &name,
                                       const CallableIdentity &left, const CallableIdentity &right) {
    if (left == right) return left;
    const auto identity = local_callable(module, name);
    auto &family = module.world.callables[identity];
    if (left != identity) family.members.insert(left);
    if (right != identity) family.members.insert(right);
    return identity;
  }
  static std::set<std::string> method_names(const Module &module) {
    std::set<std::string> names;
    for (const auto &[name, identity] : module.world.callable_aliases) names.insert(name);
    for (const auto &[name, identity] : module.world.associated_aliases) names.insert(name);
    return names;
  }
  static CallableInformation lexical_callable(const Module &module, const std::string &name) {
    if (const auto found = module.world.callable_aliases.find(name); found != module.world.callable_aliases.end())
      return callable_information(module, found->second);
    if (module.world.lexical_names.contains(name)) return {};
    if (const auto found = module.world.associated_aliases.find(name); found != module.world.associated_aliases.end()) {
      auto result = callable_information(module, found->second);
      result.implicit = true;
      return result;
    }
    return {};
  }
  static void check_implicit_identity(const CallableInformation &set, const std::string &name, const Location &at) {
    if (!set.implicit || set.origins.size() < 2) return;
    std::string message = "ambiguous implicit callable '" + name +
                          "'; select its family with explicit using/import as";
    size_t shown = 0;
    for (const auto &[source, original] : set.origins) {
      if (shown++ == 4) break;
      if (!source) {
        message += "\n" + location_text(at) + ": note: compiler operation family '(" + original + ")'";
        continue;
      }
      const auto declaration = source->world.callables.find({source, original});
      const auto location = declaration != source->world.callables.end() && !declaration->second.methods.empty()
          ? location_text(declaration->second.methods.front().form->at) : source->path;
      message += "\n" + location + ": note: original callable family '" + original + "'";
    }
    if (set.origins.size() > 4)
      message += "\nnote: " + std::to_string(set.origins.size() - 4) + " additional callable families omitted";
    fail(at, message);
  }
  static bool related(const std::set<MethodOrigin> &left, const std::set<MethodOrigin> &right) {
    return std::any_of(left.begin(), left.end(), [&](const MethodOrigin &origin) { return right.contains(origin); });
  }
  static void select_type(Module &module, Type type, const std::string &alias, const Location &at) {
    require_user_type_name(alias, at);
    if (module.world.callable_aliases.contains(alias) || module.world.global_aliases.contains(alias))
      fail(at, "duplicate lexical alias '" + alias + "'");
    const auto [found, inserted] = module.world.types.emplace(alias, type);
    if (!inserted && found->second != type)
      fail(at, "duplicate lexical alias '" + alias + "'");
  }
  static CallableIdentity select_callable(Module &module, CallableInformation information, const std::string &alias,
                                          bool importing, bool implicit, const Location &at) {
    if (information.identities.size() != 1) fail(at, "a callable selection requires one identity");
    if (!implicit) {
      if (alias == "setfield")
        fail(at, "setfield overloads need a compile-time field-selector contract; outside this prototype");
      if (!intrinsic_name(alias)) require_user_type_name(alias, at);
      if (module.world.types.contains(alias) || module.world.global_aliases.contains(alias))
        fail(at, "duplicate lexical alias '" + alias + "'");
      for (const auto &method : information.methods)
        if (method.form->kind == Form::Kind::procedure && method.form->operation && !operation_name(alias))
          fail(at, "a procedure overload requires an operator or intrinsic destination");
      module.world.explicit_method_imports[alias] = module.world.explicit_method_imports[alias] || importing;
    }
    const auto source_identity = *information.identities.begin();
    auto identity = source_identity;
    if (operation_name(alias)) {
      identity = local_callable(module, alias);
    } else if (importing) {
      // Import adapts one source identity to this module's implementation
      // context. Another alias selects that same adaptation, not a new one.
      const auto proposed = CallableIdentity{&module, module.world.imported_callables.size()};
      identity = module.world.imported_callables.emplace(source_identity, proposed).first->second;
      extend_imported_callable(module, alias, identity);
    }
    auto &family = module.world.callables[identity];
    if (identity != source_identity && module.world.local_callables.contains(alias))
      family.origins.erase({&module, alias});
    merge_callable(module, identity, information);
    auto &aliases = implicit ? module.world.associated_aliases : module.world.callable_aliases;
    const auto [found, inserted] = aliases.emplace(alias, identity);
    if (!inserted) found->second = combine_alias(module, alias, found->second, identity);
    return found->second;
  }
  static bool context_dependent(const Expression &expr, const Module &lexical,
                                std::set<std::string> &locals) {
    if (expr.kind == Expression::Kind::call && !locals.contains(expr.text) && !operation_name(expr.text) &&
        !lexical.world.types.contains(expr.text)) return true;
    if (expr.kind == Expression::Kind::name && !locals.contains(expr.text) &&
        !lexical_callable(lexical, expr.text).methods.empty()) return true;
    if (expr.kind == Expression::Kind::block) {
      auto nested = locals;
      for (const auto &child : expr.children)
        if (context_dependent(*child, lexical, nested)) return true;
      return false;
    }
    const bool loop = expr.kind == Expression::Kind::range_for || expr.kind == Expression::Kind::fold;
    const auto count = expr.children.size() - (loop ? 1 : 0);
    for (size_t i = 0; i < count; ++i) {
      // The field selector is a compile-time label, not a lexical value lookup.
      if (expr.kind == Expression::Kind::call && expr.text == "setfield" && i == 1) continue;
      if (context_dependent(*expr.children[i], lexical, locals)) return true;
    }
    if (loop) {
      auto nested = locals;
      nested.insert(expr.text);
      if (expr.kind == Expression::Kind::fold) nested.insert(expr.binding);
      return context_dependent(*expr.children.back(), lexical, nested);
    }
    if (expr.kind == Expression::Kind::let) locals.insert(expr.text);
    return false;
  }
  static bool context_dependent(const Method &method) {
    std::set<std::string> locals;
    for (const auto &parameter : method.form->parameters) locals.insert(parameter.name);
    for (const auto &parameter : method.form->type_parameters) locals.insert(parameter.name);
    return context_dependent(*method.form->body, *method.lexical, locals);
  }
  CallableInformation lookup_callable(const Module &module, const std::string &name, const Location &at) {
    auto source = lexical_callable(module, name);
    check_implicit_identity(source, name, at);
    for (auto &method : source.methods)
      if (method.implementation.empty()) method.implementation = {&module};
    std::map<CallableIdentity, CallableInformation> interpretations;
    for (const auto &identity : source.identities) interpretations.emplace(identity, source);
    const auto merge_information = [](CallableInformation &target, const CallableInformation &information) {
      target.origins.insert(information.origins.begin(), information.origins.end());
      target.identities.insert(information.identities.begin(), information.identities.end());
      for (const auto &method : information.methods) append_method(target.methods, method);
      for (const auto &declaration : information.declarations) append_declaration(target.declarations, declaration);
    };
    const Module *previous_world = &module;
    if (!operation_name(name)) for (const auto *world : implementation_context_) {
      if (world == &module) { previous_world = world; continue; }
      std::set<MethodOrigin> origins;
      for (const auto &[identity, information] : interpretations)
        origins.insert(information.origins.begin(), information.origins.end());
      std::map<CallableIdentity, CallableInformation> visible;
      for (const auto &alias : method_names(*world)) {
        if (operation_name(alias)) continue;
        auto candidate = lexical_callable(*world, alias);
        if (!related(origins, candidate.origins)) continue;
        for (const auto &identity : candidate.identities) {
          const auto [found, inserted] = visible.emplace(identity, candidate);
          if (!inserted && !candidate.implicit) found->second.implicit = false;
        }
      }
      std::map<CallableIdentity, CallableInformation> next;
      std::set<CallableIdentity> incorporated;
      for (auto &[identity, inherited] : interpretations) {
        auto selected_identity = identity;
        if (const auto imported = world->world.imported_callables.find(identity);
            imported != world->world.imported_callables.end() && visible.contains(imported->second))
          selected_identity = imported->second;
        const auto selected = visible.find(selected_identity);
        if (selected != visible.end()) {
          // Only the same identity or its explicit receiver import may replace
          // an inherited declaration's interpretation. A sibling sharing its
          // source body remains a distinct candidate, including nested helpers.
          check_implicit_identity(selected->second, name, at);
          std::erase_if(inherited.methods, [&](const Method &method) {
            return std::any_of(selected->second.methods.begin(), selected->second.methods.end(),
                [&](const Method &replacement) {
                  return method.form == replacement.form && method.lexical == replacement.lexical;
                });
          });
        }
        for (auto &method : inherited.methods)
          if (method.implementation.back() == previous_world && previous_world != world)
            method.implementation.push_back(world);
        inherited.identities = {selected_identity};
        merge_information(next[selected_identity], inherited);
        if (selected != visible.end()) {
          auto information = selected->second;
          for (auto &method : information.methods)
            if (method.implementation.empty()) method.implementation = {world};
          merge_information(next[selected_identity], information);
          incorporated.insert(selected_identity);
        }
      }
      for (auto &[identity, information] : visible) {
        if (incorporated.contains(identity)) continue;
        check_implicit_identity(information, name, at);
        for (auto &method : information.methods)
          if (method.implementation.empty()) method.implementation = {world};
        merge_information(next[identity], information);
      }
      interpretations = std::move(next);
      previous_world = world;
    }
    CallableInformation result;
    for (const auto &[identity, information] : interpretations) {
      for (const auto &declaration : information.declarations) append_declaration(result.declarations, declaration);
      result.identities.insert(identity);
      result.origins.insert(information.origins.begin(), information.origins.end());
      for (auto method : information.methods) {
        if (method.form->operation || !context_dependent(method)) method.implementation = {method.lexical};
        append_method(result.methods, std::move(method));
      }
    }
    return result;
  }
  static Method selected_method(Method method, const Module &source, const Module &receiver, bool importing) {
    if (!method.form->operation) {
      if (method.implementation.empty()) method.implementation = {&source};
      if (importing && method.implementation.back() != &receiver) method.implementation.push_back(&receiver);
    }
    return method;
  }
  bool associated_type(Type signature, Type selected, const Module &lexical, const Module &source,
                       const Location &at, bool applicable) {
    if (signature.kind == Type::named) {
      const auto found = lexical.world.types.find(signature.name);
      if (found == lexical.world.types.end()) return false; // Unrelated unavailable declarations stay demand-checked.
      signature = found->second;
    }
    if (signature == selected || (applicable && signature.kind == Type::abstract_type &&
                                 subtype(selected, signature, source, at))) return true;
    if (signature.target && associated_type(*signature.target, selected, lexical, source, at, false)) return true;
    return std::any_of(signature.parameters.begin(), signature.parameters.end(), [&](const Type &parameter) {
      return associated_type(parameter, selected, lexical, source, at, false);
    });
  }
  bool associated_signature(const Form &form, const Module &lexical, const Module &source, Type selected) {
    const auto associated = [&](auto &&self, const Type &type, bool applicable) -> bool {
      if (type.kind == Type::named) {
        for (const auto &parameter : form.type_parameters)
          if (parameter.name == type.name)
            return parameter.bound && associated_type(*parameter.bound, selected, lexical, source, parameter.at, applicable);
      }
      if (associated_type(type, selected, lexical, source, form.at, applicable)) return true;
      if (type.target && self(self, *type.target, false)) return true;
      return std::any_of(type.parameters.begin(), type.parameters.end(),
          [&](const Type &parameter) { return self(self, parameter, false); });
    };
    for (const auto &parameter : form.parameters)
      if (associated(associated, parameter.type, true)) return true;
    return form.result_type && associated(associated, *form.result_type, false);
  }
  void select_associated_methods(Module &module, const Module &source, Type selected, const Form &selection) {
    const bool importing = selection.kind == Form::Kind::import_resource;
    for (const auto &[alias, published] : source.world.exports) {
      const auto *identity = std::get_if<CallableIdentity>(&published);
      if (!identity) continue;
      auto information = callable_information(source, *identity);
      std::vector<Method> associated;
      for (const auto &method : information.methods) {
        if (associated_signature(*method.form, *method.lexical, source, selected))
          append_method(associated, selected_method(method, source, module, importing));
      }
      std::vector<CallableDeclaration> declarations;
      for (const auto &declaration : information.declarations)
        if (associated_signature(*declaration.form, *declaration.lexical, source, selected))
          append_declaration(declarations, declaration);
      if (associated.empty() && declarations.empty()) continue;
      information.methods = std::move(associated);
      information.declarations = std::move(declarations);
      const auto local = select_callable(module, std::move(information), alias, importing, true, selection.at);
      if (selection.exporting) publish(module, alias, local, selection.at);
    }
  }
  void check_method_extensions(const Module &module) {
    for (const auto &[name, identity] : module.world.local_callables) {
      const auto explicit_selection = module.world.explicit_method_imports.find(name);
      if (explicit_selection == module.world.explicit_method_imports.end() || explicit_selection->second || operation_name(name)) continue;
      const auto &family = module.world.callables.at(identity);
      fail(family.methods.empty() ? family.declarations.front().form->at : family.methods.front().form->at,
           "extending selected callable '" + name + "' requires import rather than using");
    }
    std::map<CallableIdentity, std::vector<std::pair<std::string, const Form *>>> declarations;
    for (const auto &[alias, identity] : module.world.local_callables) {
      const auto &methods = module.world.callables.at(identity).methods;
      const auto local = std::find_if(methods.begin(), methods.end(), [&](const Method &method) {
        return method.lexical == &module && method.form->name == alias;
      });
      if (local != methods.end()) declarations[identity].emplace_back(alias, local->form);
    }
    for (auto &[identity, aliases] : declarations) {
      if (aliases.size() < 2) continue;
      std::sort(aliases.begin(), aliases.end(), [](const auto &a, const auto &b) {
        const auto &left = a.second->at, &right = b.second->at;
        if (left.file != right.file) return left.file < right.file;
        if (left.line != right.line) return left.line < right.line;
        return left.column < right.column;
      });
      std::string message = "overloads declared through multiple aliases of one callable identity:";
      for (const auto &[alias, form] : aliases) message += " '" + alias + "'";
      for (const auto &[alias, form] : aliases)
        message += "\n" + location_text(form->at) + ": note: alias '" + alias + "' adds overloads here";
      warn(Warning::multi_alias_extension, aliases.back().second->at, std::move(message));
    }
  }
  static void publish(Module &module, const std::string &alias, PublishedIdentity identity, const Location &at) {
    const auto [found, inserted] = module.world.exports.emplace(alias, identity);
    if (!inserted && found->second != identity)
      fail(at, "export alias '" + alias + "' already publishes a different identity");
  }
  static const PublishedIdentity &published(const Module &source, const std::string &name,
                                             bool symbolic, const Location &at) {
    const auto alias = (symbolic ? "@" : "") + name;
    const auto found = source.world.exports.find(alias);
    if (found == source.world.exports.end())
      fail(at, "module '" + source.path + "' does not export '" + alias + "'");
    return found->second;
  }
  static size_t published_symbol(const Module &source, const std::string &name, const Location &at) {
    const auto &identity = published(source, name, true, at);
    const auto *symbol = std::get_if<size_t>(&identity);
    if (!symbol) fail(at, "published resource @" + name + " is not a symbol");
    return *symbol;
  }
  void publish_declarations(Module &module) {
    for (const auto &form : module.forms) {
      if (form->kind != Form::Kind::export_resource) continue;
      if (form->symbolic) {
        if (!module.world.available_symbols.contains(form->name))
          fail(form->at, "cannot export unavailable resource '@" + form->name + "'");
        publish(module, "@" + form->name, symbol(module, form->name, form->at), form->at);
      } else if (const auto type = module.world.types.find(form->name); type != module.world.types.end()) {
        publish(module, form->name, type->second, form->at);
      } else if (const auto global = module.world.global_aliases.find(form->name); global != module.world.global_aliases.end()) {
        publish(module, form->name, GlobalIdentity{global->second}, form->at);
      } else {
        const auto methods = lexical_callable(module, form->name);
        check_implicit_identity(methods, form->name, form->at);
        if (methods.identities.size() != 1)
          fail(form->at, "cannot export unavailable resource '" + form->name + "'");
        publish(module, form->name, *methods.identities.begin(), form->at);
      }
    }
  }
  const Module &dependency(const Module &receiver, const Form &selection, const Environment &env) {
    auto &source = *receiver.dependencies.at(&selection);
    execute_module(source, env, selection.at);
    return source;
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
      for (const auto &[name, id] : module.world.symbols) {
        auto proc = std::make_shared<Procedure>();
        proc->builtin = name;
        proc->lexical = &module;
        module.world.symbolic_resources.emplace(id, Value{Type::procedure, {}, std::move(proc), &module});
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
        module.world.available_symbols.insert(form.name);
        break;
      case Form::Kind::function: case Form::Kind::array: case Form::Kind::unit:
      case Form::Kind::export_resource: break;
      case Form::Kind::abstract_type:
        resolve_abstract(module.world.types.at(form.name), form.at);
        break;
      case Form::Kind::data:
        if (form.parent_type) declare_conformance(module, form);
        break;
      case Form::Kind::conformance:
        declare_conformance(module, form);
        break;
      case Form::Kind::using_units: {
        const auto &source = dependency(module, form, env);
        if (!module.world.unit_sources.insert(&source).second)
          fail(form.at, "UoMs from a source module must be selected in one using bundle");
        std::set<size_t> selected;
        for (const auto &selection : form.units) {
          const auto *selected_type = std::get_if<Type>(&published(source, selection.source, false, selection.at));
          if (!selected_type) fail(selection.at, "using units requires UoM definitions");
          const auto type = resolve_name(*selected_type, source, selection.at);
          if (type.kind != Type::unit) fail(selection.at, "using units requires UoM definitions");
          if (!module.world.selected_units.insert(type.identity).second)
            fail(selection.at, "repeated using of UoM identity " + type_name(type));
          selected.insert(type.identity);
          select_type(module, type, selection.local, selection.at);
        }
        for (const auto &eligible : source.world.conversions) {
          const auto bundle_context = eligible.implementation.empty() ? ImplementationContext{eligible.lexical} : eligible.implementation;
          for (const auto &[name, publication] : source.world.exports) {
            const auto *identity = std::get_if<CallableIdentity>(&publication);
            if (!identity) continue;
            const auto methods = callable_information(source, *identity);
            for (const auto &function : methods.methods) {
              if (function.form != eligible.form || function.lexical != eligible.lexical) continue;
              const auto visible_context = function.implementation.empty() ? ImplementationContext{function.lexical} : function.implementation;
              if (visible_context != bundle_context) continue;
              const auto edge = conversion(function);
              if (!selected.contains(edge.source.identity) || !selected.contains(edge.target.identity)) continue;
              check_implicit_identity(methods, name, form.at);
              const auto selected_conversion = selected_method(function, source, module, false);
              append_method(module.world.conversions, selected_conversion);
              auto information = methods;
              information.methods = {selected_conversion};
              select_callable(module, std::move(information), name, false, true, form.at);
            }
          }
        }
        break;
      }
      case Form::Kind::symbol_from: {
        const auto &source = dependency(module, form, env);
        module.world.symbols.emplace(form.alias, published_symbol(source, form.name, form.at));
        module.world.available_symbols.insert(form.alias);
        break;
      }
      case Form::Kind::procedure: {
        if (!form.body) {
          if (form.symbolic) {
            declare_symbol_type(symbol(module, form.name, form.at));
            module.world.available_symbols.insert(form.name);
          }
          break;
        }
        if (form.operation) break;
        auto value = define_procedure(form, module, env);
        if (form.symbolic) {
          const auto id = symbol(module, form.name, form.at);
          module.world.available_symbols.insert(form.name);
          bind(id, std::move(value), module, env, form.at);
        } else {
          module.world.procedures.emplace(&form, std::move(value));
        }
        break;
      }
      case Form::Kind::global: {
        if (module.world.callable_aliases.contains(form.name)) fail(form.at, "duplicate lexical name '" + form.name + "'");
        auto value = form.body->kind == Expression::Kind::call
            ? call_expression(*form.body, module, env, locals, false, true)
            : form.body->kind == Expression::Kind::unary
            ? unary_expression(*form.body, module, env, locals, false, true)
            : expression(*form.body, module, env, locals, false);
        if (value.region_id || value.type.kind == Type::reference || value.type == Type::region)
          fail(form.at, "module globals contain bits values, not regional resources or references");
        require(value.type, resolve(*form.result_type, module, form.at), form.at);
        const MethodOrigin identity{&module, form.name};
        module.world.global_values.emplace(identity, std::move(value));
        module.world.global_aliases.emplace(form.name, identity);
        break;
      }
      case Form::Kind::expression:
        expression(*form.body, module, env, locals, false);
        break;
      case Form::Kind::using_resource: case Form::Kind::import_resource: case Form::Kind::mixin_resource: {
        const bool mixing = form.kind == Form::Kind::mixin_resource;
        if (form.symbolic && !form.using_symbol && !module.world.available_symbols.contains(form.alias))
          fail(form.at, "procedure destination @" + form.alias + " must be declared before using/import/mixin");
        const auto &source = dependency(module, form, env);
        const auto &publication = published(source, form.name, form.symbolic, form.at);
        if (form.symbolic) {
          const auto source_id = published_symbol(source, form.name, form.at);
          const auto resource = source.world.symbolic_resources.find(source_id);
          if (resource == source.world.symbolic_resources.end() || !resource->second.procedure)
            fail(form.at, "source resource @" + form.name + " has no procedure value; use symbol from for identity");
          if (form.using_symbol) {
            const auto alias = module.world.symbol_aliases.find(form.alias);
            if (alias != module.world.symbol_aliases.end() && alias->second == &form) {
              module.world.symbols.emplace(form.alias, source_id);
              module.world.available_symbols.insert(form.alias);
            }
          }
          const auto destination = symbol(module, form.alias, form.at);
          if (!mixing && (form.using_symbol || !form.explicit_alias) &&
              destination != source_id)
            fail(form.at, "symbol identity mismatch: source @" + form.name + " is " + symbol_names_[source_id] +
                             ", local @" + form.alias + " is " + symbol_names_[destination] +
                             "; write using \"" + form.path + "\" @" + form.name + " as @" + form.alias +
                             "; to select that destination explicitly");
          auto value = resource->second;
          if (mixing)
            value = define_procedure(mixin_body(*value.procedure, form, module), module, env);
          else if (form.kind == Form::Kind::import_resource) {
            auto imported = std::make_shared<Procedure>(*value.procedure);
            if (imported->implementation.empty()) imported->implementation = {imported->lexical};
            if (imported->implementation.back() != &module) imported->implementation.push_back(&module);
            value.procedure = std::move(imported);
          }
          bind(destination, std::move(value), module, env, form.at);
          if (form.exporting) publish(module, "@" + form.alias, destination, form.at);
        } else if (const auto *type = std::get_if<Type>(&publication)) {
          if (mixing) fail(form.at, "mixin requires a procedure body, not a type definition");
          if (form.alias_operation) fail(form.at, "an operation name requires a callable overload");
          if (type->kind == Type::unit)
            fail(form.at, "select UoMs and their conversions together with using units");
          select_type(module, *type, form.alias, form.at);
          if (type->kind == Type::regional) {
            if (form.kind == Form::Kind::import_resource) module.world.imported_data.insert(type->identity);
            for (const auto &relation : source.world.conformances) {
              if (relation.data == *type &&
                  std::find(module.world.conformances.begin(), module.world.conformances.end(), relation) == module.world.conformances.end())
                module.world.conformances.push_back(relation);
            }
          }
          select_associated_methods(module, source, *type, form);
          if (form.exporting) publish(module, form.alias, *type, form.at);
        } else if (const auto *identity = std::get_if<CallableIdentity>(&publication)) {
          const auto function = callable_information(source, *identity);
          check_implicit_identity(function, form.name, form.at);
          if (!form.alias_operation) require_user_type_name(form.alias, form.at);
          if (!mixing) {
            auto information = function;
            for (auto &method : information.methods)
              method = selected_method(method, source, module, form.kind == Form::Kind::import_resource);
            const auto local = select_callable(module, std::move(information), form.alias,
                            form.kind == Form::Kind::import_resource, false, form.at);
            if (form.exporting) publish(module, form.alias, local, form.at);
          } else {
            if (function.methods.empty()) fail(form.at, "mixin requires an implemented procedure method");
            for (const auto &candidate : function.methods) {
              if (candidate.form->kind != Form::Kind::procedure)
                fail(form.at, "mixin requires a procedure body, not a function definition");
              if (candidate.form->operation || form.alias_operation)
                fail(form.at, "mixin of operation overloads is outside this prototype");
              const auto original = candidate.lexical->world.procedures.find(candidate.form);
              if (original == candidate.lexical->world.procedures.end())
                fail(form.at, "procedure body requires its definition encounter before mixin");
              auto &body = mixin_body(*original->second.procedure, form, module);
              auto value = define_procedure(body, module, env);
              declare_callable(module, body);
              module.world.procedures.emplace(&body, std::move(value));
            }
          }
        } else if (const auto *identity = std::get_if<GlobalIdentity>(&publication)) {
          if (mixing) fail(form.at, "mixin requires a procedure body, not a global value");
          if (form.alias_operation) fail(form.at, "an operation name requires a callable overload");
          if (module.world.types.contains(form.alias) || module.world.callable_aliases.contains(form.alias))
            fail(form.at, "duplicate lexical alias '" + form.alias + "'");
          const auto [alias, inserted] = module.world.global_aliases.emplace(form.alias, identity->origin);
          if (!inserted && alias->second != identity->origin)
            fail(form.at, "duplicate lexical alias '" + form.alias + "'");
          module.world.global_values.emplace(identity->origin, source.world.global_values.at(identity->origin));
          if (form.exporting) publish(module, form.alias, *identity, form.at);
        } else {
          fail(form.at, "unavailable module resource '" + form.name + "'");
        }
        break;
      }
      }
    }
    check_method_extensions(module);
    publish_declarations(module);
    for (const auto &[id, current] : env.bindings) {
      if (current->owner != &module) continue;
      auto value = *current;
      if (value.type.kind != Type::procedure && value.type != Type::region) {
        value.native = ir_.read_symbol(id, value.type);
      }
      module.world.symbolic_resources.emplace(id, std::move(value));
    }

    close_holdings(holding_begin, {}, at);
    ir_.restore(native_scope);
    module.state = Module::State::complete;
  }

public:
  explicit Compiler(WarningOptions warnings = {}) : warning_options_(std::move(warnings)) {}
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
