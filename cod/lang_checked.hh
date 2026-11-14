//===--- lang_checked.hh - LLVM-independent scalar obligations ------===//
#pragma once

#include "lang_syntax.hh"
#include "lang_diagnostics.hh"
#include "lang_exceptions.hh"
#include "lang_completion.hh"

#include <charconv>
#include <bit>
#include <cmath>
#include <cstdint>
#include <deque>
#include <functional>
#include <limits>
#include <map>

namespace cod::language {

[[noreturn]] inline void implementation_gap(const Location &at, const std::string &detail) {
  fail(at, "micro-dispatch implementation gap: " + detail + "; this is not a language restriction");
}

inline bool scalar_type(const Type &type) { return numeric(type) || type == Type::boolean; }
inline bool bits_type(const Type &type) { return scalar_type(type) || type.kind == Type::exception; }
inline bool lexical_value_type(const Type &type) { return bits_type(type) || type.kind == Type::procedure; }
inline constexpr size_t null_target = std::numeric_limits<size_t>::max();
enum class BuiltinOperation { none, unary, binary, is_null, cause_at, reference_swap, bits_swap, print };
inline bool eager_operation(std::string_view name) {
  return name == "+" || name == "-" || name == "*" || name == "/" || name == "%" || name == "!" ||
      name == "==" || name == "!=" || name == "<" || name == ">" || name == "<=" || name == ">=";
}
struct CheckedArgument {
  ArgumentMode mode;
  Type type;
  bool operator==(const CheckedArgument &) const = default;
};
inline ArgumentMode argument_mode(const Parameter &parameter) {
  if (parameter.owner.empty()) return ArgumentMode::value;
  if (parameter.declaration_kind == "ptr") return ArgumentMode::borrowed_ptr;
  if (parameter.declaration_kind == "ref") return ArgumentMode::borrowed_ref;
  return ArgumentMode::borrowed_read;
}
inline bool borrowed_result(const Form &method) { return !method.result_owner.empty() && (!method.result_const || !method.result_kind.empty()); }
inline Type exception_type(size_t category) { return {Type::exception, exception_categories.at(category).name, category}; }
inline bool accepts_exception(const Type &source, const Type &target) {
  return source.kind == Type::exception && target.kind == Type::exception && (!target.identity || source == target);
}
inline bool transfers_bits(const Type &source, const Type &target) { return widens(source, target) || accepts_exception(source, target); }
inline bool transfers_value(const Type &from, const Type &to) {
  return from.kind == Type::procedure || to.kind == Type::procedure ? from == to : transfers_bits(from, to);
}
inline std::set<size_t> exception_possibilities(const Type &type) {
  if (type.kind != Type::exception) return {};
  if (type.identity) return {type.identity};
  std::set<size_t> result;
  for (size_t i = 1; i < exception_categories.size(); ++i) result.insert(i);
  return result;
}

struct CheckedOperation {
  Type type;
  Type operand_type;
  std::optional<size_t> callee;
  std::optional<size_t> callable, callable_binding, callable_place;
  std::vector<CheckedOperation> callable_captures;
  bool callable_call = false;
  bool callable_nullable = false;
  std::optional<size_t> place;
  std::optional<size_t> binding;
  std::set<size_t> targets, owners, origins, selection_versions;
  std::optional<size_t> accumulator;
  bool read_only = false;
  bool fixed_binding = false;
  bool view = false;
  std::uint64_t integer = 0;
  bool minimum_literal = false;
  bool initializes = false;
  std::set<size_t> exceptions;
  std::set<size_t> captures;
  bool snapshot = false;
  std::optional<size_t> captured_place;
  bool initialized_target = false;
  bool mixed_view = false;
  bool borrowed_result = false;
  BuiltinOperation builtin = BuiltinOperation::none;
  std::vector<CheckedArgument> arguments;
  std::map<size_t, CleanupEquations::Effects> invocation;
  struct NullTest { size_t binding; std::set<size_t> versions; bool null_when_true = true; };
  std::optional<NullTest> null_test;
};

struct CheckedStorage {
  std::string name;
  Type type;
  size_t scope;
  bool read_only;
  bool borrowed = false;
  bool captured = false;
  size_t binding = 0;
};

struct CheckedBinding {
  Type type;
  bool read_only;
  bool value_binding;
  bool fixed;
  bool requires_initialized = false;
  std::optional<size_t> owner_binding = {};
  bool callable_value = false;
  size_t scope = 0;
  // Alias names carry access restrictions; only the original binding has slot state.
  std::optional<size_t> alias = {};
};

struct LexicalState {
  struct Scope { size_t id; std::map<std::string, size_t> bindings; bool declarations = true; };
  std::vector<Scope> scopes{{0, {}}};
  std::set<size_t> bindings_available, available, initialized;
  std::map<size_t, std::set<size_t>> input_initialized{{0, {}}};
  struct Requirement { std::uint64_t inputs; std::set<size_t> before; };
  std::map<size_t, Requirement> requirements;
  std::map<size_t, std::set<size_t>> exceptions;
  std::map<size_t, std::set<size_t>> targets;
  std::map<size_t, std::set<size_t>> owners;
  std::set<size_t> expired_bindings;
  std::map<size_t, std::set<size_t>> target_versions;
  std::set<size_t> initialized_targets;
  CleanupEquations::Returned returned;
  std::map<size_t, size_t> queues{{0, 0}};
  std::uint64_t inputs = 1;
  bool live = true;

  std::optional<size_t> lookup(const std::string &name) const {
    for (auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope)
      if (const auto found = scope->bindings.find(name); found != scope->bindings.end()) return found->second;
    return {};
  }
};

// Substitutions are checking obligations, not semantic methods or activation records.
struct CallEffects {
  std::set<std::pair<size_t, std::set<size_t>>> reads;
  std::map<size_t, std::set<size_t>> initialized;
  CleanupEquations::Returned returned;
};
struct CheckedInstance {
  size_t method;
  std::vector<Type> arguments;
  Type result;
  std::map<const Expression *, CheckedOperation> operations;
  bool complete = false;
  std::vector<CheckedStorage> storage;
  std::vector<CheckedBinding> bindings;
  CallEffects effects;
  std::vector<Type> environment, context, retained_types;
  std::vector<std::optional<Type>> substitutions;
};
struct CheckedMethod {
  const Form *source = nullptr;
  std::vector<size_t> instances;
  std::vector<Type> parameters;
  Type result;
  std::set<size_t> throws_categories;
  std::vector<Capture> captures;
  bool captures_resolved = false;
  std::vector<std::string> type_parameters;
  std::vector<size_t> retained_types;
  std::optional<size_t> parent;
};
struct CheckedCallable { Type type; size_t instance; };
struct CheckedProgram {
  std::vector<CheckedCallable> callables;
  std::vector<CheckedMethod> methods;
  std::deque<CheckedInstance> instances;
  std::map<const Expression *, CheckedOperation> entry;
  std::vector<const Expression *> initializers;
  std::vector<WarningDiagnostic> warnings;
  std::vector<CheckedStorage> entry_storage;
  std::vector<CheckedBinding> entry_bindings;
  bool exception_values = false;
};

class ScalarChecker {
  CheckedProgram program_;
  std::map<std::string, std::vector<size_t>> families_;
  std::set<std::string> print_symbols_;
  std::map<const Form *, size_t> lexical_methods_;
  std::map<const Form *, std::vector<Capture>> free_captures_;
  std::map<const Form *, std::set<std::string>> free_types_;
  std::set<const Form *> warned_captures_;
  std::set<std::string> module_values_;
  std::map<std::string, Type> exception_types_;
  std::set<size_t> warned_missing_return_;
  std::set<const Expression *> warned_shadow_;
  LexicalState lexical_;
  std::vector<CheckedStorage> *storage_ = nullptr;
  std::vector<CheckedBinding> *bindings_ = nullptr;
  std::map<size_t, std::vector<size_t>> scope_storage_;
  bool proving_ = false;
  bool validating_ = false;
  CallEffects observed_;
  std::vector<std::set<size_t>> possible_targets_, target_dependencies_;
  std::vector<CheckedStorage> catalog_storage_;
  size_t next_target_version_ = 0;
  std::set<size_t> rebound_;
  std::map<std::string, size_t> external_owners_;
  struct LoopOwners { size_t first, last; std::set<size_t> bindings, expired; };
  std::map<const Expression *, LoopOwners> loop_owners_;
  struct AbruptPath { LexicalState state; std::set<size_t> exceptions; Location at; Completion completion{2, false}; };
  struct AbruptFlow { size_t depth; std::vector<AbruptPath> paths; };
  AbruptFlow *abrupt_ = nullptr;
  std::optional<size_t> caught_;
  std::set<size_t> catch_bindings_;
  std::map<size_t, std::map<size_t, std::uint64_t>> caught_inputs_;
  const CheckedMethod *checking_method_ = nullptr;
  const CheckedInstance *checking_instance_ = nullptr;
  std::set<size_t> loop_controls_, delayed_controls_;
  struct CatchCapture {
    CatchCapture *parent;
    std::set<size_t> bindings, required;
  };
  CatchCapture *captures_ = nullptr;
  CleanupEquations cleanup_;
  size_t cleanup_scope_ = 0;
  const Expression *prepared_call_ = nullptr;
  std::set<size_t> deferred_inputs_;
  std::vector<Location> read_sites_;
  bool delayed_ = false;
  size_t next_scope_ = 1;
  static constexpr size_t instance_limit = 4096;

  bool external_owner(size_t owner) const {
    return std::any_of(external_owners_.begin(), external_owners_.end(), [&](const auto &entry) { return entry.second == owner; });
  }
  bool owner_alive(size_t owner) const {
    return external_owner(owner) ||
        std::any_of(lexical_.scopes.begin(), lexical_.scopes.end(), [&](const auto &scope) { return scope.id == owner; });
  }
  size_t named_owner(const std::string &name, const Location &at) const {
    const auto binding = lexical_.lookup(name);
    if (!binding) fail(at, "unknown owner '" + name + "'");
    if (!lexical_.bindings_available.contains(*binding)) fail(at, "owner binding '" + name + "' is not available on every reaching path");
    const auto owner = bindings_->at(*binding).owner_binding;
    if (!owner) fail(at, "nearest binding '" + name + "' is not an owner");
    if (lexical_.live && !owner_alive(*owner)) fail(at, "owner '" + name + "' has ended");
    return *owner;
  }
  void read_view(const CheckedOperation &view, const Location &at, const std::string &name) {
    dereference(view, at);
    if (!view.initialized_target) for (const auto place : view.targets) require_initialized(place, at, name);
  }
  using Substitutions = std::vector<std::optional<Type>>;
  struct ArgumentPreparation {
    ArgumentMode mode = ArgumentMode::value;
    std::optional<Type> view;
  };
  ArgumentPreparation argument_preparation(const Expression &expr, const std::vector<Type> &prefix,
                                            const std::vector<std::optional<Type>> &evidence) const {
    if (expr.intrinsic) return {};
    std::vector<size_t> applicable;
    std::map<size_t, Substitutions> substitutions;
    bool complete = true;
    for (size_t i = prefix.size(); i < evidence.size(); ++i) complete = complete && evidence[i].has_value();
    if (const auto family = families_.find(expr.text); family != families_.end()) for (const auto id : family->second) {
      const auto &candidate = program_.methods[id];
      if (candidate.parameters.size() != expr.children.size()) continue;
      auto bindings = match_prefix(candidate, prefix);
      if (!bindings) continue;
      bool matches = true;
      for (size_t i = prefix.size(); i < evidence.size(); ++i)
        if (evidence[i] && !bind_type(candidate, candidate.parameters[i], *evidence[i], *bindings, true)) matches = false;
      if (matches) { applicable.push_back(id); substitutions.emplace(id, std::move(*bindings)); }
    }
    if (complete) {
      applicable = most_specific(applicable);
      if (applicable.size() > 1) fail(expr.at, "ambiguous methods for '" + expr.text + "'");
    }
    std::optional<ArgumentMode> mode;
    std::optional<Type> view;
    bool consistent_view = true;
    // Without complete type evidence, an eager operation may still use its value-taking builtin.
    if (eager_operation(expr.text) && !complete) mode = ArgumentMode::value;
    for (const auto id : applicable) {
      const auto &candidate = program_.methods[id];
      const auto passing = argument_mode(candidate.source->parameters[prefix.size()]);
      if (mode && *mode != passing)
        implementation_gap(expr.children[prefix.size()]->at, "overloads with different argument preparation contracts without sufficient operand type evidence");
      mode = passing;
      const auto expected = substitute(candidate.parameters[prefix.size()], candidate, substitutions.at(id), true);
      if (expected.kind != Type::procedure || nested_binder(candidate, expected) || (view && *view != expected))
        consistent_view = false;
      else view = expected;
    }
    return {mode.value_or(ArgumentMode::value), consistent_view ? view : std::nullopt};
  }

