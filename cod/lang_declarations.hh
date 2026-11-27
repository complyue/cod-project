//===--- lang_declarations.hh - CoD module declaration binding -----===//
#pragma once

#include "lang_checked_program.hh"
#include "lang_scalar_operations.hh"
#include "lang_schema_binding.hh"
#include "lang_modules.hh"
#include "native/provider_names.h"

#include <array>
#include <bit>
#include <charconv>
#include <cmath>
#include <memory>
#include <tuple>
#include <type_traits>
#include <variant>

namespace cod::language {

namespace seed { class BodyExtraction; }

class ModuleBinder {

  CheckedProgram program_;
  std::vector<WarningDiagnostic> warnings_;

  Type exception_type(size_t category) const { return program_.exception_type(category); }
  std::set<size_t> exception_possibilities(const Type &type) const {
    if (type.kind != Type::exception) return {};
    if (concrete_failure(type.identity)) {
      if (type.identity < program_.exception_types.size()) return {type.identity};
      return {};
    }
    std::set<size_t> result;
    for (size_t category = 1; category < program_.exception_types.size(); ++category)
      if (matches_failure(program_.exception_types, category, type.identity)) result.insert(category);
    return result;
  }

  struct ModuleNames {
    std::map<std::string, size_t> callables;
    std::map<std::string, size_t> native_operations;
    std::map<std::string, cod_lang_export_kind> text_primitives;
    std::map<size_t, size_t> imports;
    std::set<std::string> imported_aliases;
    std::map<size_t, std::map<std::string, Location>> extensions;
    std::map<std::string, size_t> symbols;
    std::map<size_t, size_t> families;
    struct SelectedSymbol {
      std::optional<size_t> family;
      bool grants_extension = false;
      bool identity_only = true;
    };
    std::map<std::string, SelectedSymbol> selected_symbols;
    std::set<std::string> selected_callables, values;
    std::map<std::string, Type> types;
    std::map<std::string, EnumValue> enum_members;
    std::map<std::string, SumConstructor> constructors;
    bool binds_other_than_enum_member(const std::string &name) const {
      return callables.contains(name) || native_operations.contains(name) || symbols.contains(name) ||
          values.contains(name) || types.contains(name) || text_primitives.contains(name);
    }
    struct Resource {
      enum class Kind { symbol, callable, type, native_operation } kind;
      size_t symbol = 0;
      std::optional<size_t> family;
      Type type;
      std::optional<size_t> native_operation = {};
      bool operator==(const Resource &) const = default;
    };
    struct SelectedMember {
      Token spelling;
      size_t ordinal;
    };
    struct DefaultSelection {
      std::string name;
      std::vector<SelectedMember> members;
    };
    std::map<std::string, Resource> exports;
    std::vector<std::string> public_order;
    std::vector<DefaultSelection> defaults;
    bool publication_ready = false;
  };
  std::vector<ModuleNames> modules_;

  mutable size_t module_ = 0;
  struct ModuleLookup {
    size_t &current;
    size_t previous;
    ModuleLookup(size_t &current, size_t selected) : current(current), previous(current) { current = selected; }
    ~ModuleLookup() { current = previous; }
    ModuleLookup(const ModuleLookup &) = delete;
    ModuleLookup &operator=(const ModuleLookup &) = delete;
  };
  ModuleNames &module_names() { return modules_.at(module_); }
  const ModuleNames &module_names() const { return modules_.at(module_); }
  struct Interpretation { size_t parent, receiver; };
  std::vector<Interpretation> interpretations_{{0, 0}};
  std::map<std::pair<size_t, size_t>, size_t> interpretation_ids_;

  std::map<std::tuple<size_t, size_t, size_t>, size_t> adapted_methods_;

  static constexpr size_t instance_limit = 4096;

  using Substitutions = std::vector<std::optional<Type>>;

  static bool variadic(const CheckedMethod &method) {
    return !method.input_parameters().empty() && method.input_parameters().back().variadic;
  }

  void check_variadic_family_member(std::span<const size_t> members, const CheckedMethod &addition,
                                    const Location &at) const {
    for (const auto member : members) {
      const auto &existing = program_.methods[member];
      if (existing.definition == addition.definition) continue;
      if (variadic(existing) || variadic(addition))
        fail(at, "a variadic method must be the only method in its callable family; conflicts with " +
            location_text(existing.source->at));
    }
  }
  static bool accepts_arity(const CheckedMethod &method, size_t count) {
    return variadic(method) ? count + 1 >= method.parameters.size() : count == method.parameters.size();
  }
  static size_t parameter_index(const CheckedMethod &method, size_t argument) {
    return variadic(method) ? std::min(argument, method.parameters.size() - 1) : argument;
  }
  static const Parameter &parameter_at(const CheckedMethod &method, size_t argument) {
    return method.input_parameters()[parameter_index(method, argument)];
  }

  static bool same_access(const CheckedMethod &a, const CheckedMethod &b) {
    for (size_t i = 0; i < a.parameters.size(); ++i)
      if (a.input_parameters()[i].read_only != b.input_parameters()[i].read_only) return false;
    return true;
  }

  static bool same_owner(const CheckedMethod &method, size_t a, size_t b) {
    const auto &owner = parameter_at(method, a).owner;
    return !owner.empty() && owner == parameter_at(method, b).owner;
  }
  static bool implies_owners(const CheckedMethod &narrow, const CheckedMethod &broad) {
    for (size_t i = 0; i < narrow.parameters.size(); ++i) for (size_t j = 0; j < i; ++j)
      if (same_owner(broad, i, j) && !same_owner(narrow, i, j)) return false;
    return true;
  }

  std::optional<EnumValue> enum_member(const Type &type, const std::string &name) const {
    return cod::language::enum_member(program_, type, name);
  }

  static std::optional<size_t> symbol_family(const ModuleNames &names, const std::string &name) {
    if (const auto selected = names.selected_symbols.find(name); selected != names.selected_symbols.end())
      return selected->second.family;
    if (const auto symbol = names.symbols.find(name); symbol != names.symbols.end())
      if (const auto family = names.families.find(symbol->second); family != names.families.end())
        return family->second;
    return {};
  }

  void check_symbol_binding_name(const std::string &name, const Location &at) const {
    const auto symbol = module_names().symbols.find(name);
    if (symbol == module_names().symbols.end()) return;
    const auto &defining = program_.symbols.at(symbol->second);
    std::string message = "declaration conflicts with protected symbol name '" + name + "'";
    if (defining.declaration)
      message += " (canonical " + defining.declaration->filename() + "::@" + defining.name +
          ", defined at " + location_text(*defining.declaration) + ")";
    else if (defining.intrinsic_print)
      message += " (canonical " COD_LANG_IO_MODULE "::@" + defining.name + ")";
    fail(at, std::move(message));
  }

  static bool ordinary_callable_type(const Type &type) {
    if (type.kind != Type::procedure) return true;
    return type.pure_callable && type.target && ordinary_callable_type(*type.target) &&
        std::ranges::all_of(type.parameters, ordinary_callable_type);
  }
  static void require_ordinary_callable_type(const Type &type, const Location &at) {
    if (!ordinary_callable_type(type)) fail(at, "procedure signatures cannot occupy ordinary value or reference storage");
  }

  static bool caller_bindings(const CheckedMethod &method) {
    return method.operation == "<=>";
  }

  Type resolve_type(Type type, const CheckedMethod *method = nullptr) const {
    const ModuleLookup lookup(module_, method ? method->module : module_);
    if (method && binder(*method, type)) return type;
    if (type.kind == Type::named && !type.target)
      if (const auto found = module_names().types.find(type.name); found != module_names().types.end()) return found->second;
    if (type.kind == Type::procedure) {
      for (auto &parameter : type.parameters) parameter = resolve_type(std::move(parameter), method);
      // Throws declarations are resolved independently of argument inference.
      for (auto &failure : type.throws_types) failure = resolve_type(std::move(failure), method);
      if (type.target) type.target = std::make_shared<Type>(resolve_type(*type.target, method));
      // Independent results have a passing mode, not a target-access qualifier.
      if (type.value_result &&
          type.const_views.size() > type.parameters.size()) {
        type.const_views.back() = false;
        if (std::none_of(type.const_views.begin(), type.const_views.end(), [](bool value) { return value; })) type.const_views.clear();
      }
    }
    return type;
  }

  static std::optional<size_t> binder(const CheckedMethod &method, const Type &type) {
    if (type.kind != Type::named) return {};
    for (size_t i = 0; i < method.type_parameters.size(); ++i)
      if (method.type_parameters[i] == type.name) return i;
    return {};
  }

  static void reject_c_type_payload(const Type &type, const Location &at) {
    if (type.kind == Type::c_type_marker)
      fail(at, "handle type '" + type.name + "' has no by-value representation");
  }
  static void signature_type(const CheckedMethod &method, const Type &type, const Location &at, bool result = false,
                             bool borrowed_target = false) {
    reject_c_type_payload(type, at);
    if (owning_exception(type)) implementation_gap(at, "owned exception arguments and results require an owner-aware signature");
    if (bits_type(type) || binder(method, type) || (result && type == Type::void_type)) return;
    if (borrowed_target && (type.kind == Type::product || text_type(type) ||
                            (!result && (type.kind == Type::array || type.kind == Type::sum ||
                                         type.kind == Type::abstract_type)))) return;
    if (type.kind == Type::array)
      implementation_gap(at, "array parameter or result of type '" + type_name(type) + "'");
    if (type.kind == Type::procedure && type.target) {
      if (type.variadic || !type.capture_owners.empty() || !type.capture_regions.empty() ||
          (type.owners.empty() && std::any_of(type.nullable_views.begin(), type.nullable_views.end(), [](bool v) { return v; })))
        implementation_gap(at, "callable view captures, owners or nullable passing modes");
      for (const auto &parameter : type.parameters) signature_type(method, parameter, at);
      signature_type(method, *type.target, at, true);
      if (type.owners.empty() && !type.value_result)
        implementation_gap(at, "callable result mode without an owner or explicit generic value contract");
      for (const auto &failure : type.throws_types)
        if (failure.kind != Type::exception) fail(at, "callable view throws require selected exception types");
      return;
    }
    implementation_gap(at, "non-scalar signature type '" + type_name(type) + "'");
  }
  CheckedHereContract here_contract(const CheckedMethod &receiver, const Parameter &formal) const {
    const auto &source = *formal.here;
    const auto owner_role = [&](const std::string &name, const Location &at) -> std::optional<size_t> {
      if (name.empty()) return {};
      if (name == "here") return 0;
      const auto &owners = receiver.input_owners();
      const auto found = std::ranges::find(owners, name, &OwnerParameter::name);
      if (found == owners.end()) fail(at, "unknown here contract owner '" + name + "'");
      return std::distance(owners.begin(), found) + 1;
    };
    const auto components = [&](std::span<const Parameter> fields, bool labelled) {
      std::vector<CheckedHereContract::Component> result;
      std::set<std::string> names;
      for (const auto &field : fields) {
        if (field.here || field.variadic || field.automatic_conversion)
          implementation_gap(field.at, "nested here formals, packs or conversions in here contracts");
        if (!field.name.empty() && !names.insert(field.name).second)
          fail(field.at, "duplicate here input name '" + field.name + "'");
        const auto type = resolve_type(field.type, &receiver);
        require_ordinary_callable_type(type, field.at);
        const auto mode = argument_mode(field);
        const bool array_ref = type.kind == Type::array && mode == ArgumentMode::borrowed_ref &&
            (field.read_only || indexed_product(program_.arrays.at(type.identity).element, field.at));
        signature_type(receiver, type, field.at, false, array_ref);
        result.push_back({labelled ? field.name : std::string{}, type, mode,
                          owner_role(field.owner, field.at), field.read_only});
      }
      if (labelled) std::ranges::sort(result, {}, &CheckedHereContract::Component::label);
      return result;
    };
    CheckedHereContract contract;
    const auto ordinary = std::ranges::find_if(source.parameters, [](const auto &parameter) { return !parameter.here; });
    std::set<std::string> names;
    for (auto parameter = source.parameters.begin(); parameter != ordinary; ++parameter) {
      if (!names.insert(parameter->name).second) fail(parameter->at, "duplicate here input name '" + parameter->name + "'");
      contract.here.push_back(here_contract(receiver, *parameter));
    }
    const auto remaining = std::span<const Parameter>(source.parameters).subspan(std::distance(source.parameters.begin(), ordinary));
    contract.inputs = components(remaining, source.labelled);
    for (const auto &parameter : remaining)
      if (names.contains(parameter.name)) fail(parameter.at, "duplicate here input name '" + parameter.name + "'");
    contract.labelled = source.labelled;
    if (source.feedback_fields) contract.feedback = components(*source.feedback_fields, true);
    else {
      contract.result.type = resolve_type(source.result, &receiver);
      require_ordinary_callable_type(contract.result.type, formal.at);
      signature_type(receiver, contract.result.type, formal.at, true);
      contract.result.owner = owner_role(source.result_owner, formal.at);
      if (contract.result.owner) {
        contract.result.mode = source.result_kind == "ptr" ? ArgumentMode::borrowed_ptr : ArgumentMode::borrowed_ref;
        contract.result.read_only = source.result_const;
        if (contract.result.type == Type::void_type) fail(formal.at, "void cannot be a borrowed here result");
      }
    }
    for (const auto &failure : source.throws_types) {
      const auto type = resolve_type(failure, &receiver);
      if (type.kind != Type::exception) fail(formal.at, "here throws contracts require selected exception types");
      contract.throws_root |= !type.identity;
      if (type.identity) contract.throws_categories.merge(exception_possibilities(type));
    }
    if (contract.throws_root) contract.throws_categories.clear();
    return contract;
  }
  static bool bind_type(const CheckedMethod &method, const Type &pattern, const Type &actual, Substitutions &substitutions, bool parameter = false) {
    if (const auto index = binder(method, pattern)) {
      auto &bound = substitutions[*index];
      if (bound && *bound != actual) return false;
      bound = actual; return true;
    }
    if (pattern.kind == Type::procedure && actual.kind == Type::procedure) {
      if (pattern.parameters.size() != actual.parameters.size() || !pattern.target || !actual.target) return false;
      auto constructor = pattern, other = actual;
      constructor.parameters.clear(); constructor.target.reset();
      other.parameters.clear(); other.target.reset();
      if (constructor != other) return false;
      for (size_t i = 0; i < pattern.parameters.size(); ++i)
        if (!bind_type(method, pattern.parameters[i], actual.parameters[i], substitutions)) return false;
      return bind_type(method, *pattern.target, *actual.target, substitutions);
    }
    return pattern == actual || (parameter && !swap_operation(method.operation) &&
                                  accepts_exception(actual, pattern));
  }
  bool bind_parameter(const CheckedMethod &method, size_t index, const Type &argument,
                      Substitutions &substitutions, bool widening) const {
    index = parameter_index(method, index);
    if (argument_mode(method.input_parameters()[index]) == ArgumentMode::owning_exception) {
      const auto &target = method.parameters[index];
      return argument.kind == Type::exception && target.kind == Type::exception &&
          (argument == target || matches_failure(program_.exception_types, argument.identity, target.identity));
    }
    if (bind_type(method, method.parameters[index], argument, substitutions, true)) return true;
    if (method.parameters[index].kind == Type::abstract_type &&
        argument_mode(method.input_parameters()[index]) == ArgumentMode::borrowed_ref &&
        !caller_bindings(method) && program_.conforms_to(argument, method.parameters[index])) return true;
    // CString supplies the same immutable text target under its original owner.
    if (argument_mode(method.input_parameters()[index]) == ArgumentMode::borrowed_ref &&
        widens_text(argument, method.parameters[index])) return true;
    if (!widening || argument_mode(method.input_parameters()[index]) != ArgumentMode::value) return false;
    auto target = method.parameters[index];
    if (const auto type = binder(method, target)) {
      // An enclosing type is already fixed. This method's own binders still
      // constrain actual input identities, including repeated occurrences.
      if (*type < method.source->type_parameters.size() || !substitutions[*type]) return false;
      target = *substitutions[*type];
    }
    return supplies_enum_code(argument, target) || (numeric(target) && widens(argument, target));
  }
  std::optional<Substitutions> match(const CheckedMethod &method, const std::vector<Type> &arguments,
                                     bool widening) const {
    if (!accepts_arity(method, arguments.size())) return {};
    Substitutions substitutions(method.type_parameters.size());
    for (size_t i = 0; i < arguments.size(); ++i) {
      if (!bind_parameter(method, i, arguments[i], substitutions, widening)) return {};
    }
    return substitutions;
  }