  LexicalState::Scope &declaration_scope(const std::string &name, const Location &at) {
    auto selected = lexical_.scopes.rbegin();
    while (!selected->declarations) {
      if (selected->bindings.contains(name)) fail(at, "duplicate declaration of catch parameter '" + name + "'");
      ++selected;
    }
    auto &scope = *selected;
    if (scope.bindings.contains(name)) fail(at, "duplicate declaration of '" + name + "' in one lexical scope");
    return scope;
  }
  size_t bind(const std::string &name, const Type &type, const std::set<size_t> &targets,
              bool read_only, bool value_binding, bool fixed, const Location &at, std::optional<size_t> alias = {}) {
    auto &scope = declaration_scope(name, at);
    const auto id = bindings_->size();
    bindings_->push_back({type, read_only, value_binding, fixed});
    bindings_->back().scope = scope.id;
    bindings_->back().alias = alias;
    if (!alias) {
      lexical_.targets[id] = targets;
      lexical_.owners[id] = target_owners(targets);
      lexical_.target_versions[id] = {next_target_version_++};
    }
    if (!proving_) {
      possible_targets_.push_back(targets);
      target_dependencies_.push_back({});
    }
    scope.bindings.emplace(name, id);
    lexical_.bindings_available.insert(id);
    return id;
  }
  size_t declare(const std::string &name, const Type &type, bool read_only, bool value_binding, const Location &at) {
    const auto scope = declaration_scope(name, at).id;
    const auto id = storage_->size();
    storage_->push_back({name, type, scope, read_only});
    scope_storage_[scope].push_back(id);
    storage_->back().binding = bind(name, type, {id}, read_only, value_binding, false, at);
    lexical_.available.insert(id);
    return id;
  }
  static std::set<size_t> intersect(const std::set<size_t> &a, const std::set<size_t> &b) {
    std::set<size_t> result;
    for (const auto id : a) if (b.contains(id)) result.insert(id);
    return result;
  }
  const CheckedStorage &storage(size_t place) const {
    return place < storage_->size() ? storage_->at(place) : catalog_storage_.at(place);
  }
  std::set<size_t> target_owners(const std::set<size_t> &targets) const {
    std::set<size_t> owners;
    for (const auto place : targets) if (place != null_target) owners.insert(storage(place).scope);
    return owners;
  }
  void select_targets(CheckedOperation &operation, const std::set<size_t> &targets) const {
    operation.targets = targets;
    operation.place = targets.size() == 1 && !targets.contains(null_target) ? std::optional<size_t>{*targets.begin()} : std::nullopt;
  }
  void assign_targets(size_t binding, const CheckedOperation &source) {
    lexical_.targets[binding] = source.targets;
    lexical_.owners[binding].insert(source.owners.begin(), source.owners.end());
    lexical_.target_versions[binding] = {next_target_version_++};
    lexical_.initialized_targets.erase(binding);
    if (source.initialized_target) lexical_.initialized_targets.insert(binding);
    if (!proving_) {
      possible_targets_.at(binding).insert(source.targets.begin(), source.targets.end());
      target_dependencies_.at(binding).insert(source.origins.begin(), source.origins.end());
    }
  }
  void require_live_view(const CheckedOperation &view, const Location &at) const {
    if (lexical_.live) for (const auto owner : view.owners)
      if (!owner_alive(owner)) fail(at, "storage view's lexical owner has ended");
  }
  const CheckedBinding &reference_destination(const CheckedOperation &view, const Location &at) const {
    if (!view.binding) implementation_gap(at, "runtime-selected reference binding destination");
    if (!lexical_.bindings_available.contains(*view.binding)) fail(at, "reference binding's lexical scope has ended");
    const auto &binding = bindings_->at(*view.binding);
    if (binding.value_binding) fail(at, "val storage has no reference binding; use '<-' for bits transfer");
    if (view.fixed_binding || binding.fixed) fail(at, "cannot rebind a const reference binding");
    return binding;
  }
  void check_rebind(const CheckedOperation &target, const CheckedOperation &source, const Location &at) {
    const auto &binding = reference_destination(target, at);
    if (!source.view || source.mixed_view) implementation_gap(at, "rebinding from a fresh value without an existing target");
    if (source.type != Type::null_ref && source.type != target.type) fail(at, "reference rebinding requires the exact target type");
    if (!binding.read_only && source.read_only) fail(at, "readonly source cannot rebind a writable reference target");
    auto owners = lexical_.owners.at(*target.binding);
    owners.insert(source.owners.begin(), source.owners.end());
    if (owners.size() > 1) fail(at, "reference rebinding cannot change the initializer-fixed owner");
    if (binding.requires_initialized && proving_ && lexical_.live && source.targets.contains(null_target))
      fail(at, "ref requires a proven non-null target");
    require_live_view(source, at);
    if (binding.requires_initialized && !source.initialized_target)
      for (const auto place : source.targets) require_initialized(place, at, "ref target");
  }
  void rebind(const CheckedOperation &target, const CheckedOperation &source) {
    assign_targets(*target.binding, source);
    if (!proving_) rebound_.insert(*target.binding);
  }
  CheckedOperation current_selection(CheckedOperation view, const Location &at) const {
    reference_destination(view, at);
    const auto binding = *view.binding;
    if (lexical_.live && lexical_.expired_bindings.contains(binding))
      fail(at, "reference binding's target owner has ended or may expire across a loop backedge");
    select_targets(view, lexical_.targets.at(binding));
    view.owners = lexical_.owners.at(binding);
    view.selection_versions = lexical_.target_versions.at(binding);
    view.origins = {binding};
    view.initialized_target = lexical_.initialized_targets.contains(binding) ||
        std::all_of(view.targets.begin(), view.targets.end(), [&](size_t place) { return lexical_.initialized.contains(place); });
    require_live_view(view, at);
    return view;
  }
  std::set<size_t> stored_exceptions(const CheckedOperation &view) const {
    std::set<size_t> result;
    for (const auto place : view.targets) if (place != null_target) {
      const auto found = lexical_.exceptions.find(place);
      const auto categories = delayed_ || found == lexical_.exceptions.end() ? exception_possibilities(view.type) : found->second;
      result.insert(categories.begin(), categories.end());
    }
    return result;
  }
  void write_exceptions(const CheckedOperation &target, const std::set<size_t> &categories) {
    if (target.type.kind != Type::exception) return;
    for (const auto place : target.targets) if (place != null_target) {
      if (target.place) lexical_.exceptions[place] = categories;
      else lexical_.exceptions[place].insert(categories.begin(), categories.end());
    }
  }
  void widen_targets(LexicalState &state) {
    if (!proving_) return;
    for (auto &[binding, targets] : state.targets) {
      targets = possible_targets_.at(binding);
      const auto owners = target_owners(targets);
      state.owners[binding].insert(owners.begin(), owners.end());
      if (rebound_.contains(binding)) {
        state.initialized_targets.erase(binding);
        state.target_versions[binding] = {next_target_version_++};
      }
    }
  }
  void solve_targets() {
    bool changed;
    do {
      changed = false;
      for (size_t i = 0; i < possible_targets_.size(); ++i) for (const auto source : target_dependencies_[i]) {
        const auto before = possible_targets_[i].size();
        possible_targets_[i].insert(possible_targets_[source].begin(), possible_targets_[source].end());
        changed = changed || before != possible_targets_[i].size();
      }
    } while (changed);
    catalog_storage_ = *storage_;
    for (auto &[expr, loop] : loop_owners_) for (const auto binding : loop.bindings)
      for (const auto place : possible_targets_.at(binding)) if (place != null_target) {
        const auto scope = storage(place).scope;
        if (scope >= loop.first && scope < loop.last) loop.expired.insert(binding);
      }
  }
  static std::vector<size_t> inputs(std::uint64_t mask) {
    std::vector<size_t> result;
    for (size_t i = 0; i <= exception_categories.size(); ++i) if (mask & (std::uint64_t{1} << i)) result.push_back(i);
    return result;
  }
  void initialize(size_t id) {
    lexical_.initialized.insert(id);
    lexical_.returned.uninitialized.erase(id);
    for (const auto input : inputs(lexical_.inputs)) lexical_.input_initialized[input].insert(id);
  }
  static void add_requirement(LexicalState &state, size_t read, std::uint64_t inputs, const std::set<size_t> &before) {
    auto [found, inserted] = state.requirements.emplace(read, LexicalState::Requirement{inputs, before});
    if (!inserted) {
      found->second.inputs |= inputs;
      found->second.before = intersect(found->second.before, before);
    }
  }
  void observe_read(size_t place, const std::set<size_t> &known) {
    if (delayed_ || !storage(place).borrowed) return;
    std::set<size_t> external;
    for (const auto id : known) if (storage(id).borrowed) external.insert(id);
    observed_.reads.emplace(place, std::move(external));
  }
  void apply_effects(LexicalState &state, const CleanupEquations::Effects &effects) {
    if (proving_) for (const auto &[read, before] : effects.reads) {
      const auto place = cleanup_.binding(read);
      if (state.initialized.contains(place) || before.contains(place)) continue;
      auto known = before;
      known.insert(state.initialized.begin(), state.initialized.end());
      if (!storage(place).borrowed && (!delayed_ || !deferred_inputs_.contains(place)) && validating_)
        fail(read_sites_.at(read), "cannot prove initialization of '" + storage(place).name + "' for deferred or borrowed read");
      add_requirement(state, read, state.inputs, known);
      observe_read(place, known);
    }
    for (const auto id : effects.initialized) if (state.available.contains(id)) {
      state.initialized.insert(id);
      for (const auto input : inputs(state.inputs)) state.input_initialized[input].insert(id);
    }
    state.returned.then(effects.returned);
    state.returned.initialize(state.initialized);
  }
  static std::set<size_t> call_reasons(const CheckedMethod &method) {
    std::set<size_t> reasons{0, 2};
    for (size_t category = 1; category < exception_categories.size(); ++category)
      if (!exception_categories[category].checked || method.throws_categories.contains(category)) reasons.insert(3 + category);
    return reasons;
  }
  static std::set<size_t> written_targets(const std::set<size_t> &parameters, const std::vector<CheckedOperation> &views) {
    std::set<size_t> written;
    for (const auto parameter : parameters) {
      auto targets = views.at(parameter).targets; targets.erase(null_target);
      if (targets.size() == 1) written.insert(*targets.begin());
    }
    return written;
  }
  std::map<size_t, CleanupEquations::Effects> invocation_effects(const CallEffects &target, const std::set<size_t> &reasons,
      const std::vector<CheckedOperation> &views, const Location &at) {
    CleanupEquations::Reads reads;
    if (proving_) for (const auto &[parameter, before] : target.reads) {
      const auto &view = views.at(parameter);
      if (view.initialized_target) continue;
      const bool same_selection = view.binding && std::any_of(before.begin(), before.end(), [&](size_t other) {
        return view.binding == views.at(other).binding && view.selection_versions == views.at(other).selection_versions;
      });
      if (same_selection) continue;
      const auto known = written_targets(before, views);
      for (const auto place : view.targets) if (place != null_target && !known.contains(place)) {
        const auto read = cleanup_.read(place); read_sites_.push_back(at);
        reads.emplace(read, known);
      }
    }
    std::map<size_t, CleanupEquations::Effects> effects;
    for (const auto reason : reasons) {
      const auto found = target.initialized.find(reason);
      effects.emplace(reason, CleanupEquations::Effects{reads,
          found == target.initialized.end() ? std::set<size_t>{} : written_targets(found->second, views)});
    }
    return effects;
  }
  void prepare_borrow(const CheckedOperation &view, ArgumentMode mode, const Location &at) {
    if (mode == ArgumentMode::value) return;
    if (!view.view || view.mixed_view) implementation_gap(at, "borrowed arguments require existing target views");
    if (mode == ArgumentMode::borrowed_read) read_view(view, at, "borrowed argument");
    if (mode == ArgumentMode::borrowed_ref && proving_ && lexical_.live && view.targets.contains(null_target))
      fail(at, "ref argument requires a proven non-null target");
    if (mode == ArgumentMode::borrowed_ref && !view.initialized_target)
      for (const auto place : view.targets) require_initialized(place, at, "ref argument");
  }
  static std::set<size_t> call_reasons(const Type &contract) {
    std::set<size_t> categories;
    for (const auto &failure : contract.throws_types) {
      const auto selected = exception_possibilities(failure); categories.insert(selected.begin(), selected.end());
    }
    std::set<size_t> reasons{0, 2};
    for (size_t category = 1; category < exception_categories.size(); ++category)
      if (!exception_categories[category].checked || categories.contains(category)) reasons.insert(3 + category);
    return reasons;
  }
  CallEffects callable_effects(const Type &contract) const {
    CallEffects effects;
    bool first = true;
    for (const auto &callable : program_.callables) if (callable.type == contract) {
      const auto &next = program_.instances[callable.instance].effects;
      if (first) { effects = next; first = false; continue; }
      effects.reads.insert(next.reads.begin(), next.reads.end());
      for (auto &[reason, known] : effects.initialized) {
        const auto found = next.initialized.find(reason);
        known = found == next.initialized.end() ? std::set<size_t>{} : intersect(known, found->second);
      }
      effects.returned.merge(next.returned);
    }
    for (const auto reason : call_reasons(contract)) effects.initialized.try_emplace(reason);
    return effects;
  }
  void finish_call(const Expression &expr, const Type &contract, const CallEffects &summary,
      const std::set<size_t> &reasons, const std::vector<Type> &arguments,
      const std::vector<CheckedOperation> &views, CheckedOperation &operation, bool read, bool discard, size_t offset = 0) {
    std::map<size_t, size_t> owners;
    for (size_t i = 0; i < arguments.size(); ++i) {
      const auto role = contract.owners.empty() ? 0 : contract.owners.at(i);
      if (!role) continue;
      const bool readonly = !contract.const_views.empty() && contract.const_views.at(i);
      const auto &view = views[i];
      if (view.owners.size() != 1) implementation_gap(expr.children[i + offset]->at, "borrowed argument requires one established owner");
      const auto owner = *view.owners.begin();
      if (!owner_alive(owner)) fail(expr.children[i + offset]->at, "borrowed argument's lexical owner has ended");
      if (&expr == prepared_call_ && !external_owner(owner)) {
        const auto scope = std::find_if(lexical_.scopes.begin(), lexical_.scopes.end(), [&](const auto &entry) { return entry.id == owner; });
        const auto cleanup = std::find_if(lexical_.scopes.begin(), lexical_.scopes.end(), [&](const auto &entry) { return entry.id == cleanup_scope_; });
        if (scope > cleanup) fail(expr.children[i + offset]->at, "deferred borrowed argument does not outlive its cleanup scope");
      }
      if (const auto [found, inserted] = owners.emplace(role, owner); !inserted && found->second != owner)
        fail(expr.at, "arguments for one owner role have different actual owners");
      if (!readonly && view.read_only) fail(expr.children[i + offset]->at, "readonly argument cannot supply a writable borrowed parameter");
      if (!readonly) for (const auto place : view.targets) if (place != null_target && storage(place).captured)
        implementation_gap(expr.children[i + offset]->at, "writable borrowing of a captured callable payload");
      if (arguments[i] != contract.parameters[i]) implementation_gap(expr.children[i + offset]->at, "borrowed exception upcasts");
      if (!readonly) for (const auto place : view.targets) if (place != null_target && storage(place).type.kind == Type::exception)
        lexical_.exceptions[place] = exception_possibilities(storage(place).type);
    }
    operation.type = *contract.target;
    operation.exceptions = exception_possibilities(operation.type);
    operation.invocation = invocation_effects(summary, reasons, views, expr.at);
    if (&expr != prepared_call_) {
      const auto before = lexical_;
      for (const auto &[reason, effects] : operation.invocation) if (reason) {
        lexical_ = before; apply_effects(lexical_, effects);
        if (const auto found = summary.initialized.find(reason); found != summary.initialized.end())
          initialize_selections(lexical_, found->second, views);
        note_abrupt(expr.at, reason >= 3 ? std::set<size_t>{reason - 3} : std::set<size_t>{}, {reason, false});
      }
      lexical_ = before; apply_effects(lexical_, operation.invocation.at(0));
      if (const auto found = summary.initialized.find(0); found != summary.initialized.end())
        initialize_selections(lexical_, found->second, views);
    }
    const auto result_owner = contract.owners.empty() ? 0 : contract.owners.back();
    if (result_owner) {
      operation.targets.clear();
      if (!owners.contains(result_owner)) implementation_gap(expr.at, "result owner is not determined by borrowed arguments");
      operation.borrowed_result = operation.view = true;
      operation.read_only = !contract.const_views.empty() && contract.const_views.back();
      operation.owners = {owners.at(result_owner)};
      operation.initialized_target = true;
      for (const auto parameter : summary.returned.targets) {
        if (parameter == null_target) { operation.targets.insert(null_target); continue; }
        const auto &view = views.at(parameter);
        operation.targets.insert(view.targets.begin(), view.targets.end());
        operation.origins.insert(view.origins.begin(), view.origins.end());
        if (summary.returned.uninitialized.contains(parameter) && !view.initialized_target &&
            !std::all_of(view.targets.begin(), view.targets.end(), [&](size_t place) {
              return place == null_target || lexical_.initialized.contains(place);
            })) operation.initialized_target = false;
      }
      if (!summary.returned.targets.contains(null_target)) operation.targets.erase(null_target);
      if (contract.nullable_views.empty() || !contract.nullable_views.back()) operation.targets.erase(null_target);
      select_targets(operation, operation.targets);
      if (read && !discard && &expr != prepared_call_) read_view(operation, expr.at, "borrowed result");
    }
  }
  static void initialize_selections(LexicalState &state, const std::set<size_t> &parameters,
      const std::vector<CheckedOperation> &views) {
    for (const auto parameter : parameters) {
      const auto &view = views.at(parameter);
      if (view.binding && state.target_versions.contains(*view.binding) && state.target_versions.at(*view.binding) == view.selection_versions)
        state.initialized_targets.insert(*view.binding);
    }
  }
  void require_initialized(size_t place, const Location &at, const std::string &name) {
    if (place == null_target || !proving_ || lexical_.initialized.contains(place)) return;
    std::uint64_t missing = 0;
    for (const auto input : inputs(lexical_.inputs))
      if (!lexical_.input_initialized.at(input).contains(place)) missing |= std::uint64_t{1} << input;
    if (!missing) return;
    if (!storage(place).borrowed && (!delayed_ || !deferred_inputs_.contains(place)) && validating_)
      fail(at, "read of '" + name + "' before definite initialization of every possible target");
    const auto obligation = cleanup_.read(place);
    read_sites_.push_back(at);
    add_requirement(lexical_, obligation, missing, lexical_.initialized);
    observe_read(place, lexical_.initialized);
  }
  static void refresh_initialized(LexicalState &state) {
    bool first = true;
    for (const auto input : inputs(state.inputs)) {
      const auto &known = state.input_initialized.at(input);
      state.initialized = first ? known : intersect(state.initialized, known);
      first = false;
    }
  }
  void join(const LexicalState &a, bool a_exits, const LexicalState &b, bool b_exits, const Location &at) {
    a_exits = a_exits || !a.live;
    b_exits = b_exits || !b.live;
    lexical_ = a;
    for (size_t i = 0; i < b.scopes.size(); ++i)
      for (const auto &[name, id] : b.scopes[i].bindings) {
        auto [found, inserted] = lexical_.scopes[i].bindings.emplace(name, id);
        if (!inserted && found->second != id) fail(at, "duplicate declaration of '" + name + "' in one lexical scope");
      }
    lexical_.bindings_available = a_exits ? b.bindings_available : b_exits ? a.bindings_available : intersect(a.bindings_available, b.bindings_available);
    lexical_.available = a_exits ? b.available : b_exits ? a.available : intersect(a.available, b.available);
    lexical_.initialized = a_exits ? b.initialized : b_exits ? a.initialized : intersect(a.initialized, b.initialized);
    lexical_.returned = a_exits ? b.returned : a.returned;
    if (!a_exits && !b_exits) lexical_.returned.merge(b.returned);
    lexical_.input_initialized = a_exits ? b.input_initialized : a.input_initialized;
    lexical_.requirements = a_exits ? b.requirements : a.requirements;
    if (!a_exits && !b_exits) {
      for (const auto input : inputs(b.inputs)) {
        auto &known = lexical_.input_initialized[input];
        known = a.inputs & (std::uint64_t{1} << input) ? intersect(known, b.input_initialized.at(input)) : b.input_initialized.at(input);
      }
      for (const auto &[read, requirement] : b.requirements)
        add_requirement(lexical_, read, requirement.inputs, requirement.before);
    }
    lexical_.exceptions = a_exits ? b.exceptions : a.exceptions;
    if (!a_exits && !b_exits) for (const auto &[id, categories] : b.exceptions)
      lexical_.exceptions[id].insert(categories.begin(), categories.end());
    lexical_.targets = a_exits ? b.targets : a.targets;
    lexical_.owners = a_exits ? b.owners : a.owners;
    lexical_.expired_bindings = a_exits ? b.expired_bindings : a.expired_bindings;
    if (!a_exits && !b_exits) {
      lexical_.expired_bindings.insert(b.expired_bindings.begin(), b.expired_bindings.end());
      for (const auto &[id, owners] : b.owners) {
        lexical_.owners[id].insert(owners.begin(), owners.end());
        if (lexical_.owners[id].size() > 1) fail(at, "reference owner differs between reaching paths");
      }
    }
    lexical_.target_versions = a_exits ? b.target_versions : a.target_versions;
    lexical_.initialized_targets = a_exits ? b.initialized_targets : b_exits ? a.initialized_targets : intersect(a.initialized_targets, b.initialized_targets);
    if (!a_exits && !b_exits) for (const auto &[id, targets] : b.targets) {
      lexical_.targets[id].insert(targets.begin(), targets.end());
      const auto &versions = b.target_versions.at(id);
      lexical_.target_versions[id].insert(versions.begin(), versions.end());
    }
    lexical_.queues = a_exits ? b.queues : a.queues;
    if (!a_exits && !b_exits) for (const auto &[scope, queue] : b.queues)
      if (lexical_.queues.at(scope) != queue)
        lexical_.queues[scope] = cleanup_.join(cleanup_.known(lexical_.queues.at(scope), a.initialized), cleanup_.known(queue, b.initialized));
    lexical_.inputs = a_exits ? b.inputs : b_exits ? a.inputs : a.inputs | b.inputs;
    lexical_.live = !a_exits || !b_exits;
    refresh_initialized(lexical_);
  }
  void retire_scope(LexicalState &state) {
    const auto &scope = state.scopes.back();
    for (const auto &[name, id] : scope.bindings) {
      state.bindings_available.erase(id);
      state.targets.erase(id);
      state.owners.erase(id);
      state.expired_bindings.erase(id);
      state.target_versions.erase(id);
      state.initialized_targets.erase(id);
    }
    for (const auto &[id, owners] : state.owners) if (owners.contains(scope.id)) state.expired_bindings.insert(id);
    if (const auto owned = scope_storage_.find(scope.id); owned != scope_storage_.end()) for (const auto id : owned->second) {
      state.available.erase(id);
      state.initialized.erase(id);
      for (auto &[input, initialized] : state.input_initialized) initialized.erase(id);
      state.exceptions.erase(id);
    }
    state.queues.erase(scope.id);
    state.scopes.pop_back();
  }
  void leave_scope() {
    if (const auto owned = scope_storage_.find(lexical_.scopes.back().id); owned != scope_storage_.end())
      for (const auto id : owned->second) loop_controls_.erase(id);
    retire_scope(lexical_);
  }
  bool capture_catch(size_t id) {
    bool captured = false;
    for (auto *frame = captures_; frame; frame = frame->parent)
      if (frame->bindings.contains(id)) { frame->required.insert(id); captured = true; }
    return captured;
  }
  void note_abrupt(const Location &at, std::set<size_t> exceptions = {}, Completion completion = {2, false}) {
    if (!lexical_.live) return;
    if (!exceptions.empty()) completion.reason = 3 + *exceptions.begin();
    if (!abrupt_) {
      if (!proving_) return;
      for (const auto category : exceptions)
        if (exception_categories.at(category).checked && (!checking_method_ || !checking_method_->throws_categories.contains(category)))
          fail(at, "checked exception '" + std::string(exception_categories[category].name) + "' requires a covering catch or throws declaration");
      return;
    }
    auto state = lexical_;
    while (state.scopes.size() > abrupt_->depth) retire_scope(state);
    abrupt_->paths.push_back({std::move(state), std::move(exceptions), at, completion});
  }
  void refine_test(const CheckedOperation::NullTest &test, bool value) {
    if (!lexical_.bindings_available.contains(test.binding) || lexical_.target_versions.at(test.binding) != test.versions) return;
    auto &targets = lexical_.targets.at(test.binding);
    const bool null = value == test.null_when_true;
    std::erase_if(targets, [&](size_t place) { return (place == null_target) != null; });
    lexical_.live = lexical_.live && !targets.empty();
  }
  void refine(const Expression &expr, bool value, const std::map<const Expression *, CheckedOperation> &operations) {
    if (operations.at(&expr).type == Type::never) { lexical_.live = false; return; }
    if (const auto &test = operations.at(&expr).null_test) { refine_test(*test, value); return; }
    if (expr.kind != Expression::Kind::binary || (expr.text != "&&" && expr.text != "||")) return;
    const bool conjunction = expr.text == "&&";
    if (value == conjunction) {
      refine(*expr.children[0], value, operations); refine(*expr.children[1], value, operations);
    } else {
      const auto before = lexical_;
      refine(*expr.children[0], value, operations);
      const auto first = lexical_;
      lexical_ = before;
      refine(*expr.children[0], !value, operations); refine(*expr.children[1], value, operations);
      const auto second = lexical_;
      join(first, false, second, false, expr.at);
    }
  }
  void dereference(const CheckedOperation &view, const Location &at) {
    if (!view.targets.contains(null_target)) return;
    note_abrupt(at, {exception_id(ExceptionCategory::null_pointer)});
    if (view.binding) refine_test({*view.binding, view.selection_versions}, false);
    else if (view.targets.size() == 1) lexical_.live = false;
  }
  CheckedOperation finish_scope(CheckedOperation result, AbruptFlow &flow, AbruptFlow *outer,
                                size_t scope, const Location &at) {
    const auto body = lexical_;
    if (result.type != Type::never && lexical_.live) flow.paths.push_back({lexical_, {}, at, {0, false}});
    std::vector<LexicalState> normals;
    std::vector<AbruptPath> exits;
    std::optional<Type> normal_type;
    for (const auto &path : flow.paths) {
      const auto queue = path.state.queues.at(scope);
      std::vector<Completion> inputs;
      if (path.exceptions.empty()) inputs.push_back(path.completion);
      else for (const auto category : path.exceptions) inputs.push_back({3 + category, path.completion.returned});
      for (const auto input : inputs) for (const auto &[encoded, effects] : cleanup_.apply(queue, input))
          for (const auto origin : ScalarChecker::inputs(path.state.inputs)) {
        const auto completion = Completion::from(encoded);
        auto state = path.state;
        state.inputs = std::uint64_t{1} << origin;
        state.initialized = path.state.input_initialized.at(origin);
        for (auto &[read, requirement] : state.requirements) requirement.inputs &= state.inputs;
        apply_effects(state, effects);
        state.input_initialized = {{origin, state.initialized}};
        if (cleanup_.has_actions(queue)) {
          widen_targets(state);
          for (auto &[id, categories] : state.exceptions)
            if (!(*storage_)[id].read_only) categories = exception_possibilities((*storage_)[id].type);
        }
        state.queues[scope] = 0;
        if (!completion.reason) {
          const auto type = path.completion.reason ? Type{Type::void_type} : result.type;
          if (normal_type && *normal_type != type)
            fail(at, "scope recovery has no original value; use an explicit return in the recovering handler");
          normal_type = type;
          normals.push_back(std::move(state));
        } else exits.push_back({std::move(state), completion.reason >= 3 ? std::set<size_t>{completion.reason - 3} : std::set<size_t>{}, path.at, completion});
      }
    }
    abrupt_ = outer;
    for (const auto &path : exits) {
      lexical_ = path.state;
      note_abrupt(path.at, path.exceptions, path.completion);
    }
    lexical_ = body;
    if (normals.empty()) { lexical_.live = false; result.type = Type::never; }
    else {
      join(body, true, normals.front(), false, at);
      for (size_t i = 1; i < normals.size(); ++i) { const auto previous = lexical_; join(previous, false, normals[i], false, at); }
      result.type = *normal_type;
      if (result.type == Type::void_type) {
        result.place.reset(); result.binding.reset(); result.targets.clear(); result.origins.clear();
        result.view = result.read_only = false; result.exceptions.clear();
      }
    }
    return result;
  }
  static std::optional<std::uint64_t> constant_integer(const Expression &expr,
      const std::map<const Expression *, CheckedOperation> &operations) {
    const auto &fact = operations.at(&expr);
    if (expr.kind == Expression::Kind::integer) return fact.integer;
    if (fact.minimum_literal) return std::uint64_t{1} << 63;
    if (fact.builtin == BuiltinOperation::unary && (expr.text == "+" || expr.text == "-"))
      if (const auto value = constant_integer(*expr.children.front(), operations))
        return expr.text == "-" ? std::uint64_t{0} - *value : *value;
    return {};
  }
  Type resolve_local_type(const Type &type, const CheckedInstance *instance, const Location &at) const {
    Type resolved;
    if (instance) {
      const auto &method = program_.methods[instance->method];
      resolved = substitute(type, method, instance->substitutions);
    } else resolved = resolve_type(type);
    if (!lexical_value_type(resolved)) implementation_gap(at, "non-value local storage type '" + type_name(type) + "'");
    return resolved;
  }
  Type resolve_type(Type type, const CheckedMethod *method = nullptr, const Substitutions *bindings = nullptr,
                    bool partial = false) const {
    if (method) if (const auto index = binder(*method, type)) {
      if (!bindings) return type;
      if (const auto &bound = (*bindings)[*index]) return *bound;
      if (partial) return type;
      implementation_gap(method->source->at, "type parameter is not determined by the arguments");
    }
    if (type.kind == Type::named) if (const auto found = exception_types_.find(type.name); found != exception_types_.end()) return found->second;
    if (type.kind == Type::procedure) {
      for (auto &parameter : type.parameters) parameter = resolve_type(std::move(parameter), method, bindings, partial);
      // Throws declarations are resolved independently of argument inference.
      for (auto &failure : type.throws_types) failure = resolve_type(std::move(failure), method);
      if (type.target) type.target = std::make_shared<Type>(resolve_type(*type.target, method, bindings, partial));
    }
    return type;
  }