  static Type rigid_type(const CheckedMethod &method, const Type &type) {
    if (const auto index = binder(method, type)) return {Type::parameter, {}, *index + 1};
    auto rigid = type;
    if (type.kind == Type::procedure) {
      for (auto &parameter : rigid.parameters) parameter = rigid_type(method, parameter);
      if (type.target) rigid.target = std::make_shared<Type>(rigid_type(method, *type.target));
    }
    return rigid;
  }
  template <typename SameType>
  static bool same_here_signature(const CheckedMethod &a, const CheckedMethod &b, size_t arguments, SameType same_type) {
    if (a.here.size() != b.here.size()) return false;
    std::map<size_t, size_t> right_roles, left_roles;
    const auto same_owner_role = [&](std::optional<size_t> x, std::optional<size_t> y) {
      if (!x || !y || *x == 0 || *y == 0) return x == y;
      const auto &x_name = a.input_owners().at(*x - 1).name;
      const auto &y_name = b.input_owners().at(*y - 1).name;
      for (size_t i = 0; i < arguments; ++i)
        if ((parameter_at(a, i).owner == x_name) != (parameter_at(b, i).owner == y_name)) return false;
      return right_roles.try_emplace(*x, *y).first->second == *y &&
             left_roles.try_emplace(*y, *x).first->second == *x;
    };
    const auto same = [&](const auto &x, const auto &y) {
      return x.label == y.label && x.mode == y.mode && x.read_only == y.read_only &&
             same_type(x.type, y.type) && same_owner_role(x.owner, y.owner);
    };
    const auto same_components = [&](const auto &x, const auto &y) {
      if (x.size() != y.size()) return false;
      for (size_t i = 0; i < x.size(); ++i) if (!same(x[i], y[i])) return false;
      return true;
    };
    const auto same_contract = [&](const auto &self, const CheckedHereContract &left, const CheckedHereContract &right) -> bool {
      if (left.here.size() != right.here.size() || left.labelled != right.labelled ||
          left.throws_categories != right.throws_categories || left.throws_root != right.throws_root ||
          !same_components(left.inputs, right.inputs) || !same(left.result, right.result) ||
          left.feedback.has_value() != right.feedback.has_value() || (left.feedback && !same_components(*left.feedback, *right.feedback))) return false;
      for (size_t i = 0; i < left.here.size(); ++i) if (!self(self, left.here[i], right.here[i])) return false;
      return true;
    };
    for (size_t i = 0; i < a.here.size(); ++i) if (!same_contract(same_contract, a.here[i], b.here[i])) return false;
    return true;
  }
  static bool same_here_signature(const CheckedMethod &a, const CheckedMethod &b) {
    return same_here_signature(a, b, a.parameters.size(), [&](const Type &left, const Type &right) {
      return rigid_type(a, left) == rigid_type(b, right);
    });
  }
  static Type independent_input(Type type, size_t parameter, ArgumentMode mode) {
    // A broad value input admits an independent concrete category; a borrow
    // retains the declared type of its storage target.
    if (mode == ArgumentMode::value && type.kind == Type::exception && !type.identity)
      type.name += ".alternative." + std::to_string(parameter);
    return type;
  }
  // Signature containment includes repeated binders; layout equality is irrelevant.
  bool implies(const CheckedMethod &narrow, const CheckedMethod &broad, bool widening = false) const {
    if (narrow.parameters.size() != broad.parameters.size() || narrow.here.size() != broad.here.size()) return false;
    std::vector<Type> arguments;
    for (size_t i = 0; i < narrow.parameters.size(); ++i) {
      arguments.push_back(independent_input(rigid_type(narrow, narrow.parameters[i]), i, argument_mode(parameter_at(narrow, i))));
    }
    return match(broad, arguments, widening).has_value();
  }
  static Type callable_signature(const CheckedMethod &checked, const std::vector<Type> &arguments, const ResultContract &contract) {
    const auto &method = *checked.source;
    Type type{Type::procedure}; type.pure_callable = method.kind == Form::Kind::function;
    type.value_result = !contract.borrowed();
    type.parameters = arguments; type.target = std::make_shared<Type>(*contract.type);
    std::map<std::string, size_t> roles;
    const auto role = [&](const std::string &owner) {
      if (owner.empty()) return size_t{0};
      return roles.try_emplace(owner, roles.size() + 1).first->second;
    };
    for (size_t i = 0; i < arguments.size(); ++i) {
      const auto &parameter = parameter_at(checked, i);
      type.const_views.push_back(parameter.read_only);
      type.owners.push_back(role(parameter.owner));
      type.argument_modes.push_back(argument_mode(parameter));
      type.nullable_views.push_back(argument_mode(parameter) == ArgumentMode::borrowed_ptr);
    }
    type.owners.push_back(contract.borrowed() ? role(contract.owner) : 0);
    type.const_views.push_back(contract.read_only);
    type.nullable_views.push_back(contract.nullable());
    if (roles.empty()) { type.owners.clear(); type.argument_modes.clear(); }
    if (std::none_of(type.const_views.begin(), type.const_views.end(), [](bool v) { return v; })) type.const_views.clear();
    if (std::none_of(type.nullable_views.begin(), type.nullable_views.end(), [](bool v) { return v; })) type.nullable_views.clear();
    type.throws_types = checked.throws_types;
    return type;
  }
  bool indexed_product(const Type &type, const Location &at) const {
    if (type.kind != Type::product) return false;
    const auto &schema = product_schema(type, at);
    // These initialized fields can change bits without changing reference or predicate facts.
    return std::ranges::all_of(schema.fields, [](const auto &field) {
      return field.storage == PlaceKind::value && numeric(field.type);
    });
  }

  const CheckedSchema &product_schema(const Type &type, const Location &at) const {
    reject_c_type_payload(type, at);
    if (owning_exception(type)) fail(at, "use exception construction for '" + type.name + "'");
    if (type.kind != Type::product) implementation_gap(at, "ordinary construction requires a supported product type");
    return program_.schemas.at(type.identity);
  }
  static void check_aggregate_owner_roles(const CheckedMethod &method, const std::vector<Type> &types) {
    // Lexical scopes retain the supplying activation's owner capabilities.
    if (method.scope_inputs) return;
    if (std::ranges::none_of(types, [](const auto &type) { return type.kind == Type::product || type.kind == Type::array; })) return;
    for (const auto &owner : method.input_owners()) if (!owner.read_only) {
      bool product = false, array = false, writable = false;
      for (size_t i = 0; i < types.size(); ++i) {
        const auto &parameter = parameter_at(method, i);
        if (parameter.owner != owner.name) continue;
        product |= types[i].kind == Type::product;
        array |= types[i].kind == Type::array;
        writable |= !parameter.read_only;
      }
      if (product && !writable) fail(owner.at, "readonly product inputs require 'const owner " + owner.name + "'");
      if (array && !writable) fail(owner.at, "readonly array inputs require 'const owner " + owner.name + "'");
    }
  }

  Type sum_type(size_t identity) const {
    return {Type::sum, program_.sums.at(identity).source->name, identity};
  }

  const CheckedSum::Alternative &sum_alternative(SumConstructor constructor) const {
    return program_.sums.at(constructor.sum).alternatives.at(constructor.alternative);
  }

  static bool prepared_source_contract(const CheckedHereContract &expected) {
    return expected.here.size() == 1 && expected.inputs.empty() && expected.result.type == Type::void_type &&
        !expected.result.owner && !expected.feedback && expected.here.front().here.empty();
  }

  static void relate_module_findings(const SourceProgram &source, size_t module,
                                     CompilationDiagnostics &diagnostics, size_t first) {
    const auto past = diagnostics.size();
    for (const auto &requester : source.modules) for (const auto &form : requester.forms()) {
      const auto dependency = requester.dependencies.find(form.get());
      if (dependency != requester.dependencies.end() && dependency->second == module)
        diagnostics.relate_import(first, past, form->at);
    }
  }

  size_t adapt_method(size_t method, size_t receiver, size_t family) {
    const auto &original = program_.methods[method];
    if (original.module == receiver) return method;
    if (original.interpretation && interpretations_[original.interpretation].receiver == receiver &&
        family == original.family) return method;
    const auto key = std::tuple{method, receiver, family};
    if (const auto found = adapted_methods_.find(key); found != adapted_methods_.end()) return found->second;
    if (program_.methods.size() >= instance_limit)
      fail(original.source->at, "compiler resource limit: 4096 selected method interpretations; coverage remains unproved");
    auto context = original.interpretation;
    if (!context || interpretations_[context].receiver != receiver) {
      const auto [found, inserted] = interpretation_ids_.emplace(std::pair{context, receiver}, interpretations_.size());
      if (inserted) interpretations_.push_back({context, receiver});
      context = found->second;
    }
    auto adapted = original;
    adapted.interpretation = context;
    adapted.family = family;
    const auto id = program_.methods.size();
    program_.methods.push_back(std::move(adapted));
    adapted_methods_.emplace(key, id);
    return id;
  }

  size_t register_family(std::optional<size_t> symbol, const Location &at, const std::string &operation) {
    const auto id = program_.families.size();
    program_.families.push_back({symbol, module_, id, at, {}, {}, {}, operation});
    return id;
  }

  size_t import_family(size_t source, const Location &at) {
    if (const auto found = module_names().imports.find(source); found != module_names().imports.end()) return found->second;
    const auto &original = program_.families[source];
    const auto id = program_.families.size();
    program_.families.push_back({original.symbol, module_, original.origin, at, source, {}, original.declarations, original.operation});
    module_names().imports.emplace(source, id);
    for (auto &declaration : program_.families[id].declarations) declaration.family = id;
    for (const auto method : original.methods) program_.families[id].methods.push_back(adapt_method(method, module_, id));
    return id;
  }

  void publish(const std::string &name, const ModuleNames::Resource &resource, const Location &at) {
    const auto [published, inserted] = module_names().exports.emplace(name, resource);
    if (!inserted && published->second != resource) fail(at, "conflicting public alias '" + name + "'");
    if (inserted) module_names().public_order.push_back(name);
  }

  void bind_sum_constructor(const Token &name, SumConstructor constructor) {
    const auto [selected, inserted] = module_names().constructors.emplace(name.text, constructor);
    if (!inserted && selected->second != constructor)
      fail(name.at, "conflicting data constructor '" + name.text + "'");
  }

  template <typename Select>
  void select_resource_members(const ModuleNames::Resource &resource, const std::vector<Token> &names,
      const Location &at, const Select &select) const {
    if (resource.kind != ModuleNames::Resource::Kind::type) fail(at, "member selection requires a type");
    const auto &type = resource.type;
    if (type.kind == Type::sum) {
      const auto &alternatives = program_.sums.at(type.identity).alternatives;
      if (names.empty()) {
        for (size_t i = 0; i < alternatives.size(); ++i) select(alternatives[i].source->name, i);
      } else for (const auto &name : names) {
        const auto found = std::ranges::find_if(alternatives, [&](const auto &alternative) {
          return alternative.source->name.text == name.text;
        });
        if (found == alternatives.end())
          fail(name.at, "unknown data constructor '" + name.text + "' of '" + type.name + "'");
        select(name, static_cast<size_t>(found - alternatives.begin()));
      }
    } else {
      if (type.kind != Type::enumeration) implementation_gap(at, "member selection outside an enum");
      const auto select_enum = [&](const Token &name) {
        const auto value = enum_member(type, name.text);
        if (!value) fail(name.at, "unknown member '" + name.text + "' of enum '" + type.name + "'");
        select(name, value->member);
      };
      if (names.empty()) {
        for (const auto &member : program_.enums.at(type.identity).source->enum_members) select_enum(member.name);
      } else for (const auto &name : names) select_enum(name);
    }
  }

  void bind_resource_member(const Type &type, const Token &name, size_t ordinal) {
    if (type.kind == Type::sum) return bind_sum_constructor(name, {type.identity, ordinal});
    const EnumValue value{type, ordinal};
    const auto [binding, inserted] = module_names().enum_members.emplace(name.text, value);
    if (!inserted && binding->second != value)
      fail(name.at, "conflicting enum member selection '" + name.text + "'");
  }

  std::optional<Form::ResourceNature> publication_nature(const ModuleNames::Resource &resource) const {
    if (resource.kind != ModuleNames::Resource::Kind::symbol) return {};
    const auto &symbol = program_.symbols.at(resource.symbol);
    return symbol.intrinsic_print || (symbol.type && symbol.type->kind == Type::procedure)
        ? Form::ResourceNature::procedure : Form::ResourceNature::variable;
  }

  void check_procedure_selection_identity(const Form &form, const std::string &local,
                                          size_t symbol, const std::set<std::string_view> &fresh_symbols) {
    const auto destination = module_names().symbols.find(local);
    if (fresh_symbols.contains(local) ||
        (destination != module_names().symbols.end() && destination->second != symbol))
      fail(form.at, "conflicting symbol identity for '" + local + "'");
  }

  void bind_module_resource(const Form &form, const std::string &local, ModuleNames::Resource resource,
                            std::optional<Form::ResourceNature> nature,
                            const std::set<std::string_view> &fresh_symbols,
                            std::span<const ModuleNames::SelectedMember> members = {}) {
    using Resource = ModuleNames::Resource;
    const bool importing = form.kind == Form::Kind::import_resource;
    const bool reexporting = form.kind == Form::Kind::export_from_resource;
    if (nature && publication_nature(resource) != nature)
      fail(form.at, "symbolic resource kind does not match its publication");
    if (form.selected_members) {
      select_resource_members(resource, *form.selected_members, form.at,
          [&](const Token &name, size_t ordinal) { bind_resource_member(resource.type, name, ordinal); });
    } else for (const auto &[name, ordinal] : members) {
      bind_resource_member(resource.type, name, ordinal);
    }
    if (reexporting) {
      publish(local, resource, form.at);
      return;
    }
    if (resource.kind == Resource::Kind::symbol) {
      // Selection introduces the identity. Only an import grants extension
      // through that spelling, and installation is a separate operation.
      // See ModuleWorlds.md#selecting-a-symbolic-procedure-family.
      const bool procedure_selection = nature == Form::ResourceNature::procedure &&
          (importing || (form.kind == Form::Kind::using_resource && !form.using_symbol));
      if (procedure_selection)
        check_procedure_selection_identity(form, local, resource.symbol, fresh_symbols);
      if (importing) {
        if (!procedure_selection) {
          const auto destination = module_names().symbols.find(local);
          if (destination == module_names().symbols.end())
            fail(form.at, "select the destination symbol identity before importing its callable family");
          if (destination->second != resource.symbol)
            implementation_gap(form.at, "callable import into a different symbol identity; use explicit effect installation for a value");
        }
        if (!resource.family) fail(form.at, "symbol does not export a callable family");
      }
      const auto [alias, inserted] = module_names().symbols.emplace(local, resource.symbol);
      if (!inserted && alias->second != resource.symbol) fail(form.at, "conflicting symbol alias '" + local + "'");
      const auto [binding, fresh] = module_names().selected_symbols.try_emplace(local);
      auto &selection = binding->second;
      if (form.kind == Form::Kind::symbol_from) {
        if (fresh) selection.family = resource.family;
      } else if (resource.family) {
        const auto source = *resource.family;
        const auto previous = module_names().imports.find(source);
        const auto receiver = previous == module_names().imports.end() ? std::nullopt : std::optional{previous->second};
        if (!selection.identity_only && selection.family != source && selection.family != receiver)
          implementation_gap(form.at, "composition of distinct selected interpretations for one symbol");
        if (importing) {
          const auto family = module_names().families.find(resource.symbol);
          if (family != module_names().families.end() && family->second != receiver)
            implementation_gap(form.at, "composition of distinct selected interpretations for one symbol");
          resource.family = import_family(source, form.at);
          module_names().families.try_emplace(resource.symbol, *resource.family);
          selection.grants_extension = true;
          selection.family = resource.family;
        } else if (!selection.grants_extension) {
          selection.family = source;
        }
        selection.identity_only = false;
      }
    } else if (resource.kind == Resource::Kind::native_operation) {
      const auto [binding, inserted] = module_names().native_operations.emplace(local, *resource.native_operation);
      if (!inserted && binding->second != *resource.native_operation)
        fail(form.at, "conflicting native provider selection '" + local + "'");
    } else if (resource.kind == Resource::Kind::type) {
      const auto [alias, inserted] = module_names().types.emplace(local, resource.type);
      if (!inserted && alias->second != resource.type) fail(form.at, "conflicting type alias '" + local + "'");
    } else {
      if (importing) resource.family = import_family(*resource.family, form.at);
      const auto [alias, inserted] = module_names().callables.emplace(local, *resource.family);
      if (!inserted && alias->second != *resource.family)
        implementation_gap(form.at, "same-alias callable composition remains unproved");
      if (importing) module_names().imported_aliases.insert(local);
      else module_names().selected_callables.insert(local);
    }
    if (form.exporting) publish(local, resource, form.at);
  }

  void select_module_resource(const Form &form, size_t dependency,
                              const std::set<std::string_view> &fresh_symbols) {
    using Resource = ModuleNames::Resource;
    const auto &publisher = modules_.at(dependency);
    if (!publisher.publication_ready)
      throw std::logic_error("selecting resources from an incomplete module");
    if (form.kind != Form::Kind::using_resource && form.kind != Form::Kind::symbol_from &&
        form.kind != Form::Kind::import_resource && form.kind != Form::Kind::export_from_resource)
      implementation_gap(form.at, "mixin body redefinition remains unproved");
    if (form.selection == Form::Selection::defaults) {
      if (publisher.defaults.empty())
        fail(form.at, "source module has no default exports; select names explicitly or use '{..}'");
      for (const auto &selection : publisher.defaults) {
        const auto &resource = publisher.exports.at(selection.name);
        bind_module_resource(form, selection.name, resource, publication_nature(resource), fresh_symbols, selection.members);
      }
      return;
    }
    if (form.selection == Form::Selection::all_public) {
      for (const auto &name : publisher.public_order) {
        const auto &resource = publisher.exports.at(name);
        bind_module_resource(form, name, resource, publication_nature(resource), fresh_symbols);
      }
      return;
    }
    const auto found = publisher.exports.find(form.name);
    if (found == publisher.exports.end()) fail(form.at, "source module does not export '" + form.name + "'");
    if (form.symbolic != (found->second.kind == Resource::Kind::symbol))
      fail(form.at, "selected resource does not match the requested symbol or lexical category");
    bind_module_resource(form, form.alias, found->second, form.resource_nature, fresh_symbols);
  }

  void mark_default_export(std::vector<ModuleNames::DefaultSelection> &defaults, const std::string &name,
      std::optional<Form::ResourceNature> nature, const Location &at,
      const std::optional<std::vector<Token>> &selected_members = {}) const {
    const auto &publisher = module_names();
    const auto found = publisher.exports.find(name);
    if (found == publisher.exports.end() || publication_nature(found->second) != nature)
      fail(at, std::string("default selection requires a public name with the stated kind '") + (nature ? "@" : "") + name + "'");
    std::vector<ModuleNames::SelectedMember> members;
    if (selected_members) select_resource_members(found->second, *selected_members, at,
        [&](const Token &member, size_t ordinal) { members.push_back({member, ordinal}); });
    if (std::ranges::none_of(defaults, [&](const auto &entry) {
          return entry.name == name && std::ranges::equal(entry.members, members, [](const auto &a, const auto &b) {
            return a.spelling.text == b.spelling.text && a.ordinal == b.ordinal;
          });
        })) defaults.push_back({name, std::move(members)});
  }

  void publish_module(const SourceModule &source) {
    using Resource = ModuleNames::Resource;
    for (const auto &form : source.forms()) if (form->kind == Form::Kind::export_resource || form->publishes_definition()) {
      Resource resource{};
      if (form->symbolic) {
        const auto found = module_names().symbols.find(form->name);
        if (found == module_names().symbols.end()) fail(form->at, "unknown exported symbol '" + form->name + "'");
        resource.kind = Resource::Kind::symbol; resource.symbol = found->second;
        resource.family = symbol_family(module_names(), form->name);
      } else if (const auto found = module_names().callables.find(form->name); found != module_names().callables.end()) {
        resource.kind = Resource::Kind::callable; resource.family = found->second;
      } else if (const auto found = module_names().types.find(form->name); found != module_names().types.end()) {
        resource.kind = Resource::Kind::type; resource.type = found->second;
      } else if (const auto native = module_names().native_operations.find(form->name); native != module_names().native_operations.end()) {
        resource.kind = Resource::Kind::native_operation; resource.native_operation = native->second;
      } else fail(form->at, "unknown exported resource '" + form->name + "'");
      if (form->resource_nature && publication_nature(resource) != form->resource_nature)
        fail(form->at, "symbolic resource kind does not match its definition");
      publish(form->name, resource, form->at);
    }
    std::vector<ModuleNames::DefaultSelection> defaults;
    for (const auto &form : source.forms()) {
      if (form->default_exporting) {
        const auto &resource = module_names().exports.at(form->name);
        mark_default_export(defaults, form->name, publication_nature(resource), form->at);
      }
      for (const auto &mark : form->default_names)
        mark_default_export(defaults, mark.name.text, mark.nature, mark.name.at, mark.selected_members);
    }
    module_names().defaults = std::move(defaults);
  }

  Type register_enum(const Form &form, const Type &backing) {
    if (!integral(backing)) fail(form.at, "enum backing type must be a fixed-width integer");
    auto type = Type{Type::enumeration, form.name, program_.enums.size()};
    type.enum_backing = backing.kind;
    auto &domain = program_.enums.emplace_back(CheckedEnum{&form, backing, {}, {}});
    const bool signed_backing = backing == Type::integer;
    const auto maximum = backing == Type::uint8 ? std::uint64_t{UINT8_MAX}
        : backing == Type::uint32 ? std::uint64_t{UINT32_MAX}
        : signed_backing ? std::uint64_t{INT64_MAX} : std::uint64_t{UINT64_MAX};
    std::optional<std::uint64_t> previous;
    std::set<std::uint64_t> codes;
    for (const auto &source : form.enum_members) {
      const auto &name = source.name;
      const auto ordinal = domain.members.size();
      if (domain.spellings.contains(name.text)) fail(name.at, "duplicate enum member '" + name.text + "'");
      CheckedEnum::Member member{0, ordinal};
      if (source.initializer && source.initializer->kind == TokenKind::identifier) {
        const auto original = domain.spellings.find(source.initializer->text);
        if (original == domain.spellings.end()) fail(source.initializer->at, "enum member identity requires an earlier member");
        member = domain.members[original->second];
      } else {
        if (source.initializer) {
          const auto &token = *source.initializer;
          auto digits = std::string_view(token.text);
          auto literal_maximum = std::uint64_t{UINT64_MAX};
          const bool has_suffix = digits.find('u') != std::string_view::npos;
          if (digits.ends_with("u8")) { digits.remove_suffix(2); literal_maximum = UINT8_MAX; }
          else if (digits.ends_with("u32")) { digits.remove_suffix(3); literal_maximum = UINT32_MAX; }
          else if (digits.ends_with("u64")) digits.remove_suffix(3);
          const auto [end, error] = std::from_chars(digits.data(), digits.data() + digits.size(), member.code);
          if (error != std::errc{} || end != digits.data() + digits.size() ||
              member.code > literal_maximum ||
              (source.negative ? !signed_backing || has_suffix || member.code > maximum + 1 : member.code > maximum))
            fail(token.at, "enum member code is outside the range of '" + type_name(backing) + "'");
          if (source.negative) member.code = std::uint64_t{0} - member.code;
        } else if (previous) {
          if (*previous == maximum) fail(name.at, "automatic enum member code overflows '" + type_name(backing) + "'");
          member.code = *previous + 1;
        }
        if (!codes.insert(member.code).second) fail(name.at, "duplicate enum member code");
        previous = member.code;
      }
      domain.members.push_back(member);
      domain.spellings.emplace(name.text, ordinal);
    }
    return type;
  }