  const std::set<std::string> &free_type_names(const Form &method) {
    if (const auto found = free_types_.find(&method); found != free_types_.end()) return found->second;
    std::set<std::string> required;
    const std::function<void(const Type &)> type = [&](const Type &t) {
      if (t.kind == Type::named) required.insert(t.name);
      if (t.target) type(*t.target);
      for (const auto &p : t.parameters) type(p);
      for (const auto &p : t.throws_types) type(p);
      for (const auto &p : t.bounds) type(p);
    };
    for (const auto &p : method.parameters) type(p.type);
    if (method.result_type) type(*method.result_type);
    for (const auto &t : method.throws_types) type(t);
    const std::function<void(const Expression &)> visit = [&](const Expression &expr) {
      if (expr.annotation) type(*expr.annotation);
      if (expr.definition) {
        const auto &nested = free_type_names(*expr.definition);
        required.insert(nested.begin(), nested.end());
      }
      for (const auto &child : expr.children) visit(*child);
    };
    if (method.body) visit(*method.body);
    for (const auto &p : method.type_parameters) required.erase(p.name);
    return free_types_.emplace(&method, std::move(required)).first->second;
  }

  const std::vector<Capture> &free_captures(const Form &method) {
    if (const auto found = free_captures_.find(&method); found != free_captures_.end()) return found->second;
    std::vector<Capture> required;
    std::set<std::string> names;
    for (const auto &p : method.parameters) names.insert(p.name);
    for (const auto &p : method.owner_parameters) names.insert(p.name);
    if (!method.name.empty()) names.insert(method.name);
    const auto use = [&](const std::string &name, const Location &at, const std::set<std::string> &bound) {
      if (!bound.contains(name) && std::none_of(required.begin(), required.end(), [&](const auto &c) { return c.name == name; }))
        required.push_back({name, at});
    };
    const std::function<void(const Expression &, std::set<std::string> &)> visit = [&](const Expression &expr, auto &bound) {
      using K = Expression::Kind;
      if (expr.definition) {
        const auto &nested = *expr.definition;
        for (const auto &capture : free_captures(nested)) use(capture.name, capture.at, bound);
        for (const auto &capture : nested.captures) use(capture.name, capture.at, bound);
        if (!nested.name.empty()) bound.insert(nested.name);
        return;
      }
      if (expr.kind == K::pointer_group) {
        for (const auto &declaration : expr.children) {
          if (declaration->kind == K::block) { visit(*declaration, bound); continue; }
          for (const auto &child : declaration->children) visit(*child, bound);
          bound.insert(declaration->text);
        }
        return;
      }
      if (expr.kind == K::alias) {
        for (const auto &child : expr.children) visit(*child, bound);
        if (!expr.owner_name.empty()) use(expr.owner_name, expr.at, bound);
        bound.insert(expr.text);
        return;
      }
      if (expr.kind == K::range_for || expr.kind == K::fold) {
        const bool fold = expr.kind == K::fold;
        for (size_t i = 0; i < (fold ? 3 : 2); ++i) visit(*expr.children[i], bound);
        auto iteration = bound; iteration.insert(expr.text);
        if (fold) iteration.insert(expr.binding);
        visit(*expr.children[fold ? 3 : 2], iteration);
        if (!fold && expr.children.size() == 4) visit(*expr.children[3], bound);
        return;
      }
      if (expr.kind == K::handler) {
        auto caught = bound; caught.insert(expr.text);
        for (const auto &child : expr.children) visit(*child, caught);
        return;
      }
      if (expr.kind == K::name || (expr.kind == K::call && !expr.intrinsic && !swap_operation(expr.text) && !eager_operation(expr.text)))
        use(expr.text, expr.at, bound);
      auto local = bound;
      auto &scope = expr.kind == K::block && expr.scoped ? local : bound;
      for (const auto &child : expr.children) visit(*child, scope);
    };
    visit(*method.body, names);
    return free_captures_.emplace(&method, std::move(required)).first->second;
  }

  void resolve_captures(CheckedMethod &method) {
    if (method.captures_resolved) return;
    const auto &source = *method.source;
    const auto &free = free_captures(source);
    const auto required = [&](const Capture &capture) {
      return lexical_.lookup(capture.name).has_value() ||
          (!families_.contains(capture.name) && capture.name != "isnull" && capture.name != "cause_at");
    };
    if (source.explicit_captures) {
      std::set<std::string> listed;
      for (const auto &capture : source.captures) {
        if (!listed.insert(capture.name).second) fail(capture.at, "duplicate callable capture '" + capture.name + "'");
        method.captures.push_back(capture);
      }
      for (const auto &capture : free) if (required(capture) && !listed.contains(capture.name))
        fail(capture.at, "explicit capture list omits '" + capture.name + "'");
    } else for (const auto &capture : free) if (required(capture)) method.captures.push_back(capture);
    method.captures_resolved = true;
  }