  using TypeSynonymTargets = std::map<std::string_view, const Form *>;

  static TypeSynonymTargets complete_type_synonym_targets(const SourceModule &source) {
    TypeSynonymTargets definitions;
    for (const auto &form : source.forms()) if (form->kind == Form::Kind::type_synonym) {
      const auto &target = *form->result_type;
      if (target.target || target.kind == Type::procedure || target.kind == Type::region)
        implementation_gap(form->at, "type synonym target other than an ordinary type name");
      if (!definitions.emplace(form->name, form.get()).second)
        fail(form->at, "duplicate type binding '" + form->name + "'");
    }
    TypeSynonymTargets completed;
    if (definitions.empty()) return completed;
    for (const auto &form : source.forms()) if (form->kind == Form::Kind::type_synonym &&
                                               !completed.contains(form->name)) {
      std::vector<const Form *> path;
      std::set<const Form *> active;
      const Form *terminal = form.get();
      while (true) {
        if (const auto cached = completed.find(terminal->name); cached != completed.end()) {
          terminal = cached->second;
          break;
        }
        if (!active.insert(terminal).second)
          fail(terminal->at, "type synonym cycle has no defining type");
        path.push_back(terminal);
        const auto &target = *terminal->result_type;
        if (target.kind != Type::named) break;
        const auto next = definitions.find(target.name);
        if (next == definitions.end()) break;
        terminal = next->second;
      }
      for (const auto *synonym : path) completed.emplace(synonym->name, terminal);
    }
    return completed;
  }

  Type resolve_declaration_head(Type type, const TypeSynonymTargets &synonyms) const {
    if (type.kind == Type::named && !type.target)
      if (const auto synonym = synonyms.find(type.name); synonym != synonyms.end())
        return resolve_type(*synonym->second->result_type);
    return resolve_type(std::move(type));
  }

  void register_abstract_types(const SourceModule &source, const TypeSynonymTargets &synonyms) {
    const auto first = program_.abstract_types.size();
    for (const auto &form : source.forms()) if (form->kind == Form::Kind::abstract_type) {
      const Type type{Type::abstract_type, form->name, program_.abstract_types.size()};
      if (!module_names().types.emplace(form->name, type).second)
        fail(form->at, "duplicate type binding '" + form->name + "'");
      program_.abstract_types.push_back({form.get()});
    }
    for (auto &type : std::span(program_.abstract_types).subspan(first)) if (type.source->parent_type) {
      const auto parent = resolve_declaration_head(*type.source->parent_type, synonyms);
      if (parent.kind != Type::abstract_type)
        fail(type.source->at, "an abstract parent must name an abstract type");
      type.parent = parent.identity;
    }
    enum class Visit { unseen, active, complete };
    std::vector<Visit> visited(program_.abstract_types.size() - first);
    for (size_t i = first; i < program_.abstract_types.size(); ++i) {
      std::vector<size_t> path;
      for (std::optional<size_t> type = i; type && *type >= first; type = program_.abstract_types[*type].parent) {
        auto &state = visited[*type - first];
        if (state == Visit::complete) break;
        if (state == Visit::active) fail(program_.abstract_types[*type].source->at, "abstract parent declarations form a cycle");
        state = Visit::active;
        path.push_back(*type);
      }
      for (const auto type : path) visited[type - first] = Visit::complete;
    }
  }
  void register_sum_type(const Form &form) {
    const auto identity = program_.sums.size();
    if (!module_names().types.emplace(form.name, Type{Type::sum, form.name, identity}).second)
      fail(form.at, "duplicate type binding '" + form.name + "'");
    auto &sum = program_.sums.emplace_back(CheckedSum{&form, {}, false, module_});
    for (const auto &alternative : form.sum_alternatives) {
      bind_sum_constructor(alternative.name, {identity, sum.alternatives.size()});
      sum.alternatives.push_back({&alternative, {}});
    }
  }

  std::optional<SumConstructor> regional_initializer(const Form &form) const {
    if (form.symbolic || form.operation) return {};
    const auto selected = module_names().constructors.find(form.name);
    if (selected == module_names().constructors.end()) return {};
    const auto constructor = selected->second;
    const auto &sum = program_.sums.at(constructor.sum);
    if (!sum.regional_only) return {};
    if (sum.module != module_ || form.c_linkage || form.exporting || !form.body ||
        !form.type_parameters.empty() || form.owner_parameters.size() != 1 ||
        form.owner_parameters.front().read_only ||
        (!form.parameters.empty() && form.parameters.front().here))
      implementation_gap(form.at, "initializer outside the local regional sum constructor profile");
    return constructor;
  }

  template <typename Check>
  static bool check_schema_part(CompilationDiagnostics *diagnostics, const Check &check) {
    if (diagnostics && !diagnostics->continue_work()) return false;
    try {
      check();
      return true;
    } catch (const LocatedFailure &failure) {
      if (!diagnostics) throw;
      diagnostics->append(failure.diagnostic);
      return false;
    }
  }

  bool resolve_sum_fields(CheckedSum &sum, CompilationDiagnostics *diagnostics) {
    if (sum.source->parent_type)
      implementation_gap(sum.source->at, "sum conformance to a parent type");
    const auto &nominal = module_names().types.at(sum.source->name);
    bool valid = true;
    for (auto &alternative : sum.alternatives) {
      std::set<std::string> names;
      for (const auto &field : alternative.source->fields) {
        const auto checked = check_schema_part(diagnostics, [&] {
          if (!names.insert(field.name).second)
            fail(field.at, "duplicate field '" + field.name + "' in '" + alternative.source->name.text + "'");
          auto type = resolve_type(field.type);
          const bool self_edge = field.declaration_kind == "ref" && type == nominal;
          if (!self_edge && ((!field.declaration_kind.empty() && field.declaration_kind != "val") ||
              (!scalar_type(type) && type.kind != Type::product && type.kind != Type::sum)))
            implementation_gap(field.at, "sum payload fields outside direct bits and same-sum regional refs");
          alternative.fields.push_back({&field, std::move(type), self_edge ? PlaceKind::reference : PlaceKind::value});
          sum.regional_only |= self_edge;
        });
        valid &= checked;
        if (diagnostics && !diagnostics->complete()) return false;
      }
    }
    return valid;
  }

  void check_schema_field(CheckedSchema &schema, const Parameter &field) {
    const auto storage = place_kind(field.declaration_kind.empty() || field.declaration_kind == "val",
                                    field.declaration_kind == "ref");
    auto type = resolve_type(field.type);
    require_ordinary_callable_type(type, field.at);
    if (type.kind == Type::builder_storage) {
      if (schema.source->kind != Form::Kind::data || schema.source->c_data || schema.source->parameters.size() != 1 ||
          storage != PlaceKind::value || !schema.bits)
        fail(field.at, "text builder storage requires the sole inline marker of a lexical product");
      schema.lexical_builder = true;
      schema.bits = false;
      return;
    }
    if (schema.source->c_data && storage != PlaceKind::value)
      fail(field.at, "data \"C\" requires by-value bits fields");
    reject_c_type_payload(type, field.at);
    if (type.kind == Type::region_schema) fail(field.at, "a region schema does not name a field payload type");
    const bool text_place = !schema.source->c_data && schema.source->kind == Form::Kind::data &&
        storage == PlaceKind::value && type == Type::string;
    if (!scalar_type(type) && type.kind != Type::product && type.kind != Type::sum && !text_place)
      implementation_gap(field.at, "field representation for type '" + type_name(type) + "'");
    if (type.kind == Type::sum && storage != PlaceKind::value)
      implementation_gap(field.at, "stored sum reference fields");
    schema.has_enum_storage |= type.kind == Type::enumeration;
    schema.bits &= storage == PlaceKind::value && !text_place;
    schema.fields.push_back({&field, std::move(type), storage});
  }

  bool check_schema_fields(CheckedSchema &schema, CompilationDiagnostics *diagnostics) {
    if (schema.source->native_owner_type) {
      const auto owner = resolve_type(*schema.source->native_owner_type);
      if (owner.kind != Type::c_type_marker || program_.schemas.at(owner.identity).source->native_owner_type)
        fail(schema.source->at, "a borrowed native handle requires a fieldless owning CType");
      schema.native_owner = owner.identity;
    }
    std::set<std::string> names;
    bool valid = true;
    for (const auto &field : schema.source->parameters) {
      const auto checked = check_schema_part(diagnostics, [&] {
        if (!names.insert(field.name).second)
          fail(field.at, "duplicate field '" + field.name + "' in '" + schema.source->name + "'");
        check_schema_field(schema, field);
      });
      valid &= checked;
      if (diagnostics && !diagnostics->complete()) return false;
    }
    return valid;
  }

  using UnavailableLayouts = std::set<std::pair<Type::Kind, size_t>>;

  bool register_nominal_types(const SourceModule &source, CompilationDiagnostics *diagnostics) {
    const auto synonyms = complete_type_synonym_targets(source);
    register_abstract_types(source, synonyms);
    const auto first = program_.schemas.size();
    const auto first_array = program_.arrays.size();
    const auto first_sum = program_.sums.size();
    for (const auto &form : source.forms()) {
      if (form->kind == Form::Kind::sum_data) {
        register_sum_type(*form);
        continue;
      }
      const bool product = form->kind == Form::Kind::data;
      const bool array = form->kind == Form::Kind::array;
      if (form->kind == Form::Kind::enumeration) {
        const auto type = register_enum(*form, resolve_declaration_head(*form->result_type, synonyms));
        if (!module_names().types.emplace(form->name, type).second)
          fail(form->at, "duplicate type binding '" + form->name + "'");
        continue;
      }
      const bool marker = form->kind == Form::Kind::native_handle_type;
      if (!product && !array && !marker && form->kind != Form::Kind::region_definition) continue;
      const auto kind = array ? Type::array : marker ? Type::c_type_marker : product ? Type::product : Type::region_schema;
      if (form->parent_type && (!product || form->c_data))
        implementation_gap(form->at, "a parent type on this data layout");
      std::optional<size_t> abstract_parent;
      bool exception = false, checked = false;
      if (form->parent_type) {
        const auto parent = resolve_declaration_head(*form->parent_type, synonyms);
        checked = parent == exception_type(0);
        exception = checked || parent == exception_type(error_root_id);
        if (parent.kind == Type::abstract_type) abstract_parent = parent.identity;
        else if (!exception) {
          if (parent.kind == Type::exception)
            implementation_gap(form->at, "library-defined exception inheritance beyond the Exception and Error roots");
          fail(form->at, "a data parent must name an abstract type, Exception or Error");
        }
      }
      const auto type = exception ? program_.append_exception_type(*form, checked)
          : Type{kind, form->name, array ? program_.arrays.size() : program_.schemas.size()};
      if (!module_names().types.emplace(form->name, type).second)
        fail(form->at, "duplicate type binding '" + form->name + "'");
      if (array) program_.arrays.push_back({form.get(), *form->index_type, *form->result_type});
      else if (!exception) {
        program_.schemas.push_back({form.get(), {}, kind == Type::product});
        program_.schemas.back().abstract_parent = abstract_parent;
      }
    }
    // Nominal declarations and imported types supply identities to every
    // completed synonym before any field layout or callable signature uses it.
    for (const auto &form : source.forms()) if (form->kind == Form::Kind::type_synonym) {
      const auto &target = *synonyms.at(form->name);
      const auto canonical = resolve_type(*target.result_type);
      if (canonical.kind == Type::named)
        fail(target.at, "unknown type synonym target '" + canonical.name + "'");
      if (!module_names().types.emplace(form->name, canonical).second)
        fail(form->at, "duplicate type binding '" + form->name + "'");
    }
    UnavailableLayouts unavailable;
    for (size_t id = first_sum; id < program_.sums.size(); ++id) {
      auto staged = program_.sums[id];
      if (resolve_sum_fields(staged, diagnostics)) program_.sums[id] = std::move(staged);
      else unavailable.emplace(Type::sum, id);
      if (diagnostics && !diagnostics->complete()) return false;
    }
    struct ArrayDraft {
      CheckedArray definition;
      bool index_ready;
      bool element_ready;
    };
    std::vector<ArrayDraft> arrays;
    for (const auto &array : std::span(program_.arrays).subspan(first_array)) {
      auto &draft = arrays.emplace_back(ArrayDraft{array, false, false});
      draft.index_ready = check_schema_part(diagnostics, [&] {
        auto index = resolve_type(array.index);
        reject_c_type_payload(index, array.source->at);
        if (!integral(index))
          implementation_gap(array.source->at, "array index domain '" + type_name(index) + "'");
        draft.definition.index = std::move(index);
      });
      if (diagnostics && !diagnostics->complete()) return false;
      draft.element_ready = check_schema_part(diagnostics, [&] {
        auto element = resolve_type(array.element);
        reject_c_type_payload(element, array.source->at);
        if (element.kind == Type::procedure)
          fail(array.source->at, "callable views require lexical storage and cannot be array elements");
        if (!scalar_type(element) && element.kind != Type::product && element.kind != Type::sum)
          implementation_gap(array.source->at, "array element type '" + type_name(element) + "'");
        draft.definition.element = std::move(element);
      });
      if (diagnostics && !diagnostics->complete()) return false;
    }
    for (size_t id = first; id < program_.schemas.size(); ++id) {
      auto staged = program_.schemas[id];
      const auto &form = *staged.source;
      const bool recoverable = form.kind == Form::Kind::data && !form.c_data && !form.parent_type;
      if (!recoverable && diagnostics && !diagnostics->continue_work()) return false;
      if (check_schema_fields(staged, recoverable ? diagnostics : nullptr)) program_.schemas[id] = std::move(staged);
      else unavailable.emplace(Type::product, id);
      if (diagnostics && !diagnostics->complete()) return false;
    }
    if (!check_nominal_layouts(first, first_sum, unavailable, diagnostics)) return false;
    bool valid = unavailable.empty();
    for (size_t offset = 0; offset < arrays.size(); ++offset) {
      auto &draft = arrays[offset];
      const auto &element = draft.definition.element;
      if (draft.element_ready && !unavailable.contains({element.kind, element.identity}) &&
          (element.kind == Type::product || element.kind == Type::sum)) {
        draft.element_ready = check_schema_part(diagnostics, [&] {
          if (element.kind == Type::sum && !program_.bits_payload(element))
            implementation_gap(draft.definition.source->at, "regional reference-bearing sum array elements");
          if (program_.builder_type(element))
            implementation_gap(draft.definition.source->at, "storing a lexical text builder in an array");
        });
        if (diagnostics && !diagnostics->complete()) return false;
      }
      if (draft.index_ready && draft.element_ready && !unavailable.contains({element.kind, element.identity}))
        program_.arrays[first_array + offset] = std::move(draft.definition);
      else valid = false;
    }
    return valid;
  }

  bool check_nominal_layouts(size_t first_product, size_t first_sum, UnavailableLayouts &unavailable,
                             CompilationDiagnostics *diagnostics) {
    struct Layout {
      Type type;
      const Form *source;
      size_t pending = 0;
      std::vector<size_t> dependents = {};
      bool shape_ready = true;
      bool blocked = false;
    };
    std::vector<Layout> layouts;
    for (size_t id = first_product; id < program_.schemas.size(); ++id)
      layouts.push_back({Type{Type::product, {}, id}, program_.schemas[id].source});
    const auto sum_start = layouts.size();
    for (size_t id = first_sum; id < program_.sums.size(); ++id)
      layouts.push_back({Type{Type::sum, {}, id}, program_.sums[id].source});

    const auto local_layout = [&](const Type &type) -> std::optional<size_t> {
      if (type.kind == Type::product && type.identity >= first_product) return type.identity - first_product;
      if (type.kind == Type::sum && type.identity >= first_sum) return sum_start + type.identity - first_sum;
      return {};
    };
    // Only inline fields require the child's layout. Stored targets can be recursive.
    for (size_t i = 0; i < layouts.size(); ++i) {
      auto &layout = layouts[i];
      program_.visit_inline_fields(layout.type, [&](const auto &field) {
        if (const auto dependency = local_layout(field.type)) {
          ++layout.pending;
          layouts[*dependency].dependents.push_back(i);
        }
      });
    }
    const auto block_layout = [&](size_t first) {
      std::deque<size_t> blocked{first};
      while (!blocked.empty()) {
        const auto id = blocked.front();
        blocked.pop_front();
        auto &layout = layouts[id];
        if (layout.blocked) continue;
        layout.blocked = true;
        unavailable.emplace(layout.type.kind, layout.type.identity);
        for (const auto dependent : layout.dependents) blocked.push_back(dependent);
      }
    };
    for (auto &layout : layouts)
      layout.shape_ready = !unavailable.contains({layout.type.kind, layout.type.identity});
    for (size_t i = 0; i < layouts.size(); ++i)
      if (!layouts[i].shape_ready) block_layout(i);

    std::deque<size_t> ready;
    for (size_t i = 0; i < layouts.size(); ++i)
      if (!layouts[i].blocked && !layouts[i].pending) ready.push_back(i);
    while (!ready.empty()) {
      const auto next = ready.front();
      ready.pop_front();
      const auto &layout = layouts[next];
      if (layout.blocked) continue;
      bool valid = true;
      program_.visit_inline_fields(layout.type, [&](const auto &field) {
        const auto &type = field.type;
        if (type.kind != Type::product && type.kind != Type::sum) return;
        valid &= check_schema_part(diagnostics, [&] {
          if (program_.builder_type(type))
            implementation_gap(field.source->at, "storing a lexical text builder in a data field");
          if (program_.bits_payload(type)) return;
          if (type.kind == Type::product) {
            if (layout.source->c_data) fail(field.source->at, "data \"C\" requires recursively bits-only fields");
            implementation_gap(field.source->at, "inline field with a reference-bearing product");
          }
          implementation_gap(field.source->at, "inline field with a reference-bearing sum");
        });
      });
      if (diagnostics && !diagnostics->complete()) return false;
      if (layout.type.kind == Type::product) {
        auto &schema = program_.schemas[layout.type.identity];
        for (const auto &field : schema.fields) {
          if (!field.stores_target() || field.type.kind != Type::product) continue;
          const auto child = local_layout(field.type);
          if (child && layouts[*child].blocked) continue;
          valid &= check_schema_part(diagnostics, [&] {
            if (program_.builder_type(field.type))
              implementation_gap(field.source->at, "storing a lexical text builder in a data field");
          });
          if (diagnostics && !diagnostics->complete()) return false;
        }
        if (valid) {
          bool has_sum = schema.has_sum_storage;
          bool has_enum = schema.has_enum_storage;
          program_.visit_inline_fields(layout.type, [&](const auto &field) {
            if (field.type.kind == Type::sum) has_sum = true;
            if (field.type.kind == Type::product) {
              const auto &nested = program_.schemas.at(field.type.identity);
              has_sum |= nested.has_sum_storage;
              has_enum |= nested.has_enum_storage;
            }
          });
          if (schema.source->c_data && (has_sum || has_enum)) {
            valid = check_schema_part(diagnostics, [&] {
              if (has_sum)
                implementation_gap(schema.source->at, "sum fields in a foreign record require a checked storage boundary");
              implementation_gap(schema.source->at, "enumeration fields in a foreign record require a checked storage boundary");
            });
          }
          if (valid) {
            schema.has_sum_storage = has_sum;
            schema.has_enum_storage = has_enum;
            schema.layout_checked = true;
          }
        }
      }
      if (diagnostics && !diagnostics->complete()) return false;
      if (!valid) {
        block_layout(next);
        continue;
      }
      for (const auto dependent : layout.dependents)
        if (!layouts[dependent].blocked && !--layouts[dependent].pending) ready.push_back(dependent);
    }
    const auto unfinished_shape = [](const Layout &layout) { return layout.shape_ready && layout.pending; };
    if (std::ranges::none_of(layouts, unfinished_shape)) return true;

    // Find components using both directions of the checked inline edges.
    // A blocked layout can still prove a cycle through its own fields.
    struct Visit {
      size_t id;
      size_t next_dependent = 0;
    };
    std::vector<bool> visited(layouts.size());
    std::vector<size_t> finished;
    std::vector<Visit> visits;
    for (size_t i = 0; i < layouts.size(); ++i) {
      if (visited[i] || !unfinished_shape(layouts[i])) continue;
      visited[i] = true;
      visits.push_back({i});
      while (!visits.empty()) {
        auto &visit = visits.back();
        const auto &dependents = layouts[visit.id].dependents;
        if (visit.next_dependent == dependents.size()) {
          finished.push_back(visit.id);
          visits.pop_back();
          continue;
        }
        const auto next = dependents[visit.next_dependent++];
        if (visited[next] || !unfinished_shape(layouts[next])) continue;
        visited[next] = true;
        visits.push_back({next});
      }
    }

    const auto earlier = [&](size_t left, size_t right) {
      return layouts[left].source->at.span.first < layouts[right].source->at.span.first;
    };
    std::fill(visited.begin(), visited.end(), false);
    std::vector<size_t> members, cycles;
    for (auto it = finished.rbegin(); it != finished.rend(); ++it) {
      if (visited[*it]) continue;
      visited[*it] = true;
      members.push_back(*it);
      size_t first = *it, count = 0;
      bool self_edge = false;
      while (!members.empty()) {
        const auto current = members.back();
        members.pop_back();
        ++count;
        if (earlier(current, first)) first = current;
        program_.visit_inline_fields(layouts[current].type, [&](const auto &field) {
          const auto next = local_layout(field.type);
          if (!next || !unfinished_shape(layouts[*next])) return;
          self_edge |= *next == current;
          if (visited[*next]) return;
          visited[*next] = true;
          members.push_back(*next);
        });
      }
      if (count > 1 || self_edge) cycles.push_back(first);
    }
    std::ranges::sort(cycles, earlier);
    for (const auto first : cycles) {
      const auto &cycle = *layouts[first].source;
      check_schema_part(diagnostics, [&] {
        fail(cycle.at, "inline fields of '" + cycle.name + "' have no finite layout");
      });
      if (diagnostics && !diagnostics->complete()) return false;
      block_layout(first);
    }
    return true;
  }