  static std::optional<size_t> binder(const CheckedMethod &method, const Type &type) {
    if (type.kind != Type::named) return {};
    for (size_t i = 0; i < method.type_parameters.size(); ++i)
      if (method.type_parameters[i] == type.name) return i;
    return {};
  }
  Type substitute(const Type &type, const CheckedMethod &method, const Substitutions &bindings, bool partial = false) const {
    return resolve_type(type, &method, &bindings, partial);
  }
  static bool nested_binder(const CheckedMethod &method, const Type &type) {
    if (binder(method, type)) return true;
    return type.kind == Type::procedure &&
        ((type.target && nested_binder(method, *type.target)) ||
         std::any_of(type.parameters.begin(), type.parameters.end(), [&](const auto &p) { return nested_binder(method, p); }));
  }
  static void signature_type(const CheckedMethod &method, const Type &type, const Location &at, bool result = false) {
    if (bits_type(type) || binder(method, type) || (result && type == Type::void_type)) return;
    if (type.kind == Type::procedure && type.target) {
      if (type.variadic || !type.capture_owners.empty() || !type.capture_regions.empty() ||
          (type.owners.empty() && std::any_of(type.nullable_views.begin(), type.nullable_views.end(), [](bool v) { return v; })))
        implementation_gap(at, "callable view captures, owners or nullable passing modes");
      for (const auto &parameter : type.parameters) signature_type(method, parameter, at);
      signature_type(method, *type.target, at, true);
      if (type.owners.empty() && type.target->kind != Type::procedure && *type.target != Type::void_type &&
          (type.const_views.size() <= type.parameters.size() || !type.const_views.back()))
        implementation_gap(at, "callable views with borrowed results");
      for (const auto &failure : type.throws_types)
        if (failure.kind != Type::exception) implementation_gap(at, "callable view throws require intrinsic exceptions");
      return;
    }
    implementation_gap(at, "non-scalar signature type '" + type_name(type) + "'");
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
    return pattern == actual || (parameter && accepts_exception(actual, pattern));
  }
  static Substitutions type_context(const CheckedMethod &method, const std::vector<Type> &retained) {
    Substitutions substitutions(method.type_parameters.size());
    for (size_t i = 0; i < retained.size(); ++i) substitutions.at(method.source->type_parameters.size() + i) = retained[i];
    return substitutions;
  }
  static std::optional<Substitutions> match_prefix(const CheckedMethod &method, const std::vector<Type> &arguments,
                                                 const std::vector<Type> &retained = {}) {
    if (method.parameters.size() < arguments.size() || method.retained_types.size() != retained.size()) return {};
    auto substitutions = type_context(method, retained);
    for (size_t i = 0; i < arguments.size(); ++i) {
      if (!bind_type(method, method.parameters[i], arguments[i], substitutions, true)) return {};
    }
    return substitutions;
  }
  static std::optional<Substitutions> match(const CheckedMethod &method, const std::vector<Type> &arguments,
                                          const std::vector<Type> &retained = {}) {
    return method.parameters.size() == arguments.size() ? match_prefix(method, arguments, retained) : std::nullopt;
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
  // Signature containment includes repeated binders; layout equality is irrelevant.
  static bool implies(const CheckedMethod &narrow, const CheckedMethod &broad) {
    if (narrow.parameters.size() != broad.parameters.size()) return false;
    std::vector<Type> arguments;
    for (size_t i = 0; i < narrow.parameters.size(); ++i) {
      auto type = rigid_type(narrow, narrow.parameters[i]);
      // Each broad exception parameter admits an independent concrete category.
      if (type.kind == Type::exception && !type.identity) type.name += ".alternative." + std::to_string(i);
      arguments.push_back(std::move(type));
    }
    return match(broad, arguments).has_value();
  }
  size_t demand(size_t method, std::vector<Type> arguments, const Location &at, const std::vector<Type> &environment = {},
                const std::vector<Type> &retained = {}) {
    auto &definition = program_.methods[method];
    for (size_t i = 0; i < arguments.size(); ++i)
      if (accepts_exception(arguments[i], definition.parameters[i])) arguments[i] = definition.parameters[i];
    for (const auto id : definition.instances)
      if (program_.instances[id].arguments == arguments && program_.instances[id].environment == environment &&
          program_.instances[id].retained_types == retained) return id;
    if (program_.instances.size() == instance_limit)
      fail(at, "compiler resource limit: 4096 scalar checking obligations while preparing '" + definition.source->name + "'");
    const auto substitutions = match(definition, arguments, retained);
    if (!substitutions) throw std::logic_error("unchecked method demand");
    auto result = substitute(definition.result, definition, *substitutions);
    const auto id = program_.instances.size();
    auto context = arguments; context.insert(context.end(), environment.begin(), environment.end());
    context.insert(context.end(), retained.begin(), retained.end());
    program_.instances.push_back({method, arguments, result, {}, false, {}, {}, {}, environment, std::move(context), retained, *substitutions});
    std::set<size_t> borrowed;
    for (size_t p = 0; p < definition.source->parameters.size(); ++p)
      if (!definition.source->parameters[p].owner.empty()) borrowed.insert(p);
    for (const auto reason : call_reasons(definition)) program_.instances.back().effects.initialized[reason] = borrowed;
    definition.instances.push_back(id);
    return id;
  }
  std::vector<size_t> most_specific(const std::vector<size_t> &applicable) const {
    std::vector<size_t> best;
    for (const auto candidate : applicable) {
      const auto &definition = program_.methods[candidate];
      const bool dominated = std::any_of(applicable.begin(), applicable.end(), [&](size_t other) {
        const auto &alternative = program_.methods[other];
        return implies(alternative, definition) && !implies(definition, alternative);
      });
      if (!dominated) best.push_back(candidate);
    }
    return best;
  }
  size_t select_family(const std::vector<size_t> &family, const std::vector<Type> &arguments, const Location &at, const std::string &name,
                       const std::vector<Type> &environment = {}, const std::vector<Type> &retained = {}) {
    std::vector<size_t> applicable;
    for (const auto method : family)
      if (match(program_.methods[method], arguments, retained)) applicable.push_back(method);
    const auto best = most_specific(applicable);
    if (best.empty()) fail(at, "no applicable method for '" + name + "' (argument types or same-T relation do not match)");
    if (best.size() != 1) fail(at, "ambiguous methods for '" + name + "'");
    return demand(best.front(), arguments, at, environment, retained);
  }
  size_t select(const Expression &expr, const std::vector<Type> &arguments) {
    if (checking_method_ && lexical_methods_.contains(checking_method_->source) && expr.text == checking_method_->source->name) {
      return select_family({lexical_methods_.at(checking_method_->source)}, arguments, expr.at, expr.text,
                           checking_instance_->environment, checking_instance_->retained_types);
    }
    const auto family = families_.find(expr.text);
    if (family == families_.end()) fail(expr.at, "unknown callable '" + expr.text + "'");
    return select_family(family->second, arguments, expr.at, expr.text);
  }
  bool applicable_operation(const std::string &name, const std::vector<Type> &arguments) const {
    const auto family = families_.find(name);
    return family != families_.end() && std::any_of(family->second.begin(), family->second.end(), [&](size_t id) {
      return match(program_.methods[id], arguments).has_value();
    });
  }
  struct EagerTypes {
    Type result, operand;
    bool unary = false, failing = false;
    std::string error;
  };
  static EagerTypes eager_types(const Expression &expr, const std::vector<Type> &arguments) {
    EagerTypes types;
    types.unary = arguments.size() == 1 && (expr.text == "+" || expr.text == "-" || expr.text == "!");
    const auto error = [&](const std::string &message) { types.error = message; return types; };
    if (!types.unary && (arguments.size() != 2 || expr.text == "!")) return error("builtin operator argument count does not match");
    if (types.unary) {
      const auto &argument = arguments.front();
      if (expr.text == "!") {
        if (argument != Type::boolean) return error("logical negation requires bool");
      } else if (argument != Type::integer && !(expr.text == "-" && floating(argument)))
        return error("numeric unary operation requires i64, or floating point for negation");
      types.result = types.operand = argument;
      types.failing = expr.text == "-" && argument == Type::integer;
    } else {
      const auto &left = arguments[0], &right = arguments[1];
      const bool equality = expr.text == "==" || expr.text == "!=";
      const bool comparison = equality || expr.text == "<" || expr.text == ">" || expr.text == "<=" || expr.text == ">=";
      if (equality && left == Type::boolean && right == Type::boolean) types.operand = Type::boolean;
      else {
        if (!numeric(left) || !numeric(right)) return error("no applicable numeric operation '" + expr.text + "'");
        if (widens(left, right)) types.operand = right;
        else if (widens(right, left)) types.operand = left;
        else return error("numeric operands have no lossless common type");
        if (expr.text == "%" && floating(types.operand)) return error("remainder requires integer operands");
      }
      types.result = comparison ? Type{Type::boolean} : types.operand;
      types.failing = !comparison && integral(types.operand);
    }
    return types;
  }
  void check_eager(const Expression &expr, const std::vector<Type> &arguments,
                   const std::vector<CheckedOperation> &views, CheckedOperation &operation) {
    const auto types = eager_types(expr, arguments);
    if (!types.error.empty()) fail(expr.at, types.error);
    operation.builtin = types.unary ? BuiltinOperation::unary : BuiltinOperation::binary;
    operation.type = types.result; operation.operand_type = types.operand;
    if (types.unary && expr.text == "!") {
      operation.null_test = views.front().null_test;
      if (operation.null_test) operation.null_test->null_when_true = !operation.null_test->null_when_true;
    }
    operation.invocation.emplace(0, CleanupEquations::Effects{});
    if (types.failing) {
      operation.invocation.emplace(2, CleanupEquations::Effects{});
      if (&expr != prepared_call_) note_abrupt(expr.at);
    }
  }
  Type callable_type(const Form &method, const std::vector<Type> &arguments, const Type &result) const {
    Type type{Type::procedure}; type.pure_callable = method.kind == Form::Kind::function;
    type.parameters = arguments; type.target = std::make_shared<Type>(result);
    std::map<std::string, size_t> roles;
    const auto role = [&](const std::string &owner) {
      if (owner.empty()) return size_t{0};
      return roles.try_emplace(owner, roles.size() + 1).first->second;
    };
    for (const auto &parameter : method.parameters) {
      type.const_views.push_back(parameter.read_only);
      type.owners.push_back(role(parameter.owner));
      type.argument_modes.push_back(argument_mode(parameter));
      type.nullable_views.push_back(argument_mode(parameter) == ArgumentMode::borrowed_ptr);
    }
    type.owners.push_back(borrowed_result(method) ? role(method.result_owner) : 0);
    type.const_views.push_back(method.result_const);
    type.nullable_views.push_back(borrowed_result(method) && method.result_kind == "ptr");
    if (roles.empty()) { type.owners.clear(); type.argument_modes.clear(); }
    if (std::none_of(type.const_views.begin(), type.const_views.end(), [](bool v) { return v; })) type.const_views.clear();
    if (std::none_of(type.nullable_views.begin(), type.nullable_views.end(), [](bool v) { return v; })) type.nullable_views.clear();
    for (const auto &failure : method.throws_types) type.throws_types.push_back(resolve_type(failure));
    return type;
  }
  std::optional<Type> callable_evidence(size_t id) const {
    const auto &method = program_.methods[id];
    if (!method.source->type_parameters.empty()) return {};
    std::vector<Type> retained;
    if (!method.retained_types.empty()) {
      if (!checking_instance_ || method.parent != checking_instance_->method) return {};
      for (const auto index : method.retained_types) {
        const auto &type = checking_instance_->substitutions.at(index);
        if (!type) return {};
        retained.push_back(*type);
      }
    }
    const auto bindings = type_context(method, retained);
    auto arguments = method.parameters;
    for (auto &argument : arguments) argument = substitute(argument, method, bindings, true);
    const auto result = substitute(method.result, method, bindings, true);
    if (nested_binder(method, result) || std::any_of(arguments.begin(), arguments.end(), [&](const auto &type) { return nested_binder(method, type); })) return {};
    return callable_type(*method.source, arguments, result);
  }
  struct TypeEvidenceState {
    std::map<std::string, std::optional<Type>> bindings;
    bool complete = true;
  };
  static TypeEvidenceState merge_evidence(TypeEvidenceState left, const TypeEvidenceState &right) {
    for (auto &[name, type] : left.bindings) {
      const auto other = right.bindings.find(name);
      if (other == right.bindings.end() || other->second != type) type.reset();
    }
    for (const auto &[name, type] : right.bindings)
      if (!left.bindings.contains(name)) left.bindings.emplace(name, std::nullopt);
    left.complete = left.complete && right.complete;
    return left;
  }
  // Evidence only: no reads, captures, demands, storage or cleanup equations are created here.
  // Unknown binding effects invalidate subsequent name queries; the ordered checker validates every operand.
  std::optional<Type> type_evidence(const Expression &expr, TypeEvidenceState &state, bool contextual = false) const {
    using K = Expression::Kind;
    const auto child = [&](size_t i, bool context = false) { return type_evidence(*expr.children.at(i), state, context); };
    const auto annotation = [&](const Type &type) {
      return checking_instance_ ? substitute(type, *checking_method_, checking_instance_->substitutions, true) : resolve_type(type);
    };
    switch (expr.kind) {
    case K::integer:
      if (expr.text.ends_with("u8")) return Type::uint8;
      if (expr.text.ends_with("u32")) return Type::uint32;
      if (expr.text.ends_with("u64")) return Type::uint64;
      return Type::integer;
    case K::real: return expr.text.ends_with("f32") ? Type::float32 : Type::float64;
    case K::boolean: return Type::boolean;
    case K::null_ref: return Type::null_ref;
    case K::name:
      if (!state.complete) return {};
      if (const auto found = state.bindings.find(expr.text); found != state.bindings.end()) return found->second;
      if (!contextual) if (const auto family = families_.find(expr.text); family != families_.end() && family->second.size() == 1)
        return callable_evidence(family->second.front());
      return {};
    case K::block: {
      auto inner = state;
      auto &active = expr.scoped ? inner : state;
      std::optional<Type> result = Type::void_type;
      bool exits = false;
      for (size_t i = 0; i < expr.children.size(); ++i) {
        result = type_evidence(*expr.children[i], active, contextual && i + 1 == expr.children.size());
        exits = exits || result == Type::never;
      }
      return exits ? std::optional<Type>{Type::never} : result;
    }
    case K::pointer_group:
      for (const auto &declaration : expr.children) {
        if (declaration->kind == K::block) { type_evidence(*declaration, state); continue; }
        const auto initial = type_evidence(*declaration->children.front(), state, declaration->annotation.has_value());
        state.bindings[declaration->text] = declaration->annotation ? annotation(*declaration->annotation) : initial;
      }
      return Type::void_type;
    case K::alias: {
      const auto type = child(0);
      state.bindings[expr.text] = type;
      return Type::void_type;
    }
    case K::make_local:
      for (const auto &item : expr.children) type_evidence(*item, state);
      return expr.annotation ? std::optional<Type>{annotation(*expr.annotation)} : std::nullopt;
    case K::transfer: case K::rebind: {
      const auto type = child(0); child(1); return type;
    }
    case K::conditional: {
      child(0);
      auto yes_state = state, no_state = state;
      const auto yes = type_evidence(*expr.children[1], yes_state, contextual), no = type_evidence(*expr.children[2], no_state, contextual);
      state = merge_evidence(std::move(yes_state), no_state);
      if (yes == Type::never) return no;
      if (no == Type::never || yes == no) return yes;
      return {};
    }
    case K::try_scope: {
      const auto before = state;
      auto result = child(0, contextual);
      const auto body_state = state;
      const auto catch_entry = merge_evidence(before, body_state);
      for (size_t i = 1; i < expr.children.size(); ++i) {
        const auto &handler = *expr.children[i];
        if (handler.kind == K::block) {
          if (type_evidence(handler, state) == Type::never) result = Type::never;
          break;
        }
        auto caught = catch_entry;
        caught.bindings[handler.text] = handler.annotation ? annotation(*handler.annotation) : exception_type(0);
        const auto type = type_evidence(*handler.children.front(), caught, contextual);
        if (const auto outer = catch_entry.bindings.find(handler.text); outer != catch_entry.bindings.end())
          caught.bindings[handler.text] = outer->second;
        else caught.bindings.erase(handler.text);
        state = merge_evidence(std::move(state), caught);
        if (result == Type::never) result = type;
        else if (type != Type::never && type != result) result.reset();
      }
      return result;
    }
    case K::range_for: case K::fold: {
      const bool fold = expr.kind == K::fold;
      const auto index = child(0); child(1);
      const auto accumulator = fold ? child(2) : std::optional<Type>{Type::void_type};
      auto iteration = state;
      iteration.bindings[expr.text] = index;
      if (fold) iteration.bindings[expr.binding] = accumulator;
      type_evidence(*expr.children[fold ? 3 : 2], iteration);
      if (!fold && expr.children.size() == 4) {
        auto empty = state; type_evidence(*expr.children[3], empty);
        state = merge_evidence(std::move(state), empty);
      }
      return accumulator;
    }
    case K::binary:
      if (expr.text == "&&" || expr.text == "||") {
        child(0); auto skipped = state; child(1);
        state = merge_evidence(std::move(skipped), state);
        return Type::boolean;
      }
      [[fallthrough]];
    case K::unary: case K::call: case K::value_call: {
      std::optional<Type> target;
      bool local = false;
      if (expr.kind == K::value_call) { target = child(0); local = true; }
      else if (!expr.intrinsic && state.complete) {
        if (const auto found = state.bindings.find(expr.text); found != state.bindings.end()) {
          target = found->second; local = true;
        }
      }
      std::vector<Type> arguments;
      bool complete = true;
      for (size_t i = expr.kind == K::value_call ? 1 : 0; i < expr.children.size(); ++i) {
        const auto type = child(i, true); complete = complete && type.has_value();
        arguments.push_back(type.value_or(Type::never));
      }
      if (local) return target && target->kind == Type::procedure && target->target ? std::optional<Type>{*target->target} : std::nullopt;
      if (!state.complete) return {};
      if (expr.text == "isnull") return Type::boolean;
      if (swap_operation(expr.text)) return Type::void_type;
      if (expr.text == "cause_at" && (expr.intrinsic || !families_.contains(expr.text))) return exception_type(0);
      if (!complete) return {};
      std::vector<size_t> applicable;
      std::vector<Type> retained;
      if (!expr.intrinsic) {
        if (checking_method_ && lexical_methods_.contains(checking_method_->source) && expr.text == checking_method_->source->name) {
          retained = checking_instance_->retained_types;
          const auto id = lexical_methods_.at(checking_method_->source);
          if (match(program_.methods[id], arguments, retained)) applicable.push_back(id);
        } else if (const auto family = families_.find(expr.text); family != families_.end())
          for (const auto id : family->second) if (match(program_.methods[id], arguments)) applicable.push_back(id);
      }
      if (applicable.empty() && eager_operation(expr.text)) {
        const auto types = eager_types(expr, arguments);
        return types.error.empty() ? std::optional<Type>{types.result} : std::nullopt;
      }
      const auto best = most_specific(applicable);
      if (best.size() != 1) return {};
      const auto &method = program_.methods[best.front()];
      const auto type = substitute(method.result, method, *match(method, arguments, retained), true);
      return nested_binder(method, type) ? std::nullopt : std::optional<Type>{type};
    }
    case K::field: {
      const auto type = child(0);
      return type && type->kind == Type::exception ? std::optional<Type>{Type::uint64} : std::nullopt;
    }
    case K::behave: case K::perform:
      for (const auto &item : expr.children) type_evidence(*item, state);
      return print_symbols_.contains(expr.text) ? std::optional<Type>{Type::void_type} : std::nullopt;
    case K::deferred:
      if (expr.children.front()->kind != K::block) child(0);
      return Type::void_type;
    case K::return_value: case K::raise:
      for (const auto &item : expr.children) type_evidence(*item, state);
      return Type::never;
    case K::callable:
      if (!expr.definition->name.empty()) {
        state.bindings[expr.definition->name] = callable_evidence(lexical_methods_.at(expr.definition.get()));
        return Type::void_type;
      }
      return contextual ? std::nullopt : callable_evidence(lexical_methods_.at(expr.definition.get()));
    default:
      state.complete = false;
      return {};
    }
  }
  std::vector<std::optional<Type>> argument_evidence(const Expression &expr) const {
    std::vector<std::optional<Type>> result(expr.children.size());
    if (expr.intrinsic) return result;
    const auto family = families_.find(expr.text);
    if (family == families_.end()) return result;
    std::vector<ArgumentMode> modes;
    bool needed = false;
    for (const auto id : family->second) {
      const auto &parameters = program_.methods[id].source->parameters;
      if (parameters.size() != expr.children.size()) continue;
      if (modes.empty()) for (const auto &parameter : parameters) modes.push_back(argument_mode(parameter));
      for (size_t i = 0; i < parameters.size(); ++i)
        needed = needed || modes[i] != argument_mode(parameters[i]) || (eager_operation(expr.text) && modes[i] != ArgumentMode::value);
    }
    if (!needed) return result;
    TypeEvidenceState state;
    for (const auto &scope : lexical_.scopes) for (const auto &[name, id] : scope.bindings) state.bindings[name] = bindings_->at(id).type;
    for (size_t i = 0; i < expr.children.size(); ++i) result[i] = type_evidence(*expr.children[i], state, true);
    return result;
  }
  CheckedOperation materialize(const std::vector<size_t> &family, const Type *expected, const Location &at) {
    auto &definition = program_.methods[family.front()];
    if (lexical_methods_.contains(definition.source)) resolve_captures(definition);
    if (expected) signature_type(CheckedMethod{}, *expected, at);
    if (!expected && (family.size() != 1 || !program_.methods[family.front()].source->type_parameters.empty()))
      implementation_gap(at, "overloaded or generic callable values require an explicit concrete view type");
    std::vector<Type> retained;
    if (!definition.retained_types.empty()) {
      if (!checking_instance_ || definition.parent != checking_instance_->method)
        throw std::logic_error("callable materialized outside its lexical type context");
      for (const auto index : definition.retained_types) {
        const auto &type = checking_instance_->substitutions.at(index);
        if (!type) implementation_gap(at, "enclosing type parameter is not determined by the arguments");
        retained.push_back(*type);
      }
    }
    auto arguments = expected ? expected->parameters : definition.parameters;
    if (!expected) for (auto &type : arguments) type = substitute(type, definition, type_context(definition, retained));
    std::vector<Type> environment;
    std::vector<CheckedOperation> captures;
    for (const auto &capture : definition.captures) {
      Expression name{}; name.kind = Expression::Kind::name; name.at = capture.at; name.text = capture.name;
      const auto value = !lexical_.lookup(capture.name) && families_.contains(capture.name)
          ? materialize(families_.at(capture.name), nullptr, capture.at) : read_binding(name);
      if (value.type.kind != Type::procedure || (value.binding && !bindings_->at(*value.binding).callable_value))
        implementation_gap(capture.at, "capture of ordinary lexical cells or non-callable values");
      environment.push_back(value.type); captures.push_back(value);
    }
    if (validating_ && definition.source->explicit_captures && warned_captures_.insert(definition.source).second)
      for (const auto &capture : definition.captures)
        if (std::none_of(free_captures(*definition.source).begin(), free_captures(*definition.source).end(),
                         [&](const auto &used) { return used.name == capture.name; }))
          program_.warnings.push_back({Warning::unused_capture, capture.at, "unused callable capture '" + capture.name + "' remains retained"});
    const auto selected = select_family(family, arguments, at, "callable value", environment, retained);
    const auto &instance = program_.instances[selected];
    const auto &method = *program_.methods[instance.method].source;
    auto type = callable_type(method, arguments, instance.result);
    signature_type(definition, type, at);
    if (expected) {
      std::set<size_t> allowed;
      for (const auto &failure : expected->throws_types) {
        const auto categories = exception_possibilities(failure); allowed.insert(categories.begin(), categories.end());
      }
      for (const auto category : program_.methods[instance.method].throws_categories)
        if (exception_categories[category].checked && !allowed.contains(category)) fail(at, "callable view omits a required checked exception");
      type.throws_types = expected->throws_types;
      if (type != *expected) fail(at, "selected method does not satisfy the callable view contract");
    }
    const auto found = std::find_if(program_.callables.begin(), program_.callables.end(), [&](const auto &value) {
      return value.instance == selected && value.type == type;
    });
    const auto id = std::distance(program_.callables.begin(), found);
    if (found == program_.callables.end()) program_.callables.push_back({type, selected});
    CheckedOperation value; value.type = type; value.callable = id; value.callable_captures = std::move(captures);
    if (!environment.empty()) note_abrupt(at, {exception_id(ExceptionCategory::out_of_memory)});
    return value;
  }
  CheckedOperation check_guarded(const Expression &expr, CheckedInstance *instance,
      std::map<const Expression *, CheckedOperation> &operations, bool read = true, bool discard = false, const Type *expected = nullptr) {
    const auto before = lexical_;
    auto *outer = abrupt_;
    AbruptFlow incoming{lexical_.scopes.size(), {}};
    if (expr.kind == Expression::Kind::deferred) {
      for (size_t category = 1; category < exception_categories.size(); ++category) {
        auto state = lexical_; state.inputs = std::uint64_t{1} << category;
        state.input_initialized = {{category, state.initialized}};
        incoming.paths.push_back({std::move(state), {category}, expr.at, Completion::failure(category)});
      }
      auto state = lexical_; state.inputs = std::uint64_t{1} << exception_categories.size();
      state.input_initialized = {{exception_categories.size(), state.initialized}};
      incoming.paths.push_back({std::move(state), {}, expr.at, {2, false}});
    }
    abrupt_ = &incoming;
    const auto body = check(*expr.children[0], instance, operations, read, discard, expected);
    if (read && !discard && body.kind == Type::procedure) note_abrupt(expr.at, {exception_id(ExceptionCategory::out_of_memory)});
    const auto body_state = lexical_;
    std::vector<AbruptPath> control_inputs;
    for (const auto &path : incoming.paths) if (path.exceptions.empty()) control_inputs.push_back(path);
    std::vector<LexicalState> normals;
    std::vector<const CheckedOperation *> views;
    if (body != Type::never) { normals.push_back(body_state); views.push_back(&operations.at(expr.children[0].get())); }
    Type result_type = body;
    auto result_exceptions = body == Type::never ? std::set<size_t>{} : operations.at(expr.children[0].get()).exceptions;
    std::set<size_t> handled;
    AbruptFlow outgoing{before.scopes.size(), {}};
    const Expression *finalizer = nullptr;
    for (size_t i = 1; i < expr.children.size(); ++i) {
      const auto &handler = *expr.children[i];
      if (handler.kind == Expression::Kind::block) { finalizer = &handler; break; }
      program_.exception_values = true;
      const auto type = !handler.annotation ? exception_type(0) : instance
          ? substitute(*handler.annotation, program_.methods[instance->method], instance->substitutions)
          : resolve_type(*handler.annotation);
      if (type.kind != Type::exception) fail(handler.at, "catch requires a selected intrinsic exception type");
      if (handler.annotation && !type.identity) fail(handler.at, "use an untyped catch for SystemException");
      if (!handled.insert(type.identity).second) fail(handler.at, "duplicate exception category in catch clauses");
      std::vector<LexicalState> entries;
      std::set<size_t> categories;
      std::map<size_t, std::uint64_t> category_inputs;
      for (auto &path : incoming.paths) {
        auto selected = type.identity ? intersect(path.exceptions, {type.identity}) : path.exceptions;
        if (selected.empty()) continue;
        entries.push_back(path.state);
        categories.insert(selected.begin(), selected.end());
        for (const auto category : selected) category_inputs[category] |= path.state.inputs;
        for (const auto category : selected) path.exceptions.erase(category);
      }
      lexical_ = body_state;
      if (entries.empty()) join(body_state, true, before, false, handler.at);
      else {
        join(body_state, true, entries.front(), false, handler.at);
        for (size_t p = 1; p < entries.size(); ++p) { const auto joined = lexical_; join(joined, false, entries[p], false, handler.at); }
      }
      lexical_.returned = {};
      lexical_.scopes.push_back({next_scope_++, {}});
      const auto id = declare(handler.text, type, true, true, handler.at);
      lexical_.scopes.back().declarations = false;
      initialize(id);
      lexical_.exceptions[id] = categories.empty() ? exception_possibilities(type) : categories;
      auto active = caught_; caught_ = id; catch_bindings_.insert(id);
      caught_inputs_[id] = std::move(category_inputs);
      AbruptFlow handler_exits{before.scopes.size(), {}};
      abrupt_ = &handler_exits;
      const auto handled_type = check(*handler.children.front(), instance, operations, read, discard, expected);
      if (read && !discard && handled_type.kind == Type::procedure) note_abrupt(handler.at, {exception_id(ExceptionCategory::out_of_memory)});
      caught_ = active; catch_bindings_.erase(id);
      caught_inputs_.erase(id);
      leave_scope();
      CheckedOperation fact; fact.type = type; fact.place = id; fact.exceptions = categories;
      operations[&handler] = fact;
      if (handled_type != Type::never) {
        if (result_type != Type::never && result_type != handled_type && expr.kind != Expression::Kind::deferred)
          fail(handler.at, "try body and catch arms must have the same result type");
        result_type = handled_type;
        if (!entries.empty()) {
          normals.push_back(lexical_);
          views.push_back(&operations.at(handler.children.front().get()));
          const auto &values = operations.at(handler.children.front().get()).exceptions;
          result_exceptions.insert(values.begin(), values.end());
        }
      }
      if (!entries.empty()) outgoing.paths.insert(outgoing.paths.end(), handler_exits.paths.begin(), handler_exits.paths.end());
    }
    for (auto &path : incoming.paths) if (!path.exceptions.empty()) outgoing.paths.push_back(path);
    // Empty-category paths include returns and terminal numeric failures; catches cannot handle them.
    std::vector<LexicalState> final_inputs = normals;
    for (const auto &path : control_inputs) final_inputs.push_back(path.state);
    for (const auto &path : outgoing.paths) final_inputs.push_back(path.state);
    lexical_ = body_state;
    if (!final_inputs.empty()) {
      join(body_state, true, final_inputs.front(), false, expr.at);
      for (size_t i = 1; i < final_inputs.size(); ++i) { const auto joined = lexical_; join(joined, false, final_inputs[i], false, expr.at); }
    }
    AbruptFlow final_exits{before.scopes.size(), {}};
    abrupt_ = &final_exits;
    const auto final_returned = lexical_.returned;
    if (finalizer) lexical_.returned = {};
    const auto cleanup = finalizer ? check(*finalizer, instance, operations) : Type{Type::void_type};
    if (cleanup != Type::void_type && cleanup != Type::never) fail(expr.at, "finally must complete with void or an explicit return");
    const auto final_state = lexical_;
    abrupt_ = outer;
    for (const auto &path : final_exits.paths) {
      lexical_ = path.state;
      lexical_.returned = final_returned;
      lexical_.returned.then(path.state.returned);
      lexical_.returned.initialize(lexical_.initialized);
      if (path.completion.reason == 1) note_abrupt(path.at, path.exceptions, path.completion);
      else {
        bool pending_return = false, pending_normal = !normals.empty();
        for (const auto *paths : {&outgoing.paths, &control_inputs}) for (const auto &input : *paths) {
          pending_return = pending_return || input.completion.returned;
          pending_normal = pending_normal || !input.completion.returned;
        }
        if (pending_normal) note_abrupt(path.at, path.exceptions, path.completion);
        if (pending_return) { auto retained = path.completion; retained.returned = true; note_abrupt(path.at, path.exceptions, retained); }
      }
    }
    if (cleanup != Type::never) {
      for (const auto *paths : {&outgoing.paths, &control_inputs}) for (const auto &path : *paths) {
        lexical_ = finalizer ? final_state : path.state;
        lexical_.returned = path.state.returned;
        if (finalizer) lexical_.returned.then(final_state.returned);
        lexical_.returned.initialize(lexical_.initialized);
        lexical_.inputs = finalizer ? path.state.inputs & final_state.inputs : path.state.inputs;
        lexical_.live = lexical_.live && lexical_.inputs;
        refresh_initialized(lexical_);
        note_abrupt(path.at, path.exceptions, path.completion);
      }
    }
    lexical_ = body_state;
    if (!normals.empty()) {
      join(body_state, true, normals.front(), false, expr.at);
      for (size_t i = 1; i < normals.size(); ++i) { const auto joined = lexical_; join(joined, false, normals[i], false, expr.at); }
    }
    if (finalizer) {
      const auto normal = lexical_;
      lexical_ = final_state;
      lexical_.returned = normal.returned;
      lexical_.returned.then(final_state.returned);
      lexical_.returned.initialize(final_state.initialized);
      lexical_.bindings_available.insert(normal.bindings_available.begin(), normal.bindings_available.end());
      lexical_.available.insert(normal.available.begin(), normal.available.end());
      lexical_.initialized.insert(normal.initialized.begin(), normal.initialized.end());
      lexical_.inputs = normal.inputs;
      lexical_.live = normal.live;
      for (const auto input : inputs(normal.inputs)) {
        const auto &known = normal.input_initialized.at(input);
        lexical_.input_initialized[input].insert(known.begin(), known.end());
      }
      refresh_initialized(lexical_);
    }
    if (cleanup == Type::never || normals.empty()) lexical_.live = false;
    CheckedOperation result;
    result.type = cleanup == Type::never || normals.empty() ? Type{Type::never} :
        expr.kind == Expression::Kind::deferred ? Type{Type::void_type} : result_type;
    if (!views.empty()) {
      result.place = views.front()->place;
      result.binding = views.front()->binding;
      result.initialized_target = true;
      for (const auto *view : views) {
        result.view = result.view || view->view;
        result.read_only = result.read_only || view->read_only;
        result.fixed_binding = result.fixed_binding || view->fixed_binding;
        if (result.place != view->place) result.place.reset();
        if (result.binding != view->binding) result.binding.reset();
        result.targets.insert(view->targets.begin(), view->targets.end());
        result.owners.insert(view->owners.begin(), view->owners.end());
        result.origins.insert(view->origins.begin(), view->origins.end());
        result.selection_versions.insert(view->selection_versions.begin(), view->selection_versions.end());
        result.initialized_target = result.initialized_target && view->initialized_target;
      }
      result.mixed_view = std::any_of(views.begin(), views.end(), [](const auto *view) { return !view->view || view->mixed_view; });
      if (result.mixed_view) result.targets.clear();
      select_targets(result, result.targets);
    }
    result.exceptions = std::move(result_exceptions);
    return result;
  }
  CheckedOperation read_binding(const Expression &expr, bool read = true) {
    CheckedOperation operation;
    const auto binding = lexical_.lookup(expr.text);
    if (!binding) fail(expr.at, "unknown scalar value '" + expr.text + "'");
    if (!lexical_.bindings_available.contains(*binding)) fail(expr.at, "binding '" + expr.text + "' is not available on every reaching path");
    const auto &slot = (*bindings_)[*binding];
    if (slot.owner_binding) fail(expr.at, "owner binding '" + expr.text + "' is not a scalar value");
    const auto selected = slot.alias.value_or(*binding);
    if (!lexical_.bindings_available.contains(selected))
      fail(expr.at, "aliased binding's lexical scope has ended");
    if (lexical_.live && lexical_.expired_bindings.contains(selected))
      fail(expr.at, "binding '" + expr.text + "' refers to storage whose lexical owner has ended or may expire across a loop backedge");
    if (lexical_.live) for (const auto owner : lexical_.owners.at(selected))
      if (!owner_alive(owner))
        fail(expr.at, "binding '" + expr.text + "' refers to storage whose lexical owner has ended");
    operation.binding = selected;
    operation.owners = lexical_.owners.at(selected);
    operation.selection_versions = lexical_.target_versions.at(selected);
    select_targets(operation, lexical_.targets.at(selected));
    operation.origins = {selected};
    operation.type = slot.type;
    operation.read_only = slot.read_only;
    operation.fixed_binding = slot.fixed;
    operation.view = true;
    operation.initialized_target = lexical_.initialized_targets.contains(selected) ||
        std::all_of(operation.targets.begin(), operation.targets.end(), [&](size_t place) { return lexical_.initialized.contains(place); });
    if (read) dereference(operation, expr.at);
    for (const auto place : operation.targets) {
      if (place == null_target) continue;
      if (delayed_controls_.contains(place)) fail(expr.at, "a deferred block cannot capture loop-control binding '" + expr.text + "'");
      const bool captured = capture_catch(place);
      operation.snapshot = operation.snapshot || captured;
      if (captured) operation.captured_place = place;
      if (read && !operation.initialized_target) require_initialized(place, expr.at, expr.text);
      if (operation.type.kind == Type::exception) {
        const auto known = lexical_.exceptions.find(place);
        const auto categories = (delayed_ && !captured) || known == lexical_.exceptions.end() ? exception_possibilities(operation.type) : known->second;
        operation.exceptions.insert(categories.begin(), categories.end());
      }
    }
    return operation;
  }
  Type check(const Expression &expr, CheckedInstance *instance,
             std::map<const Expression *, CheckedOperation> &operations, bool read = true, bool discard = false, const Type *expected = nullptr) {
    using K = Expression::Kind;
    CheckedOperation operation;
    const auto child = [&](size_t i, bool read_value = true, bool discarded = false) { return check(*expr.children.at(i), instance, operations, read_value, discarded); };
    const auto view = [&](const CheckedOperation &source) {
      operation.place = source.place;
      operation.binding = source.binding;
      operation.targets = source.targets;
      operation.owners = source.owners;
      operation.origins = source.origins;
      operation.selection_versions = source.selection_versions;
      operation.initialized_target = source.initialized_target;
      operation.mixed_view = source.mixed_view;
      operation.view = source.view;
      operation.read_only = source.read_only;
      operation.fixed_binding = source.fixed_binding;
    };
    const auto require_value = [&](const Type &type) {
      if (!lexical_value_type(type)) implementation_gap(expr.at, "non-value or abruptly completing operand");
    };
    const auto &method = instance ? program_.methods[instance->method].source : nullptr;
    if (instance && !(lexical_methods_.contains(method) && expr.text == method->name) &&
        !lexical_.lookup(expr.text) && module_values_.contains(expr.text) &&
        (expr.kind == K::name || expr.kind == K::call || expr.kind == K::perform || expr.kind == K::behave))
      implementation_gap(expr.at, "module-local binding lookup and capture in callable bodies");
    if (expr.spread) implementation_gap(expr.at, "spread arguments");
    switch (expr.kind) {
    case K::integer: {
      auto digits = std::string_view(expr.text);
      operation.type = Type::integer;
      if (digits.ends_with("u8")) { operation.type = Type::uint8; digits.remove_suffix(2); }
      else if (digits.ends_with("u32")) { operation.type = Type::uint32; digits.remove_suffix(3); }
      else if (digits.ends_with("u64")) { operation.type = Type::uint64; digits.remove_suffix(3); }
      const auto parsed = std::from_chars(digits.data(), digits.data() + digits.size(), operation.integer);
      const auto limit = operation.type == Type::uint8 ? std::uint64_t{255} :
          operation.type == Type::uint32 ? std::uint64_t{0xffffffff} :
          operation.type == Type::integer ? std::uint64_t{0x7fffffffffffffff} : std::numeric_limits<std::uint64_t>::max();
      if (parsed.ec != std::errc{} || parsed.ptr != digits.data() + digits.size() || operation.integer > limit)
        fail(expr.at, "integer literal is outside " + type_name(operation.type));
      break;
    }
    case K::real: {
      operation.type = expr.text.ends_with("f32") ? Type::float32 : Type::float64;
      auto digits = std::string_view(expr.text);
      if (digits.ends_with("f32") || digits.ends_with("f64")) digits.remove_suffix(3);
      const auto in_range = [&](auto value) {
        const auto parsed = std::from_chars(digits.data(), digits.data() + digits.size(), value);
        return parsed.ec == std::errc{} && parsed.ptr == digits.data() + digits.size() && std::isfinite(value);
      };
      if (!(operation.type == Type::float32 ? in_range(float{}) : in_range(double{})))
        fail(expr.at, "floating literal is outside " + type_name(operation.type) + " range");
      break;
    }
    case K::boolean: operation.type = Type::boolean; operation.integer = expr.text == "true"; break;
    case K::null_ref:
      if (read) implementation_gap(expr.at, "untyped null in a payload expression; bind a typed ptr view first");
      operation.type = Type::null_ref; operation.view = true; select_targets(operation, {null_target}); break;
    case K::name: {
      if (!lexical_.lookup(expr.text) && families_.contains(expr.text))
        operation = materialize(families_.at(expr.text), expected, expr.at);
      else operation = read_binding(expr, read);
      break;
    }
    case K::callable: {
      operation = materialize({lexical_methods_.at(expr.definition.get())}, expected, expr.at);
      if (!expr.definition->name.empty()) {
        const auto id = declare(expr.definition->name, operation.type, false, true, expr.at);
        initialize(id); operation.place = id;
        operation.binding = lexical_.lookup(expr.definition->name);
        bindings_->at(*operation.binding).callable_value = true;
        operation.operand_type = operation.type; operation.type = Type::void_type;
      }
      break;
    }
    case K::unary: {
      if (expr.text == "-" && expr.children.front()->kind == K::integer) {
        auto magnitude = std::string_view(expr.children.front()->text);
        while (magnitude.starts_with('0')) magnitude.remove_prefix(1);
        if (magnitude == "9223372036854775808") {
          if (applicable_operation(expr.text, {Type::integer}))
            implementation_gap(expr.at, "signed minimum literal with overloaded unary negation");
          operation.type = operation.operand_type = Type::integer;
          operation.minimum_literal = true;
          break;
        }
      }
      [[fallthrough]];
    }
    case K::binary: {
      if (expr.text == "&&" || expr.text == "||") {
        const auto left = child(0);
        const auto before_right = lexical_;
        refine(*expr.children[0], expr.text == "&&", operations);
        const auto right = child(1);
        const auto after_right = lexical_;
        lexical_ = before_right;
        refine(*expr.children[0], expr.text != "&&", operations);
        const auto skipped = lexical_;
        join(skipped, false, after_right, right == Type::never, expr.at);
        if ((left != Type::boolean && left != Type::never) || (right != Type::boolean && right != Type::never))
          fail(expr.at, "logical operation requires bool operands");
        operation.operand_type = Type::boolean;
        operation.type = left == Type::never ? Type{Type::never} : Type{Type::boolean};
        break;
      }
      [[fallthrough]];
    }
    case K::value_call: case K::call: {
      const bool eager = expr.kind != K::value_call && eager_operation(expr.text);
      if (expr.kind == K::call && swap_operation(expr.text)) {
        if (&expr == prepared_call_) implementation_gap(expr.at, "deferred storage intrinsics; use a defer block");
        if (expr.children.size() != 2) fail(expr.at, "swap requires two destinations");
        child(0, false);
        auto left = operations.at(expr.children[0].get());
        child(1, false);
        auto right = operations.at(expr.children[1].get());
        if (!left.view || !right.view) fail(expr.at, "swap requires two storage destinations");
        if (left.mixed_view || right.mixed_view) implementation_gap(expr.at, "swap through a mixed value/storage view");
        operation.builtin = expr.text == "<=>" ? BuiltinOperation::reference_swap : BuiltinOperation::bits_swap;
        if (operation.builtin == BuiltinOperation::reference_swap) {
          left = current_selection(left, expr.at);
          right = current_selection(right, expr.at);
          check_rebind(left, right, expr.at);
          check_rebind(right, left, expr.at);
          if (left.binding != right.binding) { rebind(left, right); rebind(right, left); }
        } else {
          if (!bits_type(left.type) || !bits_type(right.type)) fail(expr.at, "builtin bits swap requires two bits storage destinations");
          if (left.type != right.type) fail(expr.at, "bits swap requires the exact same type");
          if (left.read_only || right.read_only) fail(expr.at, "cannot swap bits through a readonly destination");
          require_live_view(left, expr.at); require_live_view(right, expr.at);
          read_view(left, expr.at, "left swap target"); read_view(right, expr.at, "right swap target");
          const auto left_exceptions = stored_exceptions(left), right_exceptions = stored_exceptions(right);
          write_exceptions(left, right_exceptions); write_exceptions(right, left_exceptions);
        }
        break;
      }
      if (expr.kind == K::value_call || (!expr.intrinsic && lexical_.lookup(expr.text))) {
        const size_t offset = expr.kind == K::value_call ? 1 : 0;
        CheckedOperation target;
        if (offset) { child(0); target = operations.at(expr.children.front().get()); }
        else target = read_binding(expr);
        if (target.type.kind != Type::procedure || !target.type.target)
          fail(expr.at, "nearest binding '" + expr.text + "' is not callable");
        note_abrupt(expr.at, {exception_id(ExceptionCategory::out_of_memory)});
        const auto &contract = target.type;
        if (method && method->kind == Form::Kind::function && !contract.pure_callable)
          fail(expr.at, "a function cannot invoke a procedure");
        if (expr.children.size() - offset != contract.parameters.size()) fail(expr.at, "callable argument count does not match its view");
        operation.callable_call = true; operation.operand_type = contract;
        if (!offset) {
          operation.callable_binding = target.binding; operation.callable_place = target.place;
          operation.callable_nullable = target.targets.contains(null_target);
        }
        if (offset) operation.arguments.push_back({ArgumentMode::value, contract});
        bool completes = true;
        std::vector<Type> arguments;
        std::vector<CheckedOperation> views;
        for (size_t i = offset; i < expr.children.size(); ++i) {
          const auto &parameter = contract.parameters[i - offset];
          const auto mode = contract.argument_modes.empty() ? ArgumentMode::value : contract.argument_modes.at(i - offset);
          const auto argument = check(*expr.children[i], instance, operations, mode == ArgumentMode::value, false,
                                      parameter.kind == Type::procedure ? &parameter : nullptr);
          if (mode == ArgumentMode::value && argument.kind == Type::procedure)
            note_abrupt(expr.children[i]->at, {exception_id(ExceptionCategory::out_of_memory)});
          completes = completes && argument != Type::never;
          if (argument != Type::never && argument != parameter && !accepts_exception(argument, parameter)) {
            if (transfers_value(argument, parameter)) implementation_gap(expr.children[i]->at, "callable view argument adaptation");
            fail(expr.children[i]->at, "argument does not satisfy the callable view type");
          }
          arguments.push_back(argument); views.push_back(operations.at(expr.children[i].get()));
          if (argument != Type::never) prepare_borrow(views.back(), mode, expr.children[i]->at);
          operation.arguments.push_back({mode, parameter});
        }
        if (!completes) { operation.type = Type::never; break; }
        finish_call(expr, contract, callable_effects(contract), call_reasons(contract), arguments, views,
                    operation, read, discard, offset);
        break;
      }
      if (expr.text == "isnull") {
        if (&expr == prepared_call_) implementation_gap(expr.at, "deferred reference intrinsics; use a defer block");
        if (expr.children.size() != 1) fail(expr.at, "isnull requires one reference view");
        child(0, false);
        const auto &argument = operations.at(expr.children[0].get());
        if (!argument.view || argument.mixed_view) fail(expr.at, "isnull requires a reference view without loading its target");
        if (lexical_.live) for (const auto owner : argument.owners)
          if (!owner_alive(owner))
            fail(expr.at, "null-test view's lexical owner has ended");
        operation.type = Type::boolean; operation.builtin = BuiltinOperation::is_null;
        if (argument.binding) operation.null_test = CheckedOperation::NullTest{*argument.binding, argument.selection_versions};
        break;
      }
      if (expr.intrinsic && !eager && expr.text != "cause_at")
        implementation_gap(expr.at, "explicit intrinsic has no implemented builtin '" + expr.text + "'");
      std::vector<Type> arguments;
      std::vector<CheckedOperation> views;
      const auto evidence = argument_evidence(expr);
      for (size_t i = 0; i < expr.children.size(); ++i) {
        const auto preparation = argument_preparation(expr, arguments, evidence);
        const auto mode = preparation.mode;
        arguments.push_back(check(*expr.children[i], instance, operations, mode == ArgumentMode::value, false,
                                  preparation.view ? &*preparation.view : nullptr));
        operation.arguments.push_back({mode, arguments.back()});
        require_value(arguments.back());
        if (mode == ArgumentMode::value && arguments.back().kind == Type::procedure)
          note_abrupt(expr.children[i]->at, {exception_id(ExceptionCategory::out_of_memory)});
        views.push_back(operations.at(expr.children[i].get()));
        prepare_borrow(views.back(), mode, expr.children[i]->at);
      }
      for (size_t i = 0; i < arguments.size(); ++i)
        if (evidence[i] && *evidence[i] != arguments[i])
          implementation_gap(expr.children[i]->at, "operand type evidence depends on completion or contextual inference");
      if (expr.text == "cause_at" && (expr.intrinsic || !families_.contains(expr.text))) {
        if (arguments.size() != 2 || arguments[0].kind != Type::exception || arguments[1] != Type::uint64)
          fail(expr.at, "cause_at requires an exception and a u64 index");
        operation.type = exception_type(0); operation.exceptions = exception_possibilities(operation.type); operation.builtin = BuiltinOperation::cause_at;
        break;
      }
      if (eager && (expr.intrinsic || !applicable_operation(expr.text, arguments))) {
        check_eager(expr, arguments, views, operation);
        break;
      }
      operation.callee = select(expr, arguments);
      const auto &target = program_.instances[*operation.callee];
      const auto &definition = *program_.methods[target.method].source;
      for (size_t i = 0; i < arguments.size(); ++i) {
        if (operation.arguments[i].mode != argument_mode(definition.parameters[i]))
          throw std::logic_error("argument preparation disagrees with selected method");
        operation.arguments[i].type = target.arguments[i];
      }
      if (method && method->kind == Form::Kind::function && definition.kind == Form::Kind::procedure)
        fail(expr.at, "a function cannot invoke a procedure");
      finish_call(expr, callable_type(definition, target.arguments, target.result), target.effects,
          call_reasons(program_.methods[target.method]), arguments, views, operation, read, discard);
      break;
    }
    case K::behave: case K::perform: {
      if (lexical_.lookup(expr.text))
        fail(expr.at, "nearest binding '" + expr.text + "' is not a symbol");
      if (!print_symbols_.contains(expr.text)) implementation_gap(expr.at, "dynamic procedure call other than selected std:io print");
      if (method && method->kind == Form::Kind::function) fail(expr.at, "a function cannot invoke a dynamic procedure");
      if (expr.children.size() != 1) fail(expr.at, "print requires one scalar argument");
      const auto argument = child(0);
      operation.arguments.push_back({ArgumentMode::value, argument});
      if (argument == Type::never) { operation.type = Type::never; break; }
      if (!scalar_type(argument)) fail(expr.at, "print requires one primitive scalar argument");
      operation.builtin = BuiltinOperation::print;
      operation.type = Type::void_type;
      break;
    }
    case K::conditional: {
      const auto condition = child(0);
      if (condition != Type::boolean && condition != Type::never) fail(expr.at, "if condition requires bool");
      const auto before = lexical_;
      refine(*expr.children[0], true, operations);
      const auto yes = check(*expr.children[1], instance, operations, read, discard, expected);
      const auto yes_state = lexical_;
      lexical_ = before;
      refine(*expr.children[0], false, operations);
      const auto no = check(*expr.children[2], instance, operations, read, discard, expected);
      const auto no_state = lexical_;
      join(yes_state, yes == Type::never, no_state, no == Type::never, expr.at);
      if (yes == Type::never) operation.type = no;
      else if (no == Type::never || yes == no) operation.type = yes;
      else if (yes == Type::null_ref && operations.at(expr.children[2].get()).view) operation.type = no;
      else if (no == Type::null_ref && operations.at(expr.children[1].get()).view) operation.type = yes;
      else implementation_gap(expr.at, "conditional join with different scalar result types");
      const auto &yes_fact = operations.at(expr.children[1].get()), &no_fact = operations.at(expr.children[2].get());
      operation.exceptions = yes == Type::never ? no_fact.exceptions : yes_fact.exceptions;
      if (yes != Type::never && no != Type::never) operation.exceptions.insert(no_fact.exceptions.begin(), no_fact.exceptions.end());
      if (yes == Type::never) view(no_fact);
      else if (no == Type::never) view(yes_fact);
      else {
        operation.view = yes_fact.view || no_fact.view;
        operation.mixed_view = yes_fact.mixed_view || no_fact.mixed_view || yes_fact.view != no_fact.view;
        operation.read_only = yes_fact.read_only || no_fact.read_only;
        operation.fixed_binding = yes_fact.fixed_binding || no_fact.fixed_binding;
        if (yes_fact.view && no_fact.view) {
          auto targets = yes_fact.targets;
          targets.insert(no_fact.targets.begin(), no_fact.targets.end());
          select_targets(operation, targets);
        }
        operation.origins = yes_fact.origins;
        operation.origins.insert(no_fact.origins.begin(), no_fact.origins.end());
        operation.owners = yes_fact.owners;
        operation.owners.insert(no_fact.owners.begin(), no_fact.owners.end());
        operation.initialized_target = yes_fact.initialized_target && no_fact.initialized_target;
        if (yes_fact.binding == no_fact.binding) operation.binding = yes_fact.binding;
        operation.selection_versions = yes_fact.selection_versions;
        operation.selection_versions.insert(no_fact.selection_versions.begin(), no_fact.selection_versions.end());
      }
      if (condition == Type::never) operation.type = Type::never;
      break;
    }
    case K::block: {
      auto *outer = abrupt_;
      const auto parent_scope = cleanup_scope_;
      if (expr.scoped) {
        cleanup_scope_ = next_scope_++;
        lexical_.scopes.push_back({cleanup_scope_, {}});
        lexical_.queues[cleanup_scope_] = 0;
      }
      AbruptFlow flow{lexical_.scopes.size(), {}};
      if (expr.scoped) abrupt_ = &flow;
      bool exits = false;
      for (size_t i = 0; i < expr.children.size(); ++i) {
        const bool last = i + 1 == expr.children.size();
        operation.type = check(*expr.children[i], instance, operations, last ? read : true, last ? discard : true, last ? expected : nullptr);
        exits = exits || operation.type == Type::never;
        if (exits) lexical_.live = false;
      }
      if (!expr.children.empty()) view(operations.at(expr.children.back().get()));
      if (!expr.children.empty()) operation.exceptions = operations.at(expr.children.back().get()).exceptions;
      if (exits) operation.type = Type::never;
      if (expr.scoped) {
        if (read && !discard && operation.type.kind == Type::procedure) note_abrupt(expr.at, {exception_id(ExceptionCategory::out_of_memory)});
        operation = finish_scope(operation, flow, outer, cleanup_scope_, expr.at);
        leave_scope();
        cleanup_scope_ = parent_scope;
      }
      break;
    }
    case K::range_for: case K::fold: {
      const bool fold = expr.kind == K::fold;
      const auto first = child(0), last = child(1);
      if (!integral(first) || !integral(last)) fail(expr.at, "a range requires integer endpoints");
      if (first != last) implementation_gap(expr.at, "range endpoints with different integer types");
      Type accumulator;
      if (fold) { accumulator = child(2); require_value(accumulator); if (accumulator.kind == Type::procedure) note_abrupt(expr.at, {exception_id(ExceptionCategory::out_of_memory)}); }
      const auto before = lexical_;
      bool body_possible = true, empty_possible = true, repeat_possible = true;
      const auto start = constant_integer(*expr.children[0], operations), end = constant_integer(*expr.children[1], operations);
      if (start && end) {
        body_possible = first == Type::integer ? std::bit_cast<std::int64_t>(*start) < std::bit_cast<std::int64_t>(*end) : *start < *end;
        empty_possible = !body_possible;
        repeat_possible = body_possible && *end - *start > 1;
      }
      const auto header = cleanup_.header(lexical_.queues.at(cleanup_scope_));
      lexical_.queues[cleanup_scope_] = header;
      if (repeat_possible) {
        widen_targets(lexical_);
        if (proving_) lexical_.expired_bindings.insert(loop_owners_.at(&expr).expired.begin(), loop_owners_.at(&expr).expired.end());
      }
      const auto iteration_scope = next_scope_;
      lexical_.live = lexical_.live && body_possible;
      for (auto &[id, categories] : lexical_.exceptions)
        if (!(*storage_)[id].read_only) categories = exception_possibilities((*storage_)[id].type);
      lexical_.scopes.push_back({next_scope_++, {}});
      operation.place = declare(expr.text, first, true, true, expr.at);
      initialize(*operation.place);
      loop_controls_.insert(*operation.place);
      if (fold) {
        operation.accumulator = declare(expr.binding, accumulator, true, true, expr.at);
        initialize(*operation.accumulator);
        lexical_.exceptions[*operation.accumulator] = exception_possibilities(accumulator);
        loop_controls_.insert(*operation.accumulator);
      }
      const auto body = child(fold ? 3 : 2, true, !fold);
      if (fold && body.kind == Type::procedure) note_abrupt(expr.at, {exception_id(ExceptionCategory::out_of_memory)});
      if (fold && body != Type::never && body != accumulator) {
        if (widens(body, accumulator)) implementation_gap(expr.at, "fold accumulator conversion");
        fail(expr.at, "fold body does not satisfy the accumulator type");
      }
      leave_scope();
      if (!proving_ && repeat_possible) loop_owners_[&expr] = {iteration_scope, next_scope_, before.bindings_available, {}};
      const auto body_state = lexical_;
      if (repeat_possible && body != Type::never && lexical_.live)
        cleanup_.backedge(header, cleanup_.known(lexical_.queues.at(cleanup_scope_), lexical_.initialized));
      lexical_ = before;
      lexical_.live = lexical_.live && empty_possible;
      const auto otherwise = !fold && expr.children.size() == 4 ? child(3, true, true) : Type{Type::void_type};
      const auto empty_state = lexical_;
      const bool body_exits = !body_possible || body == Type::never;
      const bool empty_exits = !empty_possible || otherwise == Type::never;
      join(body_state, body_exits, empty_state, empty_exits, expr.at);
      operation.type = body_exits && empty_exits ? Type{Type::never} : fold ? accumulator : Type{Type::void_type};
      operation.operand_type = first;
      operation.exceptions = exception_possibilities(operation.type);
      break;
    }
    case K::pointer_group: {
      const bool callable_value = expr.declaration_kind == "func" || expr.declaration_kind == "proc";
      const bool value = expr.declaration_kind == "val" || callable_value;
      for (const auto &declaration : expr.children) {
        if (declaration->kind == K::block) { check(*declaration, instance, operations, true, true); continue; }
        std::optional<size_t> owner;
        if (!declaration->owner_name.empty()) {
          if (value) fail(declaration->at, "val storage belongs to its lexical scope, not an external owner");
          owner = named_owner(declaration->owner_name, declaration->at);
        }
        const auto &initializer = *declaration->children.front();
        const bool copy = value || declaration->binding == "<-";
        Type initial_type;
        bool initialized = true;
        if (initializer.kind == K::make_local) {
          if (initializer.annotation) initial_type = resolve_local_type(*initializer.annotation, instance, initializer.at);
          else {
            const auto &query = *initializer.children.front();
            const auto *name = &query;
            const bool null_query = query.kind == K::call && query.text == "isnull";
            if (null_query) {
              if (query.children.size() != 1) fail(query.at, "isnull requires one reference view");
              name = query.children.front().get();
            }
            if (name->kind == K::name) {
              const auto binding = lexical_.lookup(name->text);
              if (!binding) fail(name->at, "unknown value in typeof: '" + name->text + "'");
              if (!lexical_.bindings_available.contains(*binding)) fail(name->at, "binding '" + name->text + "' is not available on every reaching path");
              initial_type = (*bindings_)[*binding].type;
            } else if (!null_query || name->kind != K::null_ref) implementation_gap(query.at, "typeof on a non-name local initializer");
            if (null_query) initial_type = Type::boolean;
          }
          const size_t value_index = initializer.annotation ? 0 : 1;
          if (initial_type.kind == Type::procedure) implementation_gap(initializer.at, "raw callable storage construction");
          initialized = initializer.children.size() > value_index;
          if (initialized) {
            const auto value_type = check(*initializer.children[value_index], instance, operations);
            if (!transfers_bits(value_type, initial_type)) fail(initializer.at, "initializer does not satisfy local storage type");
          }
          CheckedOperation allocation;
          allocation.type = initial_type;
          allocation.initializes = initialized;
          if (initialized) allocation.exceptions = operations.at(initializer.children[value_index].get()).exceptions;
          operations[&initializer] = allocation;
        } else {
          const auto annotation = declaration->annotation ? resolve_local_type(*declaration->annotation, instance, declaration->at) : Type{};
          initial_type = check(initializer, instance, operations, copy, false, annotation.kind == Type::procedure ? &annotation : nullptr);
        }
        const auto type = declaration->annotation ? resolve_local_type(*declaration->annotation, instance, declaration->at) : initial_type;
        const auto &source = operations.at(&initializer);
        const bool borrowed = !copy && source.view;
        if (owner && !borrowed) implementation_gap(declaration->at, "fresh storage with an explicit external owner");
        if (owner && std::any_of(source.owners.begin(), source.owners.end(), [&](size_t actual) { return actual != *owner; }))
          fail(declaration->at, "reference initializer does not belong to the declared owner");
        if (!borrowed && type.kind == Type::procedure) note_abrupt(declaration->at, {exception_id(ExceptionCategory::out_of_memory)});
        if (type == Type::null_ref) fail(declaration->at, "a null initializer requires an explicit target type");
        require_value(type);
        if (callable_value && (type.kind != Type::procedure || type.pure_callable != (expr.declaration_kind == "func")))
          fail(declaration->at, "callable declaration requires a matching func/proc value");
        if (type.kind == Type::procedure && !callable_value && expr.declaration_kind != "ptr")
          implementation_gap(declaration->at, "callable storage uses func/proc values or ptr views");
        if (borrowed && source.mixed_view) implementation_gap(initializer.at, "reference join mixing fresh values and storage views");
        if (borrowed ? initial_type != Type::null_ref && initial_type != type : !transfers_value(initial_type, type))
          fail(declaration->at, "initializer does not satisfy declared " + expr.declaration_kind + " type");
        if (borrowed) {
          const auto &owners = source.owners;
          if (owners.size() > 1) fail(initializer.at, "reference initializer joins different owners");
          if (lexical_.live) for (const auto owner : owners)
            if (!owner_alive(owner))
              fail(initializer.at, "reference initializer's lexical owner has ended");
          if (expr.declaration_kind == "ref") {
            if (proving_ && lexical_.live && source.targets.contains(null_target)) fail(initializer.at, "ref requires a proven non-null target");
            if (!source.initialized_target) for (const auto place : source.targets) require_initialized(place, initializer.at, declaration->text);
          }
        }
        if (borrowed && declaration->annotation && !declaration->read_only && source.read_only)
          fail(declaration->at, "readonly source cannot initialize a writable reference target");
        if (!borrowed && !initialized && expr.declaration_kind == "ref")
          fail(declaration->at, "ref requires definitely initialized target storage");
        if (lexical_.lookup(declaration->text) && !lexical_.scopes.back().bindings.contains(declaration->text) &&
            validating_ && warned_shadow_.insert(declaration.get()).second)
          program_.warnings.push_back({Warning::shadow, declaration->at, "declaration shadows outer binding '" + declaration->text + "'"});
        const bool read_only = declaration->read_only || (value && expr.fixed_binding) || (borrowed && source.read_only);
        std::optional<size_t> place;
        if (borrowed) {
          const auto id = bind(declaration->text, type, source.targets, read_only, false, expr.fixed_binding, declaration->at);
          assign_targets(id, source);
          if (owner) lexical_.owners[id].insert(*owner);
        } else {
          place = declare(declaration->text, type, read_only, value, declaration->at);
          bindings_->back().fixed = expr.fixed_binding;
          if (initialized) initialize(*place);
          if (type.kind == Type::exception) lexical_.exceptions[*place] = initialized ? source.exceptions : exception_possibilities(type);
        }
        bindings_->back().requires_initialized = expr.declaration_kind == "ref";
        bindings_->back().callable_value = callable_value;
        CheckedOperation binding;
        binding.binding = *lexical_.lookup(declaration->text);
        binding.place = place;
        binding.operand_type = type;
        binding.view = borrowed;
        binding.initializes = !borrowed && initialized;
        operations[declaration.get()] = binding;
      }
      break;
    }
    case K::alias: {
      const auto owner = expr.owner_name.empty() ? std::optional<size_t>{} : named_owner(expr.owner_name, expr.at);
      const auto type = child(0, false);
      const auto source = operations.at(expr.children.front().get());
      if (!source.view || type == Type::null_ref) fail(expr.at, "alias requires an existing storage place");
      if (!source.binding || source.mixed_view) implementation_gap(expr.at, "alias of a runtime-selected or non-binding storage place");
      if (!lexical_.bindings_available.contains(*source.binding)) fail(expr.at, "aliased binding's lexical scope has ended");
      const auto selected = bindings_->at(*source.binding);
      const std::string kind = selected.value_binding ? "val" : selected.requires_initialized ? "ref" : "ptr";
      if (!expr.declaration_kind.empty() && expr.declaration_kind != kind)
        fail(expr.at, "alias kind must match the selected " + kind + " storage");
      if (expr.annotation && resolve_local_type(*expr.annotation, instance, expr.at) != type)
        fail(expr.at, "alias annotation requires the exact selected type");
      if (expr.annotation && !expr.read_only && source.read_only)
        fail(expr.at, "readonly source cannot supply a writable alias target");
      if (owner && *owner != selected.scope) fail(expr.at, "alias owner must match the selected binding storage, not its target owner");
      if (lexical_.lookup(expr.text) && !lexical_.scopes.back().bindings.contains(expr.text) &&
          validating_ && warned_shadow_.insert(&expr).second)
        program_.warnings.push_back({Warning::shadow, expr.at, "declaration shadows outer binding '" + expr.text + "'"});
      const auto id = bind(expr.text, type, {}, source.read_only || expr.read_only || (selected.value_binding && expr.fixed_binding),
          selected.value_binding, source.fixed_binding || expr.fixed_binding, expr.at, source.binding);
      bindings_->at(id).requires_initialized = selected.requires_initialized;
      bindings_->at(id).callable_value = selected.callable_value;
      break;
    }
    case K::transfer: case K::rebind: {
      const auto destination = child(0, false);
      const auto target = operations.at(expr.children[0].get());
      if (!target.view) fail(expr.at, "assignment requires a storage destination");
      if (target.mixed_view) implementation_gap(expr.at, "assignment through a mixed value/storage view");
      for (const auto owner : target.owners) if (lexical_.live)
        if (!owner_alive(owner))
          fail(expr.at, "assignment destination's lexical owner has ended");
      const bool binding_live = target.binding && lexical_.bindings_available.contains(*target.binding);
      const auto versions = target.selection_versions;
      if (expr.kind == K::rebind) {
        reference_destination(target, expr.at);
        child(1, false);
        const auto &selected = operations.at(expr.children[1].get());
        check_rebind(target, selected, expr.at);
        if (!selected.initialized_target && read && !discard)
          for (const auto place : selected.targets) require_initialized(place, expr.at, expr.children[0]->text);
        rebind(target, selected);
        operation = selected;
        operation.type = destination;
        operation.owners = lexical_.owners.at(*target.binding);
        operation.binding = target.binding;
        operation.selection_versions = lexical_.target_versions.at(*target.binding);
        operation.origins = {*target.binding};
        operation.read_only = target.read_only;
        operation.fixed_binding = target.fixed_binding;
        if (read && !discard) dereference(operation, expr.at);
        break;
      }
      if (target.read_only) fail(expr.at, "cannot write through a readonly destination");
      for (const auto place : target.targets) if (place != null_target && storage(place).captured)
        implementation_gap(expr.at, "replacement of a captured callable payload");
      dereference(target, expr.at);
      const auto source = check(*expr.children[1], instance, operations, true, false, destination.kind == Type::procedure ? &destination : nullptr);
      if (source == Type::never) { operation.type = Type::never; break; }
      if (!transfers_value(source, destination)) fail(expr.at, "bits transfer source does not satisfy destination type");
      if (destination.kind == Type::procedure) note_abrupt(expr.at, {exception_id(ExceptionCategory::out_of_memory)});
      auto written = target.targets; written.erase(null_target);
      if (written.size() == 1) initialize(*written.begin());
      if (binding_live && lexical_.target_versions.contains(*target.binding) && lexical_.target_versions.at(*target.binding) == versions)
        lexical_.initialized_targets.insert(*target.binding);
      operation.type = operation.operand_type = destination;
      select_targets(operation, written);
      operation.owners = target.owners;
      operation.origins = target.origins;
      operation.read_only = operation.view = true;
      operation.initialized_target = true;
      operation.exceptions = operations.at(expr.children[1].get()).exceptions;
      write_exceptions(target, operation.exceptions);
      break;
    }
    case K::return_value: {
      if (!instance) fail(expr.at, "return requires an enclosing callable");
      const bool borrowed = borrowed_result(*method);
      const auto value = expr.children.empty() ? Type{Type::void_type} : check(*expr.children[0], instance, operations, !borrowed, false, instance->result.kind == Type::procedure ? &instance->result : nullptr);
      if (!borrowed && value.kind == Type::procedure) note_abrupt(expr.at, {exception_id(ExceptionCategory::out_of_memory)});
      if (borrowed && value != Type::never) {
        if (expr.children.empty()) fail(expr.at, "borrowed result requires an explicit target view");
        const auto &selected = operations.at(expr.children.front().get());
        if (!selected.view || selected.mixed_view) implementation_gap(expr.at, "fresh ordinary borrowed results require a separate storage contract");
        if (value != Type::null_ref && value != instance->result) fail(expr.at, "borrowed return requires the exact target type");
        if (!method->result_const && selected.read_only) fail(expr.at, "readonly target cannot supply a writable borrowed result");
        const auto owner = external_owners_.at(method->result_owner);
        if (!selected.owners.empty() && selected.owners != std::set<size_t>{owner})
          fail(expr.at, "borrowed result does not retain its declared external owner");
        if (method->result_kind != "ptr" && proving_ && lexical_.live && selected.targets.contains(null_target))
          fail(expr.at, "borrowed ref result requires a proven non-null target");
        if (method->result_kind != "ptr" && !selected.initialized_target)
          for (const auto place : selected.targets) require_initialized(place, expr.at, "borrowed result");
        operation.borrowed_result = true;
        lexical_.returned = {false, selected.targets, {}};
        if (method->result_kind == "ptr" && !selected.initialized_target) for (const auto place : selected.targets)
          if (place != null_target && !lexical_.initialized.contains(place)) lexical_.returned.uninitialized.insert(place);
      } else if (value != Type::never && !transfers_value(value, instance->result))
        fail(expr.at, "return type '" + type_name(value) + "' does not satisfy '" + type_name(instance->result) + "'");
      operation.type = Type::never;
      operation.operand_type = instance->result;
      note_abrupt(expr.at, {}, {1, true});
      break;
    }
    case K::field: {
      const auto source = child(0);
      if (source.kind != Type::exception && source != Type::never) implementation_gap(expr.at, "non-exception field selection");
      if (expr.text != "category" && expr.text != "site" && expr.text != "requested" && expr.text != "available" && expr.text != "cause_count")
        fail(expr.at, "unknown intrinsic exception field '" + expr.text + "'");
      operation.type = source == Type::never ? Type{Type::never} : Type{Type::uint64};
      break;
    }
    case K::raise: {
      Type type;
      if (expr.children.empty()) {
        if (!caught_) fail(expr.at, "bare throw requires an active catch");
        capture_catch(*caught_);
        operation.place = caught_; type = (*storage_)[*caught_].type; operation.exceptions = lexical_.exceptions.at(*caught_);
      } else {
        type = child(0); operation.exceptions = operations.at(expr.children[0].get()).exceptions;
      }
      if (type.kind != Type::exception && type != Type::never) fail(expr.at, "throw requires an intrinsic exception value");
      if (expr.children.size() == 2) {
        const auto cause = child(1);
        if (cause.kind != Type::exception && cause != Type::never) fail(expr.at, "explicit cause requires an exception value");
        note_abrupt(expr.at, {exception_id(ExceptionCategory::out_of_memory)});
      }
      const auto binding = expr.children.empty() ? caught_ : expr.children[0]->kind == K::name ? operations.at(expr.children[0].get()).place : std::optional<size_t>{};
      if (binding && caught_inputs_.contains(*binding)) {
        const auto inputs = lexical_.inputs;
        for (const auto category : operation.exceptions) {
          lexical_.inputs = inputs & caught_inputs_.at(*binding)[category];
          if (lexical_.inputs) note_abrupt(expr.at, {category});
        }
        lexical_.inputs = inputs;
      } else note_abrupt(expr.at, operation.exceptions);
      operation.type = Type::never;
      break;
    }
    case K::try_scope: operation = check_guarded(expr, instance, operations, read, discard, expected); break;
    case K::deferred: {
      const auto action = cleanup_.action();
      CleanupEquations::Action summary;
      if (expr.children.front()->kind == K::block) {
        const auto before = lexical_;
        auto caught_inputs = std::move(caught_inputs_);
        caught_inputs_.clear();
        const auto controls = delayed_controls_;
        CatchCapture captures{captures_, catch_bindings_, {}};
        captures_ = &captures;
        const auto was_delayed = delayed_;
        const auto previous_inputs = deferred_inputs_;
        auto *outer = abrupt_;
        delayed_controls_.insert(loop_controls_.begin(), loop_controls_.end());
        delayed_ = true;
        deferred_inputs_ = before.available;
        if (proving_) for (size_t place = 0; place < catalog_storage_.size(); ++place)
          if (std::any_of(before.scopes.begin(), before.scopes.end(), [&](const auto &scope) { return scope.id == catalog_storage_[place].scope; }))
            deferred_inputs_.insert(place);
        widen_targets(lexical_);
        lexical_.queues[cleanup_scope_] = 0;
        lexical_.inputs = 1;
        lexical_.input_initialized = {{0, lexical_.initialized}};
        lexical_.requirements.clear();
        lexical_.returned = {};
        lexical_.live = true;
        AbruptFlow flow{lexical_.scopes.size(), {}};
        abrupt_ = &flow;
        summary.guarded = expr.children.size() > 1;
        const auto completed = summary.guarded ? check_guarded(expr, instance, operations, true, true).type : child(0, true, true);
        if (completed != Type::never && lexical_.live) flow.paths.push_back({lexical_, {}, expr.at, {0, false}});
        for (const auto &path : flow.paths) {
          const auto queue = path.state.queues.at(cleanup_scope_);
          for (const auto input : inputs(path.state.inputs)) {
            const auto selected = std::uint64_t{1} << input;
            CleanupEquations::Effects effects{{}, path.state.input_initialized.at(input), path.state.returned};
            effects.returned.initialize(effects.initialized);
            for (const auto &[read, requirement] : path.state.requirements) if (requirement.inputs & selected)
              effects.reads.emplace(read, requirement.before);
            if (path.exceptions.empty()) summary.paths.push_back({selected, path.completion, queue, effects});
            else for (const auto category : path.exceptions)
              summary.paths.push_back({selected, {3 + category, path.completion.returned}, queue, effects});
          }
        }
        lexical_ = before;
        caught_inputs_ = std::move(caught_inputs);
        delayed_controls_ = controls;
        operation.captures = std::move(captures.required);
        captures_ = captures.parent;
        delayed_ = was_delayed;
        deferred_inputs_ = previous_inputs;
        abrupt_ = outer;
      } else {
        const auto *prepared = prepared_call_;
        prepared_call_ = expr.children.front().get();
        const auto completed = child(0);
        prepared_call_ = prepared;
        const auto &call = operations.at(expr.children.front().get());
        if (completed == Type::never) { operation.type = Type::never; break; }
        if (!call.invocation.empty()) {
          for (const auto &[reason, effects] : call.invocation) summary.paths.push_back({1, {reason, false}, 0, effects});
        } else summary.paths.push_back({1, {0, false}, 0});
      }
      cleanup_.define(action, std::move(summary));
      note_abrupt(expr.at, {exception_id(ExceptionCategory::out_of_memory)});
      if (lexical_.live) lexical_.queues[cleanup_scope_] = cleanup_.push(lexical_.queues.at(cleanup_scope_), action);
      operation.type = Type::void_type;
      break;
    }
    default: implementation_gap(expr.at, "source expression '" + expr.text + "' is outside the scalar-method foundation");
    }
    if (operation.type == Type::never) lexical_.live = false;
    operations[&expr] = operation;
    return operation.type;
  }

  void begin_check(std::vector<CheckedStorage> &storage, std::vector<CheckedBinding> &bindings,
                   std::map<const Expression *, CheckedOperation> &operations, bool proving) {
    proving_ = proving; observed_ = {};
    storage_ = &storage; bindings_ = &bindings;
    storage.clear(); bindings.clear(); operations.clear();
    if (!proving) { possible_targets_.clear(); target_dependencies_.clear(); catalog_storage_.clear(); rebound_.clear(); loop_owners_.clear(); }
    lexical_ = {}; cleanup_ = {}; scope_storage_.clear(); read_sites_.clear(); external_owners_.clear();
    loop_controls_.clear(); delayed_controls_.clear(); deferred_inputs_.clear();
    caught_.reset(); catch_bindings_.clear(); caught_inputs_.clear();
    captures_ = nullptr; delayed_ = false; prepared_call_ = nullptr;
    cleanup_scope_ = 0; next_scope_ = 1;
    next_target_version_ = 0;
  }

  void check_entry() {
    checking_method_ = nullptr;
    checking_instance_ = nullptr;
    for (const bool proving : {false, true}) {
      begin_check(program_.entry_storage, program_.entry_bindings, program_.entry, proving);
      AbruptFlow entry_flow{1, {}};
      abrupt_ = &entry_flow;
      for (const auto *expr : program_.initializers)
        if (check(*expr, nullptr, program_.entry, true, true) == Type::never) lexical_.live = false;
      CheckedOperation entry_result; entry_result.type = lexical_.live ? Type{Type::void_type} : Type{Type::never};
      finish_scope(entry_result, entry_flow, nullptr, 0, program_.initializers.empty() ? Location{} : program_.initializers.front()->at);
      if (!proving) solve_targets();
    }
    for (const auto &[name, id] : lexical_.scopes.front().bindings) module_values_.insert(name);
  }
  void observe_outcome(size_t reason, const LexicalState &state) {
    std::set<size_t> known;
    for (const auto id : state.initialized) if (storage(id).borrowed) known.insert(id);
    const auto [found, inserted] = observed_.initialized.emplace(reason, known);
    if (!inserted) found->second = intersect(found->second, known);
    if (!reason && borrowed_result(*checking_method_->source)) {
      auto returned = state.returned;
      returned.initialize(state.initialized);
      observed_.returned.merge(returned);
    }
    for (const auto &[read, requirement] : state.requirements)
      if (requirement.inputs & state.inputs) observe_read(cleanup_.binding(read), requirement.before);
  }
  CallEffects check_instance(size_t i) {
    auto &instance = program_.instances[i];
    checking_instance_ = &instance;
    const auto &method = *program_.methods[instance.method].source;
    checking_method_ = &program_.methods[instance.method];
    for (const bool proving : {false, true}) {
      begin_check(instance.storage, instance.bindings, instance.operations, proving);
      for (size_t i = 0; i < method.owner_parameters.size(); ++i)
        external_owners_[method.owner_parameters[i].name] = null_target - 1 - i;
      for (size_t p = 0; p < method.parameters.size(); ++p) {
        const auto &parameter = method.parameters[p];
        const bool value = parameter.declaration_kind == "val";
        const auto id = declare(parameter.name, instance.arguments[p], parameter.read_only || (value && parameter.fixed_binding), value, parameter.at);
        bindings_->back().fixed = parameter.fixed_binding;
        bindings_->back().requires_initialized = parameter.declaration_kind == "ref";
        bindings_->back().callable_value = instance.arguments[p].kind == Type::procedure && parameter.owner.empty();
        if (!parameter.owner.empty()) {
          const auto owner = external_owners_.at(parameter.owner);
          storage_->at(id).scope = owner; storage_->at(id).borrowed = true;
          std::erase(scope_storage_[0], id);
          lexical_.owners[p] = {owner};
          if (parameter.declaration_kind == "ptr") {
            lexical_.targets[p].insert(null_target);
            if (!proving) possible_targets_[p].insert(null_target);
          }
        }
        if (argument_mode(parameter) != ArgumentMode::borrowed_ptr) initialize(id);
        lexical_.exceptions[id] = exception_possibilities(instance.arguments[p]);
      }
      for (size_t c = 0; c < instance.environment.size(); ++c) {
        const auto &capture = checking_method_->captures[c];
        const auto id = declare(capture.name, instance.environment[c], false, true, capture.at);
        storage_->at(id).captured = true;
        bindings_->back().callable_value = true;
        initialize(id);
      }
      for (const auto &owner : method.owner_parameters) {
        const auto id = bind(owner.name, Type::void_type, {}, true, true, true, owner.at);
        bindings_->at(id).owner_binding = external_owners_.at(owner.name);
      }
      AbruptFlow flow{1, {}}, outcomes{1, {}};
      abrupt_ = &flow;
      CheckedOperation completed;
      completed.type = check(*method.body, &instance, instance.operations, true, true) == Type::never ? Type{Type::never} : Type{Type::void_type};
      const auto completion = finish_scope(completed, flow, &outcomes, 0, method.at).type;
      if (validating_ && proving && instance.result != Type::void_type && completion != Type::never && warned_missing_return_.insert(instance.method).second)
        program_.warnings.push_back({Warning::missing_return, method.at, "callable may finish without an explicit return"});
      if (proving) {
        if (completion != Type::never && lexical_.live)
          observe_outcome(instance.result == Type::void_type ? 0 : 3 + exception_id(ExceptionCategory::missing_return), lexical_);
        const auto normal = lexical_;
        for (const auto &path : outcomes.paths) {
          observe_outcome(path.completion.reason == 1 ? 0 : path.completion.reason, path.state);
          lexical_ = path.state; abrupt_ = nullptr;
          note_abrupt(path.at, path.exceptions, path.completion);
        }
        lexical_ = normal;
      } else solve_targets();
    }
    instance.complete = validating_;
    for (const auto reason : call_reasons(*checking_method_)) observed_.initialized.try_emplace(reason);
    return observed_;
  }

public:
  CheckedProgram check(const std::vector<std::unique_ptr<Form>> &forms) {
    for (const auto &form : forms) {
      if ((form->kind != Form::Kind::using_resource && form->kind != Form::Kind::import_resource) || form->path != "std:exceptions" || form->using_symbol) continue;
      const auto found = std::find_if(exception_categories.begin(), exception_categories.end(), [&](const ExceptionDescriptor &entry) { return form->name == entry.name; });
      if (found == exception_categories.end()) fail(form->at, "unknown intrinsic exception selection '" + form->name + "'");
      if (!exception_types_.emplace(form->alias, exception_type(exception_id(found->category))).second) fail(form->at, "duplicate exception type selection");
      program_.exception_values = true;
    }
    std::vector<const Form *> definitions;
    for (const auto &form : forms) {
      if (form->kind == Form::Kind::expression) { program_.initializers.push_back(form->body.get()); continue; }
      if ((form->kind == Form::Kind::using_resource || form->kind == Form::Kind::import_resource) && form->path == "std:exceptions" && !form->using_symbol) continue;
      if (form->kind == Form::Kind::using_resource && form->using_symbol && form->path == "std:io" && form->name == "print") {
        if (!print_symbols_.insert(form->alias).second) fail(form->at, "duplicate print symbol binding");
        continue;
      }
      if (form->kind != Form::Kind::function && form->kind != Form::Kind::procedure)
        implementation_gap(form->at, "module declaration or selection '" + form->name + "'");
      definitions.push_back(form.get());
    }
    std::map<const Form *, const Form *> parents;
    std::map<const Form *, size_t> method_ids;
    const std::function<void(const Expression &, const Form *)> collect = [&](const Expression &expr, const Form *parent) {
      if (expr.definition) {
        lexical_methods_.emplace(expr.definition.get(), 0);
        if (parent) parents.emplace(expr.definition.get(), parent);
        definitions.push_back(expr.definition.get());
        collect(*expr.definition->body, expr.definition.get());
      }
      for (const auto &child : expr.children) collect(*child, parent);
    };
    for (const auto &form : forms) if (form->body)
      collect(*form->body, form->kind == Form::Kind::expression ? nullptr : form.get());
    for (const auto *form : definitions) {
      if (form->symbolic || (form->operation && !eager_operation(form->name)) || form->conversion || (!lexical_methods_.contains(form) && !form->captures.empty()) || !form->body)
        implementation_gap(form->at, "callable captures, effects, declarations, operators, owners or throws contracts");
      if (form->operation) {
        const auto arity = form->parameters.size();
        if (!arity || arity > 2 || (form->name == "!" && arity != 1) ||
            (form->name != "+" && form->name != "-" && form->name != "!" && arity != 2))
          implementation_gap(form->at, "eager operator method arity");
      }
      std::set<std::string> owners;
      for (const auto &owner : form->owner_parameters) {
        if (owner.root || owner.explicit_parameter) implementation_gap(owner.at, "region-specific owner parameters");
        owners.insert(owner.name);
      }
      if (!form->result_owner.empty() && !owners.contains(form->result_owner)) fail(form->at, "unknown result owner '" + form->result_owner + "'");
      if (!form->result_type) implementation_gap(form->at, "omitted callable result type");
      if (!form->result_owner.empty() && !borrowed_result(*form)) implementation_gap(form->at, "owner annotations on independent const results");
      if (borrowed_result(*form) && *form->result_type == Type::void_type) fail(form->at, "void cannot be a borrowed result");
      if (*form->result_type != Type::void_type && form->result_type->kind != Type::procedure && !borrowed_result(*form) && (!form->result_const || !form->result_kind.empty()))
        implementation_gap(form->at, "non-const scalar result contract");
      CheckedMethod prepared;
      prepared.source = form;
      for (const auto &p : form->type_parameters) prepared.type_parameters.push_back(p.name);
      if (const auto parent = parents.find(form); parent != parents.end()) {
        prepared.parent = method_ids.at(parent->second);
        const auto required = free_type_names(*form);
        const auto &context = program_.methods[*prepared.parent].type_parameters;
        for (size_t i = 0; i < context.size(); ++i) if (required.contains(context[i])) {
          prepared.type_parameters.push_back(context[i]); prepared.retained_types.push_back(i);
        }
      }
      const auto resolve_signature = [&](const Type &type) { return resolve_type(type, &prepared); };
      prepared.result = resolve_signature(*form->result_type);
      signature_type(prepared, prepared.result, form->at, true);
      for (const auto &type : form->throws_types) {
        const auto resolved = resolve_type(type);
        if (resolved.kind != Type::exception) implementation_gap(form->at, "throws contracts require selected intrinsic exception types");
        const auto categories = exception_possibilities(resolved);
        prepared.throws_categories.insert(categories.begin(), categories.end());
      }
      std::set<std::string> names;
      for (const auto &parameter : form->parameters) {
        if (!names.insert(parameter.name).second) fail(parameter.at, "duplicate parameter name");
        if (!parameter.owner.empty() && !owners.contains(parameter.owner)) fail(parameter.at, "unknown parameter owner '" + parameter.owner + "'");
        if (parameter.variadic || parameter.automatic_conversion)
          implementation_gap(parameter.at, "variadic, conversion or borrowed parameter contract");
        prepared.parameters.push_back(resolve_signature(parameter.type));
        signature_type(prepared, prepared.parameters.back(), parameter.at);
      }
      for (const auto &parameter : form->type_parameters)
        if (parameter.bound) implementation_gap(parameter.at, "bounded type parameters");
      const auto id = program_.methods.size();
      const bool lexical = lexical_methods_.contains(form);
      for (const auto other : lexical ? std::vector<size_t>{} : families_[form->name]) {
        const auto &definition = program_.methods[other];
        if (implies(prepared, definition) && implies(definition, prepared)) {
          for (size_t p = 0; p < form->parameters.size(); ++p) {
            if (form->parameters[p].owner.empty() != definition.source->parameters[p].owner.empty())
              implementation_gap(form->at, "owner-constrained overload selection");
            for (size_t q = 0; q < p; ++q)
              if ((form->parameters[p].owner == form->parameters[q].owner) !=
                  (definition.source->parameters[p].owner == definition.source->parameters[q].owner))
                implementation_gap(form->at, "owner-constrained overload selection");
          }
          for (size_t p = 0; p < form->parameters.size(); ++p)
            if (form->parameters[p].read_only != definition.source->parameters[p].read_only)
              implementation_gap(form->at, "target-access-qualified overload selection");
          fail(form->at, "duplicate method signature for '" + form->name + "'");
        }
        if (definition.source->kind != form->kind && !(form->operation && definition.source->operation))
          implementation_gap(form->at, "mixed function/procedure family");
      }
      program_.methods.push_back(std::move(prepared));
      method_ids.emplace(form, id);
      if (lexical) lexical_methods_[form] = id;
      else families_[form->name].push_back(id);
    }
    for (const auto &symbol : print_symbols_)
      if (families_.contains(symbol)) fail(program_.methods[families_[symbol].front()].source->at, "callable conflicts with symbol '" + symbol + "'");
    for (size_t i = 0; i < program_.methods.size(); ++i) {
      const auto &method = *program_.methods[i].source;
      if (!method.type_parameters.empty() || lexical_methods_.contains(&method)) continue;
      demand(i, program_.methods[i].parameters, method.at);
    }
    check_entry();
    // Solve terminating-path writes and returned selections together. Reads
    // then use the closed guarantees, retaining writes preceding each read.
    for (const bool reads : {false, true}) {
      bool changed;
      do {
        changed = false;
        const auto callable_count = program_.callables.size();
        for (size_t i = 0; i < program_.instances.size(); ++i) {
          const auto effects = check_instance(i);
          auto &known = program_.instances[i].effects;
          if (reads) {
            if (known.reads != effects.reads) { known.reads = effects.reads; changed = true; }
          } else {
            if (known.initialized != effects.initialized) { known.initialized = effects.initialized; changed = true; }
            if (known.returned != effects.returned) { known.returned = effects.returned; changed = true; }
          }
        }
        changed = changed || callable_count != program_.callables.size();
      } while (changed);
    }
    validating_ = true;
    check_entry();
    for (size_t i = 0; i < program_.instances.size(); ++i) check_instance(i);
    for (const auto &method : program_.methods) if (!method.instances.empty()) {
      const auto &first = program_.instances[method.instances.front()];
      for (const auto id : method.instances) {
        const auto &instance = program_.instances[id];
        if (instance.bindings.size() != first.bindings.size())
          implementation_gap(method.source->at, "generic instances with different lexical binding layouts");
        for (size_t i = 0; i < instance.bindings.size(); ++i)
          if (instance.bindings[i].value_binding != first.bindings[i].value_binding || instance.bindings[i].alias != first.bindings[i].alias)
            implementation_gap(method.source->at, "generic instances with different lexical binding layouts");
        const auto capture_bindings = [](const CheckedInstance &instance, const std::set<size_t> &places) {
          std::set<size_t> result;
          for (const auto place : places) result.insert(instance.storage.at(place).binding);
          return result;
        };
        const auto captured_binding = [](const CheckedInstance &instance, std::optional<size_t> place) -> std::optional<size_t> {
          return place ? std::optional<size_t>{instance.storage.at(*place).binding} : std::nullopt;
        };
        for (const auto &[expr, operation] : instance.operations) {
          const auto &baseline = first.operations.at(expr);
          if (capture_bindings(instance, operation.captures) != capture_bindings(first, baseline.captures) ||
              operation.snapshot != baseline.snapshot || captured_binding(instance, operation.captured_place) != captured_binding(first, baseline.captured_place))
            implementation_gap(expr->at, "generic instances with different deferred capture layouts");
        }
      }
    }
    return std::move(program_);
  }
};

} // namespace cod::language