  void check_native_array(const Type &type, bool read_only, const Location &at) const {
    const auto &array = program_.arrays.at(type.identity);
    const bool readonly_bits = read_only && numeric(array.element);
    if (array.index != Type::uint64 || (!readonly_bits &&
        (!indexed_product(array.element, at) || program_.schemas.at(array.element.identity).fields.empty() ||
         !std::ranges::all_of(program_.schemas.at(array.element.identity).fields, [](const auto &field) {
           return cod_provider_member_name(field.source->name.c_str());
         })))) implementation_gap(at, "C array loans outside the UInt64-indexed scalar and numeric record profile");
  }
  void constrain_native_owner(std::map<std::string, size_t> &roles, const Parameter &parameter, const Type &type) const {
    const auto owner = program_.schemas.at(type.identity).native_owner.value_or(type.identity);
    const auto [known, inserted] = roles.try_emplace(parameter.owner, owner);
    if (!inserted && known->second != owner)
      fail(parameter.at, "native owner role '" + parameter.owner + "' requires one CType, found '" +
           program_.schemas.at(known->second).source->name + "' and '" + program_.schemas.at(owner).source->name + "'");
  }
  std::vector<CheckedNativeInput> checked_native_inputs(const Form &form, const std::string &boundary,
                                                        bool readonly_arrays = false,
                                                        std::optional<size_t> retained_parent = {}) const {
    std::vector<CheckedNativeInput> inputs;
    std::set<std::string> names;
    std::map<std::string, size_t> native_owners;
    for (const auto &parameter : form.parameters) {
      if (!names.insert(parameter.name).second) fail(parameter.at, "duplicate " + boundary + " parameter name");
      if (parameter.selects_caller_place())
        implementation_gap(parameter.at, "a caller's direct place at a C boundary");
      if (parameter.here || parameter.variadic || parameter.automatic_conversion)
        implementation_gap(parameter.at, "here, variadic or conversion parameters of a " + boundary);
      const auto type = resolve_type(parameter.type);
      std::optional<size_t> role;
      const bool handle = parameter.declaration_kind == "handle";
      const bool array = readonly_arrays && parameter.declaration_kind == "ref" && type.kind == Type::array;
      const bool text = text_type(type) && (form.body || readonly_arrays);
      if (retained_parent == inputs.size()) {
        if (type.kind != Type::c_type_marker) fail(parameter.at, "C constructor parent requires a 'handle type' name");
        inputs.push_back({type, {}});
        continue;
      }
      if (handle || array || text) {
        const auto owner = std::ranges::find(form.owner_parameters, parameter.owner, &OwnerParameter::name);
        if (owner == form.owner_parameters.end() || owner->root || owner->explicit_parameter)
          fail(parameter.at, "C borrow requires a declared owner");
        if (handle && type.kind != Type::c_type_marker)
          fail(parameter.at, "handle parameter requires a 'handle type' name");
        if (handle) constrain_native_owner(native_owners, parameter, type);
        if (text && (argument_mode(parameter) != ArgumentMode::borrowed_ref || !owner->read_only || !parameter.read_only))
          fail(parameter.at, "C text parameters require readonly loans under a const owner");
        if (array) {
          if (!owner->read_only || !parameter.read_only)
            implementation_gap(parameter.at, "C operation array loans other than readonly views under a const owner");
          check_native_array(type, true, parameter.at);
        }
        if (readonly_arrays && handle && parameter.read_only && !owner->read_only)
          fail(parameter.at, "a readonly native handle cannot establish a writable provider owner");
        role = std::distance(form.owner_parameters.begin(), owner);
      } else {
        if (!parameter.owner.empty() || (parameter.declaration_kind != "val" && !parameter.declaration_kind.empty()))
          fail(parameter.at, boundary + (readonly_arrays ? " parameters receive bits, a borrowed handle or a readonly array view"
                                                         : " parameters receive bits or a borrowed handle"));
        reject_c_type_payload(type, parameter.at);
        if (type.kind == Type::exception) fail(parameter.at, "an exception object is not a " + boundary + " bits argument");
        if (!scalar_type(type))
          implementation_gap(parameter.at, boundary + " bits arguments outside the generated scalar profile");
      }
      inputs.push_back({type, role});
    }
    for (size_t i = 0; i < form.owner_parameters.size(); ++i)
      if (std::ranges::none_of(inputs, [&](const auto &input) { return input.owner == i; }))
        implementation_gap(form.owner_parameters[i].at, "a " + boundary + " owner without a supplying borrow");
    return inputs;
  }
  void register_native_constructor(const Form &form, const SourceModule &source) {
    const auto handle = resolve_type(Type{Type::named, form.name});
    if (handle.kind != Type::c_type_marker)
      implementation_gap(form.at, "a C operation without an explicit result or a fieldless CType constructor");
    const auto *declaration = program_.schemas.at(handle.identity).source;
    if (std::ranges::none_of(source.forms(), [&](const auto &candidate) { return candidate.get() == declaration; }))
      implementation_gap(form.at, "C constructors declared outside their handle type's defining module");
    if (form.symbolic || form.operation || form.result_type || !form.type_parameters.empty() || form.explicit_captures)
      fail(form.at, "a C handle constructor requires a named, bodyless declaration without an explicit result or type parameters");
    if (std::ranges::any_of(program_.native_constructors, [&](const auto &existing) { return existing.handle == handle; }))
      implementation_gap(form.at, "multiple C constructor declarations for '" + handle.name + "'");
    const auto within = std::ranges::find_if(form.parameters, [](const auto &parameter) {
      return parameter.declaration_kind == "handle" && parameter.name == "within" && parameter.owner.empty();
    });
    std::optional<size_t> retained_parent;
    if (within != form.parameters.end()) retained_parent = std::distance(form.parameters.begin(), within);
    CheckedNativeConstructor constructor{
        &form, handle, checked_native_inputs(form, "C constructor", false, retained_parent), {}, {}, retained_parent};
    if (const auto parent = program_.schemas.at(handle.identity).native_owner) {
      if (retained_parent) fail(within->at, "a C constructor parent requires an owning handle type");
      constructor.parent = Type{Type::c_type_marker, program_.schemas.at(*parent).source->name, *parent};
    }
    if (!form.throws_types.empty()) {
      const auto type = resolve_type(form.throws_types.front());
      if (!nominal_exception(type)) implementation_gap(form.at, "C constructor failures other than a concrete library-defined exception");
      const auto &schema = scalar_exception_schema(program_, type, form.at);
      if (schema.has_enum_storage)
        implementation_gap(form.at, "C constructor failure payload with enumeration storage");
      for (const auto &field : schema.fields)
        if (!cod_provider_member_name(field.source->name.c_str())) implementation_gap(field.source->at, "a reserved C name in a generated failure payload");
      constructor.failure = type;
    }
    program_.native_constructors.push_back(std::move(constructor));
  }
  void add_native_operation(CheckedNativeOperation native) {
    const auto &form = *native.contract.source;
    if (!module_names().native_operations.emplace(form.name, program_.native_operations.size()).second)
      fail(form.at, "conflicting native provider name '" + form.name + "'");
    program_.native_operations.push_back(std::move(native));
  }
  void register_native_call(const Form &form) {
    const auto result = form.result_type ? resolve_type(*form.result_type) : Type{Type::void_type};
    if (form.symbolic || form.operation || form.explicit_captures || !form.type_parameters.empty() ||
        !form.result_owner.empty() || !form.throws_types.empty() ||
        (result != Type::boolean && result != Type::uint8 && result != Type::uint64 &&
         result != Type::void_type && result.kind != Type::enumeration))
      implementation_gap(form.at, "C operations outside the nonthrowing bool, UInt8, UInt64, enum or void result profile");
    CheckedNativeOperation native;
    native.contract.source = &form;
    native.contract.module = module_;
    native.contract.result.type = result;
    native.inputs = checked_native_inputs(form, "C operation", true);
    add_native_operation(std::move(native));
  }
  void register_native_here(const Form &form) {
    const auto result = resolve_type(*form.result_type);
    if (form.symbolic || form.operation || form.explicit_captures || !form.type_parameters.empty() ||
        !form.result_owner.empty() || !form.throws_types.empty() ||
        (result != Type::uint8 && result.kind != Type::enumeration) ||
        form.parameters.size() != 2 || form.owner_parameters.size() != 1)
      implementation_gap(form.at, "C here providers outside the array loan and scalar feedback profile");
    const auto &role = form.owner_parameters.front();
    const auto &handle = form.parameters.back();
    const auto type = resolve_type(handle.type);
    if (role.root || role.explicit_parameter || handle.declaration_kind != "handle" ||
        handle.here || handle.variadic || handle.automatic_conversion || handle.owner != role.name || type.kind != Type::c_type_marker)
      implementation_gap(form.at, "C here provider requires one borrowed handle with an established owner");
    if (handle.read_only && !role.read_only)
      fail(handle.at, "a readonly native handle cannot establish a writable provider owner");
    if (program_.schemas.at(type.identity).native_owner)
      implementation_gap(handle.at, "native here operations supplied by an attached child handle");
    CheckedNativeOperation native;
    native.inputs.push_back({type, 0});
    auto &contract = native.contract;
    contract.source = &form;
    contract.module = module_;
    contract.result.type = result;
    contract.here.push_back(here_contract(contract, form.parameters.front()));
    const auto &here = contract.here.front();
    if (here.inputs.empty() || here.labelled || !here.here.empty() || here.feedback || here.result.owner)
      implementation_gap(form.at, "C here provider requires an ordered array callback with a scalar feedback value");
    if (here.result.type != result)
      fail(form.at, "C here feedback and provider result require the same scalar type");
    for (const auto &input : here.inputs) {
      if (input.type.kind != Type::array || input.mode != ArgumentMode::borrowed_ref || input.owner != 1)
        implementation_gap(form.at, "C here array inputs must share the handle's owner");
      if (role.read_only && !input.read_only)
        fail(form.at, "a readonly native owner cannot supply a writable array loan");
      check_native_array(input.type, input.read_only, form.at);
    }
    add_native_operation(std::move(native));
  }
  void check_native_type_names() const {
    std::map<std::string_view, size_t> handles;
    const auto record = [&](const Type &type, const Location &at) {
      const auto [entry, inserted] = handles.try_emplace(type.name, type.identity);
      if (!inserted && entry->second != type.identity)
        implementation_gap(at, "distinct C handle types with the same generated C name '" + type.name + "'");
    };
    for (const auto &constructor : program_.native_constructors) {
      record(constructor.handle, constructor.source->at);
      if (constructor.parent) record(*constructor.parent, constructor.source->at);
      for (const auto &input : constructor.inputs)
        if (input.type.kind == Type::c_type_marker) record(input.type, constructor.source->at);
    }
    std::set<std::string_view> entries;
    for (const auto &entry : program_.native_entries) {
      const auto &form = *program_.methods.at(entry.method).source;
      if (!entries.insert(form.name).second) fail(form.at, "conflicting C entry name '" + form.name + "'");
      for (const auto &input : entry.inputs) if (input.owner) record(input.type, form.at);
    }
    std::set<std::string_view> operations;
    for (const auto &native : program_.native_operations) {
      const auto &form = *native.contract.source;
      if (!operations.insert(form.name).second) fail(form.at, "conflicting C provider name '" + form.name + "'");
      for (const auto &input : native.inputs)
        if (input.type.kind == Type::c_type_marker) record(input.type, form.at);
    }
  }

  static std::optional<cod_lang_export_kind> text_storage_kind(std::string_view name) {
#define COD_LANG_TEXT_KIND(key, spelling, kind, id) if (name == #spelling) return COD_LANG_ ## kind ## _EXPORT;
    COD_LANG_TEXT_INTRINSICS(COD_LANG_TEXT_KIND)
#undef COD_LANG_TEXT_KIND
    return {};
  }
  bool register_module(const SourceModule &source, std::optional<size_t> &print_symbol,
                       CompilationDiagnostics *diagnostics) {
    const auto &forms = source.forms();
    std::set<std::string_view> fresh_symbols;
    for (const auto &form : forms)
      if (form->kind == Form::Kind::symbol) fresh_symbols.insert(form->name);
    for (const auto &form : forms)
      if (const auto dependency = source.dependencies.find(form.get()); dependency != source.dependencies.end())
        select_module_resource(*form, dependency->second, fresh_symbols);
    for (const auto &form : forms) {
      if (form->path.starts_with("std:") && form->selection != Form::Selection::named) {
        if (form->selection == Form::Selection::defaults)
          fail(form->at, "intrinsic module has no default exports; select names explicitly");
        implementation_gap(form->at, "complete selection of an intrinsic catalog module");
      }
      if ((form->kind == Form::Kind::using_resource || form->kind == Form::Kind::import_resource) &&
          form->path == COD_LANG_TEXT_STORAGE_MODULE && !form->using_symbol) {
        if (form->selected_members) implementation_gap(form->at, "member selection outside an enum");
        const auto kind = text_storage_kind(form->name);
        if (!kind) fail(form->at, "unknown text storage selection '" + form->name + "'");
        if (kind != COD_LANG_BUILDER_STORAGE_EXPORT) {
          if (form->exporting) implementation_gap(form->at, "re-exporting a private text storage primitive");
          if (!module_names().text_primitives.emplace(form->alias, *kind).second)
            fail(form->at, "duplicate text storage selection");
          continue;
        }
        const Type type{Type::builder_storage, form->name};
        if (!module_names().types.emplace(form->alias, type).second)
          fail(form->at, "duplicate text storage selection");
        if (form->exporting) {
          ModuleNames::Resource resource{};
          resource.kind = ModuleNames::Resource::Kind::type;
          resource.type = type;
          publish(form->alias, resource, form->at);
        }
      }
      if ((form->kind != Form::Kind::using_resource && form->kind != Form::Kind::import_resource) || form->path != COD_LANG_EXCEPTION_MODULE || form->using_symbol) continue;
      if (form->selected_members) implementation_gap(form->at, "member selection outside an enum");
      const auto found = std::find_if(exception_categories.begin(), exception_categories.end(), [&](const ExceptionDescriptor &entry) { return form->name == entry.name; });
      if (found == exception_categories.end()) fail(form->at, "unknown intrinsic exception selection '" + form->name + "'");
      const auto type = exception_type(exception_id(found->category));
      if (!module_names().types.emplace(form->alias, type).second) fail(form->at, "duplicate exception type selection");
      if (form->exporting) {
        ModuleNames::Resource resource{};
        resource.kind = ModuleNames::Resource::Kind::type;
        resource.type = type;
        publish(form->alias, resource, form->at);
      }
      program_.exception_values = true;
    }
    if (!register_nominal_types(source, diagnostics)) return false;
    std::vector<const Form *> definitions;
    for (const auto &form : forms) {
      if (source.dependencies.contains(form.get())) continue;
      if (form->c_linkage && !form->body) {
        const auto type = module_names().types.find(form->name);
        if (!form->result_type && type != module_names().types.end() && type->second.kind == Type::c_type_marker)
          register_native_constructor(*form, source);
        else if (form->result_type && !form->parameters.empty() && form->parameters.front().here) register_native_here(*form);
        else register_native_call(*form);
        continue;
      }
      if (form->kind == Form::Kind::expression || form->kind == Form::Kind::export_resource ||
          form->kind == Form::Kind::default_exports || form->kind == Form::Kind::data || form->kind == Form::Kind::sum_data || form->kind == Form::Kind::abstract_type ||
          form->kind == Form::Kind::type_synonym || form->kind == Form::Kind::native_handle_type ||
          form->kind == Form::Kind::region_definition || form->kind == Form::Kind::array ||
          form->kind == Form::Kind::enumeration)
        continue;
      if ((form->kind == Form::Kind::using_resource || form->kind == Form::Kind::import_resource) &&
          (form->path == COD_LANG_EXCEPTION_MODULE || form->path == COD_LANG_TEXT_STORAGE_MODULE) && !form->using_symbol) continue;
      if (form->kind == Form::Kind::using_resource && form->symbolic &&
          form->path == COD_LANG_IO_MODULE && form->name == COD_LANG_PRINT_NAME) {
        if (form->resource_nature == Form::ResourceNature::variable)
          fail(form->at, "symbolic resource kind does not match its publication");
        if (!print_symbol) {
          print_symbol = program_.symbols.size();
          program_.symbols.push_back({{}, true, COD_LANG_PRINT_NAME});
        }
        if (form->resource_nature == Form::ResourceNature::procedure) {
          check_procedure_selection_identity(*form, form->alias, *print_symbol, fresh_symbols);
          module_names().symbols.try_emplace(form->alias, *print_symbol);
          continue;
        }
        if (!module_names().symbols.emplace(form->alias, *print_symbol).second) fail(form->at, "duplicate print symbol binding");
        continue;
      }
      if (form->kind == Form::Kind::symbol) {
        if (form->explicit_value_symbol && form->symbol_owner)
          fail(form->at, "a value symbol cannot name an external storage owner");
        if (form->symbol_owner) implementation_gap(form->at, "regional symbol ownership contracts");
        if (module_names().types.contains(form->name))
          fail(form->at, "symbol declaration conflicts with type name '" + form->name + "'");
        auto type = resolve_type(*form->result_type);
        // Nominal layouts have completed before symbolic declarations are registered.
        if (form->explicit_value_symbol && !program_.bits_payload(type))
          fail(form->at, "val symbol requires a bits type");
        if (form->owning_handle_symbol) {
          if (type.kind != Type::c_type_marker || program_.schemas.at(type.identity).native_owner)
            fail(form->at, "an owning native symbol requires an owning 'handle type'");
          type = native_handle_type(std::move(type), true);
        } else {
          CheckedMethod contract; contract.source = form.get();
          signature_type(contract, type, form->at, false, form->symbol_reference.has_value());
        }
        const auto symbol = program_.symbols.size();
        if (!module_names().symbols.emplace(form->name, symbol).second)
          fail(form->at, "duplicate symbol binding '" + form->name + "'");
        program_.symbols.push_back({type, false, form->name, form->symbol_reference});
        program_.symbols.back().declaration = form->at;
        if (type.kind == Type::procedure && !type.pure_callable && !form->symbol_reference)
          module_names().families.emplace(symbol, register_family(symbol, form->at, {}));
        continue;
      }
      if (form->kind != Form::Kind::function && form->kind != Form::Kind::procedure)
        implementation_gap(form->at, "module declaration or selection '" + form->name + "'");
      definitions.push_back(form.get());
    }
    for (const auto *form : definitions) {
      const auto constructor = regional_initializer(*form);
      for (const auto &parameter : form->parameters)
        if (parameter.not_init_ref && !constructor)
          implementation_gap(*parameter.not_init_ref, "not init ref outside a regional sum initializer");
      std::vector<CheckedNativeInput> native_inputs;
      if (form->c_linkage) {
        if (form->symbolic || form->operation || !form->type_parameters.empty() || form->explicit_captures ||
            !form->result_type || !form->result_owner.empty() || !form->throws_types.empty())
          implementation_gap(form->at, "a C entry requires a concrete named signature with a scalar or void result and internal checked failure handling");
        native_inputs = checked_native_inputs(*form, "C entry");
      }
      const auto *here = !form->parameters.empty() && form->parameters.front().here ? &form->parameters.front() : nullptr;
      std::string operation = form->operation ? form->name : "";
      if (!form->symbolic)
        if (const auto family = module_names().callables.find(form->name); family != module_names().callables.end())
          operation = program_.families[family->second].operation;
      if (identity_operation(operation))
        fail(form->at, "reference identity is an intrinsic operation and cannot have user methods");
      const auto pipeline = pipeline_operator(operation);
      if (pipeline) {
        using Kind = PipelineOperator::Kind;
        const auto &parameters = form->parameters;
        const bool valid = pipeline->kind == Kind::composition ? parameters.size() == 2 && !here && !parameters[1].here
            : pipeline->kind == Kind::source ? parameters.size() == 3 && here && !parameters[1].here && !parameters[2].here
                                            : parameters.size() == 2 && here && !parameters[1].here;
        if (!valid) implementation_gap(form->at, "this pipeline operator method signature");
      }
      if ((!operation.empty() && !eager_operation(operation) && operation != "++" && operation != "<-" && !swap_operation(operation) && !pipeline) || form->conversion || !form->captures.empty())
        implementation_gap(form->at, "callable captures, effects, declarations, operators, owners or throws contracts");
      if (!operation.empty() && !pipeline) {
        const auto arity = form->parameters.size();
        if (!arity || arity > 2 || (operation == "!" && arity != 1) ||
            (operation != "+" && operation != "-" && operation != "!" && arity != 2))
          implementation_gap(form->at, "eager operator method arity");
      }
      std::set<std::string> owners;
      for (const auto &owner : form->owner_parameters) {
        if (owner.root || owner.explicit_parameter) implementation_gap(owner.at, "region-specific owner parameters");
        owners.insert(owner.name);
      }
      if (!form->result_owner.empty() && !owners.contains(form->result_owner)) fail(form->at, "unknown result owner '" + form->result_owner + "'");
      if (swap_operation(operation)) {
        if (form->result_type && *form->result_type != Type::void_type) fail(form->at, "swap method must return void");
        for (const auto &parameter : form->parameters)
          if (parameter.owner.empty() || parameter.variadic || parameter.automatic_conversion)
            fail(parameter.at, "swap destinations require explicit external owners without variadic parameters or conversions");
      }
      if (!form->result_owner.empty() && form->result_type == Type::void_type) fail(form->at, "void cannot be a borrowed result");
      CheckedMethod prepared;
      prepared.source = form;
      prepared.operation = operation;
      prepared.module = module_;
      prepared.regional_initializer = constructor;
      for (const auto &p : form->type_parameters) prepared.type_parameters.push_back(p.name);
      const auto resolve_signature = [&](const Type &type) { return resolve_type(type, &prepared); };
      const bool owns_exception_result = form->result_type && form->result_type->kind == Type::exception_holder;
      const bool consumes_exception = std::ranges::any_of(form->parameters, [](const auto &parameter) {
        return parameter.type.kind == Type::exception_holder;
      });
      if ((owns_exception_result || consumes_exception) &&
          (form->c_linkage || form->symbolic || !operation.empty() || here || constructor ||
           !form->type_parameters.empty() || !form->owner_parameters.empty() ||
           !form->body))
        implementation_gap(form->at, "owning exception signature outside an ordinary named CoD callable");
      for (const auto &parameter : form->parameters) {
        if (!parameter.here) break;
        prepared.here.push_back(here_contract(prepared, parameter));
      }
      if (pipeline && pipeline->kind == PipelineOperator::Kind::consumer && !prepared_source_contract(prepared.here.front()))
        implementation_gap(form->at, "a whole-traversal terminal requires a prepared producer parameter");
      if (pipeline && pipeline->kind == PipelineOperator::Kind::source &&
          (!prepared.here.front().here.empty() || form->result_type != Type::void_type))
        implementation_gap(form->at, "a source-attachment method requires a generator result contract");
      const bool handle_result = form->result_kind == "handle";
      if (constructor) {
        const auto &owner = form->owner_parameters.front().name;
        if (form->result_type &&
            (resolve_signature(*form->result_type) != sum_type(constructor->sum) ||
             form->result_owner != owner || form->result_const ||
             (!form->result_kind.empty() && form->result_kind != "ref")))
          fail(form->at, "initializer result must be a writable reference to its sum in the destination owner");
        prepared.result.type = Type::void_type;
      } else if (form->result_type) {
        prepared.result.type = resolve_signature(owns_exception_result ? *form->result_type->target : *form->result_type);
        if (handle_result) {
          if (form->kind != Form::Kind::procedure || form->symbolic || !operation.empty())
            implementation_gap(form->at, "borrowed handle results outside named CoD procedures");
          if (prepared.result.type->kind != Type::c_type_marker)
            fail(form->at, "handle result requires a 'handle type' name");
          prepared.result.type = native_handle_type(*prepared.result.type, false);
        } else if (owns_exception_result) {
          if (prepared.result.type->kind != Type::exception || !form->result_owner.empty())
            fail(form->at, "an owning exception result requires an exception type and establishes its own owner");
          prepared.result.mode = ResultMode::owned_exception;
          prepared.result.read_only = form->result_const;
        } else if (prepared.result.type->kind == Type::region && prepared.result.type->owned) {
          if (form->c_linkage || form->symbolic || !operation.empty() || here ||
              !form->result_owner.empty() || form->result_const || !form->result_kind.empty() ||
              !form->type_parameters.empty())
            implementation_gap(form->at, "an owning region result outside an ordinary named CoD factory");
          if (!prepared.result.type->target) fail(form->at, "an owning region result requires a root schema");
          const auto schema = resolve_signature(*prepared.result.type->target);
          direct_region_roots(program_, schema, form->at);
          prepared.result.type = owned_region_type(schema);
          prepared.result.mode = ResultMode::owned_region;
        } else reject_c_type_payload(*prepared.result.type, form->at);
      }
      if (!constructor) prepared.result.owner = form->result_owner;
      if (!prepared.result.owner.empty()) {
        prepared.result.mode = handle_result ? ResultMode::borrowed_handle :
            form->result_kind == "ptr" ? ResultMode::borrowed_ptr : ResultMode::borrowed_ref;
        prepared.result.read_only = form->result_const;
      }
      if (prepared.result.type && text_type(*prepared.result.type) &&
          (prepared.result.mode != ResultMode::borrowed_ref || !prepared.result.read_only))
        fail(form->at, "text results require a readonly, non-null owner-qualified reference");
      const bool ordinary_bits_aggregate = prepared.result.type && inline_aggregate_type(*prepared.result.type) &&
          program_.bits_payload(*prepared.result.type) && !form->c_linkage && !form->symbolic &&
          !form->implicit_scope && operation.empty() && !constructor && form->type_parameters.empty() &&
          std::ranges::none_of(form->parameters, [](const auto &parameter) { return parameter.here != nullptr; });
      const bool aggregate_value_result = ordinary_bits_aggregate && form->result_kind == "val" &&
          prepared.result.owner.empty();
      const bool aggregate_borrowed_result = ordinary_bits_aggregate && prepared.result.borrowed();
      if (form->result_kind == "val" && prepared.result.type && !scalar_type(*prepared.result.type) &&
          !aggregate_value_result)
        implementation_gap(form->at, "explicit val results for non-scalar or generic types");
      if (prepared.result.type && !prepared.result.borrowed() && !prepared.result.owns_region() &&
          !prepared.result.owns_exception() && !aggregate_value_result &&
          *prepared.result.type != Type::void_type && prepared.result.type->kind != Type::procedure && !scalar_type(*prepared.result.type)) {
        if (!form->result_const) implementation_gap(form->at, "default result mode for non-scalar or generic types");
      }
      if (prepared.result.type && !handle_result && !prepared.result.owns_region() &&
          !prepared.result.owns_exception() && !aggregate_value_result && !aggregate_borrowed_result)
        signature_type(prepared, *prepared.result.type, form->at, true, prepared.result.borrowed());
      if (prepared.result.type) require_ordinary_callable_type(*prepared.result.type, form->at);
      for (const auto &type : form->throws_types) {
        const auto resolved = resolve_type(type);
        if (resolved.kind != Type::exception) fail(form->at, "throws contracts require selected exception types");
        prepared.throws_types.push_back(resolved);
      }
      std::set<std::string> names;
      if (constructor) for (const auto &field : sum_alternative(*constructor).fields) {
        if (!field.stores_target() && !scalar_type(field.type))
          implementation_gap(field.source->at, "an inline aggregate field in a regional sum initializer");
        if ((!field.stores_target() && readonly_field(field)) ||
            (field.stores_target() && field.source->fixed_binding))
          implementation_gap(field.source->at, "a fixed field in a regional sum initializer");
        if (owners.contains(field.source->name))
          fail(field.source->at, "an initializer field conflicts with its destination owner name");
        names.insert(field.source->name);
      }
      std::map<std::string, size_t> native_owners;
      for (size_t i = 0; i < prepared.here.size(); ++i)
        if (!names.insert(form->parameters[i].name).second) fail(form->parameters[i].at, "duplicate parameter name");
      for (const auto &parameter : prepared.input_parameters()) {
        if (!names.insert(parameter.name).second) fail(parameter.at, "duplicate parameter name");
        if (form->supplied_scope) continue; // The selected receiver supplies this signature.
        if (!parameter.owner.empty() && !owners.contains(parameter.owner)) fail(parameter.at, "unknown parameter owner '" + parameter.owner + "'");
        if (parameter.automatic_conversion) implementation_gap(parameter.at, "automatic parameter conversion contract");
        if (parameter.selects_caller_ref_slot()) {
          const auto type = resolve_signature(parameter.type);
          if (type.kind != Type::array || parameter.owner.empty() || parameter.read_only || parameter.fixed_binding)
            fail(parameter.at, "ref var requires a writable owner-qualified named array binding");
          if (form->kind != Form::Kind::procedure || !operation.empty() || !form->body || here || form->symbolic ||
              form->c_linkage || prepared.result.borrowed())
            implementation_gap(parameter.at, "ref var named-array input outside an ordinary procedure with an independent result");
        } else if (parameter.selects_caller_place()) {
          if (resolve_signature(parameter.type) != Type::string || parameter.read_only || parameter.fixed_binding)
            fail(parameter.at, "a direct String place parameter requires writable val var storage");
          if (operation != "<-" || &parameter != &prepared.input_parameters().front() || !form->body ||
              !form->result_type || *form->result_type != Type::void_type || here || form->symbolic ||
              form->c_linkage)
            implementation_gap(parameter.at, "a direct String place outside a void transfer method");
        }
        if (parameter.declaration_kind == "handle") {
          if (form->symbolic || !operation.empty())
            implementation_gap(parameter.at, "native handle parameters outside ordinary named CoD callables");
          const auto marker = resolve_signature(parameter.type);
          if (marker.kind != Type::c_type_marker)
            fail(parameter.at, "handle parameter requires a 'handle type' name");
          const auto owner = std::ranges::find(prepared.input_owners(), parameter.owner, &OwnerParameter::name);
          if (owner == prepared.input_owners().end() || owner->root || owner->explicit_parameter)
            fail(parameter.at, "native handle parameter requires a declared owner");
          constrain_native_owner(native_owners, parameter, marker);
          prepared.parameters.push_back(native_handle_type(marker, false));
          continue;
        }
        const auto mode = argument_mode(parameter);
        if (mode == ArgumentMode::owning_exception) {
          const auto type = resolve_signature(*parameter.type.target);
          if (type.kind != Type::exception || parameter.variadic)
            fail(parameter.at, "a consuming exception parameter requires one exception holder");
          prepared.parameters.push_back(type);
          continue;
        }
        prepared.parameters.push_back(resolve_signature(parameter.type));
        require_ordinary_callable_type(prepared.parameters.back(), parameter.at);
        if (constructor) {
          const auto &argument = prepared.parameters.back();
          const bool independent = mode == ArgumentMode::value && scalar_type(argument) && parameter.owner.empty();
          const bool identity = mode == ArgumentMode::borrowed_ref && parameter.not_init_ref &&
              argument == sum_type(constructor->sum) &&
              parameter.owner == form->owner_parameters.front().name;
          if (parameter.variadic || parameter.selects_caller_place() || (!independent && !identity))
            implementation_gap(parameter.at, "argument outside the direct regional sum initializer profile");
        }
        if (!parameter.selects_caller_place() && text_type(prepared.parameters.back()) &&
            (mode != ArgumentMode::borrowed_ref || !parameter.read_only))
          fail(parameter.at, "text parameters require a readonly, non-null owner-qualified reference");
        signature_type(prepared, prepared.parameters.back(), parameter.at, false, mode != ArgumentMode::value);
      }
      if (operation == "<-") {
        const auto inputs = prepared.input_parameters();
        if (prepared.parameters.size() != 2 || !form->result_type || *form->result_type != Type::void_type ||
            (!inputs[0].selects_caller_place() && (prepared.parameters[0].kind != Type::product ||
              argument_mode(inputs[0]) != ArgumentMode::borrowed_ref || inputs[0].read_only)) ||
            !text_type(prepared.parameters[1]) || argument_mode(inputs[1]) != ArgumentMode::borrowed_ref)
          implementation_gap(form->at, "a transfer method outside the void String place and product policy profile");
      }
      if (handle_result) {
        const auto &owner = prepared.result.owner;
        if (!native_owners.contains(owner))
          fail(form->at, "handle result owner '" + owner + "' must be established by a handle input");
        const auto handle = prepared.result.type->identity;
        if (program_.schemas.at(handle).native_owner.value_or(handle) != native_owners.at(owner))
          fail(form->at, "handle result CType does not match its declared input owner CType");
        const auto parameter = std::ranges::find(prepared.input_owners(), owner, &OwnerParameter::name);
        prepared.result.read_only |= parameter->read_only;
      }
      check_aggregate_owner_roles(prepared, prepared.parameters);
      for (const auto &parameter : form->type_parameters)
        if (parameter.bound) implementation_gap(parameter.at, "bounded type parameters");
      std::optional<size_t> symbol;
      if (form->symbolic) {
        if (here) implementation_gap(here->at, "here signatures of symbolic callable families");
        if (module_names().types.contains(form->name)) implementation_gap(form->at, "same-scope type and symbol declarations");
        if (const auto found = module_names().symbols.find(form->name); found != module_names().symbols.end()) symbol = found->second;
        else {
          if (!prepared.type_parameters.empty() || !prepared.result.type)
            implementation_gap(form->at, "implicit symbolic declaration requires an independently established concrete signature");
          const auto type = resolve_type(callable_signature(prepared, prepared.parameters, prepared.result));
          signature_type(prepared, type, form->at);
          symbol = program_.symbols.size(); module_names().symbols.emplace(form->name, *symbol);
          program_.symbols.push_back({type, false, form->name});
          program_.symbols.back().declaration = form->at;
        }
        const auto &declared = program_.symbols[*symbol];
        if (declared.reference) fail(form->at, "a reference symbol cannot declare a procedure family");
        if (!declared.type || declared.type->kind != Type::procedure || declared.type->pure_callable)
          fail(form->at, "symbolic procedure definition requires a procedure symbol");
      }
      if (form->body) prepared.definition = program_.methods.size();
      if (symbol) {
        const auto selection = module_names().selected_symbols.find(form->name);
        if (selection != module_names().selected_symbols.end() && !selection->second.grants_extension)
          fail(form->at, "using and symbol selection do not grant callable extension permission");
      }
      if (!symbol && module_names().selected_callables.contains(form->name))
        fail(form->at, "using does not grant callable extension permission");
      if (symbol) {
        if (!module_names().families.contains(*symbol))
          module_names().families.emplace(*symbol, register_family(symbol, form->at, operation));
        prepared.family = module_names().families.at(*symbol);
        const auto &family = program_.families[*prepared.family];
        if (form->body) check_variadic_family_member(family.methods, prepared, form->at);
        if (form->body && family.module == module_ && family.imported_from)
          module_names().extensions[*prepared.family].emplace(form->name, form->at);
      } else {
        if (!module_names().callables.contains(form->name))
          module_names().callables.emplace(form->name, register_family({}, form->at, operation));
        prepared.family = module_names().callables.at(form->name);
        if (form->body) check_variadic_family_member(program_.families[*prepared.family].methods, prepared, form->at);
        if (module_names().imported_aliases.contains(form->name))
          module_names().extensions[*prepared.family].emplace(form->name, form->at);
      }
      if (!form->body) {
        program_.families[*prepared.family].declarations.push_back(std::move(prepared));
        continue;
      }
      const auto id = program_.methods.size();
      if (constructor) {
        auto &initializers = program_.sums.at(constructor->sum).alternatives.at(constructor->alternative).initializers;
        for (const auto other : initializers) {
          const auto &definition = program_.methods[other];
          if (implies(prepared, definition) && implies(definition, prepared) &&
              same_access(prepared, definition) && implies_owners(prepared, definition) && implies_owners(definition, prepared))
            fail(form->at, "duplicate initializer signature for '" + form->name + "'");
        }
        // The alternative selects the initializer family. A bare ordinary
        // call cannot enter a body with unbound destination fields.
        program_.methods.push_back(std::move(prepared));
        initializers.push_back(id);
        continue;
      }
      if (form->c_linkage) {
        if (!scalar_type(*prepared.result.type) && *prepared.result.type != Type::void_type)
          implementation_gap(form->at, "C entry results other than direct scalars or void");
        program_.native_entries.push_back({id, std::move(native_inputs), *prepared.result.type});
      }
      auto &family = program_.families[*prepared.family].methods;
      for (const auto other : family) {
        const auto &definition = program_.methods[other];
        if (implies(prepared, definition) && implies(definition, prepared) && same_here_signature(prepared, definition)) {
          const auto inputs = prepared.input_parameters();
          for (size_t p = 0; p < inputs.size(); ++p) {
            if (inputs[p].owner.empty() != definition.input_parameters()[p].owner.empty())
              implementation_gap(form->at, "owner-constrained overload selection");
          }
          if (same_access(prepared, definition) && implies_owners(prepared, definition) && implies_owners(definition, prepared))
            fail(form->at, "duplicate method signature for '" + form->name + "'");
        }
        if (definition.source->kind != form->kind && (prepared.operation.empty() || definition.operation.empty()))
          implementation_gap(form->at, "mixed function/procedure family");
      }
      program_.methods.push_back(std::move(prepared));
      family.push_back(id);
    }
    // Method registration may also have introduced implicit symbols. Both
    // maps are ordered by spelling, so the first conflict stays the same.
    for (const auto &[name, symbol] : module_names().symbols)
      if (const auto family = module_names().callables.find(name); family != module_names().callables.end())
        check_symbol_binding_name(name, program_.families[family->second].at);
    for (const auto &[name, type] : module_names().types)
      if (module_names().symbols.contains(name) || module_names().callables.contains(name))
        implementation_gap({source.path.string()}, "same-scope type and callable/symbol bindings");
    for (const auto &[name, operation] : module_names().native_operations)
      if (module_names().types.contains(name) || module_names().callables.contains(name) || module_names().symbols.contains(name))
        fail(program_.native_operations[operation].contract.source->at, "native provider conflicts with the binding '" + name + "'");
    for (const auto &[name, primitive] : module_names().text_primitives)
      if (module_names().callables.contains(name) || module_names().types.contains(name) ||
          module_names().symbols.contains(name) || module_names().native_operations.contains(name))
        fail({source.path.string()}, "text storage primitive conflicts with the binding '" + name + "'");
    for (const auto &[name, member] : module_names().enum_members)
      if (module_names().binds_other_than_enum_member(name))
        fail({source.path.string()}, "selected enum member conflicts with the binding '" + name + "'");
    for (const auto &[family, aliases] : module_names().extensions) if (aliases.size() > 1) {
      std::string message = "one callable identity is extended through multiple aliases:";
      for (const auto &[name, at] : aliases) message += " '" + name + "' at " + location_text(at);
      warnings_.push_back({Warning::multi_alias_extension, aliases.begin()->second, std::move(message)});
    }
    publish_module(source);
    module_names().publication_ready = true;
    return true;
  }

  static void require_implemented_source_forms(const Type &type, const Location &at, const char *holding_form) {
    if (type.kind == Type::exception_holder) implementation_gap(at, holding_form);
    if (type.target) require_implemented_source_forms(*type.target, at, "owning exception callable type");
    for (const auto &parameter : type.parameters)
      require_implemented_source_forms(parameter, at, "owning exception callable type");
  }

  static void require_implemented_source_forms(const Parameter &parameter, bool check_after_nominal_registration = false) {
    if (parameter.not_init_ref && !check_after_nominal_registration)
      implementation_gap(*parameter.not_init_ref, "not init ref parameters");
    if (check_after_nominal_registration && parameter.type.kind == Type::exception_holder)
      require_implemented_source_forms(*parameter.type.target, parameter.at, "owning exception parameter");
    else require_implemented_source_forms(parameter.type, parameter.at, "owning exception parameter");
    if (!parameter.here) return;
    for (const auto &input : parameter.here->parameters) require_implemented_source_forms(input);
    require_implemented_source_forms(parameter.here->result, parameter.at, "owning exception result");
  }

  static void require_implemented_header(const Form &form) {
    const bool named_co_d_definition = !form.c_linkage &&
        (form.kind == Form::Kind::function || form.kind == Form::Kind::procedure);
    for (const auto &parameter : form.parameters)
      require_implemented_source_forms(parameter, named_co_d_definition);
    if (form.result_type) {
      const auto &written = *form.result_type;
      require_implemented_source_forms(named_co_d_definition && written.kind == Type::exception_holder
          ? *written.target : written, form.at, "owning exception result");
    }
  }

  bool register_source_modules(const SourceProgram &source, CompilationDiagnostics &diagnostics) {
    modules_.resize(source.modules.size());
    std::optional<size_t> print_symbol;
    size_t reported_warnings = 0;
    bool registered = source.ready_for_checking();
    for (const auto module : source.construction_order) {
      const auto &input = source.modules[module];
      if (input.read_state != SourceModule::ReadState::parsed || !input.failed_edges.empty() ||
          std::ranges::any_of(input.dependencies, [&](const auto &edge) {
            return !modules_[edge.second].publication_ready;
          })) {
        registered = false;
        continue;
      }
      if (!diagnostics.continue_work()) return false;
      const ModuleLookup lookup(module_, module);
      if (input.syntax.needs_source_form_check())
        for (const auto &form : input.forms()) require_implemented_header(*form);
      const auto first_finding = diagnostics.size();
      if (!register_module(input, print_symbol, &diagnostics)) registered = false;
      if (diagnostics.size() != first_finding) relate_module_findings(source, module, diagnostics, first_finding);
      while (reported_warnings < warnings_.size()) {
        if (!diagnostics.append(warnings_[reported_warnings])) return false;
        ++reported_warnings;
      }
    }
    if (!registered || diagnostics.has_error() || !diagnostics.continue_work()) return false;
    check_native_type_names();
    return true;
  }

public:
  class ModuleDeclarations {
    friend class ModuleBinder;
    friend class seed::BodyExtraction;

    // Declaration and method source links point into this exact revision.
    SourceProgram source_;
    std::vector<ModuleNames> modules_;
    CheckedProgram declarations_;

    ModuleDeclarations(SourceProgram source, std::vector<ModuleNames> modules, CheckedProgram declarations)
        : source_(std::move(source)), modules_(std::move(modules)), declarations_(std::move(declarations)) {}

  public:
    const SourceProgram &source() const { return source_; }
    const CheckedMethod &method(size_t id) const { return declarations_.methods.at(id); }

    // A module family is only a candidate for a call. Lexical shadowing,
    // receiving interpretation and operand preparation belong to extraction.
    const CheckedFamily *ordinary_family(size_t module, const std::string &spelling) const {
      const auto &names = modules_.at(module).callables;
      const auto family = names.find(spelling);
      return family == names.end() ? nullptr : &declarations_.families.at(family->second);
    }
  };

  static std::optional<ModuleDeclarations> bind_module_declarations(SourceProgram source,
                                                                    CompilationDiagnostics &diagnostics) {
    ModuleBinder binder;
    if (!binder.register_source_modules(source, diagnostics)) return {};
    return ModuleDeclarations{std::move(source), std::move(binder.modules_), std::move(binder.program_)};
  }
};

} // namespace cod::language
