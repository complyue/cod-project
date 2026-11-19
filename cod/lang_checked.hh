//===--- lang_checked.hh - Source analysis and checked obligations ---===//
#pragma once

#include "lang_checked_program.hh"
#include "lang_modules.hh"
#include "lang_callable_flow.hh"

#include <bit>
#include <charconv>
#include <cmath>
#include <tuple>
#include <type_traits>
#include <variant>

namespace cod::language {

using CompletionInputs = std::set<size_t>;

struct LexicalState {
  struct Scope {
    size_t id;
    std::map<std::string, size_t> bindings;
    bool declarations = true;
    std::optional<std::set<size_t>> symbol_entry = {};
  };
  std::vector<Scope> scopes{{0, {}}};
  std::set<size_t> bound_symbols, required_symbols;
  std::set<size_t> bindings_available, available, initialized, present;
  struct InputFacts {
    std::set<size_t> initialized;
    CleanupEquations::Selections selections;
    std::map<size_t, ExceptionFacts> exceptions;
    std::map<size_t, size_t> queues{{0, 0}};
    std::set<size_t> present;
    std::set<size_t> initialized_saved_selections{};
  };
  std::map<size_t, InputFacts> input_facts{{0, {}}};
  struct Requirement { CompletionInputs inputs; CleanupEquations::ReadRequirement before; };
  std::map<size_t, Requirement> requirements;
  std::map<size_t, ExceptionFacts> exceptions;
  // Boolean payload snapshots retain the tested binding versions.
  std::map<size_t, std::shared_ptr<const CheckedOperation::Predicate>> predicates;
  std::set<std::pair<size_t, bool>> boolean_inputs;
  std::set<std::pair<CleanupEquations::BooleanPayload, bool>> boolean_payloads;
  SelectionConditions selection_conditions;
  std::set<ExceptionFacts> exception_conditions;
  // Necessary alternatives for paths still lacking initialization.
  CleanupEquations::MissingInitialization uninitialized;
  CleanupEquations::ConditionClauses predicate_conditions{CleanupEquations::ConditionFacts{}};
  std::set<size_t> payload_writes;
  std::map<size_t, std::set<size_t>> targets;
  std::map<size_t, std::set<size_t>> null_sources;
  std::map<size_t, std::set<size_t>> owners;
  std::set<size_t> expired_bindings;
  std::map<size_t, std::set<size_t>> target_versions;
  std::map<size_t, size_t> selection_identities;
  // A finalizer may initialize a saved target and then rebind its last slot.
  std::set<size_t> observed_saved_selections;
  CleanupEquations::Selections selections;
  CleanupEquations::Returned returned;
  CompletionInputs inputs{0};
  bool live = true;

  std::optional<size_t> lookup(const std::string &name) const {
    for (auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope)
      if (const auto found = scope->bindings.find(name); found != scope->bindings.end()) return found->second;
    return {};
  }
};

class ScalarChecker {
  CheckedProgram program_;
  using CallableChoices = std::vector<std::map<const Expression *, std::set<size_t>>>;
  CallableChoices callable_choices_;
  size_t checking_context_ = 0;
  // Context zero is module initialization; other contexts are instance IDs + 1.
  mutable std::map<size_t, std::set<size_t>> effect_dependents_;
  mutable std::set<size_t> callable_effect_consumers_;
  // Physical binding distinctness refines the proof for the same checked body.
  std::map<size_t, CallEffects> distinct_binding_effects_;
  const CallEffects &read_effects(size_t instance, bool distinct_bindings = false) const {
    effect_dependents_[instance].insert(checking_context_);
    if (distinct_bindings)
      if (const auto found = distinct_binding_effects_.find(instance); found != distinct_binding_effects_.end())
        return found->second;
    return program_.instances[instance].effects;
  }
  struct ModuleNames {
    std::map<std::string, size_t> callables;
    std::map<size_t, size_t> imports;
    std::set<std::string> imported_aliases;
    std::map<size_t, std::map<std::string, Location>> extensions;
    std::map<std::string, size_t> symbols;
    std::map<size_t, size_t> families;
    std::set<size_t> selected_symbols;
    std::set<std::string> selected_callables, values;
    std::map<std::string, Type> exception_types;
    struct Resource {
      enum class Kind { symbol, callable, exception } kind;
      size_t symbol = 0;
      std::optional<size_t> family;
      Type type;
      bool operator==(const Resource &) const = default;
    };
    std::map<std::string, Resource> exports;
  };
  std::vector<ModuleNames> modules_;
  std::map<size_t, const Form *> entry_extensions_;
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
  mutable size_t interpretation_ = 0;
  struct FamilyView { size_t identity; std::vector<size_t> methods; std::string unproved; };
  std::map<std::pair<size_t, size_t>, FamilyView> family_views_;
  std::map<std::tuple<size_t, size_t, size_t, size_t>, size_t> adapted_methods_;
  std::map<std::pair<const Form *, size_t>, size_t> lexical_variants_;
  const FamilyView &interpreted_family(size_t family) const {
    const auto &view = family_views_.at({family, interpretation_});
    if (!view.unproved.empty()) implementation_gap(program_.families[family].at, view.unproved);
    return view;
  }
  const std::vector<size_t> &family_methods(size_t family) const {
    return interpretation_ ? interpreted_family(family).methods : program_.families.at(family).methods;
  }
  const std::vector<CheckedMethod> &declarations(const std::string &name) const {
    static const std::vector<CheckedMethod> empty;
    const auto found = module_names().callables.find(name);
    if (found == module_names().callables.end()) return empty;
    const auto family = interpretation_ ? interpreted_family(found->second).identity : found->second;
    return program_.families[family].declarations;
  }
  bool has_declarations(const std::string &name) const { return !declarations(name).empty(); }
  std::optional<size_t> lexical_template(std::optional<size_t> method) const {
    if (method && program_.methods[*method].scope_inputs) return program_.methods[*method].scope_inputs->lexical_method;
    return method;
  }
  size_t lexical_method(const Form *form, std::optional<size_t> parent = {}) const {
    if (!parent && checking_instance_) parent = checking_instance_->method;
    parent = lexical_template(parent);
    if (parent) if (const auto found = lexical_variants_.find({form, *parent}); found != lexical_variants_.end()) return found->second;
    return lexical_methods_.at(form);
  }
  std::map<const Form *, size_t> lexical_methods_;
  std::map<std::pair<size_t, std::string>, const Expression *> scope_operand_names_;
  std::map<size_t, std::vector<size_t>> scope_profiles_;
  std::map<std::pair<size_t, size_t>, size_t> here_receiver_profiles_;
  struct FreeBindings {
    std::vector<Capture> captures, queries, symbols;
    std::set<std::string> declarations;
    std::vector<const Expression *> runtime_calls;
    bool opaque_storage = false;
  };
  mutable std::map<const Form *, FreeBindings> free_bindings_;
  mutable std::map<size_t, bool> closed_module_storage_;
  mutable std::map<const Form *, std::set<std::string>> definition_dependencies_;
  std::map<const Form *, std::set<std::string>> free_types_;
  std::map<const Form *, std::set<std::string>> enclosing_names_;
  std::set<const Form *> warned_captures_;
  std::set<size_t> warned_missing_return_;
  std::set<std::tuple<std::string, unsigned, unsigned, std::string>> warned_shadow_;
  LexicalState lexical_;
  std::vector<CheckedStorage> *storage_ = nullptr;
  std::vector<CheckedBinding> *bindings_ = nullptr;
  std::map<size_t, std::vector<size_t>> scope_storage_;
  std::map<const Expression *, size_t> checked_scope_owners_;
  bool proving_ = false;
  bool validating_ = false;
  CallEffects observed_;
  std::vector<std::set<size_t>> possible_targets_, possible_null_sources_, target_dependencies_;
  std::vector<CheckedStorage> catalog_storage_;
  size_t next_target_version_ = 0;
  size_t next_selection_identity_ = 0;
  std::map<size_t, std::set<size_t>> selection_sources_;
  // These immutable selections observe the return that entered an action.
  std::map<size_t, size_t> return_selection_actions_;
  struct BooleanDecision {
    enum class Origin { input, storage };
    Origin origin;
    size_t id;
    bool value_when_true;
  };
  size_t next_boolean_observation_ = 0;
  std::map<std::tuple<BooleanDecision::Origin, size_t, size_t, size_t>, size_t> conditional_selections_;
  std::map<size_t, size_t> incoming_selections_;
  std::set<size_t> rebound_;
  std::map<std::string, size_t> external_owners_;
  struct PayloadWrites {
    std::set<size_t> places, origins;
    void include(const CheckedOperation &target) {
      places.insert(target.targets.begin(), target.targets.end());
      origins.insert(target.origins.begin(), target.origins.end());
    }
  } payload_writes_;
  struct LoopState {
    size_t first, last;
    std::set<size_t> bindings, rebound, expired;
    PayloadWrites writes;
    std::map<size_t, CleanupEquations::Initialization> initialized = {};
    std::map<size_t, CleanupEquations::Targets> header_targets = {};
  };
  std::map<const Expression *, LoopState> loop_states_;
  bool loop_evidence_changed_ = false;
  std::vector<const Expression *> active_loops_;
  struct AbruptPath {
    LexicalState state;
    ExceptionFacts exceptions;
    Location at;
    Completion completion{2, false};
    bool immediate_scope_failure = false;
    std::optional<HereExit> here_exit{};
    bool receiver_transfer_pending = false;
  };
  struct AbruptFlow { size_t depth; std::vector<AbruptPath> paths; std::set<size_t> declared{}; };
  AbruptFlow *abrupt_ = nullptr;
  std::optional<size_t> caught_;
  std::set<size_t> catch_bindings_;
  std::map<size_t, std::map<size_t, CompletionInputs>> caught_inputs_;
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
  std::optional<size_t> deferred_action_;
  struct ReturnInputs {
    CompletionInputs present, absent;
    std::optional<CleanupEquations::Targets> borrowed_selection{};
  } return_inputs_;
  size_t next_finalizer_input_ = 2 * CleanupEquations::deferred_return_offset;
  std::vector<CallEffects::ReadSite> read_sites_;
  std::vector<Location> declared_sites_;
  std::map<const Expression *, size_t> declared_requirements_;
  std::set<size_t> deferred_actions_with_checked_scopes_;
  bool delayed_ = false;
  size_t next_scope_ = 1;
  static constexpr size_t instance_limit = 4096;
  static constexpr size_t family_coverage_limit = 4096;

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
    require_initialized(view, at, name);
  }
  using Substitutions = std::vector<std::optional<Type>>;
  using Access = std::vector<std::optional<bool>>;
  // Only a known argument or return contract can supply an implicit callable's result.
  enum class ExpectedTypeUse { materialization, callable_result };
  enum class EvidenceRead { payload, target, discarded };
  static ExpectedTypeUse expected_type_use(ArgumentMode mode) {
    return mode == ArgumentMode::value ? ExpectedTypeUse::callable_result : ExpectedTypeUse::materialization;
  }
  static const Type *callable_expectation(const Type *type) {
    return type && type->kind == Type::procedure ? type : nullptr;
  }
  struct TypeEvidenceExits;
  struct TypeDeferredFlow;
  enum class TargetPresence { unknown, present, absent };
  using PreparedEntryTarget = std::pair<const Expression *, size_t>;
  using EvidenceSlot = std::variant<size_t, const Expression *, std::string, std::pair<const Form *, std::string>, PreparedEntryTarget>;
  using PresenceFacts = std::map<EvidenceSlot, TargetPresence>;
  using EvidencePlaces = std::set<EvidenceSlot>;
  using PresenceSlotMap = std::map<EvidenceSlot, EvidencePlaces>;
  using BooleanObservations = std::map<size_t, bool>;
  struct ObservationPath {
    BooleanObservations booleans;
    PresenceFacts presence;
  };
  // Share Boolean decisions while keeping the proof for each reference
  // separate. Independent conditional writes need one decision per reference.
  struct ObservationDecisions {
    using Condition = size_t;
    struct Node {
      size_t observation;
      Condition when_false, when_true;
      auto operator<=>(const Node &) const = default;
    };
    size_t next_observation = 0;
    std::vector<Node> nodes;
    std::map<Node, Condition> indexed;
    std::map<std::pair<Condition, Condition>, Condition> intersections;

    static Condition negate(Condition value) { return value ^ 1; }
    Condition decision(size_t observation, Condition when_false, Condition when_true) {
      if (when_false == when_true) return when_true;
      const auto flipped = when_true & 1;
      const Node node{observation, when_false ^ flipped, when_true ^ flipped};
      const auto [entry, inserted] = indexed.try_emplace(node, 2 * (nodes.size() + 1));
      if (inserted) nodes.push_back(node);
      return entry->second ^ flipped;
    }
    Node split(Condition condition) const {
      const auto &node = nodes.at((condition >> 1) - 1);
      return {node.observation, node.when_false ^ (condition & 1), node.when_true ^ (condition & 1)};
    }
    Condition both(Condition left, Condition right) {
      if (left > right) std::swap(left, right);
      if (!left || left == negate(right)) return 0;
      if (left == right || left == 1) return right;
      const auto key = std::pair{left, right};
      if (const auto known = intersections.find(key); known != intersections.end()) return known->second;
      const auto a = split(left), b = split(right);
      const auto observation = std::min(a.observation, b.observation);
      const auto no = both(a.observation == observation ? a.when_false : left,
                           b.observation == observation ? b.when_false : right);
      const auto yes = both(a.observation == observation ? a.when_true : left,
                            b.observation == observation ? b.when_true : right);
      const auto result = decision(observation, no, yes);
      intersections.emplace(key, result);
      return result;
    }
    Condition either(Condition left, Condition right) { return negate(both(negate(left), negate(right))); }
    Condition project(Condition value, size_t first, std::map<Condition, Condition> &known) {
      if (value < 2) return value;
      const auto node = split(value);
      if (node.observation >= first) return 1;
      if (const auto found = known.find(value); found != known.end()) return found->second;
      const auto no = project(node.when_false, first, known), yes = project(node.when_true, first, known);
      const auto result = decision(node.observation, no, yes);
      known.emplace(value, result);
      return result;
    }
  };
  struct ObservationPaths {
    using Condition = ObservationDecisions::Condition;
    struct Outcomes {
      Condition present = 0, absent = 0;
      bool operator==(const Outcomes &) const = default;
    };
    std::shared_ptr<ObservationDecisions> decisions;
    Condition reachable = 0;
    std::map<EvidenceSlot, Outcomes> presence;

    static ObservationPaths unconditional() { return {{}, 1, {}}; }
    explicit operator bool() const { return reachable != 0; }
    bool operator==(const ObservationPaths &other) const {
      return reachable == other.reachable && presence == other.presence;
    }
    Condition both(Condition left, Condition right) const { return decisions ? decisions->both(left, right) : left & right; }
    Condition either(Condition left, Condition right) const { return decisions ? decisions->either(left, right) : left | right; }
    static Condition negate(Condition condition) { return ObservationDecisions::negate(condition); }
    static PresenceFacts common_presence(PresenceFacts left, const PresenceFacts &right) {
      std::erase_if(left, [&](const auto &fact) {
        const auto other = right.find(fact.first);
        return other == right.end() || other->second != fact.second;
      });
      return left;
    }
    PresenceFacts known_presence() const {
      PresenceFacts known;
      if (reachable) for (const auto &[slot, evidence] : presence) {
        if (evidence.present == reachable) known.emplace(slot, TargetPresence::present);
        if (evidence.absent == reachable) known.emplace(slot, TargetPresence::absent);
      }
      return known;
    }
    void mask_presence() {
      for (auto &[slot, evidence] : presence) {
        evidence.present = both(evidence.present, reachable);
        evidence.absent = both(evidence.absent, reachable);
      }
      std::erase_if(presence, [](const auto &entry) { return entry.second == Outcomes{}; });
    }
    void add(const ObservationPath &path) {
      auto branch = unconditional();
      branch.decisions = decisions;
      for (const auto &[id, value] : path.booleans) {
        const auto observed = decisions->decision(id, 0, 1);
        branch.reachable = both(branch.reachable, value ? observed : negate(observed));
      }
      for (const auto &[slot, fact] : path.presence) {
        if (fact == TargetPresence::present) branch.presence[slot].present = branch.reachable;
        if (fact == TargetPresence::absent) branch.presence[slot].absent = branch.reachable;
      }
      join(branch);
    }
    void join(const ObservationPaths &other) {
      if (!other) return;
      if (!*this) { *this = other; return; }
      if (!decisions) decisions = other.decisions;
      const auto left = reachable, right = other.reachable;
      const auto joined = either(left, right);
      auto at_join = [&](Condition a, Condition b) {
        return both(joined, both(either(a, negate(left)), either(b, negate(right))));
      };
      for (auto &[slot, evidence] : presence) {
        const auto found = other.presence.find(slot);
        const auto fact = found == other.presence.end() ? Outcomes{} : found->second;
        evidence = {at_join(evidence.present, fact.present), at_join(evidence.absent, fact.absent)};
      }
      for (const auto &[slot, fact] : other.presence) if (!presence.contains(slot))
        presence.emplace(slot, Outcomes{at_join(0, fact.present), at_join(0, fact.absent)});
      reachable = joined;
      std::erase_if(presence, [](const auto &entry) { return entry.second == Outcomes{}; });
    }
    void exclude_conflicts() {
      for (const auto &[slot, fact] : presence)
        reachable = both(reachable, negate(both(fact.present, fact.absent)));
      mask_presence();
    }
    void conjoin(const ObservationPaths &other) {
      if (!decisions) decisions = other.decisions;
      reachable = both(reachable, other.reachable);
      for (const auto &[slot, fact] : other.presence) {
        auto &retained = presence[slot];
        retained = {either(retained.present, fact.present), either(retained.absent, fact.absent)};
      }
      exclude_conflicts();
    }
    template <typename Map> ObservationPaths map_presence(Map map, bool same_selection) const {
      auto result = *this;
      result.presence.clear();
      for (const auto &[slot, fact] : presence) map(slot, [&](const auto &mapped) {
        auto &retained = result.presence[mapped];
        retained = {either(retained.present, fact.present), either(retained.absent, fact.absent)};
      });
      if (same_selection) result.exclude_conflicts();
      else for (auto &[slot, fact] : result.presence) {
        const auto consistent = negate(both(fact.present, fact.absent));
        fact = {both(fact.present, consistent), both(fact.absent, consistent)};
      }
      std::erase_if(result.presence, [](const auto &entry) { return entry.second == Outcomes{}; });
      return result;
    }
    template <typename Predicate> void forget_presence(Predicate removed) {
      std::erase_if(presence, [&](const auto &fact) { return removed(fact.first); });
    }
    void forget_observations(size_t first) {
      if (!decisions) return;
      std::map<Condition, Condition> known;
      const auto projected = decisions->project(reachable, first, known);
      const auto must_hold = [&](Condition condition) {
        const auto counterexample = decisions->project(both(reachable, negate(condition)), first, known);
        return both(projected, negate(counterexample));
      };
      for (auto &[slot, fact] : presence) fact = {must_hold(fact.present), must_hold(fact.absent)};
      reachable = projected;
      std::erase_if(presence, [](const auto &entry) { return entry.second == Outcomes{}; });
    }
    ObservationPaths remap(const PresenceSlotMap &slots) const {
      return map_presence([&](const auto &slot, const auto &accept) {
        if (const auto mapped = slots.find(slot); mapped != slots.end())
          for (const auto &id : mapped->second) accept(id);
      }, false);
    }
  };
  struct EvidenceSelection {
    std::optional<EvidenceSlot> slot;
    std::set<EvidenceSlot> footprint;
    bool operator==(const EvidenceSelection &) const = default;
  };
  struct PresenceBranches {
    ObservationPaths when_true = ObservationPaths::unconditional(), when_false = ObservationPaths::unconditional();
    auto &at(bool value) { return value ? when_true : when_false; }
    const auto &at(bool value) const { return value ? when_true : when_false; }
    bool operator==(const PresenceBranches &) const = default;
    static PresenceBranches constant(bool value) {
      PresenceBranches result;
      result.at(!value) = {};
      return result;
    }
    static PresenceBranches observed(const std::shared_ptr<ObservationDecisions> &decisions, size_t id) {
      PresenceBranches result{{}, {}};
      for (const bool value : {false, true}) {
        auto &outcome = result.at(value);
        outcome.decisions = decisions;
        outcome.add({{{id, value}}, {}});
      }
      return result;
    }
    template <typename Removed> void forget(Removed removed) {
      for (const bool value : {false, true}) at(value).forget_presence(removed);
    }
    void forget_observations(size_t first) {
      for (const bool value : {false, true}) at(value).forget_observations(first);
    }
    PresenceBranches remap(const PresenceSlotMap &slots) const {
      return {when_true.remap(slots), when_false.remap(slots)};
    }
  };
  struct TargetEvidence {
    EvidenceSelection selection{};
    TargetPresence presence = TargetPresence::unknown;
    std::optional<PresenceBranches> branches;
    EvidencePlaces places;
    bool contextual_null = false;
    EvidencePlaces same_selected;
    // Facts implied by the present and absent outcomes of the selected value.
    std::optional<PresenceBranches> presence_proof;
    bool initialized = false;
  };
  struct QueryWrites {
    EvidencePlaces places;
    bool unknown = false;
    void record(const EvidencePlaces &written) {
      if (written.empty()) unknown = true;
      else places.insert(written.begin(), written.end());
    }
    void merge(const QueryWrites &other) {
      places.insert(other.places.begin(), other.places.end());
      unknown |= other.unknown;
    }
    bool touches(const EvidencePlaces &selected) const {
      return unknown || std::ranges::any_of(selected, [&](const auto &place) { return places.contains(place); });
    }
  };
  struct PreparedTransfer {
    EvidencePlaces destination;
    std::optional<PresenceBranches> boolean;
    bool writes_boolean;
    std::optional<EvidenceSlot> saved_selection;
  };
  struct PreparedReferenceAssignment {
    EvidenceSelection destination;
    TargetEvidence value;
  };
  struct PreparedBooleanSwap {
    EvidencePlaces left, right;
  };
  struct PreparedReferenceSwap {
    EvidenceSelection left, right;
  };
  using PreparedBuiltin = std::variant<std::monostate, PreparedTransfer, PreparedReferenceAssignment,
                                       PreparedBooleanSwap, PreparedReferenceSwap>;
  struct TypeDeferredEffects {
    QueryWrites payloads, slots;
    EvidencePlaces initialized;
    EvidencePlaces initialized_selections;
    std::set<size_t> outcomes;
    bool opaque = false;
    PreparedBuiltin builtin;
  };
  struct TypePreparedCall {
    const Expression *expression;
    TypeDeferredEffects effects{};
  };
  struct QuerySelections {
    using Observations = std::map<EvidenceSlot, PresenceBranches>;
    using Uninitialized = std::map<EvidenceSlot, ObservationPaths>;
    bool tracking = false;
    // Revisions prevent a tested selection from refining a slot written
    // during preparation of the other identity operand.
    size_t writes = 0;
    std::map<std::string, EvidenceSelection> bindings;
    std::map<EvidenceSlot, std::set<EvidenceSlot>> footprints;
    std::map<EvidenceSlot, std::set<EvidenceSlot>> possible_overlap;
    PresenceFacts facts;
    std::optional<ObservationPaths> conditions;
    // Branches share an identity source. A new payload never reuses the
    // identity of a Boolean observed before a write.
    std::shared_ptr<ObservationDecisions> observation_decisions;
    // Selection slots and payload places have separate write effects. A
    // borrowed Boolean and an independent Boolean can read the same payload.
    std::map<std::string, EvidencePlaces> value_places;
    std::map<EvidenceSlot, EvidencePlaces> targets;
    // Private storage with a reaching path on which its payload is unwritten.
    Uninitialized uninitialized;
    EvidencePlaces initialized_selections;
    bool initialization_reachable = true;
    Observations booleans, reference_observations;
    // The original targets of borrowed Boolean formals may overlap even
    // though their invocation-local reference slots are independent.
    EvidencePlaces borrowed_booleans;
    std::map<EvidenceSlot, size_t> changed;
    // Invocation entry selections have no writable binding in this body.
    EvidencePlaces entry_selections;
    // The members retain one selected reference value in independent slots.
    // A write removes the affected slot from its group.
    std::set<EvidencePlaces> equal_selections;
    QueryWrites payload_writes, deferred_writes;
    bool deferred_return = false;

    static EvidencePlaces join_places(EvidencePlaces left, const EvidencePlaces &right) {
      if (left.empty() || right.empty()) return {};
      left.insert(right.begin(), right.end());
      return left;
    }
    static EvidencePlaces common_slots(EvidencePlaces left, const EvidencePlaces &right) {
      std::erase_if(left, [&](const auto &slot) { return !right.contains(slot); });
      return left;
    }
    const EvidencePlaces *selection_group(const EvidenceSlot &slot) const {
      for (const auto &group : equal_selections) if (group.contains(slot)) return &group;
      return nullptr;
    }
    EvidencePlaces same_selected(const EvidenceSlot &slot) const {
      const auto *group = selection_group(slot);
      return group ? *group : EvidencePlaces{slot};
    }
    template <typename Removed> void remove_selected(Removed removed) {
      std::set<EvidencePlaces> retained;
      for (auto group : equal_selections) {
        std::erase_if(group, removed);
        if (group.size() > 1) retained.insert(std::move(group));
      }
      equal_selections = std::move(retained);
    }
    bool include_presence(PresenceFacts &into, const PresenceFacts &known) const {
      std::set<const EvidencePlaces *> expanded;
      const auto insert = [&](const EvidenceSlot &slot, TargetPresence value) {
        const auto [entry, inserted] = into.emplace(slot, value);
        return inserted || entry->second == value;
      };
      for (const auto &[slot, fact] : known) {
        if (!insert(slot, fact)) return false;
        const auto *group = selection_group(slot);
        if (group && expanded.insert(group).second)
          for (const auto &same : *group) if (!insert(same, fact)) return false;
      }
      return true;
    }
    void retain_selection(const EvidenceSlot &slot, EvidencePlaces selected) {
      if (selected.empty()) return;
      selected.insert(slot);
      if (selected.size() < 2) return;
      equal_selections.insert(std::move(selected));
      CleanupEquations::ConditionFacts::close_equalities(equal_selections);
      auto expanded = facts;
      if (include_presence(expanded, facts)) facts = std::move(expanded);
    }
    std::optional<PresenceBranches> compact(std::optional<PresenceBranches> proof) const {
      if (!proof || equal_selections.empty()) return proof;
      for (const bool value : {false, true}) proof->at(value) = compact_paths(proof->at(value));
      return proof;
    }
    ObservationPaths compact_paths(const ObservationPaths &paths) const {
      return paths.map_presence([&](const auto &slot, const auto &accept) {
        const auto *group = selection_group(slot);
        accept(group ? *group->begin() : slot);
      }, true);
    }
    ObservationPaths expand_paths(ObservationPaths paths) const {
      if (equal_selections.empty() || paths.presence.empty()) return paths;
      return compact_paths(paths).map_presence([&](const auto &slot, const auto &accept) {
        if (const auto *group = selection_group(slot)) for (const auto &same : *group) accept(same);
        else accept(slot);
      }, true);
    }
    PresenceBranches expanded(PresenceBranches proof) const {
      for (const bool value : {false, true}) proof.at(value) = expand_paths(std::move(proof.at(value)));
      return proof;
    }
    template <typename Written> void preserve_observations(Written written, std::optional<PresenceBranches> &incoming) {
      if (booleans.empty() && reference_observations.empty() && !incoming && !conditions) return;
      std::map<EvidenceSlot, EvidenceSlot> replacements;
      for (const auto &group : equal_selections) {
        const auto survivor = std::find_if_not(group.begin(), group.end(), written);
        if (survivor == group.end()) continue;
        for (const auto &slot : group) if (written(slot)) replacements.emplace(slot, *survivor);
      }
      const auto preserve = [&](ObservationPaths &paths) {
        paths = paths.map_presence([&](const auto &slot, const auto &accept) {
          if (const auto replacement = replacements.find(slot); replacement != replacements.end()) accept(replacement->second);
          else if (!written(slot)) accept(slot);
        }, true);
      };
      const auto preserve_proof = [&](PresenceBranches &proof) {
        preserve(proof.at(true));
        preserve(proof.at(false));
      };
      for (auto &[place, proof] : booleans) preserve_proof(proof);
      for (auto &[slot, proof] : reference_observations) preserve_proof(proof);
      if (incoming) preserve_proof(*incoming);
      if (conditions) preserve(*conditions);
    }
    static std::optional<PresenceBranches> alternatives(std::optional<PresenceBranches> left,
                                                         const std::optional<PresenceBranches> &right) {
      if (!left && !right) return {};
      auto joined = left.value_or(PresenceBranches{});
      const auto other = right.value_or(PresenceBranches{});
      for (const bool value : {false, true}) joined.at(value).join(other.at(value));
      if (joined == PresenceBranches{}) return {};
      return joined;
    }
    EvidencePlaces places(const EvidenceSelection &selection) const {
      if (selection.slot) if (const auto found = targets.find(*selection.slot); found != targets.end()) return found->second;
      EvidencePlaces places;
      for (const auto &slot : selection.footprint) {
        const auto found = targets.find(slot);
        if (found == targets.end()) return {};
        places.insert(found->second.begin(), found->second.end());
      }
      return places;
    }
    std::optional<PresenceBranches> read_boolean(const EvidencePlaces &places) {
      if (places.size() == 1 && !booleans.contains(*places.begin()) && observation_decisions)
        booleans.emplace(*places.begin(), PresenceBranches::observed(observation_decisions, observation_decisions->next_observation++));
      std::optional<PresenceBranches> result;
      bool first = true;
      for (const auto &place : places) {
        const auto found = booleans.find(place);
        if (found == booleans.end()) return {};
        const auto stored = compact(found->second);
        result = first ? stored : alternatives(std::move(result), stored);
        first = false;
      }
      return result ? std::optional{expanded(std::move(*result))} : std::nullopt;
    }
    void write_boolean(const EvidencePlaces &places, const std::optional<PresenceBranches> &proof) {
      if (!tracking) return;
      payload_writes.record(places);
      if (places.empty()) { booleans.clear(); return; }
      if (std::ranges::any_of(places, [&](const auto &place) { return borrowed_booleans.contains(place); }))
        std::erase_if(booleans, [&](const auto &entry) { return borrowed_booleans.contains(entry.first) && !places.contains(entry.first); });
      const auto incoming = compact(proof);
      for (const auto &place : places) {
        const auto before = booleans.find(place);
        const auto assigned = places.size() == 1 ? incoming : alternatives(
            before == booleans.end() ? std::nullopt : compact(before->second), incoming);
        if (assigned) booleans.insert_or_assign(place, *assigned);
        else booleans.erase(place);
      }
    }
    void exchange_boolean(const EvidencePlaces &left, const EvidencePlaces &right) {
      if (left.size() == 1 && left == right) return;
      const auto lhs = read_boolean(left), rhs = read_boolean(right);
      write_boolean(left, rhs);
      write_boolean(right, lhs);
    }
    void remove_binding(const std::string &name) {
      bindings.erase(name);
      value_places.erase(name);
    }

    TargetPresence presence(const EvidenceSelection &selection) const {
      if (selection.slot) {
        if (const auto known = facts.find(*selection.slot); known != facts.end()) return known->second;
      }
      if (selection.footprint.empty()) return TargetPresence::unknown;
      const auto known = facts.find(*selection.footprint.begin());
      if (known == facts.end()) return TargetPresence::unknown;
      for (const auto &slot : selection.footprint) {
        const auto other = facts.find(slot);
        if (other == facts.end() || other->second != known->second) return TargetPresence::unknown;
      }
      return known->second;
    }
    TargetEvidence selected_target(const EvidenceSelection &selection) const {
      TargetEvidence result;
      result.selection = selection;
      result.presence = presence(selection);
      result.places = places(selection);
      if (selection.slot) {
        result.same_selected = same_selected(*selection.slot);
        if (const auto proof = reference_observations.find(*selection.slot); proof != reference_observations.end())
          result.presence_proof = expanded(proof->second);
      }
      return result;
    }
    std::optional<PresenceBranches> selection_outcomes(const TargetEvidence &target) const {
      auto proof = target.presence_proof.value_or(PresenceBranches{});
      for (const bool present : {false, true}) {
        const auto fact = present ? TargetPresence::present : TargetPresence::absent;
        auto &observed = proof.at(present);
        if (target.presence != TargetPresence::unknown && target.presence != fact) observed = {};
        if (!observed) continue;
        PresenceFacts same;
        for (const auto &slot : target.same_selected) same.emplace(slot, fact);
        if (!same.empty()) {
          ObservationPaths known;
          known.add({{}, std::move(same)});
          observed.conjoin(known);
        }
      }
      return proof == PresenceBranches{} ? std::nullopt : std::optional{std::move(proof)};
    }
    void remember_selection(const EvidenceSlot &slot, const std::optional<PresenceBranches> &proof) {
      auto stored = compact(proof);
      if (stored && *stored != PresenceBranches{}) reference_observations.insert_or_assign(slot, std::move(*stored));
    }
    void save_selection(const EvidenceSlot &slot, TargetEvidence selected) {
      if (!tracking) return;
      if (footprints.contains(slot)) {
        // The same source site can be reached on another abstract iteration.
        assign({slot, {slot}}, std::move(selected));
        return;
      }
      footprints[slot] = {slot};
      if (selected.presence != TargetPresence::unknown) facts[slot] = selected.presence;
      if (!selected.places.empty() || selected.presence == TargetPresence::absent) targets[slot] = std::move(selected.places);
      if (selected.initialized || selected.presence == TargetPresence::absent) initialized_selections.insert(slot);
      retain_selection(slot, std::move(selected.same_selected));
      remember_selection(slot, selected.presence_proof);
    }
    void bind(const std::string &name, EvidenceSlot slot, TargetEvidence selected) {
      if (!tracking) return;
      save_selection(slot, std::move(selected));
      bindings[name] = {slot, {slot}};
    }
    void declare_place(const std::string &name, EvidenceSlot slot, TargetEvidence target) {
      if (!tracking) return;
      auto &source = target.selection;
      if (!source.slot) source.slot = std::move(slot);
      footprints.try_emplace(*source.slot, source.footprint);
      if (target.presence != TargetPresence::unknown) facts[*source.slot] = target.presence;
      retain_selection(*source.slot, std::move(target.same_selected));
      remember_selection(*source.slot, target.presence_proof);
      bindings[name] = std::move(source);
    }
    void forget() {
      if (!tracking) return;
      ++writes;
      facts.clear();
      conditions.reset();
      targets.clear();
      initialized_selections.clear();
      booleans.clear();
      reference_observations.clear();
      equal_selections.clear();
      for (const auto &[slot, footprint] : footprints) changed[slot] = writes;
    }
    bool may_overlap(const EvidencePlaces &footprint, const EvidencePlaces &written) const {
      return footprint.empty() || written.empty() || std::any_of(footprint.begin(), footprint.end(),
            [&](const auto &slot) {
              if (written.contains(slot)) return true;
              const auto other = possible_overlap.find(slot);
              return other != possible_overlap.end() && std::any_of(other->second.begin(), other->second.end(),
                  [&](const auto &candidate) { return written.contains(candidate); });
            });
    }
    void record_assignment_selection(const EvidenceSelection &selection, TargetEvidence selected) {
      if (selection.footprint.size() == 1) {
        const auto &physical = *selection.footprint.begin();
        if (!selection.slot || *selection.slot != physical) {
          retain_selection(physical, selected.same_selected);
          remember_selection(physical, selected.presence_proof);
        }
      }
      if (selection.slot) {
        retain_selection(*selection.slot, std::move(selected.same_selected));
        remember_selection(*selection.slot, selected.presence_proof);
      }
    }
    void assign(const EvidenceSelection &selection, TargetEvidence selected) {
      if (!tracking) return;
      if (selection.footprint.empty()) { forget(); return; }
      ++writes;
      const auto known = selected.presence;
      const auto written = [&](const EvidenceSlot &slot) {
        return may_overlap(footprints.at(slot), selection.footprint);
      };
      std::map<EvidenceSlot, EvidencePlaces> updated_targets;
      if (!selected.places.empty() || known == TargetPresence::absent) {
        for (const auto &[slot, footprint] : footprints) {
          if (!written(slot)) continue;
          const bool definite = selection.slot == slot ||
              (selection.footprint.size() == 1 && selection.footprint.contains(slot));
          if (definite) updated_targets.emplace(slot, selected.places);
          else {
            // A write through a selected place may leave another slot intact.
            const EvidenceSelection previous{slot, footprint};
            auto places_before = places(previous);
            if (places_before.empty() && presence(previous) != TargetPresence::absent) continue;
            places_before.insert(selected.places.begin(), selected.places.end());
            updated_targets.emplace(slot, std::move(places_before));
          }
        }
      }
      std::erase_if(facts, [&](const auto &fact) {
        return fact.second != known && written(fact.first);
      });
      for (const auto &[slot, footprint] : footprints) if (written(slot)) changed[slot] = writes;
      std::erase_if(reference_observations, [&](const auto &entry) { return written(entry.first); });
      preserve_observations(written, selected.presence_proof);
      std::erase_if(targets, [&](const auto &target) { return written(target.first); });
      targets.merge(updated_targets);
      std::erase_if(initialized_selections, written);
      remove_selected(written);
      std::erase_if(selected.same_selected, written);
      if (known != TargetPresence::unknown) {
        if (selection.footprint.size() == 1) facts[*selection.footprint.begin()] = known;
        if (selection.slot) facts[*selection.slot] = known;
      }
      if (selected.initialized || known == TargetPresence::absent) {
        if (selection.footprint.size() == 1) initialized_selections.insert(*selection.footprint.begin());
        if (selection.slot) initialized_selections.insert(*selection.slot);
      }
      record_assignment_selection(selection, std::move(selected));
    }
    void exchange(const EvidenceSelection &left, const EvidenceSelection &right) {
      if (left.footprint.size() == 1 && left.footprint == right.footprint) return;
      auto left_selected = selected_target(left), right_selected = selected_target(right);
      const auto before = writes;
      const bool overlap = may_overlap(left.footprint, right.footprint);
      assign(left, {.presence = right_selected.presence, .places = right_selected.places});
      assign(right, {.presence = left_selected.presence, .places = left_selected.places});
      if (overlap) return;
      forget_changed(left_selected, before);
      forget_changed(right_selected, before);
      record_assignment_selection(left, std::move(right_selected));
      record_assignment_selection(right, std::move(left_selected));
    }
    ObservationPaths outcome(const std::optional<PresenceBranches> &proof, bool value) const {
      auto result = conditions.value_or(ObservationPaths::unconditional());
      if (proof) result.conjoin(proof->at(value));
      if (!facts.empty()) {
        ObservationPaths known;
        known.add({{}, facts});
        result.conjoin(known);
      }
      return expand_paths(std::move(result));
    }
    bool reachable(const std::optional<PresenceBranches> &proof, bool value) const {
      return !proof || static_cast<bool>(outcome(proof, value));
    }
    ObservationPaths reach_condition() const {
      auto condition = conditions.value_or(ObservationPaths::unconditional());
      condition.presence.clear();
      return condition;
    }
    void initialize(const EvidencePlaces &places) {
      if (places.size() == 1) uninitialized.erase(*places.begin());
    }
    void initialize_selection(const TargetEvidence &target) {
      if (target.selection.slot) initialized_selections.insert(*target.selection.slot);
      initialized_selections.insert(target.same_selected.begin(), target.same_selected.end());
    }
    void initialize(const TargetEvidence &target) {
      initialize(target.places);
      initialize_selection(target);
    }
    void initialize_if_present(const TargetEvidence &target) {
      initialize_selection(target);
      if (target.places.size() != 1) return;
      const auto missing = uninitialized.find(*target.places.begin());
      if (missing == uninitialized.end()) return;
      auto absent = outcome(selection_outcomes(target), false);
      // A later rebind can change a slot's presence without writing this cell.
      absent.presence.clear();
      missing->second.conjoin(absent);
      if (!missing->second) uninitialized.erase(missing);
    }
    bool selection_initialized(const TargetEvidence &target) const {
      return target.initialized || (target.selection.slot && initialized_selections.contains(*target.selection.slot)) ||
          std::ranges::any_of(target.same_selected, [&](const auto &slot) { return initialized_selections.contains(slot); });
    }
    bool may_read_uninitialized(const EvidenceSlot &place, const ObservationPaths &reaching) const {
      const auto found = uninitialized.find(place);
      if (found == uninitialized.end()) return false;
      auto missing = found->second;
      missing.conjoin(reaching);
      return static_cast<bool>(missing);
    }
    void merge_initialization(const QuerySelections &other) {
      if (!other.initialization_reachable) return;
      if (!initialization_reachable) {
        uninitialized = other.uninitialized;
        initialized_selections = other.initialized_selections;
        initialization_reachable = true;
        return;
      }
      initialized_selections = common_slots(std::move(initialized_selections), other.initialized_selections);
      if (uninitialized.empty() && other.uninitialized.empty()) return;
      const auto left_path = reach_condition(), right_path = other.reach_condition();
      for (auto &[place, missing] : uninitialized) missing.conjoin(left_path);
      for (const auto &[place, possible] : other.uninitialized) {
        auto missing = possible;
        missing.conjoin(right_path);
        uninitialized[place].join(missing);
      }
      std::erase_if(uninitialized, [](const auto &entry) { return !entry.second; });
    }
    PresenceBranches on_reaching_path(PresenceBranches proof) const {
      proof = expanded(std::move(proof));
      if (conditions) {
        const auto condition = reach_condition();
        for (const bool value : {false, true}) proof.at(value).conjoin(condition);
      }
      return proof;
    }
    void observe_choice(std::optional<PresenceBranches> &proof) const {
      if (!observation_decisions) return;
      auto branches = proof.value_or(PresenceBranches{});
      const auto yes = outcome(proof, true), no = outcome(proof, false);
      if (!observation_decisions->both(yes.reachable, no.reachable)) return;
      const auto choice = PresenceBranches::observed(observation_decisions, observation_decisions->next_observation++);
      for (const bool value : {false, true}) branches.at(value).conjoin(choice.at(value));
      proof = std::move(branches);
    }
    void retain_conditions(ObservationPaths current) {
      // Unconditional presence stays in the cheap, direct lookup table.
      current.forget_presence([&](const auto &slot) { return facts.contains(slot); });
      if (!current || current == ObservationPaths::unconditional()) conditions.reset();
      else conditions = std::move(current);
    }
    void assume(const std::optional<PresenceBranches> &proof, bool value) {
      if (!proof) return;
      auto refined = outcome(proof, value);
      if (!refined) return;
      facts = refined.known_presence();
      // Readiness describes the selected non-null payload. Absence can satisfy
      // it at an alternative join without initializing any physical target.
      for (const auto &[slot, presence] : facts)
        if (presence == TargetPresence::absent) initialized_selections.insert(slot);
      retain_conditions(std::move(refined));
    }
    void forget_observations(size_t first) {
      for (auto &[place, missing] : uninitialized) missing.forget_observations(first);
      for (auto &[place, proof] : booleans) proof.forget_observations(first);
      for (auto &[slot, proof] : reference_observations) proof.forget_observations(first);
      if (conditions) {
        auto retained = std::move(*conditions);
        retained.forget_observations(first);
        retain_conditions(std::move(retained));
      }
    }
    PresenceBranches observe(const std::optional<PresenceBranches> &proof) const {
      return {outcome(proof, true), outcome(proof, false)};
    }
    bool changed_since(const EvidenceSlot &slot, size_t since) const {
      const auto last = changed.find(slot);
      return last != changed.end() && last->second > since;
    }
    void forget_changed(TargetEvidence &target, size_t since) const {
      const auto written = [&](const EvidenceSlot &slot) { return changed_since(slot, since); };
      if (target.selection.slot && written(*target.selection.slot)) target.selection.slot.reset();
      if (target.branches) target.branches->forget(written);
      if (target.presence_proof) target.presence_proof->forget(written);
      std::erase_if(target.same_selected, written);
    }
    bool same_prepared_target(const TargetEvidence &earlier, size_t since, const TargetEvidence &later) const {
      if (earlier.presence == TargetPresence::present && later.presence == TargetPresence::present &&
          earlier.places.size() == 1 && earlier.places == later.places) return true;
      const auto same = [&](const EvidenceSlot &slot) {
        return !changed_since(slot, since) && (later.selection.slot == slot || later.same_selected.contains(slot));
      };
      return (earlier.selection.slot && same(*earlier.selection.slot)) || std::ranges::any_of(earlier.same_selected, same);
    }
    void join_flow(const QuerySelections &other) {
      merge_initialization(other);
      std::set<EvidencePlaces> shared;
      for (const auto &left : equal_selections) for (const auto &right : other.equal_selections) {
        auto group = common_slots(left, right);
        if (group.size() > 1) shared.insert(std::move(group));
      }
      std::erase_if(targets, [&](auto &target) {
        return !other.targets.contains(target.first);
      });
      for (auto &[slot, places] : targets) {
        const auto &selected = other.targets.at(slot);
        places.insert(selected.begin(), selected.end());
      }
      for (auto stored = booleans.begin(); stored != booleans.end();) {
        const auto right = other.booleans.find(stored->first);
        if (right != other.booleans.end() && stored->second == right->second) { ++stored; continue; }
        const auto joined = right == other.booleans.end() ? std::nullopt :
            alternatives(on_reaching_path(stored->second), other.on_reaching_path(right->second));
        if (joined) { stored->second = *joined; ++stored; }
        else stored = booleans.erase(stored);
      }
      Observations joined_selections;
      EvidencePlaces observed_slots;
      for (const auto &[slot, proof] : reference_observations) observed_slots.insert(slot);
      for (const auto &[slot, proof] : other.reference_observations) observed_slots.insert(slot);
      // Equality may hold on only one path. The selected value on that
      // path can still provide evidence for a particular joined outcome.
      const auto add_changed_groups = [&](const auto &groups) {
        for (const auto &group : groups) if (!shared.contains(group))
          observed_slots.insert(group.begin(), group.end());
      };
      add_changed_groups(equal_selections);
      add_changed_groups(other.equal_selections);
      const auto join_selection = [&](const EvidenceSlot &slot) {
        const EvidenceSelection selected{slot, {slot}};
        auto joined = alternatives(selection_outcomes(selected_target(selected)),
                                   other.selection_outcomes(other.selected_target(selected)));
        if (!joined) return;
        // The slot's own outcomes also follow directly from testing it.
        joined->forget([&](const auto &observed) { return observed == slot; });
        if (*joined != PresenceBranches{}) joined_selections.emplace(slot, std::move(*joined));
      };
      for (const auto &slot : observed_slots) join_selection(slot);
      // Join paths before intersecting their direct facts, so the facts
      // established under a distinguishable Boolean outcome are retained.
      std::optional<ObservationPaths> reaching;
      if (conditions || other.conditions) {
        reaching = outcome({}, true);
        reaching->join(other.outcome({}, true));
      }
      facts = ObservationPaths::common_presence(std::move(facts), other.facts);
      if (reaching) retain_conditions(std::move(*reaching));
      equal_selections = std::move(shared);
      for (auto &[place, proof] : booleans) proof = *compact(std::move(proof));
      reference_observations.clear();
      for (const auto &[slot, proof] : joined_selections) remember_selection(slot, proof);
      writes = std::max(writes, other.writes);
      for (const auto &[slot, revision] : other.changed) changed[slot] = std::max(changed[slot], revision);
      payload_writes.merge(other.payload_writes);
      deferred_writes.merge(other.deferred_writes);
      deferred_return |= other.deferred_return;
    }
    bool same_flow(const QuerySelections &other) const {
      return facts == other.facts && conditions == other.conditions && booleans == other.booleans && targets == other.targets &&
          reference_observations == other.reference_observations && equal_selections == other.equal_selections &&
          uninitialized == other.uninitialized && initialized_selections == other.initialized_selections &&
          initialization_reachable == other.initialization_reachable;
    }
  };
  struct TypeEvidenceState {
    struct Definition {
      size_t method;
      std::shared_ptr<const TypeEvidenceState> declaration;
    };
    std::map<std::string, std::optional<Type>> bindings;
    std::map<std::string, const Expression *> local_declarations;
    std::map<std::string, Definition> definitions;
    std::set<std::string> symbol_shadows;
    std::map<std::string, ViewEvidence> views;
    std::map<std::string, EvidenceOwner> owner_bindings;
    std::set<EvidenceOwner> external_owners;
    std::map<std::pair<EvidenceOwner, EvidenceOwner>, OwnerMatch> owner_relations;
    ViewEvidence value;
    QuerySelections selections;
    TargetEvidence target;
    std::optional<EvidenceOwner> full_scope_owner;
    bool complete = true;
    bool required = false;
    bool validating_body = false;
    enum class InitializationCheck { unevaluated, body, suppressed };
    InitializationCheck initialization = InitializationCheck::unevaluated;
    bool checks_initialization() const { return initialization == InitializationCheck::body; }
    bool checks_reaching_paths() const { return initialization != InitializationCheck::suppressed; }
    std::optional<Type> caught_exception;
    std::optional<Type> invocation_result;
    std::optional<EvidenceOwner> invocation_result_owner;
    std::optional<bool> pending_return;
    const ResultContract *borrowed_return = nullptr;
    bool tracks_target_places() const { return checks_initialization() || borrowed_return; }
    bool supplied_invocation_result = false;
    std::optional<size_t> method;
    Substitutions substitutions;
    std::vector<Type> retained;
    // Immutable definition evidence lives in result_obligations_; body scopes may shadow it.
    const TypeEvidenceState *declaration = nullptr;
    TypeEvidenceExits *exits = nullptr;
    TypeDeferredFlow *deferred = nullptr;
    size_t deferred_queue = 0;
    std::optional<size_t> repeated_observations;
    bool has_deferred_action = false;
    std::set<const Expression *> delayed_controls;
    std::optional<TypePreparedCall> prepared_call;
    bool preparing(const Expression &expr) const { return prepared_call && prepared_call->expression == &expr; }
    bool same_definition_inputs(const TypeEvidenceState &other) const {
      return bindings == other.bindings && symbol_shadows == other.symbol_shadows && views == other.views &&
          owner_bindings == other.owner_bindings && owner_relations == other.owner_relations;
    }
    struct Returns {
      std::vector<Type> types;
      bool value_obligation = false;
      bool unresolved = false;
      bool saw_throw = false;
      bool unresolved_throw = false;
    } *returns = nullptr;
  };
  struct TypeBorrowedReturn {
    std::map<EvidenceSlot, ObservationPaths> targets;
    QuerySelections::Uninitialized uninitialized;
    const Expression *saved_selection = nullptr;
    bool operator==(const TypeBorrowedReturn &) const = default;

    static TypeBorrowedReturn capture(const TargetEvidence &target, const QuerySelections &flow) {
      TypeBorrowedReturn result;
      auto reaching = flow.outcome(flow.selection_outcomes(target), true);
      if (!reaching) return result;
      reaching.presence.clear();
      const bool initialized = flow.selection_initialized(target);
      for (const auto &place : target.places) {
        result.targets.emplace(place, reaching);
        if (initialized) continue;
        const auto missing = flow.uninitialized.find(place);
        if (missing == flow.uninitialized.end()) continue;
        auto possible = missing->second;
        possible.conjoin(reaching);
        if (possible) result.uninitialized.emplace(place, std::move(possible));
      }
      return result;
    }

    void merge(const TypeBorrowedReturn &other) {
      for (const auto &[place, reaching] : other.targets) targets[place].join(reaching);
      for (const auto &[place, missing] : other.uninitialized) uninitialized[place].join(missing);
      if (saved_selection != other.saved_selection) saved_selection = nullptr;
    }
    void initialize(const QuerySelections &flow) {
      if (saved_selection && flow.initialized_selections.contains(saved_selection)) {
        uninitialized.clear();
        saved_selection = nullptr;
      }
    }
    void restrict_to(const ObservationPaths &reaching) {
      for (auto *paths : {&targets, &uninitialized}) {
        for (auto &[place, condition] : *paths) condition.conjoin(reaching);
        std::erase_if(*paths, [](const auto &entry) { return !entry.second; });
      }
      if (uninitialized.empty()) saved_selection = nullptr;
    }
    void forget_observations(size_t first) {
      for (auto *paths : {&targets, &uninitialized})
        for (auto &[place, condition] : *paths) condition.forget_observations(first);
    }
  };
  struct TypeReturned {
    TypeReturned() : boolean_revision(0) {}
    std::optional<PresenceBranches> boolean;
    size_t boolean_revision;
    // An empty bound denotes a null result. An unknown bound grants no narrowing.
    std::optional<TypeBorrowedReturn> borrowed;
    const Expression *saved_selection() const { return borrowed ? borrowed->saved_selection : nullptr; }
    bool operator==(const TypeReturned &other) const { return boolean == other.boolean && borrowed == other.borrowed; }
    void merge(const TypeReturned &other) {
      boolean = QuerySelections::alternatives(std::move(boolean), other.boolean);
      if (borrowed && other.borrowed) borrowed->merge(*other.borrowed);
      else borrowed.reset();
    }
    void forget_observations(size_t first) {
      if (boolean) boolean->forget_observations(first);
      if (borrowed) borrowed->forget_observations(first);
    }
    void advance_boolean(const QuerySelections &selections) {
      if (boolean) boolean->forget([&](const auto &slot) { return selections.changed_since(slot, boolean_revision); });
      boolean_revision = selections.writes;
    }
  };
  struct TypeEvidenceExits {
    struct Path {
      std::set<size_t> reasons;
      TypeEvidenceState state;
      TypeReturned result;
      bool retained_return = false;

      bool returned() const { return retained_return || reasons.contains(1); }
    };
    // Outcomes of one operation begin with the same source evidence.
    // A handler consumes only the matching reasons from each path.
    std::vector<Path> paths;
  };
  struct TypeDeferredFlow {
    struct Action {
      size_t earlier;
      const Expression *entry;
      std::unique_ptr<TypeEvidenceState> declaration;
      bool may_replace;
      TypeDeferredEffects effects;
    };
    struct Alternative {
      size_t queue;
      ObservationPaths condition;
      EvidencePlaces initialized_entry_targets;
      explicit Alternative(const TypeEvidenceState &state)
          : queue(state.deferred_queue), condition(state.selections.reach_condition()) {
        for (const auto &slot : state.selections.initialized_selections)
          if (std::holds_alternative<PreparedEntryTarget>(slot)) initialized_entry_targets.insert(slot);
      }
    };
    using Choice = std::pair<Alternative, Alternative>;
    struct Loop {
      size_t entry;
      size_t backedge = 0;
      size_t first_observation;
    };
    std::vector<std::variant<Action, Choice, Loop>> entries;

    size_t header(size_t entry, size_t first_observation) {
      entries.emplace_back(Loop{entry, 0, first_observation});
      return entries.size();
    }
    void backedge(size_t header, size_t body) {
      if (header != body) std::get<Loop>(entries.at(header - 1)).backedge = body;
    }

    size_t join(const TypeEvidenceState &left, const TypeEvidenceState &right) {
      if (!left.deferred_queue && !right.deferred_queue) return 0;
      Choice choice{Alternative(left), Alternative(right)};
      if (choice.first.queue == choice.second.queue &&
          choice.first.initialized_entry_targets == choice.second.initialized_entry_targets) return choice.first.queue;
      entries.emplace_back(std::move(choice));
      return entries.size();
    }
    size_t push(const TypeEvidenceState &state, const Expression *entry, bool may_replace, TypeDeferredEffects effects) {
      auto declaration = entry ? std::make_unique<TypeEvidenceState>(state) : nullptr;
      if (declaration) {
        declaration->exits = nullptr;
        declaration->deferred = nullptr;
      }
      entries.emplace_back(Action{state.deferred_queue, entry, std::move(declaration), may_replace, std::move(effects)});
      return entries.size();
    }
  };

  struct DefinitionContext {
    size_t method;
    std::shared_ptr<const TypeEvidenceState> declaration;
    struct CaptureReference {
      std::optional<size_t> binding;
      std::vector<size_t> methods;
      bool enclosing = false;
    };
    std::vector<CaptureReference> captures;
    std::vector<CheckedCaptureInput> environment;
    std::vector<Type> retained;
    size_t query = 0;
  };
  std::map<size_t, DefinitionContext> definitions_;

  static std::optional<bool> access_of(const CheckedOperation &view) {
    return view.type == Type::never ? std::nullopt : std::optional<bool>{view.read_only};
  }
  static Access access_of(const std::vector<CheckedOperation> &views) {
    Access access;
    for (const auto &view : views) access.push_back(access_of(view));
    return access;
  }
  static bool variadic(const CheckedMethod &method) {
    return !method.input_parameters().empty() && method.input_parameters().back().variadic;
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
  static bool admits_parameter_access(const CheckedMethod &method, size_t i, bool readonly) {
    const auto &parameter = parameter_at(method, i);
    if (swap_operation(method.operation)) return readonly == parameter.read_only;
    return !readonly || parameter.owner.empty() || parameter.read_only;
  }
  static bool admits_access(const CheckedMethod &method, const Access &access) {
    for (size_t i = 0; i < access.size(); ++i) {
      if (access[i] && !admits_parameter_access(method, i, *access[i])) return false;
    }
    return true;
  }
  enum class AccessCoverage { disjoint, partial, contained };
  static AccessCoverage access_coverage(const CheckedMethod &method, const CheckedMethod &declaration) {
    bool contained = true;
    for (size_t i = 0; i < method.parameters.size(); ++i) {
      bool overlaps = false;
      for (const bool readonly : {false, true}) {
        const auto admitted = admits_parameter_access(method, i, readonly);
        const auto covered = admits_parameter_access(declaration, i, readonly);
        overlaps = overlaps || (admitted && covered);
        contained = contained && (!admitted || covered);
      }
      if (!overlaps) return AccessCoverage::disjoint;
    }
    return contained ? AccessCoverage::contained : AccessCoverage::partial;
  }
  static bool same_access(const CheckedMethod &a, const CheckedMethod &b) {
    for (size_t i = 0; i < a.parameters.size(); ++i)
      if (a.input_parameters()[i].read_only != b.input_parameters()[i].read_only) return false;
    return true;
  }
  using Owners = std::vector<std::optional<EvidenceOwner>>;
  static std::optional<size_t> one_owner(const CheckedOperation &view) {
    return view.type != Type::never && view.owners.size() == 1 ? std::optional<size_t>{*view.owners.begin()} : std::nullopt;
  }
  EvidenceOwner checked_owner(EvidenceOwner owner) const {
    if (owner.kind == EvidenceOwner::Kind::scoped) return checked_scope_owners_.at(owner.scope);
    return owner;
  }
  static Owners owners_of(const std::vector<CheckedOperation> &views) {
    Owners owners;
    for (const auto &view : views) owners.push_back(one_owner(view));
    return owners;
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
  const QueryEnvironment &query_environment() const {
    return program_.query_environments[checking_instance_ ? checking_instance_->query_environment : 0];
  }
  OwnerMatch compare_owners(std::optional<EvidenceOwner> a, std::optional<EvidenceOwner> b, const TypeEvidenceState *state = nullptr) const {
    if (!a || !b) return OwnerMatch::unknown;
    if (a == b) return OwnerMatch::satisfied;
    if (state) {
      if (const auto found = state->owner_relations.find({*a, *b}); found != state->owner_relations.end()) return found->second;
      return state->external_owners.contains(*a) && state->external_owners.contains(*b) ? OwnerMatch::unknown : OwnerMatch::mismatch;
    }
    const auto &relations = query_environment().owners;
    const auto retained = [](const EvidenceOwner &owner) { return owner.kind == EvidenceOwner::Kind::retained; };
    const auto external = [&](const EvidenceOwner &owner) {
      return owner.kind == EvidenceOwner::Kind::checked && external_owner(owner.id);
    };
    if (retained(*a) && retained(*b)) return relations.at(a->id).at(b->id);
    if (retained(*a) || retained(*b)) {
      const auto [saved, other] = retained(*a) ? std::pair{*a, *b} : std::pair{*b, *a};
      if (!external(other)) return OwnerMatch::mismatch;
      const auto &scope_owners = query_environment().scope_owners;
      return scope_owners.empty() ? OwnerMatch::unknown : scope_owners.at(saved.id).at(null_target - 1 - other.id);
    }
    if (external(*a) && external(*b) && checking_method_ && checking_method_->scope_inputs)
      return checking_method_->scope_inputs->relations.at(null_target - 1 - a->id).at(null_target - 1 - b->id);
    // Distinct formal names admit equal actual owners; local scopes do not.
    return external(*a) && external(*b) ? OwnerMatch::unknown : OwnerMatch::mismatch;
  }
  OwnerMatch match_owners(const CheckedMethod &method, const Owners &owners, const TypeEvidenceState *state = nullptr) const {
    auto result = OwnerMatch::satisfied;
    const auto count = variadic(method) ? owners.size() : method.parameters.size();
    for (size_t i = 0; i < count; ++i) for (size_t j = 0; j < i; ++j) {
      if (!same_owner(method, i, j)) continue;
      const auto compared = i < owners.size() ? compare_owners(owners[i], owners[j], state) : OwnerMatch::unknown;
      if (compared == OwnerMatch::mismatch) return compared;
      if (compared == OwnerMatch::unknown) result = compared;
    }
    return result;
  }
  struct ArgumentEvidence { std::optional<Type> type; std::optional<bool> read_only; std::optional<EvidenceOwner> owner; };
  struct ArgumentPreparation {
    ArgumentMode mode = ArgumentMode::value;
    std::optional<Type> type;
  };
  struct NamedFamily {
    std::vector<size_t> methods;
    const CheckedInstance *context = nullptr;
    std::vector<Type> evidence_retained = {};
    const DefinitionContext *definition = nullptr;
    const TypeEvidenceState *declaration = nullptr;
    const std::vector<Type> &retained_types() const {
      return context ? context->retained_types : definition ? definition->retained : evidence_retained;
    }
  };
  bool lexical_self(const std::string &name, std::optional<size_t> method) const {
    if (!method) return false;
    const auto *source = program_.methods[*method].source;
    return lexical_methods_.contains(source) && source->name == name;
  }
  bool symbol_shadowed(const std::string &name, const TypeEvidenceState &state) const {
    return state.bindings.contains(name) || state.owner_bindings.contains(name) ||
        state.symbol_shadows.contains(name) || lexical_self(name, state.method) || active_here(name, &state);
  }
  size_t resolve_symbol(const std::string &name, const Location &at, bool local,
                        std::optional<size_t> method = {}) const {
    const ModuleLookup lookup(module_, method ? program_.methods[*method].module : module_);
    if (local || lexical_self(name, method)) fail(at, "nearest binding '" + name + "' is not a symbol");
    if (method && module_names().values.contains(name))
      implementation_gap(at, "module-local binding lookup and capture in callable bodies");
    const auto found = module_names().symbols.find(name);
    if (found == module_names().symbols.end()) fail(at, "unknown symbol '" + name + "'");
    return found->second;
  }
  static const Type &dynamic_procedure_type(const CheckedSymbol &symbol, const Location &at) {
    if (!symbol.type || symbol.type->kind != Type::procedure || symbol.type->pure_callable)
      fail(at, "dynamic invocation requires a procedure symbol");
    return *symbol.type;
  }
  static Type print_result(const Expression &expr, const std::optional<Type> &argument = {}) {
    if (expr.children.size() != 1) fail(expr.at, "print requires one scalar argument");
    if (argument == Type::never) return Type::never;
    if (argument && !scalar_type(*argument)) fail(expr.at, "print requires one primitive scalar argument");
    return Type::void_type;
  }
  NamedFamily named_family(const std::string &name, const TypeEvidenceState *state = nullptr) const {
    const ModuleLookup lookup(module_, state && state->method ? program_.methods[*state->method].module : module_);
    const ModuleLookup context(interpretation_, state && state->method ? program_.methods[*state->method].interpretation : interpretation_);
    if (state) {
      if (const auto found = state->definitions.find(name); found != state->definitions.end()) {
        const auto &definition = found->second;
        return {{definition.method}, nullptr, definition_retained(definition.method, *definition.declaration), nullptr,
                definition.declaration.get()};
      }
      if (state->bindings.contains(name)) return {};
      if (state->method) {
        const auto &method = program_.methods[*state->method];
        if (lexical_methods_.contains(method.source) && name == method.source->name)
          return {{*state->method}, nullptr, state->retained};
      }
      const auto found = module_names().callables.find(name);
      return found == module_names().callables.end() ? NamedFamily{} : NamedFamily{family_methods(found->second)};
    }
    if (const auto binding = lexical_.lookup(name)) {
      const auto found = definitions_.find(*binding);
      if (found == definitions_.end()) return {};
      return {{found->second.method}, nullptr, {}, &found->second, found->second.declaration.get()};
    }
    if (checking_method_ && lexical_methods_.contains(checking_method_->source) && name == checking_method_->source->name)
      return {{checking_instance_->method}, checking_instance_};
    const auto found = module_names().callables.find(name);
    return found == module_names().callables.end() ? NamedFamily{} : NamedFamily{family_methods(found->second)};
  }
  std::string_view operation_name(const Expression &expr, const TypeEvidenceState *state = nullptr) const {
    if (expr.intrinsic) return expr.text;
    if (state ? (state->bindings.contains(expr.text) || state->owner_bindings.contains(expr.text) ||
                 state->symbol_shadows.contains(expr.text)) : lexical_.lookup(expr.text).has_value()) return {};
    const auto method = state ? state->method : checking_instance_ ? std::optional<size_t>{checking_instance_->method} : std::nullopt;
    if (lexical_self(expr.text, method)) return program_.methods[*method].operation;
    const ModuleLookup lookup(module_, method ? program_.methods[*method].module : module_);
    const auto family = module_names().callables.find(expr.text);
    return family == module_names().callables.end() ? std::string_view{expr.text}
        : std::string_view{program_.families[family->second].operation};
  }
  [[noreturn]] void ambiguous_methods(const Location &at, const std::string &name, const std::vector<size_t> &methods) const {
    std::string message = "ambiguous methods for '" + name + "':";
    for (const auto method : methods) message += " " + location_text(program_.methods[method].source->at);
    fail(at, message);
  }
  ArgumentPreparation argument_preparation(const Expression &expr, std::string_view operation, const std::vector<Type> &prefix,
                                            const std::vector<ArgumentEvidence> &evidence, const Access &prefix_access, const Owners &prefix_owners, bool required = true, const TypeEvidenceState *state = nullptr) const {
    if (expr.text == "cause_at" && (expr.intrinsic || (!module_names().callables.contains(expr.text) && !has_declarations(expr.text))) && expr.children.size() == 2)
      return {ArgumentMode::value, prefix.empty() ? exception_type(0) : Type{Type::uint64}};
    if (expr.intrinsic) return {};
    std::vector<std::optional<Type>> arguments;
    Access access;
    Owners owners;
    for (const auto &item : evidence) { arguments.push_back(item.type); access.push_back(item.read_only); owners.push_back(item.owner); }
    for (size_t i = 0; i < prefix.size(); ++i) arguments[i] = prefix[i];
    for (size_t i = 0; i < prefix_access.size(); ++i) access[i] = prefix_access[i];
    for (size_t i = 0; i < prefix_owners.size(); ++i) owners[i] = prefix_owners[i];
    bool complete = true;
    for (size_t i = prefix.size(); i < evidence.size(); ++i)
      complete = complete && evidence[i].type.has_value() && evidence[i].type != Type::never;
    const auto family = named_family(expr.text, state);
    const auto candidates = method_candidates(family.methods, arguments, family.retained_types(), access, owners, state);
    auto applicable = candidates.methods;
    if (complete) {
      applicable = most_specific(applicable, access, owners, state, candidates.widening);
      if (required && applicable.size() > 1 && std::all_of(applicable.begin() + 1, applicable.end(), [&](size_t id) {
            const auto &first = program_.methods[applicable.front()], &other = program_.methods[id];
            return same_access(first, other) && implies_owners(first, other) && implies_owners(other, first);
          })) ambiguous_methods(expr.at, expr.text, applicable);
    }
    std::optional<ArgumentMode> mode;
    std::optional<Type> type;
    bool consistent_type = true;
    // Without complete type evidence, an eager operation may still use its value-taking builtin.
    if (eager_operation(operation) && !complete) mode = ArgumentMode::value;
    const auto accumulate = [&](const CheckedMethod &candidate, const Substitutions &substitutions) {
      const auto passing = argument_mode(parameter_at(candidate, prefix.size()));
      if (mode && *mode != passing) {
        if (!required) return false;
        implementation_gap(expr.children[prefix.size()]->at, "overloads with different argument preparation contracts without sufficient operand type evidence");
      }
      mode = passing;
      const auto expected = substitute(candidate.parameters[parameter_index(candidate, prefix.size())], candidate, substitutions, true);
      if (nested_binder(candidate, expected) || (type && *type != expected))
        consistent_type = false;
      else type = expected;
      return true;
    };
    for (const auto id : applicable)
      if (!accumulate(program_.methods[id], candidates.substitutions.at(id))) return {};
    // Queries and definite builtin calls can use independently declared input types.
    // Every applicable declaration remains an obligation, not an overload choice.
    if ((state || (eager_operation(operation) && family.methods.empty())) && applicable.empty()) {
      const ModuleLookup lookup(module_, state && state->method ? program_.methods[*state->method].module : module_);
      for (const auto &declaration : declarations(expr.text)) {
        if (!state && std::any_of(declaration.input_parameters().begin(), declaration.input_parameters().end(),
            [](const auto &parameter) { return argument_mode(parameter) != ArgumentMode::value; })) continue;
        const auto substitutions = match_evidence(declaration, arguments, {}, true);
        if (!substitutions || !admits_access(declaration, access) ||
            match_owners(declaration, owners, state) == OwnerMatch::mismatch) continue;
        if (!accumulate(declaration, *substitutions)) return {};
      }
    }
    return {mode.value_or(ArgumentMode::value), consistent_type ? type : std::nullopt};
  }

  LexicalState::Scope &declaration_scope(const std::string &name, const Location &at) {
    auto selected = lexical_.scopes.rbegin();
    while (!selected->declarations) {
      if (selected->bindings.contains(name)) fail(at, "duplicate declaration of catch parameter '" + name + "'");
      ++selected;
    }
    auto &scope = *selected;
    if (!checking_instance_ && !scope.id && module_names().symbols.contains(name) &&
        !program_.symbols[module_names().symbols.at(name)].intrinsic_print)
      implementation_gap(at, "same-scope value and symbol declarations");
    if (scope.bindings.contains(name)) fail(at, "duplicate declaration of '" + name + "' in one lexical scope");
    return scope;
  }
  void warn_shadow(const std::string &name, const Location &at) {
    if (!validating_) return;
    const bool outer = lexical_.lookup(name).has_value() ||
        (checking_method_ && (enclosing_names_[checking_method_->source].contains(name) ||
         lexical_self(name, checking_instance_->method) || module_names().values.contains(name))) ||
        module_names().callables.contains(name) || module_names().symbols.contains(name) ||
        module_names().exception_types.contains(name);
    if (outer && warned_shadow_.emplace(at.file, at.line, at.column, name).second)
      program_.warnings.push_back({Warning::shadow, at, "declaration shadows outer binding '" + name + "'"});
  }
  size_t new_selection_identity(std::set<size_t> sources = {null_target}) {
    const auto id = next_selection_identity_++;
    selection_sources_.emplace(id, std::move(sources));
    return id;
  }
  std::set<size_t> selection_sources(std::optional<size_t> identity) const {
    return identity ? selection_sources_.at(*identity) : std::set<size_t>{null_target};
  }
  size_t merge_selection_identities(std::optional<size_t> left, std::optional<size_t> right) {
    if (left && left == right) return *left;
    auto sources = selection_sources(left);
    const auto other = selection_sources(right); sources.insert(other.begin(), other.end());
    return new_selection_identity(std::move(sources));
  }
  static bool same_selected_reference(const CheckedOperation &left, const CheckedOperation &right) {
    return (left.selection_identity && left.selection_identity == right.selection_identity) ||
        (left.binding && left.binding == right.binding && !left.selection_versions.empty() &&
         left.selection_versions == right.selection_versions);
  }
  size_t bind(const std::string &name, const Type &type, const std::set<size_t> &targets,
              bool read_only, bool value_binding, bool fixed, const Location &at, std::optional<size_t> alias = {},
              bool source_declaration = true, LexicalState::InputFacts *callable_entry = nullptr) {
    auto &scope = declaration_scope(name, at);
    if (source_declaration) warn_shadow(name, at);
    const auto id = bindings_->size();
    bindings_->push_back({type, read_only, value_binding, fixed});
    bindings_->back().place_owners.insert(scope.id);
    bindings_->back().alias = alias;
    if (!alias) {
      lexical_.targets[id] = targets;
      lexical_.null_sources[id] = targets.contains(null_target) ? std::set<size_t>{null_target} : std::set<size_t>{};
      lexical_.owners[id] = target_owners(targets);
      lexical_.target_versions[id] = {next_target_version_++};
      lexical_.selection_identities[id] = new_selection_identity();
      const auto selection = selection_targets(targets, lexical_.null_sources.at(id));
      if (callable_entry) callable_entry->selections.assign(id, selection);
      else assign_selection(lexical_, id, selection);
    }
    if (!proving_) {
      possible_targets_.push_back(targets);
      possible_null_sources_.push_back({});
      target_dependencies_.push_back({});
    }
    scope.bindings.emplace(name, id);
    lexical_.bindings_available.insert(id);
    return id;
  }
  size_t declare(const std::string &name, const Type &type, bool read_only, bool value_binding, const Location &at,
                 bool source_declaration = true, LexicalState::InputFacts *callable_entry = nullptr) {
    const auto scope = declaration_scope(name, at).id;
    const auto id = storage_->size();
    storage_->push_back({name, type, scope, read_only});
    scope_storage_[scope].push_back(id);
    storage_->back().binding = bind(name, type, {id}, read_only, value_binding, false, at, {}, source_declaration, callable_entry);
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
    if (!targets.contains(null_target)) operation.null_sources.clear();
    else if (operation.null_sources.empty()) operation.null_sources.insert(null_target);
    operation.place = targets.size() == 1 && !targets.contains(null_target) ? std::optional<size_t>{*targets.begin()} : std::nullopt;
  }
  static CleanupEquations::Targets selection_targets(const std::set<size_t> &places, const std::set<size_t> &null_sources) {
    CleanupEquations::Targets result{places, {}, {}};
    if (result.places.erase(null_target)) {
      if (null_sources.empty()) result.null_sources.insert(null_target);
      else result.null_sources = null_sources;
    }
    return result;
  }
  CleanupEquations::Targets snapshot_selection(const CheckedOperation &source, const Location &at) {
    auto selected = selection_targets(source.targets, source.null_sources);
    if (proving_ && lexical_.live && selected.nullable() && (source.nullness || selected.places.size() > 1)) {
      auto proof = snapshot_boolean(null_predicate(source), lexical_, at);
      for (auto &[null, clauses] : proof) clauses = CleanupEquations::stable_input_conditions(clauses);
      if (proof != selected.null_observations()) {
        cleanup_.observe_reference(proof);
        selected.nullness = std::move(proof);
      }
    }
    return selected;
  }
  void assign_targets(size_t binding, const CheckedOperation &source, const CleanupEquations::Targets &update) {
    lexical_.targets[binding] = source.targets;
    lexical_.null_sources[binding] = source.null_sources;
    lexical_.owners[binding].insert(source.owners.begin(), source.owners.end());
    lexical_.target_versions[binding] = {next_target_version_++};
    lexical_.selection_identities[binding] = source.selection_identity ? *source.selection_identity : new_selection_identity();
    if (source.binding && !source.selection_versions.empty() && source.targets.contains(null_target) && source.targets.size() > 1) {
      CleanupEquations::ConditionFacts copies;
      copies.same_bindings.insert({{binding, lexical_.target_versions.at(binding)}, {*source.binding, source.selection_versions}});
      lexical_.predicate_conditions = CleanupEquations::conjoin_conditions(lexical_.predicate_conditions, {copies});
    }
    if (source.nullness && source.targets.contains(null_target) && source.targets.size() > 1) {
      CleanupEquations::ConditionClauses alternatives;
      for (const auto &[null, clauses] : *source.nullness) for (auto facts : clauses) {
        (null ? facts.absent_bindings : facts.bindings).emplace(binding, lexical_.target_versions.at(binding));
        CleanupEquations::add_condition(alternatives, std::move(facts));
      }
      lexical_.predicate_conditions = CleanupEquations::conjoin_conditions(lexical_.predicate_conditions, alternatives);
    }
    assign_selection(lexical_, binding, update);
    assign_initialization(lexical_, binding, source.initialization);
    if (!proving_) {
      possible_targets_.at(binding).insert(source.targets.begin(), source.targets.end());
      possible_null_sources_.at(binding).insert(source.null_sources.begin(), source.null_sources.end());
      target_dependencies_.at(binding).insert(source.origins.begin(), source.origins.end());
    }
  }
  void require_live_view(const CheckedOperation &view, const Location &at) const {
    if (lexical_.live) for (const auto owner : view.owners)
      if (!owner_alive(owner)) fail(at, "storage view's lexical owner has ended");
  }
  std::set<size_t> physical_slots(const std::set<size_t> &slots) const {
    std::set<size_t> result;
    for (const auto id : slots) {
      const auto &selection = bindings_->at(id).selection;
      if (selection.empty()) result.insert(id);
      else result.insert(selection.begin(), selection.end());
    }
    return result;
  }
  std::set<size_t> overlapping_slots(const std::set<size_t> &slots) const {
    auto result = physical_slots(slots);
    const auto physical = result;
    for (const auto id : physical) {
      const auto &overlaps = bindings_->at(id).overlaps;
      result.insert(overlaps.begin(), overlaps.end());
    }
    return result;
  }
  void require_slots_live(const std::set<size_t> &slots, const Location &at, const std::string &message) const {
    for (const auto id : physical_slots(slots))
      if (!lexical_.bindings_available.contains(id)) fail(at, message);
  }
  static void reference_slot(bool value_binding, bool fixed, const Location &at) {
    if (value_binding) fail(at, "val storage has no reference binding; use '<-' for bits transfer");
    if (fixed) fail(at, "cannot rebind a const reference binding");
  }
  static void reference_destination(const ViewEvidence &view, const Location &at) {
    if (!view.value_binding) fail(at, "reference operation requires a binding slot");
    reference_slot(*view.value_binding, view.fixed_binding.value_or(false), at);
    if (!view.fixed_binding) implementation_gap(at, "reference binding mutability is not established");
  }
  static void rebind_type(const Type &target, const Type &source, const Location &at) {
    if (source != Type::null_ref && source != target) fail(at, "reference rebinding requires the exact target type");
  }
  static void rebind_access(bool target_read_only, bool source_read_only, const Location &at) {
    if (!target_read_only && source_read_only) fail(at, "readonly source cannot rebind a writable reference target");
  }
  static void bits_swap_target(const Type &type, bool read_only, const Location &at) {
    if (!bits_type(type)) fail(at, "builtin bits swap requires two bits storage destinations");
    if (read_only) fail(at, "cannot swap bits through a readonly destination");
  }
  static void assignment_destination(bool view, bool mixed, const Location &at) {
    if (!view) fail(at, "assignment requires a storage destination");
    if (mixed) implementation_gap(at, "assignment through a mixed value/storage view");
  }
  static void transfer_access(bool read_only, const Location &at) {
    if (read_only) fail(at, "cannot write through a readonly destination");
  }
  static void transfer_type(const Type &source, const Type &destination, const Location &at) {
    if (!transfers_value(source, destination)) fail(at, "bits transfer source does not satisfy destination type");
  }
  static void require_value(const Type &type, const Location &at) {
    if (!lexical_value_type(type)) implementation_gap(at, "non-value or abruptly completing operand");
  }
  static void check_initializer_type(const Expression &group, const Expression &declaration,
      const Type &initial, const Type &type, std::optional<bool> borrowed) {
    using K = Expression::Kind;
    if (type == Type::null_ref) fail(declaration.at, "a null initializer requires an explicit target type");
    if (initial == Type::never && !declaration.annotation && declaration.children.front()->kind != K::make_local) return;
    require_value(type, group.at);
    const auto &kind = group.declaration_kind;
    const bool callable = kind == "func" || kind == "proc";
    if (callable && (type.kind != Type::procedure || type.pure_callable != (kind == "func")))
      fail(declaration.at, "callable declaration requires a matching func/proc value");
    if (type.kind == Type::procedure && !callable && kind != "ptr")
      implementation_gap(declaration.at, "callable storage uses func/proc values or ptr views");
    if (initial == Type::never) return;
    const bool value_matches = borrowed != true && transfers_value(initial, type);
    const bool view_matches = borrowed != false && (initial == Type::null_ref || initial == type);
    if (!value_matches && !view_matches) fail(declaration.at, "initializer does not satisfy declared " + kind + " type");
  }
  static void check_initializer_access(const Expression &declaration, bool source_read_only) {
    if (declaration.annotation && !declaration.read_only && source_read_only)
      fail(declaration.at, "readonly source cannot initialize a writable reference target");
  }
  static void check_initializer_owner(std::optional<bool> borrowed, bool mixed, bool mismatch, const Location &at) {
    if (borrowed == false || mixed) implementation_gap(at, "fresh storage with an explicit external owner");
    if (mismatch) fail(at, "reference initializer does not belong to the declared owner");
  }
  static void check_local_initializer(const Type &initial, const Type &type, const Location &at) {
    if (initial != Type::never && !transfers_bits(initial, type)) fail(at, "initializer does not satisfy local storage type");
  }
  static Type range_index_type(const Type &first, const Type &last, const Location &at) {
    if ((first != Type::never && !integral(first)) || (last != Type::never && !integral(last)))
      fail(at, "a range requires integer endpoints");
    if (first == Type::never && last == Type::never)
      implementation_gap(at, "range index type is not established by an endpoint");
    if (first != Type::never && last != Type::never && first != last)
      implementation_gap(at, "range endpoints with different integer types");
    return first == Type::never ? last : first;
  }
  struct RangeReachability {
    bool body;
    bool empty;
    bool repeat;
  };
  static RangeReachability range_reachability(bool completes, const std::optional<Type> &index,
      std::optional<std::uint64_t> first, std::optional<std::uint64_t> last) {
    if (!completes) return {};
    if (!index || !first || !last) return {true, true, true};
    const bool enters = index == Type::integer ? std::bit_cast<std::int64_t>(*first) < std::bit_cast<std::int64_t>(*last) : *first < *last;
    return {enters, !enters, enters && *last - *first > 1};
  }
  static void check_fold_body(const Type &body, const Type &accumulator, const Location &at) {
    if (body != Type::never && body != accumulator && !widens(body, accumulator))
      fail(at, "fold body does not satisfy the accumulator type");
  }
  static void check_catch_category(const Type &type, std::set<size_t> &handled, const Location &at) {
    if (type.kind != Type::exception) fail(at, "catch requires a selected intrinsic exception type");
    if (!handled.insert(type.identity).second) fail(at, "duplicate exception category in catch clauses");
  }
  static void check_guarded_result(const Type &body, const Type &handler, const Location &at) {
    if (body != Type::never && handler != Type::never && body != handler)
      fail(at, "try body and catch arms must have the same result type");
  }
  void reference_destination(const CheckedOperation &view, const Location &at) const {
    if (view.slots.empty()) fail(at, "reference operation requires a binding slot");
    require_slots_live(view.slots, at, "reference binding's lexical scope has ended");
    for (const auto id : physical_slots(view.slots)) {
      const auto &binding = bindings_->at(id);
      reference_slot(binding.value_binding, view.fixed_binding || binding.fixed, at);
    }
  }
  void require_deferred_slots(const CheckedOperation &view, const Location &at) const {
    const auto cleanup = std::find_if(lexical_.scopes.begin(), lexical_.scopes.end(), [&](const auto &entry) {
      return entry.id == cleanup_scope_;
    });
    for (const auto id : physical_slots(view.slots)) for (const auto owner : bindings_->at(id).place_owners) {
      if (external_owner(owner)) continue;
      const auto scope = std::find_if(lexical_.scopes.begin(), lexical_.scopes.end(), [&](const auto &entry) {
        return entry.id == owner;
      });
      if (scope > cleanup) fail(at, "deferred reference binding does not outlive its cleanup scope");
    }
  }
  void check_rebind(const CheckedOperation &target, const CheckedOperation &source, const Location &at) {
    reference_destination(target, at);
    if (!source.view || source.mixed_view) implementation_gap(at, "rebinding from a fresh value without an existing target");
    rebind_type(target.type, source.type, at);
    require_live_view(source, at);
    const auto physical = physical_slots(target.slots);
    for (const auto id : overlapping_slots(target.slots)) {
      const auto &binding = bindings_->at(id);
      rebind_access(binding.read_only, source.read_only, at);
      auto owners = lexical_.owners.at(id);
      owners.insert(source.owners.begin(), source.owners.end());
      if (physical.contains(id) && owners.size() > 1) fail(at, "reference rebinding cannot change the initializer-fixed owner");
      if (binding.requires_initialized && proving_ && validating_ && lexical_.live && source.targets.contains(null_target))
        fail(at, "ref requires a proven non-null target");
      if (binding.requires_initialized) require_initialized(source, at, "ref target");
    }
  }
  void record_rebinding(size_t binding, bool deferred = false) {
    rebound_.insert(binding);
    if (!deferred) for (const auto *loop : active_loops_) loop_states_.at(loop).rebound.insert(binding);
  }
  void rebind(const CheckedOperation &target, const CheckedOperation &source,
              const CleanupEquations::Targets &snapshot, bool deferred = false) {
    const auto written = overlapping_slots(target.slots);
    std::vector<size_t> affected;
    for (const auto &[id, targets] : lexical_.targets)
      if (!intersect(written, physical_slots({id})).empty()) affected.push_back(id);
    // A definite local rebind inside a delayed body can retain the selected
    // entry target in another slot. Prepared rebinds already carry a snapshot.
    const auto entry = delayed_ && !deferred && affected.size() == 1 ? current_binding(source) : std::nullopt;
    for (const auto id : affected) {
      auto selected = source;
      auto update = entry ? CleanupEquations::Targets{{}, {}, {*entry}} : snapshot;
      const bool weak = target.slots.size() != 1 || !target.slots.contains(id);
      if (weak) {
        update.entries.insert(id);
        selected.selection_identity = merge_selection_identities(source.selection_identity, lexical_.selection_identities.at(id));
        selected.binding.reset();
        selected.nullness.reset();
        const auto &previous = lexical_.targets.at(id);
        selected.targets.insert(previous.begin(), previous.end());
        const auto &nulls = lexical_.null_sources.at(id);
        selected.null_sources.insert(nulls.begin(), nulls.end());
        selected.origins.insert(id);
        selected.constrain_initialization(selection_initialization(lexical_, id));
      }
      assign_targets(id, selected, update);
      if (!proving_) record_rebinding(id, deferred);
    }
  }
  CheckedOperation current_selection(CheckedOperation view, const Location &at) const {
    reference_destination(view, at);
    return selection_state(std::move(view), at);
  }
  size_t read_obligation(std::optional<size_t> place, CallEffects::ReadSite site) {
    read_sites_.push_back(std::move(site));
    return cleanup_.read(place);
  }
  CleanupEquations::Reads prepared_binding_reads(const CheckedOperation &view, const Location &at,
      bool nonnull = false, const std::set<size_t> &initialized = {}) {
    CleanupEquations::Reads reads;
    if (!proving_) return reads;
    const auto incoming = physical_slots(view.slots);
    if (nonnull) {
      const auto read = read_obligation(null_target, {at, {}});
      auto &requirement = reads[read];
      requirement.nonnull = incoming;
      std::erase_if(*requirement.nonnull, [&](size_t id) { return !possible_targets_.at(id).contains(null_target); });
      // A source version distinguishes iterations only for invariant slots.
      for (const auto &[id, versions] : lexical_.target_versions)
        if (!lexical_.targets.at(id).contains(null_target) &&
            std::none_of(active_loops_.begin(), active_loops_.end(), [&](const auto *loop) {
              const auto &state = loop_states_.at(loop);
              return state.first > cleanup_scope_ && state.rebound.contains(id);
            })) requirement.nonnull_versions.emplace(id, versions);
    }
    for (const auto id : incoming) {
      if (initialized.contains(id)) continue;
      for (const auto place : possible_targets_.at(id)) if (place != null_target) {
        const auto read = read_obligation(place, {at, {}});
        auto &requirement = reads[read];
        requirement.selection = view.binding ? view.binding : std::optional<size_t>{id};
        requirement.entry_initialized = initialized;
        requirement.entry_nonnull = initialized;
      }
    }
    return reads;
  }
  void prepare_reference_swap(const CheckedOperation &left, const CheckedOperation &right,
                              CheckedOperation &operation, const Location &at) {
    require_deferred_slots(left, at); require_deferred_slots(right, at);
    auto &effects = operation.invocation[0];
    if (left.slots.size() == 1 && left.slots == right.slots) return;
    const auto exchange = [&](const CheckedOperation &target, const CheckedOperation &source) {
      const auto written = overlapping_slots(target.slots);
      const auto incoming = physical_slots(source.slots);
      for (const auto &[id, current] : lexical_.targets) {
        if (intersect(written, physical_slots({id})).empty()) continue;
        CleanupEquations::Targets update{{}, {}, incoming};
        if (target.slots.size() != 1 || !target.slots.contains(id)) update.entries.insert(id);
        effects.selections.targets[id].merge(update);
        effects.selections.invalidated.insert(id);
        if (!proving_) {
          possible_targets_.at(id).insert(source.targets.begin(), source.targets.end());
          possible_null_sources_.at(id).insert(source.null_sources.begin(), source.null_sources.end());
          target_dependencies_.at(id).insert(incoming.begin(), incoming.end());
          record_rebinding(id, true);
        }
      }
      if (!proving_ || std::none_of(written.begin(), written.end(), [&](size_t id) { return bindings_->at(id).requires_initialized; })) return;
      const auto reads = prepared_binding_reads(source, at, true);
      effects.reads.insert(reads.begin(), reads.end());
    };
    exchange(left, right); exchange(right, left);
  }
  CheckedOperation selection_state(CheckedOperation view, const Location &at) const {
    require_slots_live(view.slots, at, "aliased binding's lexical scope has ended");
    view.targets.clear(); view.null_sources.clear(); view.owners.clear(); view.selection_versions.clear();
    view.selection_identity.reset();
    std::set<size_t> identities;
    view.origins = view.slots;
    view.initialization = CleanupEquations::Initialization{};
    for (const auto id : view.slots) {
      identities.insert(lexical_.selection_identities.at(id));
      if (lexical_.live && lexical_.expired_bindings.contains(id))
        fail(at, "reference binding's target owner has ended or may expire across a loop backedge");
      const auto &targets = lexical_.targets.at(id), &nulls = lexical_.null_sources.at(id);
      const auto &owners = lexical_.owners.at(id), &versions = lexical_.target_versions.at(id);
      view.targets.insert(targets.begin(), targets.end());
      view.null_sources.insert(nulls.begin(), nulls.end());
      view.owners.insert(owners.begin(), owners.end());
      view.selection_versions.insert(versions.begin(), versions.end());
      view.constrain_initialization(selection_initialization(lexical_, id));
    }
    if (identities.size() == 1) view.selection_identity = *identities.begin();
    select_targets(view, view.targets);
    require_live_view(view, at);
    return view;
  }
  bool mutable_payload_target(size_t place) const {
    return storage(place).borrowed || !storage(place).read_only;
  }
  static ExceptionFacts stored_exceptions(size_t place, const Type &type,
                                          const std::map<size_t, ExceptionFacts> &values) {
    const auto found = values.find(place);
    return found == values.end() ? exception_possibilities(type) : found->second;
  }
  void assign_exceptions(size_t place, const ExceptionFacts &value) {
    for (const auto input : lexical_.inputs) lexical_.input_facts[input].exceptions[place] = value;
    refresh_input_facts(lexical_);
  }
  std::map<size_t, ExceptionFacts> incoming_payloads(std::optional<size_t> action = {}) const {
    auto values = lexical_.exceptions;
    const auto seed = [&](size_t place) {
      const auto &target = storage(place);
      if (target.type.kind == Type::exception && mutable_payload_target(place))
        values[place] = ExceptionFacts::payload(place, action).restricted(exception_possibilities(target.type));
    };
    for (const auto place : lexical_.available) seed(place);
    if (proving_) for (size_t place = 0; place < catalog_storage_.size(); ++place)
      if (std::any_of(lexical_.scopes.begin(), lexical_.scopes.end(), [&](const auto &scope) {
            return scope.id == catalog_storage_[place].scope;
          })) seed(place);
    return values;
  }
  static std::map<size_t, ExceptionFacts> payload_changes(const std::map<size_t, ExceptionFacts> &before,
                                                        const std::map<size_t, ExceptionFacts> &after) {
    std::map<size_t, ExceptionFacts> changed;
    for (const auto &[target, value] : after)
      if (!before.contains(target) || before.at(target) != value) changed.emplace(target, value);
    return changed;
  }
  std::optional<SelectionCondition> deferred_selection_condition(size_t binding, size_t target, bool equal) const {
    if (!deferred_action_) return {};
    if (const auto identity = lexical_.selection_identities.find(binding); identity != lexical_.selection_identities.end())
      if (const auto action = return_selection_actions_.find(identity->second); action != return_selection_actions_.end())
        return SelectionCondition{action->second, CleanupEquations::pending_return_selection, target, equal};
    const auto selected = lexical_.selections.substitute({{}, {}, {binding}});
    if (selected) if (const auto entry = selected->single_entry())
      return SelectionCondition{deferred_action_, *entry, target, equal};
    return {};
  }
  void record_return_readiness(LexicalState &state, size_t binding) const {
    const auto identity = state.selection_identities.find(binding);
    if (identity == state.selection_identities.end()) return;
    const auto action = return_selection_actions_.find(identity->second);
    if (action == return_selection_actions_.end() || action->second != deferred_action_) return;
    for (const auto input : state.inputs)
      state.input_facts.at(input).selections.entry_initialized.insert(CleanupEquations::pending_return_selection);
  }
  std::optional<SelectionCondition> selected_payload_condition(const CheckedOperation &view, size_t place) const {
    if (!deferred_action_ || !view.targets.contains(place) || view.place) return {};
    const auto binding = current_binding(view);
    return binding ? deferred_selection_condition(*binding, place, true) : std::nullopt;
  }
  template <typename Value, typename Merge, typename Guard> Value updated_payload(const CheckedOperation &target,
      size_t place, const Value &before, const Value &value, const Merge &merge, const Guard &guard) const {
    if (target.place == place) return value;
    // An unselected alternative leaves this payload intact when it cannot
    // reach the same storage through another input.
    if (const auto selected = selected_payload_condition(target, place); selected &&
        std::ranges::none_of(target.targets, [&](size_t other) {
          return other != place && payload_targets_may_share(other, place);
        })) {
      auto inverse = *selected; inverse.equal = false;
      auto result = guard(before, SelectionConditions{{inverse}});
      merge(result, guard(value, SelectionConditions{{*selected}}));
      return result;
    }
    auto result = before; merge(result, value); return result;
  }
  ExceptionFacts stored_exceptions(const CheckedOperation &view, const std::map<size_t, ExceptionFacts> &values) const {
    ExceptionFacts result;
    if (view.type.kind != Type::exception) return result;
    for (const auto place : view.targets) if (place != null_target) {
      auto categories = stored_exceptions(place, view.type, values);
      if (const auto selected = selected_payload_condition(view, place)) categories = categories.guarded({{*selected}});
      result.merge(categories);
    }
    return result;
  }
  ExceptionFacts stored_exceptions(const CheckedOperation &view) const {
    return stored_exceptions(view, lexical_.exceptions);
  }
  bool payload_targets_may_share(size_t left, size_t right) const {
    if (left == null_target || right == null_target) return false;
    if (left == right) return true;
    const auto &a = storage(left), &b = storage(right);
    if (!a.borrowed || !b.borrowed || a.type != b.type ||
        compare_owners(a.scope, b.scope) == OwnerMatch::mismatch) return false;
    return !checking_method_ || !checking_method_->scope_inputs ||
        !checking_method_->scope_inputs->disjoint_input_targets.contains(
            std::minmax(input_ordinal(left), input_ordinal(right)));
  }
  void record_payload_write(const CheckedOperation &target, bool deferred = false) {
    if (proving_) return;
    // A write can preserve the first iteration's value yet change a later one.
    // Record its destination independently of the computed postcondition.
    payload_writes_.include(target);
    if (!deferred) for (const auto *expr : active_loops_) loop_states_.at(expr).writes.include(target);
  }
  void write_exception_payloads(std::map<size_t, ExceptionFacts> &values,
                                const CheckedOperation &target, const ExceptionFacts &categories) {
    if (target.type.kind != Type::exception) return;
    for (const auto place : payload_targets(target)) values[place] = updated_payload(target, place,
        stored_exceptions(place, target.type, values), categories,
        [](auto &a, const auto &b) { a.merge(b); }, [](const auto &value, const auto &conditions) { return value.guarded(conditions); });
  }
  void write_exceptions(const CheckedOperation &target, const ExceptionFacts &categories) {
    for (const auto input : lexical_.inputs)
      write_exception_payloads(lexical_.input_facts[input].exceptions, target, categories);
    refresh_input_facts(lexical_);
  }
  std::set<size_t> payload_targets(const CheckedOperation &target) const {
    auto written = target.targets;
    written.erase(null_target);
    for (const auto place : lexical_.available)
      if (std::any_of(target.targets.begin(), target.targets.end(), [&](size_t other) {
            return payload_targets_may_share(other, place);
          })) written.insert(place);
    return written;
  }
  static void invalidate_payloads(LexicalState &state, const std::set<size_t> &written) {
    state.payload_writes.insert(written.begin(), written.end());
    for (const auto place : written) state.predicates.erase(place);
  }
  std::shared_ptr<const CheckedOperation::Predicate> stored_predicate(const CheckedOperation &view) {
    if (view.type != Type::boolean) return {};
    if (view.place) {
      auto &proof = lexical_.predicates[*view.place];
      if (!boolean_decision(proof.get())) {
        // A definite place supplies one Boolean until the next possible write.
        // The proof follows value copies even after that place is overwritten.
        auto observed = proof ? std::make_shared<Predicate>(*proof) :
                                std::make_shared<Predicate>(Predicate{Predicate::Kind::presence, {}, {}, {}});
        observed->storage_observation = next_boolean_observation_++;
        proof = std::move(observed);
      }
      return proof;
    }
    std::shared_ptr<const Predicate> result;
    for (const auto place : view.targets) if (place != null_target) {
      const auto found = lexical_.predicates.find(place);
      if (found == lexical_.predicates.end()) return {};
      auto proof = found->second;
      if (const auto selected = selected_payload_condition(view, place)) proof = guard_predicate(proof, {{*selected}});
      result = result ? predicate(Predicate::Kind::alternatives, result, proof) : proof;
    }
    return result;
  }
  std::shared_ptr<const CheckedOperation::Predicate> projected_predicate(const CheckedOperation &view) {
    // Cleanup can change the selected target after the operand produced its
    // view. A mixed result may also contain bits selected before cleanup.
    return view.mixed_view ? nullptr : stored_predicate(view);
  }
  static std::shared_ptr<const CheckedOperation::Predicate> boolean_predicate(const CleanupEquations::BooleanValue &value) {
    auto proof = std::make_shared<CheckedOperation::Predicate>();
    proof->kind = CheckedOperation::Predicate::Kind::presence;
    proof->results = value;
    proof->complete_results = true;
    return proof;
  }
  static std::shared_ptr<const CheckedOperation::Predicate> guard_predicate(const std::shared_ptr<const CheckedOperation::Predicate> &proof,
      const SelectionConditions &conditions) {
    CleanupEquations::ConditionFacts facts; facts.conditions = conditions;
    CleanupEquations::ConditionClauses clauses; CleanupEquations::add_condition(clauses, std::move(facts));
    return constrain_predicate(proof, clauses);
  }
  static std::shared_ptr<const CheckedOperation::Predicate> constrain_predicate(
      const std::shared_ptr<const CheckedOperation::Predicate> &proof, const CleanupEquations::BooleanValue &conditions) {
    auto result = proof ? std::make_shared<Predicate>(*proof) : std::make_shared<Predicate>();
    if (!proof) result->kind = Predicate::Kind::presence;
    for (const bool truth : {false, true}) {
      auto &clauses = result->results[truth];
      if (clauses.empty() && !result->complete_results) clauses.insert(CleanupEquations::ConditionFacts{});
      const auto known = conditions.find(truth);
      clauses = known == conditions.end() ? CleanupEquations::ConditionClauses{} :
          CleanupEquations::conjoin_conditions(clauses, known->second);
    }
    result->complete_results = true;
    return result;
  }
  static std::shared_ptr<const CheckedOperation::Predicate> constrain_predicate(
      const std::shared_ptr<const CheckedOperation::Predicate> &proof, const CleanupEquations::ConditionClauses &conditions) {
    if (conditions == CleanupEquations::ConditionClauses{CleanupEquations::ConditionFacts{}}) return proof;
    return constrain_predicate(proof, CleanupEquations::BooleanValue{{false, conditions}, {true, conditions}});
  }
  CleanupEquations::BooleanValue snapshot_boolean(const std::shared_ptr<const CheckedOperation::Predicate> &proof,
      const LexicalState &state, const Location &at) {
    const auto before = lexical_, input = state;
    CleanupEquations::BooleanValue value;
    for (const bool result : {false, true}) {
      lexical_ = input;
      predicate_paths(proof, result, at, [&] {
        for (auto facts : path_conditions(lexical_)) {
          for (const auto &[binding, targets] : lexical_.targets) if (!targets.empty()) {
            if (!targets.contains(null_target)) facts.bindings.emplace(binding, lexical_.target_versions.at(binding));
            else if (targets.size() == 1) facts.absent_bindings.emplace(binding, lexical_.target_versions.at(binding));
          }
          CleanupEquations::add_condition(value[result], std::move(facts));
        }
      });
    }
    lexical_ = before;
    return value;
  }
  void merge_nullness(CheckedOperation &result, const CheckedOperation &source,
                      const LexicalState &state, const Location &at) {
    if (!result.view || result.mixed_view || !result.targets.contains(null_target) || result.targets.size() == 1) return;
    if (!result.nullness) result.nullness.emplace();
    CleanupEquations::merge_boolean(*result.nullness, snapshot_boolean(null_predicate(source), state, at));
  }
  CleanupEquations::BooleanValues current_booleans(const LexicalState &state, const Location &at) {
    const auto input = state;
    CleanupEquations::BooleanValues values;
    for (const auto place : input.available) if (storage(place).type == Type::boolean) {
      const auto found = input.predicates.find(place);
      values[place] = snapshot_boolean(found == input.predicates.end() ? nullptr : found->second, input, at);
    }
    return values;
  }
  CleanupEquations::BooleanValues incoming_booleans(std::optional<size_t> action = {}) const {
    CleanupEquations::BooleanValues values;
    const auto seed = [&](size_t place) {
      if (storage(place).type == Type::boolean) values[place] = CleanupEquations::boolean_payload(place, action);
    };
    for (const auto place : lexical_.available) seed(place);
    if (proving_) for (size_t place = 0; place < catalog_storage_.size(); ++place)
      if (std::any_of(lexical_.scopes.begin(), lexical_.scopes.end(), [&](const auto &scope) {
            return scope.id == catalog_storage_[place].scope;
          })) seed(place);
    return values;
  }
  CleanupEquations::BooleanValue stored_boolean(const CheckedOperation &view,
      const CleanupEquations::BooleanValues &values) const {
    CleanupEquations::BooleanValue result;
    for (const auto place : view.targets) if (place != null_target) {
      const auto found = values.find(place);
      auto value = found == values.end() ? CleanupEquations::unknown_boolean() : found->second;
      if (const auto selected = selected_payload_condition(view, place)) value = CleanupEquations::guard_boolean(value, {{*selected}});
      CleanupEquations::merge_boolean(result, value);
    }
    return result;
  }
  void write_boolean_payloads(CleanupEquations::BooleanValues &values, const CheckedOperation &target,
      const CleanupEquations::BooleanValue &value) const {
    if (target.type != Type::boolean) return;
    for (const auto place : payload_targets(target)) {
      const auto before = values.find(place);
      values[place] = updated_payload(target, place, before == values.end() ? CleanupEquations::unknown_boolean() : before->second,
          value, CleanupEquations::merge_boolean, CleanupEquations::guard_boolean);
    }
  }
  CleanupEquations::ConditionClauses instantiate_conditions(const CleanupEquations::ConditionClauses &alternatives,
      const std::vector<CleanupEquations::BooleanValue> &inputs, const std::vector<CheckedOperation> &views,
      bool slots, const std::vector<ExceptionFacts> &exceptions = {}) const {
    CleanupEquations::ConditionClauses result;
    for (const auto &known : alternatives) {
      CleanupEquations::ConditionFacts facts;
      std::set<size_t> observed_selections;
      if (!slots && std::ranges::any_of(known.present, [&](size_t parameter) {
            const auto &targets = views.at(parameter).targets;
            return targets.size() == 1 && targets.contains(null_target);
          })) continue;
      for (const auto parameter : known.present) {
        const auto &view = views.at(parameter);
        const auto present = selected_input(view.targets);
        facts.present.insert(present.begin(), present.end());
        if (!slots && view.binding) facts.bindings.emplace(*view.binding, view.selection_versions);
      }
      for (const auto &value : known.exceptions) facts.exceptions.insert(value.substitute(exceptions, true));
      if (!slots) for (const auto &group : known.same_inputs) {
        std::set<size_t> selections;
        for (const auto parameter : group)
          if (const auto identity = views.at(parameter).selection_identity) {
            selections.insert(*identity); observed_selections.insert(parameter);
          }
        if (selections.size() > 1) facts.same_selections.insert(std::move(selections));
      }
      if (!slots) for (const auto &[left, right] : known.different_inputs) {
        const auto lhs = views.at(left).selection_identity, rhs = views.at(right).selection_identity;
        if (lhs && rhs) {
          facts.different_selections.emplace(*lhs, *rhs);
          observed_selections.insert(left); observed_selections.insert(right);
        }
      }
      // Caller-slot invocations do not provide a stable selection snapshot here.
      if (!slots) for (const auto parameter : known.absent) {
        const auto &view = views.at(parameter);
        if (view.binding) facts.absent_bindings.emplace(*view.binding, view.selection_versions);
      }
      CleanupEquations::ConditionClauses clauses;
      CleanupEquations::add_condition(clauses, std::move(facts));
      for (const auto parameter : observed_selections)
        clauses = CleanupEquations::conjoin_conditions(clauses, selection_bounds(views.at(parameter)));
      if (!slots) for (const bool null : {false, true})
        for (const auto parameter : null ? known.absent : known.present) if (views.at(parameter).nullness) {
          const auto &value = *views[parameter].nullness;
          const auto found = value.find(null);
          clauses = found == value.end() ? CleanupEquations::ConditionClauses{} :
              CleanupEquations::conjoin_conditions(clauses, found->second);
        }
      if (!slots) for (const auto parameter : known.absent)
        clauses = CleanupEquations::conjoin_conditions(clauses, absence_conditions(views.at(parameter).null_sources));
      for (const auto &[parameter, input] : known.inputs)
        if (const auto found = inputs.at(parameter).find(input); found != inputs.at(parameter).end())
          clauses = CleanupEquations::conjoin_conditions(clauses, found->second);
        else clauses.clear();
      for (const auto &clause : clauses) CleanupEquations::add_condition(result, clause);
    }
    return result;
  }
  static CleanupEquations::ConditionClauses absence_conditions(const std::set<size_t> &sources) {
    CleanupEquations::ConditionClauses clauses;
    for (const auto source : sources) {
      CleanupEquations::ConditionFacts facts;
      if (source != null_target) facts.absent.insert(source);
      CleanupEquations::add_condition(clauses, std::move(facts));
    }
    return clauses;
  }
  CleanupEquations::BooleanValue instantiate_boolean(const CleanupEquations::BooleanValue &value,
      const std::vector<CleanupEquations::BooleanValue> &inputs, const std::vector<CheckedOperation> &views, bool slots,
      const std::vector<ExceptionFacts> &exceptions) const {
    CleanupEquations::BooleanValue result;
    for (const auto &[truth, clauses] : value)
      for (const auto &facts : instantiate_conditions(clauses, inputs, views, slots, exceptions))
        CleanupEquations::add_condition(result[truth], facts);
    return result;
  }
  void write_predicate(const CheckedOperation &target, std::shared_ptr<const CheckedOperation::Predicate> proof, const Location &at) {
    if (target.type == Type::boolean) {
      const auto value = snapshot_boolean(proof, lexical_, at);
      auto retained = proof ? std::make_shared<Predicate>(*proof) : std::make_shared<Predicate>();
      if (!proof) retained->kind = Predicate::Kind::presence;
      retained->results = value;
      retained->complete_results = true;
      proof = std::move(retained);
    }
    const auto previous = lexical_.predicates;
    record_payload_write(target);
    const auto written = payload_targets(target);
    invalidate_payloads(lexical_, written);
    if (target.type == Type::boolean) for (const auto place : written) {
      const auto before = previous.find(place);
      lexical_.predicates[place] = updated_payload(target, place, before == previous.end() ? nullptr : before->second, proof,
          [](auto &a, const auto &b) { a = predicate(Predicate::Kind::alternatives, a, b); }, guard_predicate);
    }
  }
  void refresh_targets(LexicalState &state, bool invalidate = true, const std::set<size_t> *changed = nullptr) {
    if (!proving_) return;
    for (auto &[binding, targets] : state.targets) {
      // A copied reference keeps its selected target when its source is rebound.
      if (!rebound_.contains(binding)) continue;
      if (changed && !changed->contains(binding)) continue;
      targets = possible_targets_.at(binding);
      state.null_sources[binding] = possible_null_sources_.at(binding);
      if (!invalidate) {
        // Completed cleanup can supply tighter bounds than whole-method discovery.
        if (const auto known = state.selections.targets.find(binding);
            known != state.selections.targets.end() && known->second.entries.empty()) {
          targets = known->second.places;
          state.null_sources[binding] = known->second.null_sources;
          if (known->second.nullable()) targets.insert(null_target);
        }
      }
      // Every mutation of a ref must preserve its checked non-null target.
      if (bindings_->at(binding).requires_initialized) targets.erase(null_target);
      if (!targets.contains(null_target)) state.null_sources[binding].clear();
      if (targets.contains(null_target) && state.null_sources[binding].empty()) state.null_sources[binding].insert(null_target);
      const auto owners = target_owners(targets);
      state.owners[binding].insert(owners.begin(), owners.end());
      if (invalidate) invalidate_selection(state, binding);
      state.target_versions[binding] = {next_target_version_++};
      state.selection_identities[binding] = new_selection_identity();
    }
    refine_present(state);
  }
  void retain_loop_targets(LoopState &loop, const LexicalState &state) {
    for (const auto binding : loop.rebound) {
      if (!loop.bindings.contains(binding)) continue;
      const auto selected = state.targets.find(binding);
      if (selected == state.targets.end()) continue;
      auto &known = loop.header_targets[binding];
      const auto count = known.places.size() + known.null_sources.size();
      known.merge(selection_targets(selected->second, state.null_sources.at(binding)));
      loop_evidence_changed_ |= count != known.places.size() + known.null_sources.size();
    }
  }
  void retain_loop_initialization(LoopState &loop, const LexicalState &state) {
    for (auto it = loop.initialized.begin(); it != loop.initialized.end();) {
      const auto required = selection_initialization(state, it->first);
      if (!required) { it = loop.initialized.erase(it); loop_evidence_changed_ = true; continue; }
      const auto count = it->second.size();
      it->second.insert(required->begin(), required->end());
      loop_evidence_changed_ |= it->second.size() != count;
      ++it;
    }
  }
  void resolve_payload_writes(PayloadWrites &writes) const {
    // Later iterations and deferred actions can select targets discovered
    // only after the binding dependencies have closed.
    for (const auto origin : writes.origins) {
      const auto &places = possible_targets_.at(origin);
      writes.places.insert(places.begin(), places.end());
    }
    writes.places.erase(null_target);
    const auto written = writes.places;
    for (size_t place = 0; place < storage_->size(); ++place)
      if (storage(place).borrowed && std::any_of(written.begin(), written.end(), [&](size_t target) {
            return payload_targets_may_share(target, place);
          })) writes.places.insert(place);
  }
  void solve_targets() {
    bool changed;
    do {
      changed = false;
      for (size_t i = 0; i < possible_targets_.size(); ++i) for (const auto source : target_dependencies_[i]) {
        const auto before = possible_targets_[i].size();
        possible_targets_[i].insert(possible_targets_[source].begin(), possible_targets_[source].end());
        changed = changed || before != possible_targets_[i].size();
        const auto nulls = possible_null_sources_[i].size();
        possible_null_sources_[i].insert(possible_null_sources_[source].begin(), possible_null_sources_[source].end());
        changed = changed || nulls != possible_null_sources_[i].size();
      }
    } while (changed);
    catalog_storage_ = *storage_;
    resolve_payload_writes(payload_writes_);
    for (auto &[expr, loop] : loop_states_) {
      for (const auto binding : loop.bindings)
        if (loop.rebound.contains(binding)) for (const auto place : possible_targets_.at(binding)) if (place != null_target) {
          const auto scope = storage(place).scope;
          if (scope >= loop.first && scope < loop.last) loop.expired.insert(binding);
        }
      resolve_payload_writes(loop.writes);
    }
  }
  void initialize(size_t id) {
    lexical_.initialized.insert(id);
    lexical_.uninitialized.erase(id);
    lexical_.returned.uninitialized.erase(id);
    for (const auto input : lexical_.inputs) lexical_.input_facts[input].initialized.insert(id);
    refresh_input_facts(lexical_);
  }
  std::optional<CleanupEquations::ConditionFacts> selected_target(size_t selection, size_t place, bool equal) const {
    if (place == null_target) return {};
    CleanupEquations::ConditionFacts facts;
    if (const auto action = return_selection_actions_.find(selection); action != return_selection_actions_.end())
      facts.conditions.add({action->second, CleanupEquations::pending_return_selection, place, equal});
    if (storage(place).borrowed) {
      const auto incoming = incoming_selections_.find(place);
      if (incoming == incoming_selections_.end()) return {};
      if (equal) facts.same_selections.insert({selection, incoming->second});
      else facts.different_selections.emplace(selection, incoming->second);
    } else facts.conditions.add({{}, selection, place, equal, true});
    return facts;
  }
  std::optional<CleanupEquations::ConditionFacts> selected_target(const CheckedOperation &view, size_t place, bool equal) const {
    return view.selection_identity && view.targets.contains(place) ? selected_target(*view.selection_identity, place, equal) : std::nullopt;
  }
  CleanupEquations::ConditionClauses selection_bounds(const CheckedOperation &view) const {
    CleanupEquations::ConditionClauses bounds;
    if (view.targets.empty()) bounds.insert(CleanupEquations::ConditionFacts{});
    for (const auto place : view.targets)
      CleanupEquations::add_condition(bounds, selected_target(view, place, true).value_or(CleanupEquations::ConditionFacts{}));
    return bounds;
  }
  void initialize_targets(const CheckedOperation &view, std::set<size_t> &initialized,
      CleanupEquations::MissingInitialization &missing,
      const CleanupEquations::ConditionClauses &unwritten = {}) const {
    auto targets = view.targets; targets.erase(null_target);
    for (const auto place : targets) {
      auto absent = unwritten;
      if (targets.size() != 1) {
        const auto condition = selected_target(view, place, false);
        if (!condition) continue;
        CleanupEquations::add_condition(absent, *condition);
      }
      missing[place] = CleanupEquations::conjoin_conditions(absent,
          CleanupEquations::missing_initialization(place, initialized, missing));
    }
    CleanupEquations::resolve_initialization(initialized, missing);
  }
  void initialize_selection(LexicalState &state, size_t id) {
    state.selections.initialize(id);
    for (const auto input : state.inputs) state.input_facts[input].selections.initialize(id);
    record_return_readiness(state, id);
    refresh_input_facts(state);
  }
  std::optional<CleanupEquations::Initialization> selection_initialization(const LexicalState &state, size_t id) const {
    const auto &targets = state.targets.at(id);
    const auto selection = state.selection_identities.find(id);
    if (state.selections.initialized.contains(id) ||
        std::all_of(targets.begin(), targets.end(), [&](size_t place) {
          if (place == null_target || state.initialized.contains(place)) return true;
          if (!proving_) return false;
          const auto selected = selection == state.selection_identities.end()
              ? std::nullopt : selected_target(selection->second, place, true);
          return missing_initialization(state, place, selected).empty();
        })) return CleanupEquations::Initialization{};
    return state.selections.initialization_requirements(id);
  }
  void assign_initialization(LexicalState &state, size_t id,
      const std::optional<CleanupEquations::Initialization> &condition) {
    if (!condition) return;
    if (condition->empty()) { initialize_selection(state, id); return; }
    state.selections.initialized_if[id] = *condition;
    for (const auto input : state.inputs) state.input_facts[input].selections.initialized_if[id] = *condition;
    refresh_input_facts(state);
  }
  static void assign_selection(LexicalState &state, size_t id, const CleanupEquations::Targets &targets) {
    state.returned.selection_bindings.erase(id);
    state.selections.assign(id, targets);
    for (const auto input : state.inputs) state.input_facts[input].selections.assign(id, targets);
    refresh_input_facts(state);
  }
  static void invalidate_selection(LexicalState &state, size_t id) {
    state.returned.selection_bindings.erase(id);
    state.selections.invalidate(id);
    for (const auto input : state.inputs) state.input_facts[input].selections.invalidate(id);
  }
  std::optional<size_t> current_binding(const CheckedOperation &view) const {
    if (view.binding && lexical_.target_versions.contains(*view.binding) &&
        lexical_.target_versions.at(*view.binding) == view.selection_versions) return view.binding;
    if (view.selection_identity) for (const auto &[binding, identity] : lexical_.selection_identities)
      if (identity == *view.selection_identity && lexical_.bindings_available.contains(binding)) return binding;
    return {};
  }
  static void add_requirement(LexicalState &state, size_t read, const CompletionInputs &inputs, const CleanupEquations::ReadRequirement &before) {
    auto [found, inserted] = state.requirements.emplace(read, LexicalState::Requirement{inputs, before});
    if (!inserted) {
      found->second.inputs.insert(inputs.begin(), inputs.end());
      found->second.before.merge(before);
    }
  }
  size_t input_ordinal(size_t place) const {
    if (place == null_target) return null_target;
    const auto &inputs = checking_instance_->frame.inputs;
    for (size_t i = 0; i < inputs.size(); ++i)
      if (inputs[i].place == place && inputs[i].mode != ArgumentMode::value) return i;
    throw std::logic_error("call summary refers to non-input storage");
  }
  std::set<size_t> input_ordinals(const std::set<size_t> &places) const {
    std::set<size_t> result;
    for (const auto place : places) result.insert(input_ordinal(place));
    return result;
  }
  std::set<size_t> selected_input(const std::set<size_t> &targets) const {
    auto places = targets;
    places.erase(null_target);
    if (places.size() != 1 || !storage(*places.begin()).borrowed) return {};
    return places;
  }
  static void refine_present(LexicalState &state) {
    for (auto &[binding, targets] : state.targets) {
      const auto &sources = state.null_sources.at(binding);
      if (!targets.contains(null_target) || targets.size() == 1 || sources.empty() ||
          !std::all_of(sources.begin(), sources.end(), [&](size_t source) { return state.present.contains(source); })) continue;
      targets.erase(null_target);
      state.null_sources[binding].clear();
      // Binding effects can already describe the next selection while these
      // target bounds still describe the previous one. Refine only its bounds.
    }
  }
  void record_present(const std::set<size_t> &present) {
    if (!lexical_.live) return;
    lexical_.present.insert(present.begin(), present.end());
    for (const auto input : lexical_.inputs)
      lexical_.input_facts.at(input).present.insert(present.begin(), present.end());
    refine_present(lexical_);
  }
  void record_presence(const std::set<size_t> &targets) { record_present(selected_input(targets)); }
  std::optional<CleanupEquations::Initialization> external_initialization(
      const std::optional<CleanupEquations::Initialization> &conditions) const {
    std::optional<CleanupEquations::Initialization> initialized_inputs;
    if (conditions) {
      initialized_inputs.emplace();
      for (const auto &condition : *conditions) {
        if (condition.physical && storage(condition.id).borrowed)
          initialized_inputs->insert(CleanupEquations::InitializationInput::payload(input_ordinal(condition.id)));
        else {
          const auto &inputs = checking_instance_->frame.inputs;
          const auto found = std::find_if(inputs.begin(), inputs.end(), [&](const auto &input) {
            return !condition.physical && !condition.frame && storage(input.place).borrowed && storage(input.place).binding == condition.id;
          });
          if (found == inputs.end()) { initialized_inputs.reset(); break; }
          initialized_inputs->insert(CleanupEquations::InitializationInput::payload(std::distance(inputs.begin(), found)));
        }
      }
    }
    return initialized_inputs;
  }
  static std::optional<CleanupEquations::Initialization> instantiate_initialization(
      const std::optional<CleanupEquations::Initialization> &conditions, const std::vector<CheckedOperation> &views) {
    if (!conditions) return {};
    CleanupEquations::Initialization result;
    for (const auto &input : *conditions) {
      const auto &source = views.at(input.id).initialization;
      if (!source) return {};
      result.insert(source->begin(), source->end());
    }
    return result;
  }
  void observe_read(size_t place, const CleanupEquations::ReadRequirement &before) {
    if (delayed_ || !storage(place).borrowed) return;
    std::set<size_t> external;
    for (const auto id : before.initialized) if (storage(id).borrowed) external.insert(input_ordinal(id));
    CleanupEquations::MissingInitialization missing;
    for (const auto &[id, clauses] : before.uninitialized) if (storage(id).borrowed)
      missing.emplace(input_ordinal(id), external_conditions(clauses));
    observed_.reads.insert({input_ordinal(place), std::move(external), std::move(missing), external_conditions(before.when),
        external_initialization(before.initialized_selections)});
  }
  bool in_immediate_scope() const {
    return checking_method_ && checking_method_->source->implicit_scope;
  }
  void require_local_read(const CallEffects::ReadSite &site, const CleanupEquations::ConditionClauses &when) {
    if (in_immediate_scope()) {
      auto missing = external_conditions(when, true);
      if (!CleanupEquations::unconditional(missing)) {
        if (!missing.empty()) observed_.reads.insert({site, {}, {}, std::move(missing)});
        return;
      }
    }
    if (validating_) fail(site.at, site.diagnostic);
  }
  void require_symbol(LexicalState &state, size_t symbol, const Location &at) {
    if (!proving_ || !state.live || state.bound_symbols.contains(symbol)) return;
    if (!delayed_ && !checking_instance_ && validating_)
      fail(at, "cannot prove an installed binding for symbol '" + program_.symbols[symbol].name + "'");
    state.required_symbols.insert(symbol);
    if (!delayed_) observed_.symbols.insert(symbol);
  }
  static bool initialized_entry_alias(const CleanupEquations::ReadRequirement &read, const LexicalState &state) {
    if (read.nonnull || !read.selection) return false;
    const auto selected = state.selection_identities.find(*read.selection);
    if (selected == state.selection_identities.end()) return false;
    return std::all_of(read.when.begin(), read.when.end(), [&](const auto &facts) {
      return std::any_of(read.entry_initialized.begin(), read.entry_initialized.end(), [&](size_t binding) {
        const auto written = state.selection_identities.find(binding);
        return written != state.selection_identities.end() && facts.same_selection(selected->second, written->second);
      });
    });
  }
  size_t initialization_frame() const { return deferred_action_ ? CleanupEquations::action_frame(*deferred_action_) : 0; }
  void apply_effects(LexicalState &state, const CleanupEquations::Effects &incoming, const Location &at) {
    auto effects = incoming;
    const auto entry = path_conditions(state);
    effects.transform_conditions([&](const auto &clauses) { return CleanupEquations::conjoin_conditions(clauses, entry); });
    for (const auto input : state.inputs) state.input_facts[input].selections.constrain(effects.selections);
    refresh_input_facts(state);
    state.live = state.live && state.selections.possible;
    if (!state.live) return;
    auto retained_return = std::exchange(state.returned, CleanupEquations::Returned{});
    retained_return.initialize_from_observation(effects.selections);
    retained_return.advance(effects.selections);
    for (const auto binding : effects.selections.entry_initialized)
      if (const auto selection = state.selection_identities.find(binding);
          selection != state.selection_identities.end() && state.observed_saved_selections.contains(selection->second))
        for (const auto input : state.inputs) state.input_facts.at(input).initialized_saved_selections.insert(selection->second);
    effects.returned.substitute(state.selections);
    invalidate_payloads(state, effects.payload_writes);
    for (const auto &[place, value] : effects.booleans) if (state.available.contains(place))
      state.predicates[place] = boolean_predicate(value);
    for (const auto symbol : effects.symbols) require_symbol(state, symbol, at);
    if (proving_) for (const auto &[read, before] : effects.reads) {
      const auto place = cleanup_.read_target(read);
      if (before.satisfied(place, state.initialized, state.selections)) continue;
      if (!place) {
        if (delayed_) add_requirement(state, read, state.inputs, before);
        else require_local_read(read_sites_.at(read), before.when);
        continue;
      }
      auto known = before;
      CleanupEquations::combine_initialization(known.initialized, known.uninitialized,
          state.initialized, state.uninitialized, false);
      known.when = CleanupEquations::conjoin_conditions(known.when,
          CleanupEquations::missing_initialization(*place, known.initialized, known.uninitialized));
      if (known.nonnull) std::erase_if(*known.nonnull, [&](size_t id) {
        const auto guard = known.nonnull_versions.find(id), current = state.target_versions.find(id);
        return guard != known.nonnull_versions.end() && current != state.target_versions.end() && guard->second == current->second;
      });
      if (known.satisfied(place, state.initialized, state.selections) || initialized_entry_alias(known, state)) continue;
      known.initialized.insert(state.initialized.begin(), state.initialized.end());
      known.substitute(state.selections);
      known.enter_initialization(initialization_frame());
      if (known.satisfied(place, state.initialized, {})) continue;
      if (before.nonnull) {
        if (!delayed_ && validating_) fail(read_sites_.at(read).at, "cannot prove a non-null target for deferred reference swap");
        add_requirement(state, read, state.inputs, known);
        continue;
      }
      if (!storage(*place).borrowed) {
        const auto diagnostic = "cannot prove initialization of '" + storage(*place).name + "' for deferred or borrowed read";
        if (!delayed_ && in_immediate_scope()) {
          require_local_read({read_sites_.at(read).at, diagnostic}, known.when);
          continue;
        }
        if ((!delayed_ || (!deferred_inputs_.contains(*place) && !conditional_read(known))) && validating_)
          fail(read_sites_.at(read).at, diagnostic);
      }
      add_requirement(state, read, state.inputs, known);
      observe_read(*place, known);
    }
    CleanupEquations::combine_initialization(state.initialized, state.uninitialized,
        effects.initialized, effects.uninitialized, false);
    std::erase_if(state.initialized, [&](size_t place) { return !state.available.contains(place); });
    std::erase_if(state.uninitialized, [&](const auto &entry) { return !state.available.contains(entry.first); });
    for (const auto id : state.initialized)
      for (const auto input : state.inputs) state.input_facts[input].initialized.insert(id);
    for (const auto input : state.inputs)
      state.input_facts[input].present.insert(effects.present.begin(), effects.present.end());
    for (const auto binding : effects.selections.invalidated)
      if (state.selection_identities.contains(binding)) state.selection_identities[binding] = new_selection_identity();
    for (const auto input : state.inputs) {
      auto &facts = state.input_facts[input];
      facts.selections.then(effects.selections, facts.initialized);
      facts.selections.enter_initialization(initialization_frame());
      facts.selections.resolve(facts.initialized);
    }
    for (const auto input : state.inputs)
      for (const auto &[target, value] : effects.payloads) state.input_facts[input].exceptions[target] = value;
    refresh_input_facts(state);
    state.returned = std::move(retained_return);
    state.returned.then(effects.returned);
    state.returned.enter_initialization(initialization_frame());
    state.returned.initialize(state.initialized, state.selections);
  }
  static std::set<size_t> call_reasons(const std::set<size_t> &declared_categories) {
    std::set<size_t> reasons{0, 2};
    for (size_t category = 1; category < exception_categories.size(); ++category)
      if (!exception_categories[category].checked || declared_categories.contains(category)) reasons.insert(3 + category);
    return reasons;
  }
  static std::set<size_t> call_reasons(const CheckedMethod &method) {
    return call_reasons(method.throws_categories);
  }
  static bool caller_bindings(const CheckedMethod &method) {
    return method.operation == "<=>";
  }
  static bool initialized_when_present(size_t parameter, const CleanupEquations::ConditionClauses &missing) {
    return std::ranges::all_of(missing, [&](const auto &facts) { return facts.absent.contains(parameter); });
  }
  bool stable_loop_binding(size_t binding) const {
    return std::ranges::all_of(active_loops_, [&](const auto *loop) {
      const auto &state = loop_states_.at(loop);
      return state.bindings.contains(binding) && !state.rebound.contains(binding);
    });
  }
  bool stable_prepared_selection(const CheckedOperation &view) const {
    if (delayed_ || !view.selection_identity) return false;
    if (active_loops_.empty()) return true;
    if (!proving_) return false;
    return std::ranges::any_of(lexical_.selection_identities, [&](const auto &entry) {
      const auto [binding, identity] = entry;
      return identity == *view.selection_identity && lexical_.bindings_available.contains(binding) && stable_loop_binding(binding);
    });
  }
  void initialize_call_selection(CleanupEquations::Selections &selections, const CheckedOperation &view, bool prepared) const {
    if (prepared && stable_prepared_selection(view))
      selections.initialized_captures.insert(*view.selection_identity);
    if (const auto binding = current_binding(view); binding && (!prepared || !rebound_.contains(*binding)))
      selections.initialize(*binding);
  }
  static CallEffects initial_effects(const CheckedMethod &method, const std::vector<Type> &arguments) {
    CallEffects effects;
    std::set<size_t> borrowed;
    for (size_t p = 0; p < method.input_parameters().size(); ++p)
      if (!method.input_parameters()[p].owner.empty()) borrowed.insert(p);
    for (const auto reason : call_reasons(method)) {
      effects.initialized[reason] = borrowed;
      effects.present[reason] = borrowed;
    }
    // Value possibilities grow from no observed return, independently of
    // the optimistic initialization/presence guarantees above.
    for (const auto reason : call_reasons(method)) for (const auto parameter : borrowed)
      if (arguments[parameter] == Type::boolean) effects.booleans[reason][parameter] = effects.returned.boolean;
    for (const auto reason : call_reasons(method)) for (const auto parameter : borrowed)
      if (arguments[parameter].kind == Type::exception)
        effects.payloads[reason].try_emplace(parameter);
    if (caller_bindings(method)) for (const auto reason : call_reasons(method))
      for (size_t p = 0; p < arguments.size(); ++p) {
        auto &targets = effects.bindings[reason][p];
        targets.initialized = true;
        // Nullable input origins stay distinct from absence introduced by a body.
        for (size_t q = 0; q < arguments.size(); ++q)
          if (arguments[p] == arguments[q] && same_owner(method, p, q)) {
            targets.targets.insert(q);
            if (argument_mode(method.input_parameters()[p]) != ArgumentMode::borrowed_ref &&
                argument_mode(method.input_parameters()[q]) == ArgumentMode::borrowed_ptr)
              targets.null_sources.insert(q);
          }
      }
    return effects;
  }
  std::map<size_t, CleanupEquations::Effects> invocation_effects(const CallEffects &target, const std::set<size_t> &reasons,
      const std::vector<CheckedOperation> &views, const Location &at, bool prepared, bool slots = false,
      const std::vector<CleanupEquations::BooleanValue> &booleans = {}, const std::vector<ExceptionFacts> &exceptions = {}) {
    const auto selection = [&](const CheckedOperation &view) {
      auto binding = current_binding(view);
      // Prepared arguments retain their target even if the source slot is rebound.
      if (prepared && binding && rebound_.contains(*binding)) binding.reset();
      return binding;
    };
    CleanupEquations::Reads reads;
    if (proving_) for (const auto &[source, preceding, conditional, condition, initialized_inputs] : target.reads) {
      auto when = instantiate_conditions(condition, booleans, views, slots, exceptions);
      if (when.empty()) continue;
      if (const auto *local = std::get_if<CallEffects::ReadSite>(&source)) {
        const auto read = read_obligation({}, *local);
        reads[read].when = std::move(when);
        continue;
      }
      const auto parameter = std::get<size_t>(source);
      auto before = preceding;
      CleanupEquations::MissingInitialization missing;
      std::set<size_t> known;
      if (!slots) for (const auto parameter : before) initialize_targets(views.at(parameter), known, missing);
      for (const auto &[other, clauses] : conditional) {
        auto absent = instantiate_conditions(clauses, booleans, views, slots, exceptions);
        if (absent.empty()) before.insert(other);
        if (!slots) initialize_targets(views.at(other), known, missing, absent);
      }
      const auto &view = views.at(parameter);
      if (slots) {
        std::set<size_t> initialized;
        for (const auto other : before) {
          const auto selected = physical_slots(views.at(other).slots);
          if (selected.size() == 1) initialized.insert(*selected.begin());
        }
        auto required = prepared_binding_reads(view, at, false, initialized);
        for (auto &[read, requirement] : required) requirement.when = when;
        reads.insert(required.begin(), required.end());
        continue;
      }
      if (view.initialized_target()) continue;
      auto initialization = instantiate_initialization(initialized_inputs, views);
      if (initialization) {
        initialization = CleanupEquations::capture_initialization(*initialization, initialization_frame());
        if (initialization && initialization->empty()) continue;
      }
      const bool same_selection = std::any_of(before.begin(), before.end(), [&](size_t other) {
        return same_selected_reference(view, views.at(other));
      });
      if (same_selection) continue;
      for (const auto place : view.targets) if (place != null_target && !known.contains(place)) {
        const auto read = read_obligation(place, {at, {}});
        auto &requirement = reads.emplace(read, CleanupEquations::ReadRequirement{known, selection(view)}).first->second;
        requirement.initialized_selections = initialization;
        requirement.uninitialized = missing;
        requirement.when = CleanupEquations::conjoin_conditions(when,
            CleanupEquations::missing_initialization(place, known, missing));
        if (const auto condition = selected_target(view, place, true)) {
          requirement.when = CleanupEquations::conjoin_conditions(requirement.when, {*condition});
        }
      }
    }
    std::map<size_t, CleanupEquations::Effects> effects;
    for (const auto reason : reasons) {
      const auto found = target.initialized.find(reason);
      auto &effect = effects.emplace(reason, CleanupEquations::Effects{reads, {}}).first->second;
      effect.symbols = target.symbols;
      if (!slots) if (const auto outcome = target.present.find(reason); outcome != target.present.end())
        for (const auto parameter : outcome->second) {
          const auto present = selected_input(views.at(parameter).targets);
          effect.present.insert(present.begin(), present.end());
        }
      if (found != target.initialized.end()) for (const auto parameter : found->second) {
        const auto &view = views.at(parameter);
        if (!slots) {
          initialize_targets(view, effect.initialized, effect.uninitialized);
          // An invariant preparation retains its selection through later rebinds.
          initialize_call_selection(effect.selections, view, prepared);
        }
        const auto selected = physical_slots(view.slots);
        const auto binding = slots ? (selected.size() == 1 ? std::optional<size_t>{*selected.begin()} : std::nullopt)
                                   : selection(view);
        if (binding) {
          if (slots) effect.selections.initialize(*binding);
          effect.selections.refine(*binding, false);
        }
      }
      if (slots) if (const auto outcome = target.uninitialized.find(reason); outcome != target.uninitialized.end())
        for (const auto &[parameter, missing] : outcome->second) {
          // When only a null input can lack a write, its invocation-time
          // target is initialized if present, independently of slot updates.
          if (!initialized_when_present(parameter, missing)) continue;
          const auto selected = physical_slots(views.at(parameter).slots);
          if (selected.size() == 1) effect.selections.entry_initialized.insert(*selected.begin());
        }
    }
    return effects;
  }
  static bool exact_binding_selection(const CallEffects::BindingResult &result, size_t input, const CheckedOperation &view) {
    return result.targets.size() == 1 && result.targets.contains(input) &&
        (result.null_sources.empty() ? !view.targets.contains(null_target) :
            result.null_sources.size() == 1 && result.null_sources.contains(input));
  }
  void invocation_bindings(const CallEffects &summary, const std::vector<CheckedOperation> &views,
                           CheckedOperation &operation, bool prepared, bool immediate_scope) {
    if (!proving_) for (const auto &[parameter, sources] : summary.binding_sources) {
      const auto written = overlapping_slots(views.at(parameter).slots);
      for (const auto &[binding, current] : lexical_.targets) {
        if (intersect(written, physical_slots({binding})).empty()) continue;
        for (const auto input : sources) {
          if (input == null_target) {
            possible_targets_.at(binding).insert(null_target);
            possible_null_sources_.at(binding).insert(null_target);
            continue;
          }
          const auto &source = views.at(input);
          for (const auto place : source.targets) if (place != null_target) possible_targets_.at(binding).insert(place);
          possible_null_sources_.at(binding).insert(source.null_sources.begin(), source.null_sources.end());
          target_dependencies_.at(binding).insert(source.origins.begin(), source.origins.end());
        }
      }
    }
    for (auto &[reason, effects] : operation.invocation) {
      const auto outcome = summary.bindings.find(reason);
      if (outcome == summary.bindings.end()) continue;
      // Payload guarantees concern entry targets. Current selections are
      // established only after the binding updates have been substituted.
      if (!immediate_scope) effects.selections.initialized.clear();
      std::set<size_t> initialized_outputs;
      for (const auto &[parameter, targets] : outcome->second) {
        const auto &destination = views.at(parameter);
        const auto written = overlapping_slots(destination.slots);
        const bool unchanged = immediate_scope && exact_binding_selection(targets, parameter, destination);
        if (targets.initialized && destination.binding) initialized_outputs.insert(*destination.binding);
        if (unchanged) continue;
        CleanupEquations::Targets selected;
        for (const auto input : targets.targets) {
          if (prepared) {
            const auto incoming = physical_slots(views.at(input).slots);
            selected.entries.insert(incoming.begin(), incoming.end());
          } else {
            auto incoming = views.at(input).targets;
            incoming.erase(null_target);
            selected.places.insert(incoming.begin(), incoming.end());
          }
        }
        // Nullable input provenance is distinct from an independently absent target.
        for (const auto input : targets.null_sources) {
          if (input == null_target) selected.null_sources.insert(null_target);
          else if (prepared) {
            const auto incoming = physical_slots(views.at(input).slots);
            selected.entries.insert(incoming.begin(), incoming.end());
          } else {
            const auto &view = views.at(input);
            if (view.targets.contains(null_target)) {
              if (view.null_sources.empty()) selected.null_sources.insert(null_target);
              else selected.null_sources.insert(view.null_sources.begin(), view.null_sources.end());
            }
          }
        }
        for (const auto &[binding, current] : lexical_.targets) {
          if (intersect(written, physical_slots({binding})).empty()) continue;
          if (immediate_scope) {
            effects.selections.initialized.erase(binding);
            effects.selections.initialized_if.erase(binding);
          }
          auto update = selected;
          if (destination.slots.size() != 1 || !destination.slots.contains(binding)) update.entries.insert(binding);
          effects.selections.targets[binding].merge(update);
          effects.selections.invalidated.insert(binding);
          if (!proving_) {
            possible_targets_.at(binding).insert(selected.places.begin(), selected.places.end());
            if (selected.nullable()) {
              possible_targets_.at(binding).insert(null_target);
              for (const auto input : targets.null_sources) {
                if (input == null_target) possible_null_sources_.at(binding).insert(null_target);
                else if (prepared) {
                  for (const auto slot : physical_slots(views.at(input).slots))
                    possible_null_sources_.at(binding).insert(possible_null_sources_.at(slot).begin(), possible_null_sources_.at(slot).end());
                } else {
                  const auto &sources = views.at(input).null_sources;
                  possible_null_sources_.at(binding).insert(sources.begin(), sources.end());
                }
              }
            }
            for (const auto input : targets.targets) {
              const auto &origins = views.at(input).origins;
              target_dependencies_.at(binding).insert(origins.begin(), origins.end());
            }
            record_rebinding(binding, prepared);
          }
        }
      }
      effects.selections.initialized.insert(initialized_outputs.begin(), initialized_outputs.end());
    }
  }
  void prepare_borrow(const CheckedOperation &view, ArgumentMode mode, const Location &at) {
    if (mode == ArgumentMode::value) return;
    if (!view.view || view.mixed_view) implementation_gap(at, "borrowed arguments require existing target views");
    if (mode == ArgumentMode::borrowed_read) read_view(view, at, "borrowed argument");
    if (mode == ArgumentMode::borrowed_ref && proving_ && validating_ && lexical_.live && view.targets.contains(null_target))
      fail(at, "ref argument requires a proven non-null target");
    if (mode == ArgumentMode::borrowed_ref) require_initialized(view, at, "ref argument");
  }
  static std::set<size_t> call_reasons(const Type &contract) {
    std::set<size_t> categories;
    for (const auto &failure : contract.throws_types) {
      const auto selected = exception_possibilities(failure); categories.insert(selected.begin(), selected.end());
    }
    return call_reasons(categories);
  }
  std::set<size_t> direct_call_reasons(const Type &contract, const CallEffects &effects) {
    auto reasons = call_reasons(contract);
    if (!proving_) return reasons;
    if (effects.requires_deferred_failure_delivery) {
      if (lexical_.live) observed_.requires_deferred_failure_delivery = true;
    } else std::erase_if(reasons, [&](size_t reason) {
      // The declaration supplies checked obligations independently of the
      // concrete body's executable failures.
      const auto completion = Completion{reason};
      const bool checked = completion.exception() && exception_categories.at(reason - 3).checked;
      return completion.failed() && !checked && !effects.escaping_failures.contains(reason);
    });
    return reasons;
  }
  bool capture_free_callable(size_t id) const {
    const auto &callable = program_.callables.at(id);
    return callable.type.pure_callable && std::ranges::all_of(callable.implementations(), [&](size_t target) {
      return program_.instances[target].environment.empty() && read_effects(target).symbols.empty();
    });
  }
  const std::set<size_t> *observed_callables(const Expression &expression) const {
    if (checking_context_ >= callable_choices_.size()) return nullptr;
    const auto &observations = callable_choices_[checking_context_];
    const auto found = observations.find(&expression);
    return found == observations.end() ? nullptr : &found->second;
  }
  bool capture_free_callables(const std::set<size_t> *choices) const {
    return choices && !choices->empty() && std::ranges::all_of(*choices, [&](size_t id) { return capture_free_callable(id); });
  }
  struct CallableInvocationEffects {
    CallEffects effects;
    bool closed;
  };
  CallableInvocationEffects callable_effects(const Type &contract, const std::vector<Type> &arguments,
                                             const std::optional<std::set<size_t>> &choices) const {
    callable_effect_consumers_.insert(checking_context_);
    CallEffects effects;
    bool first = true;
    size_t covered = 0;
    for (size_t id = 0; id < program_.callables.size(); ++id) {
      const auto &callable = program_.callables[id];
      if (callable.type != contract || (choices && !choices->contains(id))) continue;
      ++covered;
      for (const auto &[inputs, target] : callable.alternatives(arguments)) {
        const auto &next = read_effects(target);
        if (first) { effects = next; first = false; continue; }
        effects.reads.insert(next.reads.begin(), next.reads.end());
        effects.symbols.insert(next.symbols.begin(), next.symbols.end());
        effects.written_inputs.insert(next.written_inputs.begin(), next.written_inputs.end());
        effects.escaping_failures.insert(next.escaping_failures.begin(), next.escaping_failures.end());
        effects.requires_deferred_failure_delivery |= next.requires_deferred_failure_delivery;
        for (auto &[reason, known] : effects.initialized) {
          const auto found = next.initialized.find(reason);
          const auto missing = next.uninitialized.find(reason);
          CleanupEquations::combine_initialization(known, effects.uninitialized[reason],
              found == next.initialized.end() ? std::set<size_t>{} : found->second,
              missing == next.uninitialized.end() ? CleanupEquations::MissingInitialization{} : missing->second, true);
        }
        for (auto &[reason, known] : effects.present) {
          const auto found = next.present.find(reason);
          known = found == next.present.end() ? std::set<size_t>{} : intersect(known, found->second);
        }
        effects.returned.merge(next.returned);
        for (const auto &[reason, payloads] : next.payloads)
          for (const auto &[parameter, value] : payloads) effects.payloads[reason][parameter].merge(value);
        for (const auto &[reason, values] : next.booleans)
          for (const auto &[parameter, value] : values) CleanupEquations::merge_boolean(effects.booleans[reason][parameter], value);
      }
    }
    if (first) for (size_t i = 0; i < arguments.size(); ++i)
      if (!contract.owners.empty() && contract.owners[i] && (contract.const_views.empty() || !contract.const_views[i]))
        effects.written_inputs.insert(i);
    std::erase_if(effects.uninitialized, [](const auto &entry) { return entry.second.empty(); });
    for (const auto reason : call_reasons(contract)) effects.initialized.try_emplace(reason);
    for (const auto reason : call_reasons(contract)) effects.present.try_emplace(reason);
    return {std::move(effects), choices && covered && choices->size() == covered};
  }
  void require_borrow_lifetime(const CheckedOperation &view, const Location &at, bool prepared) const {
    require_live_view(view, at);
    if (prepared) for (const auto owner : view.owners) if (!external_owner(owner)) {
      const auto scope = std::find_if(lexical_.scopes.begin(), lexical_.scopes.end(), [&](const auto &entry) { return entry.id == owner; });
      const auto cleanup = std::find_if(lexical_.scopes.begin(), lexical_.scopes.end(), [&](const auto &entry) { return entry.id == cleanup_scope_; });
      if (scope > cleanup) fail(at, "deferred borrowed argument does not outlive its cleanup scope");
    }
  }
  void finish_call(const Expression &expr, const Type &contract, const CallEffects &summary,
      const std::set<size_t> &reasons, const std::vector<Type> &arguments,
      const std::vector<CheckedOperation> &views, CheckedOperation &operation, bool read, bool discard,
      size_t offset = 0, bool immediate_scope = false) {
    std::vector<ExceptionFacts> exception_inputs;
    const bool prepared = &expr == prepared_call_;
    const bool prepared_slots = prepared && !summary.bindings.empty();
    const bool has_invocation_time_slots = !immediate_scope && !summary.bindings.empty();
    const bool conditional_call = !summary.uninitialized.empty() || std::any_of(summary.reads.begin(), summary.reads.end(),
        [](const auto &read) { return !read.uninitialized.empty() || read.when != CleanupEquations::ConditionClauses{CleanupEquations::ConditionFacts{}}; });
    const auto entry_payloads = prepared ? incoming_payloads() : lexical_.exceptions;
    const bool boolean_call = *contract.target == Type::boolean || !summary.booleans.empty() || conditional_call;
    const auto entry_booleans = !boolean_call ? CleanupEquations::BooleanValues{} :
        prepared ? incoming_booleans() : current_booleans(lexical_, expr.at);
    std::vector<CleanupEquations::BooleanValue> boolean_inputs(arguments.size());
    for (size_t i = 0; i < arguments.size(); ++i) if (boolean_call && arguments[i] == Type::boolean) {
      const bool borrowed = !contract.owners.empty() && contract.owners[i];
      boolean_inputs[i] = borrowed ? prepared_slots ? CleanupEquations::unknown_boolean() : stored_boolean(views[i], entry_booleans)
          : snapshot_boolean(predicate_of(views[i]), lexical_, expr.at);
    }
    if (contract.target->kind == Type::exception || !summary.payloads.empty() || boolean_call) {
      exception_inputs.reserve(arguments.size());
      for (size_t i = 0; i < arguments.size(); ++i) {
        const bool borrowed = !contract.owners.empty() && contract.owners[i];
        exception_inputs.push_back(borrowed ? prepared_slots ? ExceptionFacts{exception_possibilities(arguments[i])}
            : stored_exceptions(views[i], entry_payloads) : views[i].exceptions);
      }
    }
    std::map<size_t, size_t> owners;
    for (size_t i = 0; i < arguments.size(); ++i) {
      const auto role = contract.owners.empty() ? 0 : contract.owners.at(i);
      if (!role) continue;
      const bool readonly = !contract.const_views.empty() && contract.const_views.at(i);
      const auto &view = views[i];
      if (view.owners.size() != 1) fail(operation.argument(expr, i + offset).at, "borrowed argument requires one statically provable owner");
      const auto owner = *view.owners.begin();
      require_borrow_lifetime(view, operation.argument(expr, i + offset).at, &expr == prepared_call_);
      if (const auto [found, inserted] = owners.emplace(role, owner); !inserted) {
        const auto equality = compare_owners(found->second, owner);
        if (equality == OwnerMatch::unknown) implementation_gap(expr.at, "borrowed call requires an unproved owner equality");
        if (equality == OwnerMatch::mismatch) fail(expr.at, "arguments for one owner role have different actual owners");
      }
      if (!readonly && view.read_only) fail(operation.argument(expr, i + offset).at, "readonly argument cannot supply a writable borrowed parameter");
      if (arguments[i] != contract.parameters[i]) implementation_gap(operation.argument(expr, i + offset).at, "borrowed exception upcasts");
    }
    operation.type = *contract.target;
    if (operation.type.kind == Type::exception)
      operation.exceptions = operation.type.identity ? ExceptionFacts{exception_possibilities(operation.type)}
          : summary.returned.value.substitute(exception_inputs);
    for (size_t i = 0; i < arguments.size(); ++i) {
      if (arguments[i].kind != Type::procedure || contract.owners.empty() || !contract.owners[i]) continue;
      if (operation.callable_argument_targets.empty()) operation.callable_argument_targets.resize(arguments.size());
      auto &targets = operation.callable_argument_targets[i];
      if (prepared_slots) {
        // Registration retains the slot; its later valid targets can reach the call.
        for (const auto slot : physical_slots(views[i].slots)) {
          const auto &possible = possible_targets_.at(slot);
          targets.insert(possible.begin(), possible.end());
        }
      } else targets = views[i].targets;
    }
    operation.invocation = invocation_effects(summary, reasons, views, expr.at, prepared, prepared_slots, boolean_inputs, exception_inputs);
    if (!immediate_scope && !summary.bindings.empty()) invocation_bindings(summary, views, operation, prepared_slots, false);
    std::set<size_t> ready_after_call;
    if (!prepared_slots) for (auto &[reason, effects] : operation.invocation) {
      if (!reason && summary.bindings.empty()) if (const auto known = summary.initialized.find(0); known != summary.initialized.end())
        ready_after_call = known->second;
      const auto outcome = summary.uninitialized.find(reason);
      if (outcome == summary.uninitialized.end()) continue;
      std::set<size_t> projected;
      for (const auto &[parameter, missing] : outcome->second) {
        if (projected.contains(parameter)) continue;
        const auto &view = views.at(parameter);
        auto unwritten = missing;
        std::set<size_t> same_target{parameter};
        if (immediate_scope || summary.bindings.empty()) for (const auto &[other, condition] : outcome->second) {
          if (other <= parameter || projected.contains(other) || !same_selected_reference(view, views.at(other))) continue;
          same_target.insert(other);
          projected.insert(other);
          // A shared target is unwritten only when every corresponding input
          // is unwritten. Keep the callee guards until after this intersection.
          unwritten = CleanupEquations::conjoin_conditions(unwritten, condition);
        }
        const auto when = instantiate_conditions(unwritten, boolean_inputs, views, false, exception_inputs);
        initialize_targets(view, effects.initialized, effects.uninitialized, when);
        const auto ready_if_present = [&] {
          return std::ranges::all_of(unwritten, [&](const auto &facts) {
            return std::ranges::any_of(same_target, [&](size_t input) { return facts.absent.contains(input); });
          });
        };
        if ((immediate_scope || summary.bindings.empty()) && (when.empty() || ready_if_present())) {
          for (const auto input : same_target) initialize_call_selection(effects.selections, views.at(input), prepared);
          if (!reason) ready_after_call.insert(same_target.begin(), same_target.end());
        }
      }
      effects.resolve_initialization();
    }
    if (immediate_scope && !summary.bindings.empty()) invocation_bindings(summary, views, operation, prepared_slots, true);
    if (proving_ && operation.callee && !immediate_scope && summary.bindings.empty()) {
      // Per-formal summaries can lose complementary writes when a call
      // forwards or permutes its inputs. Reuse the finite source proof for
      // the shared storage, keeping the native failure and read obligations.
      auto &normal = operation.invocation.at(0);
      for (const auto input : prove_shared_call_initialization(expr, *operation.callee, views, ready_after_call)) {
        const auto &view = views.at(input);
        CleanupEquations::ConditionFacts absent;
        absent.absent.insert(input);
        const auto when = instantiate_conditions({std::move(absent)}, boolean_inputs, views, false, exception_inputs);
        initialize_targets(view, normal.initialized, normal.uninitialized, when);
        initialize_call_selection(normal.selections, view, prepared);
        ready_after_call.insert(input);
      }
      normal.resolve_initialization();
    }
    if (prepared_slots) {
      CleanupEquations::Reads reads;
      bool checked_entry = false;
      for (size_t i = 0; i < views.size(); ++i) {
        const auto mode = operation.arguments.at(i).mode;
        if (mode == ArgumentMode::borrowed_ptr) continue;
        const auto required = prepared_binding_reads(views[i], expr.at, mode == ArgumentMode::borrowed_ref);
        reads.insert(required.begin(), required.end());
        const auto selected = physical_slots(views[i].slots);
        checked_entry = checked_entry || (mode == ArgumentMode::borrowed_read &&
            std::any_of(selected.begin(), selected.end(),
                [&](size_t id) { return possible_targets_.at(id).contains(null_target); }));
      }
      if (checked_entry) {
        // An entry check can fail before the selected method mutates any slot.
        auto &failed = operation.invocation[3 + exception_id(ExceptionCategory::null_pointer)];
        failed.initialized.clear();
        failed.selections.merge({});
      }
      for (auto &[reason, effect] : operation.invocation) effect.reads.insert(reads.begin(), reads.end());
    }
    std::set<size_t> written_targets;
    for (const auto i : summary.written_inputs) {
      if (!contract.const_views.empty() && contract.const_views[i]) continue;
      auto target = views.at(i);
      if (prepared_slots) {
        target.targets.clear();
        target.origins = physical_slots(target.slots);
        for (const auto slot : target.origins) {
          const auto &possible = possible_targets_.at(slot);
          target.targets.insert(possible.begin(), possible.end());
        }
      }
      const auto written = payload_targets(target);
      written_targets.insert(written.begin(), written.end());
      record_payload_write(target, prepared);
    }
    for (auto &[reason, effects] : operation.invocation) {
      effects.payload_writes = written_targets;
      auto payloads = entry_payloads;
      if (prepared_slots) {
        // These calls retain binding slots whose invocation-time selections
        // still need a payload/selection correlation proof.
        for (auto &[place, value] : payloads)
          if (written_targets.contains(place)) value = exception_possibilities(storage(place).type);
      } else {
        // Publish simultaneous postconditions after weakening other possible
        // aliases. Sequential strong writes would lose same-target alternatives.
        std::map<size_t, ExceptionFacts> definite;
        for (size_t i = 0; i < arguments.size(); ++i) {
          if (arguments[i].kind != Type::exception || contract.owners.empty() || !contract.owners[i]) continue;
          auto value = ExceptionFacts{exception_possibilities(arguments[i])};
          const auto outcome = summary.payloads.find(reason);
          const bool writes = summary.written_inputs.contains(i);
          // An empty outcome for an unchanged input carries no postcondition.
          // Replacing the payload with it loses its provenance in later cleanup.
          if (outcome != summary.payloads.end() && outcome->second.contains(i) &&
              (writes || !outcome->second.at(i).empty()))
            value = outcome->second.at(i).substitute(exception_inputs);
          else if (!writes) continue;
          write_exception_payloads(payloads, views[i], value);
          if (views[i].place) definite[*views[i].place].merge(value);
        }
        for (auto &[place, value] : definite) payloads[place] = std::move(value);
      }
      effects.payloads = payload_changes(entry_payloads, payloads);
      auto booleans = entry_booleans;
      if (prepared_slots) {
        for (auto &[place, value] : booleans) if (effects.payload_writes.contains(place)) value = CleanupEquations::unknown_boolean();
      } else {
        CleanupEquations::BooleanValues definite;
        for (size_t i = 0; i < arguments.size(); ++i) {
          if (arguments[i] != Type::boolean || contract.owners.empty() || !contract.owners[i]) continue;
          const bool readonly = !summary.written_inputs.contains(i) || (!contract.const_views.empty() && contract.const_views[i]);
          auto value = CleanupEquations::unknown_boolean();
          const auto outcome = summary.booleans.find(reason);
          const bool reported = outcome != summary.booleans.end() && outcome->second.contains(i);
          if (readonly && (!reported || !intersect(payload_targets(views[i]), effects.payload_writes).empty())) continue;
          if (reported)
            value = instantiate_boolean(outcome->second.at(i), boolean_inputs, views, has_invocation_time_slots, exception_inputs);
          if (readonly) {
            // A readonly postcondition refines the retained payload. Writes
            // through other actuals supply their own post-state evidence.
            const auto incoming = stored_boolean(views[i], entry_booleans);
            CleanupEquations::BooleanValue refined;
            for (const auto &[truth, clauses] : value)
              if (const auto before = incoming.find(truth); before != incoming.end())
                refined[truth] = CleanupEquations::conjoin_conditions(before->second, clauses);
            if (refined.empty()) continue;
            value = std::move(refined);
          }
          write_boolean_payloads(booleans, views[i], value);
          if (views[i].place) CleanupEquations::merge_boolean(definite[*views[i].place], value);
        }
        for (auto &[place, value] : definite) booleans[place] = std::move(value);
      }
      for (const auto &[place, value] : booleans)
        if (effects.payload_writes.contains(place) || !entry_booleans.contains(place) || entry_booleans.at(place) != value)
          effects.booleans[place] = value;
    }
    if (lexical_.live && declares_root(contract.throws_types)) {
      const auto requirement = declared_requirement(expr);
      if (prepared) for (auto &[reason, effects] : operation.invocation) effects.declared_failures.insert(requirement);
      else note_declared(requirement);
    }
    if (!prepared) {
      const auto refine_arguments = [&](size_t reason) {
        // Slot methods can replace the incoming selections. Their output
        // bindings have a separate summary; presence still names entry targets.
        if (!immediate_scope && !summary.bindings.empty()) return;
        const auto &changed = operation.invocation.at(reason).selections.invalidated;
        if (const auto outcome = summary.present.find(reason); outcome != summary.present.end())
          for (const auto parameter : outcome->second) {
            const auto &view = views.at(parameter);
            const auto binding = current_binding(view);
            if (!binding || changed.contains(*binding)) continue;
            auto &targets = lexical_.targets.at(*binding);
            // Postconditions refine a possible selection; they cannot remove a
            // declared completion when its actual argument is definitely null.
            if (targets.size() == 1 && targets.contains(null_target)) continue;
            targets.erase(null_target);
            lexical_.null_sources.at(*binding).clear();
            lexical_.selections.refine(*binding, false);
            for (const auto input : lexical_.inputs)
              lexical_.input_facts.at(input).selections.refine(*binding, false);
          }
      };
      const auto apply_invocation = [&](size_t reason, const CleanupEquations::Effects &effects) {
        apply_effects(lexical_, effects, expr.at);
        if (!summary.bindings.empty()) refresh_targets(lexical_, false,
            immediate_scope ? &effects.selections.invalidated : nullptr);
        if (immediate_scope) if (const auto outcome = summary.bindings.find(reason); outcome != summary.bindings.end())
          for (const auto &[parameter, targets] : outcome->second) {
            const auto binding = views.at(parameter).binding;
            if (!binding || !effects.selections.invalidated.contains(*binding) || targets.targets.size() != 1) continue;
            const auto input = *targets.targets.begin();
            const auto &source = views.at(input);
            if (source.selection_identity && exact_binding_selection(targets, input, source))
              lexical_.selection_identities[*binding] = *source.selection_identity;
          }
        refine_arguments(reason);
      };
      const auto before = lexical_;
      std::optional<LexicalState> here_break;
      for (const auto &[reason, effects] : operation.invocation) if (reason) {
        lexical_ = before; apply_invocation(reason, effects);
        const auto completion = Completion{reason, false};
        const bool scope_failure = immediate_scope && completion.exception() &&
            (exception_categories.at(reason - 3).checked || declares_root(contract.throws_types));
        if (completion.protected_here() && operation.supplied_here) {
          if (deliver_here_completion(expr, operation)) here_break = lexical_;
        }
        else note_abrupt(expr.at, completion.exception() ? std::set<size_t>{reason - 3} : std::set<size_t>{},
                         completion, scope_failure);
      }
      lexical_ = before; apply_invocation(0, operation.invocation.at(0));
      if (here_break) { const auto normal = lexical_; join(normal, false, *here_break, false, expr.at); }
    }
    const auto result_owner = contract.owners.empty() ? 0 : contract.owners.back();
    if (!prepared && !result_owner && operation.type == Type::boolean) {
      operation.predicate = boolean_predicate(instantiate_boolean(summary.returned.boolean, boolean_inputs, views,
                                                                   has_invocation_time_slots, exception_inputs));
    }
    if (result_owner) {
      operation.targets.clear();
      if (!owners.contains(result_owner)) implementation_gap(expr.at, "result owner is not determined by borrowed arguments");
      operation.borrowed_result = operation.view = true;
      operation.read_only = !contract.const_views.empty() && contract.const_views.back();
      operation.owners = {owners.at(result_owner)};
      operation.initialization = CleanupEquations::Initialization{};
      for (const auto parameter : summary.returned.targets) {
        if (parameter == null_target) continue;
        const auto &view = views.at(parameter);
        for (const auto target : view.targets) if (target != null_target) operation.targets.insert(target);
        operation.origins.insert(view.origins.begin(), view.origins.end());
        if (summary.returned.uninitialized.contains(parameter) && !ready_after_call.contains(parameter) &&
            !std::all_of(view.targets.begin(), view.targets.end(), [&](size_t place) {
              return place == null_target || lexical_.initialized.contains(place);
            })) operation.constrain_initialization(view.initialization);
      }
      if (!operation.initialized_target() && summary.bindings.empty()) {
        auto initialization = instantiate_initialization(summary.returned.initialization, views);
        if (initialization) {
          std::erase_if(*initialization, [&](const auto &input) {
            return input.physical ? lexical_.initialized.contains(input.id) :
                !input.frame && lexical_.selections.entry_initialized.contains(input.id);
          });
          operation.initialization = std::move(initialization);
        }
      }
      for (const auto source : summary.returned.null_sources) {
        if (source == null_target) operation.null_sources.insert(null_target);
        else if (views.at(source).targets.contains(null_target))
          operation.null_sources.insert(views[source].null_sources.begin(), views[source].null_sources.end());
      }
      if (!operation.null_sources.empty()) operation.targets.insert(null_target);
      const bool nullable_result = !contract.nullable_views.empty() && contract.nullable_views.back();
      operation.nullable_view = nullable_result;
      if (!nullable_result) operation.targets.erase(null_target);
      select_targets(operation, operation.targets);
      std::set<size_t> identities, sources;
      bool exact = !summary.returned.selection_sources.empty() && summary.bindings.empty();
      for (const auto parameter : summary.returned.selection_sources) {
        if (parameter == null_target || !summary.bindings.empty()) { exact = false; sources.insert(null_target); continue; }
        const auto identity = views.at(parameter).selection_identity;
        if (identity) identities.insert(*identity); else exact = false;
        const auto incoming = selection_sources(identity); sources.insert(incoming.begin(), incoming.end());
      }
      operation.selection_identity = exact && identities.size() == 1 ? *identities.begin() : new_selection_identity(std::move(sources));
      if (!prepared && nullable_result) operation.nullness = instantiate_boolean(summary.returned.nullness, boolean_inputs, views,
                                                                                  has_invocation_time_slots, exception_inputs);
      if (read && !discard && &expr != prepared_call_) read_view(operation, expr.at, "borrowed result");
      if (!prepared) operation.predicate = stored_predicate(operation);
    }
  }
  // Only immutable parameter evidence crosses a callable boundary. Dropping
  // local conditions weakens a necessary missing-write/read condition safely.
  CleanupEquations::ConditionClauses external_conditions(const CleanupEquations::ConditionClauses &clauses,
                                                         bool presence = false) const {
    CleanupEquations::ConditionClauses result;
    for (const auto &known : clauses) {
      CleanupEquations::ConditionFacts facts; facts.inputs = known.inputs;
      for (const auto &group : known.same_selections) {
        std::set<size_t> parameters;
        for (const auto identity : group) {
          const auto &sources = selection_sources_.at(identity);
          if (sources.size() == 1 && !sources.contains(null_target)) parameters.insert(*sources.begin());
        }
        if (parameters.size() > 1) facts.same_inputs.insert(std::move(parameters));
      }
      if (presence) facts.present = input_ordinals(known.present);
      facts.absent = input_ordinals(known.absent);
      for (const auto &value : known.exceptions) {
        ExceptionFacts retained;
        for (const auto &[origin, categories] : value.terms) {
          auto external = origin; external.conditions = {};
          if (external.kind != ExceptionFacts::Origin::Kind::input) external = {};
          retained.add(std::move(external), categories);
        }
        if (!retained.terms.contains(ExceptionFacts::Origin{})) facts.exceptions.insert(std::move(retained));
      }
      CleanupEquations::ConditionClauses projected;
      CleanupEquations::add_condition(projected, std::move(facts));
      for (const auto &[left, right] : known.different_selections) {
        const auto &lhs = selection_sources_.at(left), &rhs = selection_sources_.at(right);
        // Unknown or provisional origins cannot exclude a missing-write path.
        if (lhs.empty() || rhs.empty() || lhs.contains(null_target) || rhs.contains(null_target)) continue;
        CleanupEquations::ConditionClauses alternatives;
        for (const auto a : lhs) for (const auto b : rhs) {
          CleanupEquations::ConditionFacts different;
          different.different_inputs.emplace(a, b);
          CleanupEquations::add_condition(alternatives, std::move(different));
        }
        projected = CleanupEquations::conjoin_conditions(projected, alternatives);
      }
      for (const auto &clause : projected) CleanupEquations::add_condition(result, clause);
    }
    return result;
  }
  CleanupEquations::BooleanValue external_boolean(const CleanupEquations::BooleanValue &value) const {
    CleanupEquations::BooleanValue result;
    for (const auto &[truth, clauses] : value)
      for (const auto &facts : external_conditions(clauses, true)) CleanupEquations::add_condition(result[truth], facts);
    return result;
  }
  static CleanupEquations::ConditionFacts path_facts(const LexicalState &state) {
    CleanupEquations::ConditionFacts facts;
    facts.present = state.present;
    facts.inputs = state.boolean_inputs;
    facts.payloads = state.boolean_payloads;
    facts.conditions = state.selection_conditions;
    facts.exceptions = state.exception_conditions;
    return facts;
  }
  static CleanupEquations::ConditionClauses path_conditions(const LexicalState &state) {
    return CleanupEquations::conjoin_conditions(state.predicate_conditions, {path_facts(state)});
  }
  static CleanupEquations::ConditionClauses missing_initialization(const LexicalState &state, size_t place,
      const std::optional<CleanupEquations::ConditionFacts> &selected = {}) {
    auto missing = CleanupEquations::conjoin_conditions(path_conditions(state),
        CleanupEquations::missing_initialization(place, state.initialized, state.uninitialized));
    return selected ? CleanupEquations::conjoin_conditions(missing, {*selected}) : missing;
  }
  static bool conditional_read(const CleanupEquations::ReadRequirement &read) {
    return read.when != CleanupEquations::ConditionClauses{CleanupEquations::ConditionFacts{}};
  }
  void require_initialized(const CheckedOperation &view, const Location &at, const std::string &name) {
    if (view.initialized_target()) return;
    for (const auto place : view.targets)
      require_initialized(place, at, name, current_binding(view), selected_target(view, place, true));
  }
  void require_initialized(size_t place, const Location &at, const std::string &name, std::optional<size_t> selection = {},
      std::optional<CleanupEquations::ConditionFacts> target = {}) {
    if (place == null_target || !proving_ || lexical_.initialized.contains(place)) return;
    CompletionInputs missing;
    for (const auto input : lexical_.inputs)
      if (!lexical_.input_facts.at(input).initialized.contains(place) &&
          (!selection || !lexical_.input_facts.at(input).selections.initialized.contains(*selection))) missing.insert(input);
    if (missing.empty()) return;
    CleanupEquations::ReadRequirement before{lexical_.initialized, selection};
    before.when = missing_initialization(lexical_, place, target);
    before.uninitialized = lexical_.uninitialized;
    if (before.when.empty()) return;
    if (!storage(place).borrowed) {
      const auto diagnostic = "read of '" + name + "' before definite initialization of every possible target";
      if (!delayed_ && in_immediate_scope()) {
        require_local_read({at, diagnostic}, before.when);
        return;
      }
      if ((!delayed_ || (!deferred_inputs_.contains(place) && !conditional_read(before))) && validating_)
        fail(at, diagnostic);
    }
    const auto obligation = read_obligation(place, {at, {}});
    before.substitute(lexical_.selections);
    add_requirement(lexical_, obligation, missing, before);
    observe_read(place, before);
  }
  static void refresh_input_facts(LexicalState &state) {
    std::set<size_t> established;
    CleanupEquations::resolve_initialization(established, state.uninitialized);
    for (const auto input : state.inputs)
      state.input_facts.at(input).initialized.insert(established.begin(), established.end());
    bool first = true;
    for (const auto input : state.inputs) {
      state.input_facts.at(input).selections.resolve(state.input_facts.at(input).initialized);
      auto &selections = state.input_facts.at(input).selections;
      std::set<size_t> identities;
      for (const auto binding : selections.initialized)
        if (state.selection_identities.contains(binding)) identities.insert(state.selection_identities.at(binding));
      for (const auto &[binding, identity] : state.selection_identities)
        if (state.bindings_available.contains(binding) && identities.contains(identity)) selections.initialize(binding);
      selections.resolve(state.input_facts.at(input).initialized);
      auto &ready = state.input_facts.at(input).initialized_saved_selections;
      for (const auto identity : state.observed_saved_selections)
        if (selections.initialized_captures.contains(identity)) ready.insert(identity);
      if (!state.observed_saved_selections.empty()) for (const auto binding : selections.initialized) {
        const auto identity = state.selection_identities.find(binding);
        if (identity != state.selection_identities.end() && state.bindings_available.contains(binding) &&
            state.observed_saved_selections.contains(identity->second)) ready.insert(identity->second);
      }
      const auto &known = state.input_facts.at(input).initialized;
      state.initialized = first ? known : intersect(state.initialized, known);
      const auto &present = state.input_facts.at(input).present;
      state.present = first ? present : intersect(state.present, present);
      if (first) state.selections = state.input_facts.at(input).selections;
      else state.selections.merge(state.input_facts.at(input).selections);
      if (first) state.exceptions = state.input_facts.at(input).exceptions;
      else for (const auto &[place, value] : state.input_facts.at(input).exceptions) state.exceptions[place].merge(value);
      first = false;
    }
    state.returned.initialize(state.initialized, state.selections);
    refine_present(state);
  }
  static LexicalState remap_completion_inputs(const LexicalState &source, const std::map<size_t, size_t> &routes) {
    auto state = source;
    state.inputs.clear();
    state.input_facts.clear();
    for (const auto &[from, to] : routes) if (source.inputs.contains(from)) {
      state.inputs.insert(to);
      state.input_facts.emplace(to, source.input_facts.at(from));
    }
    for (auto &[read, requirement] : state.requirements) {
      const auto original = std::exchange(requirement.inputs, {});
      for (const auto &[from, to] : routes)
        if (source.inputs.contains(from) && original.contains(from)) requirement.inputs.insert(to);
    }
    std::erase_if(state.requirements, [](const auto &entry) { return entry.second.inputs.empty(); });
    state.live = state.live && !state.inputs.empty();
    refresh_input_facts(state);
    return state;
  }
  std::set<size_t> initialized_path_selections(const LexicalState &state, const CleanupEquations::Selections &selections) const {
    std::set<size_t> retained;
    if (proving_ && !delayed_) for (const auto binding : selections.initialized) {
      const auto identity = state.selection_identities.find(binding);
      if (identity != state.selection_identities.end() && state.bindings_available.contains(binding) && stable_loop_binding(binding))
        retained.insert(identity->second);
    }
    return retained;
  }
  void join(const LexicalState &a, bool a_exits, const LexicalState &b, bool b_exits, const Location &at) {
    a_exits = a_exits || !a.live;
    b_exits = b_exits || !b.live;
    auto left_paths = path_conditions(a), right_paths = path_conditions(b);
    if (!a_exits && !b_exits) for (const auto &[binding, versions] : a.target_versions) {
      const auto other = b.target_versions.find(binding);
      if (other == b.target_versions.end() || versions == other->second) continue;
      auto targets = a.targets.at(binding); targets.insert(b.targets.at(binding).begin(), b.targets.at(binding).end());
      if (!targets.contains(null_target) || targets.size() == 1) continue;
      auto joined = versions; joined.insert(other->second.begin(), other->second.end());
      const auto bridge = [&](auto &paths, const auto &previous) {
        CleanupEquations::ConditionFacts copies;
        copies.same_bindings.insert({{binding, joined}, {binding, previous}});
        paths = CleanupEquations::conjoin_conditions(paths, {copies});
      };
      bridge(left_paths, versions); bridge(right_paths, other->second);
    }
    lexical_ = a;
    for (size_t i = 0; i < b.scopes.size(); ++i)
      for (const auto &[name, id] : b.scopes[i].bindings) {
        auto [found, inserted] = lexical_.scopes[i].bindings.emplace(name, id);
        if (!inserted && found->second != id) fail(at, "duplicate declaration of '" + name + "' in one lexical scope");
      }
    lexical_.bindings_available = a_exits ? b.bindings_available : b_exits ? a.bindings_available : intersect(a.bindings_available, b.bindings_available);
    lexical_.available = a_exits ? b.available : b_exits ? a.available : intersect(a.available, b.available);
    lexical_.bound_symbols = a_exits ? b.bound_symbols : b_exits ? a.bound_symbols : intersect(a.bound_symbols, b.bound_symbols);
    lexical_.required_symbols.insert(b.required_symbols.begin(), b.required_symbols.end());
    lexical_.initialized = a_exits ? b.initialized : b_exits ? a.initialized : intersect(a.initialized, b.initialized);
    lexical_.returned = a_exits ? b.returned : a.returned;
    if (!a_exits && !b_exits) lexical_.returned.merge(b.returned);
    lexical_.observed_saved_selections = a_exits ? b.observed_saved_selections : b_exits ? a.observed_saved_selections :
        intersect(a.observed_saved_selections, b.observed_saved_selections);
    lexical_.predicates = a_exits ? b.predicates : a.predicates;
    lexical_.boolean_inputs = a_exits ? b.boolean_inputs : a.boolean_inputs;
    lexical_.boolean_payloads = a_exits ? b.boolean_payloads : a.boolean_payloads;
    lexical_.selection_conditions = a_exits ? b.selection_conditions : a.selection_conditions;
    lexical_.exception_conditions = a_exits ? b.exception_conditions : a.exception_conditions;
    lexical_.uninitialized = a_exits ? b.uninitialized : a.uninitialized;
    lexical_.predicate_conditions = a_exits ? right_paths : left_paths;
    if (!a_exits && !b_exits) {
      for (const auto &facts : right_paths) CleanupEquations::add_reaching_condition(lexical_.predicate_conditions, facts);
      std::set<size_t> candidates = a.initialized;
      candidates.insert(b.initialized.begin(), b.initialized.end());
      for (const auto &[place, conditions] : a.uninitialized) candidates.insert(place);
      for (const auto &[place, conditions] : b.uninitialized) candidates.insert(place);
      for (const auto place : candidates) if (lexical_.available.contains(place)) {
        auto missing = CleanupEquations::conjoin_conditions(left_paths,
            CleanupEquations::missing_initialization(place, a.initialized, a.uninitialized));
        for (const auto &facts : CleanupEquations::conjoin_conditions(right_paths,
            CleanupEquations::missing_initialization(place, b.initialized, b.uninitialized))) CleanupEquations::add_condition(missing, facts);
        lexical_.uninitialized[place] = std::move(missing);
      }
    }
    lexical_.payload_writes = a_exits ? b.payload_writes : a.payload_writes;
    if (!a_exits && !b_exits) {
      std::erase_if(lexical_.boolean_inputs, [&](const auto &input) { return !b.boolean_inputs.contains(input); });
      std::erase_if(lexical_.boolean_payloads, [&](const auto &payload) { return !b.boolean_payloads.contains(payload); });
      lexical_.selection_conditions.merge(b.selection_conditions);
      std::erase_if(lexical_.exception_conditions, [&](const auto &condition) { return !b.exception_conditions.contains(condition); });
      for (const auto &[place, proof] : b.predicates) lexical_.predicates.try_emplace(place);
      for (auto &[place, proof] : lexical_.predicates) {
        const auto left = a.predicates.find(place), right = b.predicates.find(place);
        const auto lhs = left == a.predicates.end() ? nullptr : left->second;
        const auto rhs = right == b.predicates.end() ? nullptr : right->second;
        proof = lhs == rhs ? lhs : predicate(Predicate::Kind::alternatives,
            constrain_predicate(lhs, left_paths), constrain_predicate(rhs, right_paths));
      }
      lexical_.payload_writes.insert(b.payload_writes.begin(), b.payload_writes.end());
    }
    lexical_.input_facts = a_exits ? b.input_facts : a.input_facts;
    lexical_.requirements = a_exits ? b.requirements : a.requirements;
    if (!a_exits && !b_exits) {
      for (const auto input : b.inputs) {
        auto &facts = lexical_.input_facts[input];
        const auto &other = b.input_facts.at(input);
        if (a.inputs.contains(input)) {
          for (const auto &[scope, queue] : other.queues) {
            const auto previous = facts.queues.at(scope);
            if (!previous && !queue) continue;
            auto left_ready = initialized_path_selections(a, facts.selections);
            auto right_ready = initialized_path_selections(b, other.selections);
            // Both paths can reach the same queue with different target
            // readiness, such as after an immediate fallback write.
            if (previous == queue && left_ready == right_ready) continue;
            const auto left_conditions = CleanupEquations::stable_input_conditions(left_paths);
            const auto right_conditions = CleanupEquations::stable_input_conditions(right_paths);
            if (left_ready != right_ready) {
              cleanup_.observe_input_conditions(left_conditions);
              cleanup_.observe_input_conditions(right_conditions);
            }
            facts.queues[scope] = cleanup_.join(
                cleanup_.known(previous, facts.initialized, a.uninitialized,
                               left_conditions, std::move(left_ready)),
                cleanup_.known(queue, other.initialized, b.uninitialized,
                               right_conditions, std::move(right_ready)));
          }
          facts.initialized = intersect(facts.initialized, other.initialized);
          facts.initialized_saved_selections = intersect(facts.initialized_saved_selections, other.initialized_saved_selections);
          facts.present = intersect(facts.present, other.present);
          facts.selections.merge(other.selections);
          for (const auto &[place, value] : other.exceptions) facts.exceptions[place].merge(value);
        } else facts = other;
      }
      for (const auto &[read, requirement] : b.requirements)
        add_requirement(lexical_, read, requirement.inputs, requirement.before);
    }
    lexical_.targets = a_exits ? b.targets : a.targets;
    lexical_.null_sources = a_exits ? b.null_sources : a.null_sources;
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
    lexical_.selection_identities = a_exits ? b.selection_identities : a.selection_identities;
    if (!a_exits && !b_exits) {
      std::map<std::pair<size_t, size_t>, size_t> joined;
      const auto equivalent = [](size_t left, size_t right, const auto &paths) {
        return !paths.empty() && std::all_of(paths.begin(), paths.end(), [&](const auto &facts) {
          return facts.same_selection(left, right);
        });
      };
      for (const auto &[binding, right] : b.selection_identities) {
        const auto left = a.selection_identities.find(binding);
        if (left == a.selection_identities.end()) { lexical_.selection_identities[binding] = new_selection_identity(); continue; }
        if (left->second == right) continue;
        if (equivalent(left->second, right, left_paths)) { lexical_.selection_identities[binding] = right; continue; }
        if (equivalent(left->second, right, right_paths)) continue;
        const auto key = std::pair{left->second, right};
        if (!joined.contains(key)) joined[key] = merge_selection_identities(left->second, right);
        lexical_.selection_identities[binding] = joined.at(key);
      }
    }
    if (!a_exits && !b_exits) for (const auto &[id, targets] : b.targets) {
      lexical_.targets[id].insert(targets.begin(), targets.end());
      const auto &nulls = b.null_sources.at(id);
      lexical_.null_sources[id].insert(nulls.begin(), nulls.end());
      const auto &versions = b.target_versions.at(id);
      lexical_.target_versions[id].insert(versions.begin(), versions.end());
    }
    lexical_.inputs = a_exits ? b.inputs : a.inputs;
    if (!a_exits && !b_exits) lexical_.inputs.insert(b.inputs.begin(), b.inputs.end());
    lexical_.live = !a_exits || !b_exits;
    refresh_input_facts(lexical_);
  }
  void retire_scope(LexicalState &state) {
    const auto &scope = state.scopes.back();
    if (scope.symbol_entry) state.bound_symbols = *scope.symbol_entry;
    for (const auto &[name, id] : scope.bindings) {
      state.bindings_available.erase(id);
      state.returned.selection_bindings.erase(id);
      if (!bindings_->at(id).selection.empty()) continue;
      state.targets.erase(id);
      state.null_sources.erase(id);
      state.owners.erase(id);
      state.expired_bindings.erase(id);
      state.target_versions.erase(id);
      state.selection_identities.erase(id);
      state.selections.forget_initialization(id);
      state.selections.entry_initialized.erase(id);
      state.selections.entry_nonnull.erase(id);
      state.selections.null_inputs.erase(id);
      state.selections.nonnull_inputs.erase(id);
      state.selections.targets.erase(id);
      for (auto &[input, facts] : state.input_facts) {
        auto &selections = facts.selections;
        selections.forget_initialization(id); selections.entry_initialized.erase(id);
        selections.entry_nonnull.erase(id); selections.targets.erase(id);
        selections.null_inputs.erase(id); selections.nonnull_inputs.erase(id);
      }
    }
    for (const auto &[id, owners] : state.owners) if (owners.contains(scope.id)) state.expired_bindings.insert(id);
    if (const auto owned = scope_storage_.find(scope.id); owned != scope_storage_.end()) for (const auto id : owned->second) {
      state.available.erase(id);
      state.initialized.erase(id);
      state.uninitialized.erase(id);
      state.present.erase(id);
      state.selections.forget_initialization(id, true);
      std::erase_if(state.selections.targets, [&](const auto &entry) { return entry.second.places.contains(id); });
      state.exceptions.erase(id);
      state.predicates.erase(id);
      for (auto &[input, facts] : state.input_facts) {
        facts.initialized.erase(id);
        facts.present.erase(id);
        facts.selections.forget_initialization(id, true);
        std::erase_if(facts.selections.targets, [&](const auto &entry) { return entry.second.places.contains(id); });
        facts.exceptions.erase(id);
      }
    }
    for (auto &[input, facts] : state.input_facts) facts.queues.erase(scope.id);
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
  void note_abrupt(const Location &at, ExceptionFacts exceptions = {}, Completion completion = {2, false},
                   bool immediate_scope_failure = false, std::optional<HereExit> here_exit = {},
                   bool receiver_transfer_pending = false) {
    if (!lexical_.live) return;
    const auto categories = exceptions.possibilities();
    if (!categories.empty()) completion.reason = 3 + *categories.begin();
    if (!abrupt_) {
      if (!proving_) return;
      if (here_exit == HereExit::return_supplier) observed_.returns_from_supplier = true;
      if (here_exit == HereExit::break_call) observed_.breaks_here_call = true;
      for (const auto category : categories) if (exception_categories.at(category).checked) {
        if (checking_method_ && (checking_method_->source->implicit_scope || checking_method_->here_group))
          observed_.scope_checked_failures.insert(category);
        else if (validating_ && (!checking_method_ || !checking_method_->throws_categories.contains(category)))
          fail(at, "checked exception '" + std::string(exception_categories[category].name) + "' requires a covering catch or throws declaration");
      }
      return;
    }
    auto state = lexical_;
    while (state.scopes.size() > abrupt_->depth) retire_scope(state);
    abrupt_->paths.push_back({std::move(state), std::move(exceptions), at, completion, immediate_scope_failure, here_exit,
                             receiver_transfer_pending});
  }
  void forward_abrupt(const AbruptPath &path) {
    note_abrupt(path.at, path.exceptions, path.completion, path.immediate_scope_failure, path.here_exit,
                path.receiver_transfer_pending);
  }
  static bool declares_root(const std::vector<Type> &types) {
    return std::any_of(types.begin(), types.end(), [](const Type &type) { return type.kind == Type::exception && !type.identity; });
  }
  size_t declared_requirement(const Expression &expr) {
    const auto [found, inserted] = declared_requirements_.try_emplace(&expr, declared_sites_.size());
    if (inserted) declared_sites_.push_back(expr.at);
    return found->second;
  }
  void note_declared(size_t requirement) {
    if (abrupt_) { abrupt_->declared.insert(requirement); return; }
    if (!proving_) return;
    if (checking_method_ && (checking_method_->source->implicit_scope || checking_method_->here_group)) {
      observed_.scope_checked_failures.insert(0);
      return;
    }
    if (!validating_) return;
    if (checking_method_) {
      auto types = checking_method_->throws_types;
      if (checking_instance_) for (auto &type : types) type = substitute(type, *checking_method_, checking_instance_->substitutions);
      if (declares_root(types)) return;
    }
    fail(declared_sites_.at(requirement), "declared checked exception 'Exception' requires a covering catch or throws declaration; coverage by narrower catches is unproved");
  }
  void refine_test(const CheckedOperation::NullTest &test, bool value) {
    if (!lexical_.targets.contains(test.binding) || lexical_.target_versions.at(test.binding) != test.versions) return;
    auto &targets = lexical_.targets.at(test.binding);
    const bool null = value == test.null_when_true;
    CleanupEquations::ConditionFacts tested;
    (null ? tested.absent_bindings : tested.bindings).emplace(test.binding, test.versions);
    if (const auto condition = deferred_selection_condition(test.binding, null_target, null))
      tested.conditions.add(*condition);
    lexical_.predicate_conditions = CleanupEquations::conjoin_conditions(lexical_.predicate_conditions, {tested});
    if (null) lexical_.predicate_conditions = CleanupEquations::conjoin_conditions(lexical_.predicate_conditions,
        absence_conditions(lexical_.null_sources.at(test.binding)));
    std::erase_if(targets, [&](size_t place) { return (place == null_target) != null; });
    lexical_.selections.test(test.binding, null);
    for (const auto input : lexical_.inputs) lexical_.input_facts.at(input).selections.test(test.binding, null);
    if (null) record_return_readiness(lexical_, test.binding);
    if (!null) lexical_.null_sources.at(test.binding).clear();
    lexical_.live = lexical_.live && !targets.empty() && lexical_.selections.possible;
    if (!null) record_presence(targets);
    lexical_.live = lexical_.live && !path_conditions(lexical_).empty();
  }
  void refine_known_bindings() {
    const auto paths = path_conditions(lexical_);
    if (paths.empty()) { lexical_.live = false; return; }
    auto present = paths.begin()->bindings, absent = paths.begin()->absent_bindings;
    for (const auto &facts : paths) {
      std::erase_if(present, [&](const auto &binding) { return !facts.bindings.contains(binding); });
      std::erase_if(absent, [&](const auto &binding) { return !facts.absent_bindings.contains(binding); });
    }
    for (const auto &[binding, versions] : present) refine_test({binding, versions}, false);
    for (const auto &[binding, versions] : absent) refine_test({binding, versions}, true);
    auto equal = paths.begin()->same_selections;
    for (const auto &facts : paths) {
      std::set<std::set<size_t>> shared;
      for (const auto &group : equal) for (const auto &other : facts.same_selections) {
        auto common = intersect(group, other);
        if (common.size() > 1) shared.insert(std::move(common));
      }
      equal = std::move(shared);
    }
    for (const auto &group : equal)
      for (auto &[binding, identity] : lexical_.selection_identities)
        if (group.contains(identity)) identity = *group.begin();
    if (!equal.empty()) refresh_input_facts(lexical_);
  }
  using Predicate = CheckedOperation::Predicate;
  static std::shared_ptr<const Predicate> constant_predicate(bool value) {
    static const auto yes = std::make_shared<const Predicate>(Predicate{Predicate::Kind::constant_true, {}, {}, {}});
    static const auto no = std::make_shared<const Predicate>(Predicate{Predicate::Kind::constant_false, {}, {}, {}});
    return value ? yes : no;
  }
  static std::shared_ptr<const Predicate> predicate_of(const CheckedOperation &operation) {
    if (operation.type == Type::never) {
      static const auto abrupt = std::make_shared<const Predicate>(Predicate{Predicate::Kind::abrupt, {}, {}, {}});
      return abrupt;
    }
    return operation.predicate;
  }
  static std::optional<BooleanDecision> boolean_decision(const Predicate *proof) {
    if (!proof) return {};
    if (proof->storage_observation) return BooleanDecision{BooleanDecision::Origin::storage, *proof->storage_observation, true};
    if (proof->kind == Predicate::Kind::negation) {
      if (auto decision = boolean_decision(proof->left.get())) {
        decision->value_when_true = !decision->value_when_true;
        return decision;
      }
    }
    const auto yes = proof->results.find(true), no = proof->results.find(false);
    if (yes == proof->results.end() || no == proof->results.end() || yes->second.empty() || no->second.empty()) return {};
    const auto entails = [](const CleanupEquations::ConditionClauses &paths, size_t input, bool value) {
      return std::all_of(paths.begin(), paths.end(), [&](const auto &path) { return path.inputs.contains({input, value}); });
    };
    // Both outcomes must identify the same immutable input with opposite truth.
    for (const auto &[input, value] : yes->second.begin()->inputs)
      if (entails(yes->second, input, value) && entails(no->second, input, !value))
        return BooleanDecision{BooleanDecision::Origin::input, input, value};
    return {};
  }
  size_t conditional_selection(const CheckedOperation &condition, const CheckedOperation &yes, const CheckedOperation &no) {
    auto when_true = yes.selection_identity, when_false = no.selection_identity;
    if (when_true && when_false && when_true != when_false && yes.view && no.view) {
      if (const auto decision = boolean_decision(condition.predicate.get())) {
        if (!decision->value_when_true) std::swap(when_true, when_false);
        const auto [entry, inserted] = conditional_selections_.try_emplace(
            {decision->origin, decision->id, *when_true, *when_false}, 0);
        if (inserted) entry->second = merge_selection_identities(when_true, when_false);
        return entry->second;
      }
    }
    return merge_selection_identities(when_true, when_false);
  }
  static std::shared_ptr<const Predicate> predicate(Predicate::Kind kind,
      std::shared_ptr<const Predicate> left, std::shared_ptr<const Predicate> right = {}) {
    if (!left && !right) return {};
    if (kind == Predicate::Kind::alternatives && left == right) return left;
    const auto constant = [](const std::shared_ptr<const Predicate> &proof) -> std::optional<bool> {
      if (proof && proof->kind == Predicate::Kind::constant_true) return true;
      if (proof && proof->kind == Predicate::Kind::constant_false) return false;
      return {};
    };
    if (kind == Predicate::Kind::negation) {
      if (const auto value = constant(left)) return constant_predicate(!*value);
      if (left->kind == Predicate::Kind::negation && !left->storage_observation) return left->left;
    }
    if (kind == Predicate::Kind::conjunction || kind == Predicate::Kind::disjunction) {
      const bool conjunction = kind == Predicate::Kind::conjunction;
      if (const auto value = constant(left)) return *value == conjunction ? right : left;
      if (const auto value = constant(right)) return *value == conjunction ? left : right;
    }
    return std::make_shared<const Predicate>(Predicate{kind, {}, std::move(left), std::move(right)});
  }
  std::shared_ptr<const Predicate> null_predicate(const CheckedOperation &view) const {
    if (view.targets.empty()) return view.nullness ? boolean_predicate(*view.nullness) : nullptr;
    if (!view.targets.contains(null_target)) return constant_predicate(false);
    if (view.targets.size() == 1) return constant_predicate(true);
    if (view.nullness) return boolean_predicate(*view.nullness);
    auto proof = std::make_shared<Predicate>();
    proof->kind = view.binding ? Predicate::Kind::null_test : Predicate::Kind::presence;
    if (view.binding) proof->test = {*view.binding, view.selection_versions};
    proof->results[false].insert({selected_input(view.targets), {}});
    return proof;
  }
  std::shared_ptr<const Predicate> identity_predicate(const CheckedOperation &left, const CheckedOperation &right) const {
    const auto owners = compare_owners(one_owner(left), one_owner(right));
    if (owners == OwnerMatch::mismatch) return constant_predicate(false);
    const auto lhs_null = null_predicate(left), rhs_null = null_predicate(right);
    const auto nulls = predicate(Predicate::Kind::conjunction, lhs_null, rhs_null);
    const auto nonnulls = predicate(Predicate::Kind::conjunction,
        predicate(Predicate::Kind::negation, lhs_null), predicate(Predicate::Kind::negation, rhs_null));
    // The comparison observes immutable selections, including old values whose
    // source bindings were changed while preparing the other operand.
    auto relation = predicate(Predicate::Kind::disjunction,
        predicate(Predicate::Kind::conjunction, nulls,
            owners == OwnerMatch::satisfied ? constant_predicate(true) : nullptr),
        predicate(Predicate::Kind::conjunction, nonnulls, {}));
    if (!left.selection_identity || !right.selection_identity) return relation;
    const auto bounds = CleanupEquations::conjoin_conditions(selection_bounds(left), selection_bounds(right));
    CleanupEquations::BooleanValue observations;
    for (const bool equal : {false, true}) {
      CleanupEquations::ConditionFacts observed;
      if (equal) observed.same_selections.insert({*left.selection_identity, *right.selection_identity});
      else observed.different_selections.emplace(*left.selection_identity, *right.selection_identity);
      CleanupEquations::add_condition(observations[equal], std::move(observed));
      observations[equal] = CleanupEquations::conjoin_conditions(observations[equal], bounds);
    }
    return constrain_predicate(relation, observations);
  }
  void predicate_paths(const std::shared_ptr<const Predicate> &proof, bool value, const Location &at,
      const std::function<void()> &next) {
    if (!lexical_.live) return;
    const auto before = lexical_;
    const auto visit = [&] {
      if (!lexical_.live) return;
      if (!proof || proof->kind == Predicate::Kind::presence) { next(); return; }
      if (proof->kind == Predicate::Kind::abrupt) return;
      if (proof->kind == Predicate::Kind::constant_true || proof->kind == Predicate::Kind::constant_false) {
        if (value == (proof->kind == Predicate::Kind::constant_true)) next();
        return;
      }
      if (proof->kind == Predicate::Kind::null_test) {
        refine_test(proof->test, value); if (lexical_.live) next(); return;
      }
      if (proof->kind == Predicate::Kind::negation) { predicate_paths(proof->left, !value, at, next); return; }
      if (proof->kind == Predicate::Kind::alternatives) {
        predicate_paths(proof->left, value, at, next); predicate_paths(proof->right, value, at, next); return;
      }
      if (value == (proof->kind == Predicate::Kind::conjunction)) {
        predicate_paths(proof->left, value, at, [&] { predicate_paths(proof->right, value, at, next); });
      } else {
        predicate_paths(proof->left, value, at, next);
        predicate_paths(proof->left, !value, at, [&] { predicate_paths(proof->right, value, at, next); });
      }
    };
    const auto found = proof ? proof->results.find(value) : CleanupEquations::BooleanValue::const_iterator{};
    if (!proof) visit();
    else if (found == proof->results.end() || found->second.empty()) { if (!proof->complete_results) visit(); }
    else for (const auto &facts : found->second) {
      lexical_ = before;
      record_present(facts.present);
      lexical_.boolean_inputs.insert(facts.inputs.begin(), facts.inputs.end());
      lexical_.boolean_payloads.insert(facts.payloads.begin(), facts.payloads.end());
      lexical_.selection_conditions.append(facts.conditions);
      lexical_.exception_conditions.insert(facts.exceptions.begin(), facts.exceptions.end());
      lexical_.predicate_conditions = CleanupEquations::conjoin_conditions(lexical_.predicate_conditions, {facts});
      lexical_.live = lexical_.live && lexical_.selection_conditions.possible;
      for (const auto &[binding, versions] : facts.bindings) refine_test({binding, versions}, false);
      for (const auto &[binding, versions] : facts.absent_bindings) refine_test({binding, versions}, true);
      lexical_.live = lexical_.live && !path_conditions(lexical_).empty();
      visit();
    }
    lexical_ = before;
  }
  void refine_predicate(const std::shared_ptr<const Predicate> &proof, bool value, const Location &at) {
    std::optional<LexicalState> joined;
    CleanupEquations::ConditionClauses conditions;
    predicate_paths(proof, value, at, [&] {
      for (const auto &facts : path_conditions(lexical_)) CleanupEquations::add_condition(conditions, facts);
      if (joined) { const auto path = lexical_; join(*joined, false, path, false, at); }
      joined = lexical_;
    });
    if (joined) {
      lexical_ = std::move(*joined);
      lexical_.predicate_conditions = std::move(conditions);
      refine_known_bindings();
    } else {
      lexical_.live = false;
      lexical_.predicate_conditions.clear();
    }
  }
  void refine_return_input(bool present) {
    lexical_.inputs = intersect(lexical_.inputs, present ? return_inputs_.present : return_inputs_.absent);
    lexical_.live = lexical_.live && !lexical_.inputs.empty();
    refresh_input_facts(lexical_);
  }
  void refine(const Expression &expr, bool value, const std::map<const Expression *, CheckedOperation> &operations) {
    if (expr.kind == Expression::Kind::return_test) {
      const auto &filter = operations.at(&expr).return_filter;
      if (!filter.can_match) {
        if (value) lexical_.live = false;
      } else if (value || !filter.owner_comparison) refine_return_input(value);
      return;
    }
    const auto proof = predicate_of(operations.at(&expr));
    if (!proof) return;
    // Related tests can constrain one another in either source order. Each
    // pass only removes possible targets, so closure is finite.
    while (lexical_.live) {
      const auto before = lexical_.targets;
      refine_predicate(proof, value, expr.at);
      if (lexical_.targets == before) break;
    }
  }
  void dereference(const CheckedOperation &view, const Location &at) {
    if (view.targets.contains(null_target)) {
      const auto before = lexical_;
      if (view.binding) refine_test({*view.binding, view.selection_versions}, true);
      note_abrupt(at, {exception_id(ExceptionCategory::null_pointer)});
      lexical_ = before;
      if (view.binding) refine_test({*view.binding, view.selection_versions}, false);
      else if (view.targets.size() == 1) lexical_.live = false;
    }
    record_presence(view.targets);
    refine_known_bindings();
  }
  CheckedOperation finish_scope(CheckedOperation result, AbruptFlow &flow, AbruptFlow *outer,
                                size_t scope, const Location &at) {
    const auto body = lexical_;
    if (result.type != Type::never && lexical_.live) flow.paths.push_back({lexical_, {}, at, {0, false}});
    std::vector<LexicalState> normals;
    std::vector<AbruptPath> exits;
    auto declared = flow.declared;
    std::optional<Type> normal_type;
    for (const auto &path : flow.paths) for (const auto origin : path.state.inputs) {
      const auto queue = path.state.input_facts.at(origin).queues.at(scope);
      if (proving_ && checking_method_ && checking_method_->here && !observed_.protected_defer_failure && cleanup_.has_actions(queue)) {
        const auto &outcomes = cleanup_.apply(queue, Completion{});
        observed_.protected_defer_failure = std::ranges::any_of(outcomes, [](const auto &outcome) {
          return Completion::from(outcome.first).failed();
        });
      }
      std::vector<Completion> inputs;
      if (path.exceptions.empty()) inputs.push_back(path.completion);
      else for (const auto category : path.exceptions.possibilities()) inputs.push_back({3 + category, path.completion.returned});
      for (const auto input : inputs) for (const auto &[encoded, effects] : cleanup_.apply(queue, input)) {
        declared.insert(effects.declared_failures.begin(), effects.declared_failures.end());
        const auto completion = Completion::from(encoded);
        auto state = path.state;
        state.inputs = {origin};
        state.initialized = path.state.input_facts.at(origin).initialized;
        state.selections = path.state.input_facts.at(origin).selections;
        state.exceptions = path.state.input_facts.at(origin).exceptions;
        for (auto &[read, requirement] : state.requirements)
          requirement.inputs = intersect(requirement.inputs, state.inputs);
        auto pending = input.exception() ? path.exceptions.restricted({input.reason - 3}) : ExceptionFacts{};
        if (input.exception() && pending.empty()) pending = {input.reason - 3};
        auto payloads = state.exceptions;
        // Empty provisional value summaries do not establish non-returning
        // behavior. Only a substituted, explicit failure relation can exclude
        // an exceptional outcome.
        for (auto &[place, value] : payloads)
          if (value.empty()) value = exception_possibilities(storage(place).type);
        auto applied = effects;
        const auto observed_selection = cleanup_.project_observed_return(applied, state.returned, state.selections);
        applied.resolve_conditions([&](size_t binding) -> std::optional<CleanupEquations::Targets> {
          if (binding == CleanupEquations::pending_return_selection) return observed_selection;
          if (deferred_action_) return state.selections.substitute({{}, {}, {binding}});
          const auto found = state.targets.find(binding);
          if (found == state.targets.end()) return {};
          auto selected = selection_targets(found->second, state.null_sources.at(binding));
          if (const auto recorded = state.selections.targets.find(binding);
              recorded != state.selections.targets.end() && recorded->second.entries.empty())
            selected.nullness = recorded->second.nullness;
          if (selected.nullable() && !selected.places.empty()) {
            // A cleanup condition observes the selection at the drain entry.
            // Relate that observation to other snapshots before applying writes.
            CleanupEquations::BooleanValue observed;
            const CleanupEquations::BindingSnapshot snapshot{binding, state.target_versions.at(binding)};
            for (const auto &[null, paths] : selected.null_observations()) for (auto facts : paths) {
              (null ? facts.absent_bindings : facts.bindings).insert(snapshot);
              CleanupEquations::add_condition(observed[null], std::move(facts));
            }
            selected.nullness = std::move(observed);
          }
          return selected;
        }, deferred_action_);
        applied.transform_values([&](const auto &value) { return value.through(payloads, pending, true); },
            [&](const auto &value) { return value.through(payloads, pending, true, true); });
        CleanupEquations::BooleanValues booleans;
        const auto resolve_boolean = [&](size_t place) -> const CleanupEquations::BooleanValue * {
          if (!state.available.contains(place) || storage(place).type != Type::boolean) return nullptr;
          const auto [found, inserted] = booleans.try_emplace(place);
          if (inserted) {
            const auto proof = state.predicates.find(place);
            found->second = snapshot_boolean(proof == state.predicates.end() ? nullptr : proof->second, state, at);
          }
          return &found->second;
        };
        if (effects.declared_failures.empty() && !CleanupEquations::unconditional(effects.action_reaching)) {
          const auto reaching = CleanupEquations::substitute_boolean_payloads(effects.action_reaching, resolve_boolean, false, true);
          if (CleanupEquations::conjoin_conditions(reaching, CleanupEquations::boolean_conditions(path_conditions(state))).empty()) continue;
        }
        const bool needs_deferred_delivery = effects.raises_deferred_failure ||
            (input.failed() && effects.guarded_pending_failure);
        if (proving_) observed_.requires_deferred_failure_delivery |= needs_deferred_delivery;
        if (proving_ && validating_) {
          if (effects.observes_return && needs_deferred_delivery)
            implementation_gap(path.at, "return inspection requires independent deferred failure delivery on this path");
          const bool checked_body_failure = input.exception() &&
              (exception_categories.at(input.reason - 3).checked || !flow.declared.empty());
          const bool guards_checked_body = effects.guarded_pending_failure &&
              (path.immediate_scope_failure || (checked_body_failure && checking_method_ && checking_method_->source->implicit_scope));
          if (effects.needs_checked_cleanup_proof || guards_checked_body)
            implementation_gap(path.at, "checked failures involving deferred execution require coverage within the owning pass");
        }
        applied.transform_conditions([&](const auto &clauses) {
          return CleanupEquations::substitute_boolean_payloads(clauses, resolve_boolean, true);
        });
        auto failure = completion.exception() ? applied.thrown.restricted({completion.reason - 3}) : ExceptionFacts{};
        if (completion.exception() && failure.empty()) {
          if (!effects.unknown_thrown) continue;
          failure = {completion.reason - 3};
        }
        apply_effects(state, applied, at);
        state.input_facts = {{origin, state.input_facts.at(origin)}};
        if (cleanup_.has_actions(queue)) {
          refresh_targets(state, false);
          if (!state.live) continue;
        }
        state.input_facts.at(origin).queues[scope] = 0;
        if (!completion.reason) {
          const auto type = path.completion.reason ? Type{Type::void_type} : result.type;
          if (normal_type && *normal_type != type)
            fail(at, "scope recovery has no original value; use an explicit return in the recovering handler");
          normal_type = type;
          normals.push_back(std::move(state));
        } else {
          const bool retained = path.receiver_transfer_pending || (input.protected_here() && checking_method_ && checking_method_->here);
          exits.push_back({std::move(state), std::move(failure), path.at, completion, path.immediate_scope_failure,
                           completion.protected_here() ? path.here_exit : std::nullopt,
                           completion.failed() && retained});
        }
      }
    }
    abrupt_ = outer;
    for (const auto requirement : declared) note_declared(requirement);
    for (const auto &path : exits) {
      lexical_ = path.state;
      forward_abrupt(path);
    }
    lexical_ = body;
    if (normals.empty()) { lexical_.live = false; result.type = Type::never; }
    else {
      join(body, true, normals.front(), false, at);
      for (size_t i = 1; i < normals.size(); ++i) { const auto previous = lexical_; join(previous, false, normals[i], false, at); }
      result.type = *normal_type;
      if (result.type == Type::void_type) {
        result.place.reset(); result.binding.reset(); result.slots.clear(); result.targets.clear(); result.origins.clear(); result.null_sources.clear();
        result.view = result.read_only = false; result.exceptions.clear();
        result.predicate.reset();
        result.nullness.reset();
        result.selection_identity.reset();
      }
    }
    return result;
  }
  static std::optional<std::uint64_t> constant_integer(const Expression &expr,
      const std::map<const Expression *, CheckedOperation> &operations) {
    const auto &fact = operations.at(&expr);
    if (expr.kind == Expression::Kind::integer) return fact.integer;
    if (fact.minimum_literal) return std::uint64_t{1} << 63;
    if (fact.builtin == BuiltinOperation::unary && (fact.builtin_name == "+" || fact.builtin_name == "-"))
      if (const auto value = constant_integer(*expr.children.front(), operations))
        return fact.builtin_name == "-" ? std::uint64_t{0} - *value : *value;
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
    const ModuleLookup lookup(module_, method ? method->module : module_);
    if (method) if (const auto index = binder(*method, type)) {
      if (!bindings) return type;
      if (const auto &bound = (*bindings)[*index]) return *bound;
      if (partial) return type;
      implementation_gap(method->source->at, "type parameter is not determined by the arguments");
    }
    if (type.kind == Type::named) if (const auto found = module_names().exception_types.find(type.name); found != module_names().exception_types.end()) return found->second;
    if (type.kind == Type::procedure) {
      for (auto &parameter : type.parameters) parameter = resolve_type(std::move(parameter), method, bindings, partial);
      // Throws declarations are resolved independently of argument inference.
      for (auto &failure : type.throws_types) failure = resolve_type(std::move(failure), method);
      if (type.target) type.target = std::make_shared<Type>(resolve_type(*type.target, method, bindings, partial));
      // Independent results have a passing mode, not a target-access qualifier.
      if (type.value_result &&
          type.const_views.size() > type.parameters.size()) {
        type.const_views.back() = false;
        if (std::none_of(type.const_views.begin(), type.const_views.end(), [](bool value) { return value; })) type.const_views.clear();
      }
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
    const auto parameter_types = [&](auto &&self, const Parameter &parameter) -> void {
      type(parameter.type);
      if (!parameter.here) return;
      const auto &contract = *parameter.here;
      for (const auto &input : contract.parameters) self(self, input);
      type(contract.result);
      for (const auto &failure : contract.throws_types) type(failure);
      if (contract.feedback_fields) for (const auto &field : *contract.feedback_fields) type(field.type);
    };
    for (const auto &p : method.parameters) parameter_types(parameter_types, p);
    if (method.result_type) type(*method.result_type);
    for (const auto &t : method.throws_types) type(t);
    const std::function<void(const Expression &)> visit = [&](const Expression &expr) {
      if (expr.annotation) type(*expr.annotation);
      if (expr.receiving) for (const auto &item : expr.receiving->items)
        if (item.annotation) type(*item.annotation);
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

  const FreeBindings &free_bindings(const Form &method) const {
    if (const auto found = free_bindings_.find(&method); found != free_bindings_.end()) return found->second;
    FreeBindings required;
    std::set<std::string> names;
    for (const auto &p : method.parameters) names.insert(p.name);
    for (const auto &p : method.owner_parameters) names.insert(p.name);
    required.declarations = names;
    if (!method.name.empty()) names.insert(method.name);
    const auto use = [&](const std::string &name, const Location &at, const std::set<std::string> &bound, bool query, bool symbol = false) {
      auto &dependencies = symbol ? required.symbols : query ? required.queries : required.captures;
      if (!bound.contains(name) && std::none_of(dependencies.begin(), dependencies.end(), [&](const auto &c) { return c.name == name; }))
        dependencies.push_back({name, at});
    };
    const std::function<void(const Expression &, std::set<std::string> &, bool)> visit = [&](const Expression &expr, auto &bound, bool query) {
      using K = Expression::Kind;
      if (!query) switch (expr.kind) {
      case K::call: case K::unary: case K::behave: case K::perform:
        required.runtime_calls.push_back(&expr);
        break;
      case K::binary:
        if (expr.text != "&&" && expr.text != "||") required.runtime_calls.push_back(&expr);
        break;
      case K::value_call: case K::symbol: case K::effect: case K::touch: case K::procedure_effect:
      case K::effect_group: case K::allocate: case K::make_region: case K::region_binding: case K::capture_owner:
        required.opaque_storage = true;
        break;
      default: break;
      }
      if (expr.definition) {
        const auto &nested = *expr.definition;
        const auto &dependencies = free_bindings(nested);
        required.declarations.insert(dependencies.declarations.begin(), dependencies.declarations.end());
        if (!nested.name.empty()) required.declarations.insert(nested.name);
        for (const auto &symbol : dependencies.symbols) use(symbol.name, symbol.at, bound, query, true);
        if (query) {
          if (nested.implicit_scope) {
            for (const auto &capture : dependencies.captures) use(capture.name, capture.at, bound, true);
            for (const auto &reference : dependencies.queries) use(reference.name, reference.at, bound, true);
          }
          return;
        }
        for (const auto &capture : dependencies.captures) use(capture.name, capture.at, bound, false);
        for (const auto &capture : dependencies.queries) use(capture.name, capture.at, bound, true);
        for (const auto &capture : nested.captures) use(capture.name, capture.at, bound, false);
        if (!nested.name.empty()) bound.insert(nested.name);
        return;
      }
      if (expr.kind == K::pointer_group) {
        for (const auto &declaration : expr.children) {
          if (declaration->kind == K::block) { visit(*declaration, bound, query); continue; }
          if (method.implicit_scope && !declaration->owner_name.empty() && declaration->owner_name != "here")
            use(declaration->owner_name, declaration->at, bound, query);
          for (const auto &child : declaration->children) visit(*child, bound, query);
          bound.insert(declaration->text);
          required.declarations.insert(declaration->text);
        }
        return;
      }
      if (expr.kind == K::alias) {
        for (const auto &child : expr.children) visit(*child, bound, query);
        if (!expr.owner_name.empty() && expr.owner_name != "here") use(expr.owner_name, expr.at, bound, query);
        bound.insert(expr.text);
        required.declarations.insert(expr.text);
        return;
      }
      if (expr.kind == K::make_local && !expr.annotation) {
        auto queried = bound; visit(*expr.children.front(), queried, true);
        for (size_t i = 1; i < expr.children.size(); ++i) visit(*expr.children[i], bound, query);
        return;
      }
      if (expr.kind == K::conditional) {
        const auto &condition = *expr.children[0];
        visit(condition, bound, query);
        auto otherwise = bound;
        const bool pattern = condition.kind == K::return_test && !condition.binding.empty();
        const bool shadowed = pattern && bound.contains(condition.binding);
        if (pattern) {
          bound.insert(condition.binding);
          required.declarations.insert(condition.binding);
        }
        visit(*expr.children[1], bound, query);
        if (pattern && !shadowed) bound.erase(condition.binding);
        visit(*expr.children[2], otherwise, query);
        // Both arms start with the same bindings. After the join, declarations
        // are syntactically visible; ordered checking proves availability.
        bound.merge(otherwise);
        return;
      }
      if (expr.kind == K::range_for || expr.kind == K::fold) {
        const bool fold = expr.kind == K::fold;
        for (size_t i = 0; i < (fold ? 3 : 2); ++i) visit(*expr.children[i], bound, query);
        auto iteration = bound; iteration.insert(expr.text);
        required.declarations.insert(expr.text);
        if (fold) { iteration.insert(expr.binding); required.declarations.insert(expr.binding); }
        visit(*expr.children[fold ? 3 : 2], iteration, query);
        if (!fold && expr.children.size() == 4) visit(*expr.children[3], bound, query);
        return;
      }
      if (expr.kind == K::handler) {
        auto caught = bound; caught.insert(expr.text);
        required.declarations.insert(expr.text);
        for (const auto &child : expr.children) visit(*child, caught, query);
        return;
      }
      if ((expr.kind == K::null_ref || expr.kind == K::return_test) && !expr.owner_name.empty() && expr.owner_name != "here")
        use(expr.owner_name, expr.at, bound, query);
      if (expr.kind == K::symbol || expr.kind == K::effect || expr.kind == K::touch || expr.kind == K::perform || expr.kind == K::behave)
        use(expr.text, expr.at, bound, query, true);
      if (expr.kind == K::name || (expr.kind == K::call && !expr.intrinsic && !swap_operation(expr.text) &&
          !identity_operation(expr.text) && !eager_operation(expr.text)))
        use(expr.text, expr.at, bound, query);
      for (const auto &child : expr.children) visit(*child, bound, query);
    };
    visit(*method.body, names, false);
    return free_bindings_.emplace(&method, std::move(required)).first->second;
  }

  const std::set<std::string> &definition_dependencies(const Form &method) const {
    if (const auto found = definition_dependencies_.find(&method); found != definition_dependencies_.end()) return found->second;
    const auto &free = free_bindings(method);
    std::set<std::string> names;
    for (const auto *dependencies : {&free.captures, &free.queries, &free.symbols, &method.captures})
      for (const auto &dependency : *dependencies) names.insert(dependency.name);
    return definition_dependencies_.emplace(&method, std::move(names)).first->second;
  }

  static TypeEvidenceState project_evidence(const TypeEvidenceState &source, const std::set<std::string> &names) {
    TypeEvidenceState state;
    state.complete = source.complete; state.required = source.required;
    state.method = source.method; state.substitutions = source.substitutions; state.retained = source.retained;
    state.declaration = source.declaration; state.full_scope_owner = source.full_scope_owner;
    const auto select = [&](const auto &from, auto &to, const std::string &name) {
      if (const auto found = from.find(name); found != from.end()) to.emplace(*found);
    };
    std::set<EvidenceOwner> owners;
    for (const auto &name : names) {
      select(source.bindings, state.bindings, name);
      select(source.definitions, state.definitions, name);
      select(source.views, state.views, name);
      select(source.owner_bindings, state.owner_bindings, name);
      if (source.symbol_shadows.contains(name)) state.symbol_shadows.insert(name);
      if (const auto view = state.views.find(name); view != state.views.end() && view->second.owner) owners.insert(*view->second.owner);
      if (const auto owner = state.owner_bindings.find(name); owner != state.owner_bindings.end()) owners.insert(owner->second);
    }
    for (const auto owner : owners) {
      if (source.external_owners.contains(owner)) state.external_owners.insert(owner);
      for (const auto other : owners)
        if (const auto found = source.owner_relations.find({owner, other}); found != source.owner_relations.end()) state.owner_relations.emplace(*found);
    }
    return state;
  }

  void resolve_captures(CheckedMethod &method) {
    if (method.captures_resolved) return;
    // Diagnostic visibility is independent of runtime capture and type evidence.
    auto &names = enclosing_names_[method.source];
    for (const auto &name : free_bindings(*method.source).declarations)
      if (lexical_.lookup(name) || (checking_method_ && (enclosing_names_[checking_method_->source].contains(name) ||
          checking_method_->source->name == name))) names.insert(name);
    const ModuleLookup lookup(module_, method.module), context(interpretation_, method.interpretation);
    const auto &source = *method.source;
    const auto &free = free_bindings(source).captures;
    for (const auto *names : {&free, &source.captures, &free_bindings(source).queries}) for (const auto &name : *names)
      if (active_here(name.name))
        implementation_gap(name.at, "access to a here formal from a nested callable scope");
    const auto required = [&](const Capture &capture) {
      return lexical_.lookup(capture.name).has_value() ||
          lexical_self(capture.name, checking_instance_ ? std::optional<size_t>{checking_instance_->method} : std::nullopt) ||
          std::any_of(query_environment().bindings.begin(), query_environment().bindings.end(), [&](const auto &binding) { return binding.name == capture.name; }) ||
          (!module_names().callables.contains(capture.name) && !has_declarations(capture.name) && !module_names().symbols.contains(capture.name) &&
           capture.name != "cause_at");
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
    for (const auto &query : free_bindings(source).queries)
      if (required(query) && std::none_of(method.captures.begin(), method.captures.end(), [&](const auto &capture) { return capture.name == query.name; }))
        method.queries.push_back(query);
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
      if (type.owners.empty() && !type.value_result)
        implementation_gap(at, "callable result mode without an owner or explicit generic value contract");
      for (const auto &failure : type.throws_types)
        if (failure.kind != Type::exception) implementation_gap(at, "callable view throws require intrinsic exceptions");
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
    const auto components = [&](const std::vector<Parameter> &fields, bool labelled) {
      std::vector<CheckedHereContract::Component> result;
      std::set<std::string> names;
      for (const auto &field : fields) {
        if (field.here || field.variadic || field.automatic_conversion)
          implementation_gap(field.at, "nested here formals, packs or conversions in here contracts");
        if (!field.name.empty() && !names.insert(field.name).second)
          fail(field.at, "duplicate here input name '" + field.name + "'");
        const auto type = resolve_type(field.type, &receiver);
        signature_type(receiver, type, field.at);
        result.push_back({labelled ? field.name : std::string{}, type, argument_mode(field),
                          owner_role(field.owner, field.at), field.read_only});
      }
      if (labelled) std::ranges::sort(result, {}, &CheckedHereContract::Component::label);
      return result;
    };
    CheckedHereContract contract;
    contract.inputs = components(source.parameters, source.labelled);
    contract.labelled = source.labelled;
    if (source.feedback_fields) contract.feedback = components(*source.feedback_fields, true);
    else {
      contract.result.type = resolve_type(source.result, &receiver);
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
      if (type.kind != Type::exception) implementation_gap(formal.at, "here throws contracts require selected intrinsic exception types");
      const auto categories = exception_possibilities(type);
      contract.throws_categories.insert(categories.begin(), categories.end());
    }
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
  static Substitutions type_context(const CheckedMethod &method, const std::vector<Type> &retained) {
    Substitutions substitutions(method.type_parameters.size());
    for (size_t i = 0; i < retained.size(); ++i) substitutions.at(method.source->type_parameters.size() + i) = retained[i];
    return substitutions;
  }
  static bool bind_parameter(const CheckedMethod &method, size_t index, const Type &argument,
                             Substitutions &substitutions, bool widening) {
    index = parameter_index(method, index);
    if (bind_type(method, method.parameters[index], argument, substitutions, true)) return true;
    if (!widening || argument_mode(method.input_parameters()[index]) != ArgumentMode::value) return false;
    auto target = method.parameters[index];
    if (const auto type = binder(method, target)) {
      // An enclosing type is already fixed. This method's own binders still
      // constrain actual input identities, including repeated occurrences.
      if (*type < method.source->type_parameters.size() || !substitutions[*type]) return false;
      target = *substitutions[*type];
    }
    return numeric(target) && widens(argument, target);
  }
  static std::optional<Substitutions> match(const CheckedMethod &method, const std::vector<Type> &arguments,
                                          const std::vector<Type> &retained = {}, bool widening = false) {
    if (!accepts_arity(method, arguments.size()) || method.retained_types.size() != retained.size()) return {};
    auto substitutions = type_context(method, retained);
    for (size_t i = 0; i < arguments.size(); ++i) {
      if (!bind_parameter(method, i, arguments[i], substitutions, widening)) return {};
    }
    return substitutions;
  }
  std::optional<Substitutions> declaration_match(const CheckedMethod &method, const std::vector<Type> &arguments) const {
    if (method.here) return {};
    auto substitutions = match(method, arguments, {}, true);
    if (substitutions && variadic(method) && nested_binder(method, substitute(method.parameters.back(), method, *substitutions, true)))
      implementation_gap(method.source->at, "variadic element type is not determined by the inputs");
    return substitutions;
  }
  struct DeclarationOverlap {
    Substitutions substitutions;
    std::set<size_t> unknown_categories;
  };
  std::optional<DeclarationOverlap> declaration_overlap(const CheckedMethod &method, const std::vector<Type> &arguments) const {
    if (!accepts_arity(method, arguments.size()) || !method.retained_types.empty()) return {};
    DeclarationOverlap overlap{type_context(method, {}), {}};
    std::set<size_t> category_parameters;
    for (size_t i = 0; i < arguments.size(); ++i) {
      const auto &actual = arguments[i];
      const auto &pattern = method.parameters[parameter_index(method, i)];
      if (argument_mode(parameter_at(method, i)) == ArgumentMode::value && actual.kind == Type::exception && !actual.identity) {
        if (const auto index = binder(method, pattern)) category_parameters.insert(*index);
        else if (pattern.kind != Type::exception) return {};
      } else if (!bind_parameter(method, i, actual, overlap.substitutions, true)) return {};
    }
    // Intersect repeated binders with concrete inputs before admitting an
    // unknown category. Equality holds only on this overlapping domain.
    bool unproved = false;
    for (const auto index : category_parameters) {
      auto &bound = overlap.substitutions[index];
      if (!bound) {
        bound = exception_type(0); overlap.unknown_categories.insert(index);
      } else if (bound->kind != Type::exception) return {};
      else if (!bound->identity) unproved = true;
    }
    if (unproved) implementation_gap(method.source->at, "declaration overlap between concrete category evidence and a borrowed or nested common type");
    if (variadic(method) && nested_binder(method, substitute(method.parameters.back(), method, overlap.substitutions, true)))
      implementation_gap(method.source->at, "variadic element type is not determined by the inputs");
    return overlap;
  }
  std::vector<Type> prepared_types(const CheckedMethod &method, const std::vector<Type> &arguments,
                                   const std::vector<Type> &retained) const {
    const auto substitutions = match(method, arguments, retained, true);
    if (!substitutions) throw std::logic_error("unchecked method arguments");
    auto prepared = method.parameters;
    for (auto &type : prepared) type = substitute(type, method, *substitutions);
    return prepared;
  }
  struct MethodCandidates {
    std::vector<size_t> methods;
    std::map<size_t, Substitutions> substitutions;
    bool widening = false, type_match = false, access_match = false;
  };
  static std::optional<Substitutions> match_evidence(const CheckedMethod &method,
      const std::vector<std::optional<Type>> &arguments, const std::vector<Type> &retained, bool widening) {
    if (!accepts_arity(method, arguments.size()) || method.retained_types.size() != retained.size()) return {};
    auto substitutions = type_context(method, retained);
    for (size_t i = 0; i < arguments.size(); ++i)
      if (arguments[i] && arguments[i] != Type::never && !bind_parameter(method, i, *arguments[i], substitutions, widening)) return {};
    return substitutions;
  }
  MethodCandidates method_candidates(const std::vector<size_t> &family, const std::vector<std::optional<Type>> &arguments,
                                     const std::vector<Type> &retained, const Access &access, const Owners &owners,
                                     const TypeEvidenceState *state = nullptr, bool allow_widening = true,
                                     bool has_here = false) const {
    MethodCandidates result;
    const bool complete = std::all_of(arguments.begin(), arguments.end(), [](const auto &type) { return type && type != Type::never; });
    for (const bool widening : {false, true}) {
      if (widening && !allow_widening) break;
      for (const auto id : family) {
        const auto &method = program_.methods[id];
        if (method.parameters.size() != arguments.size() || method.here.has_value() != has_here) continue;
        auto substitutions = match_evidence(method, arguments, retained, widening);
        if (!substitutions) continue;
        result.type_match = true;
        if (!admits_access(method, access)) continue;
        result.access_match = true;
        if (match_owners(method, owners, state) == OwnerMatch::mismatch) continue;
        if (result.substitutions.emplace(id, std::move(*substitutions)).second) result.methods.push_back(id);
      }
      if (complete && !result.methods.empty()) {
        result.widening = widening;
        break;
      }
    }
    return result;
  }
  MethodCandidates method_candidates(const std::vector<size_t> &family, const std::vector<Type> &arguments,
                                     const std::vector<Type> &retained, const Access &access, const Owners &owners,
                                     const TypeEvidenceState *state = nullptr, bool allow_widening = true,
                                     bool has_here = false) const {
    return method_candidates(family, std::vector<std::optional<Type>>{arguments.begin(), arguments.end()},
        retained, access, owners, state, allow_widening, has_here);
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
    if (!a.here || !b.here) return !a.here && !b.here;
    const auto &left = *a.here, &right = *b.here;
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
    return left.labelled == right.labelled && left.throws_categories == right.throws_categories &&
           same_components(left.inputs, right.inputs) && same(left.result, right.result) &&
           left.feedback.has_value() == right.feedback.has_value() &&
           (!left.feedback || same_components(*left.feedback, *right.feedback));
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
  static bool implies(const CheckedMethod &narrow, const CheckedMethod &broad, bool widening = false) {
    if (narrow.parameters.size() != broad.parameters.size() || narrow.here.has_value() != broad.here.has_value()) return false;
    std::vector<Type> arguments;
    for (size_t i = 0; i < narrow.parameters.size(); ++i) {
      arguments.push_back(independent_input(rigid_type(narrow, narrow.parameters[i]), i, argument_mode(parameter_at(narrow, i))));
    }
    return match(broad, arguments, {}, widening).has_value();
  }
  struct DeclarationObligation {
    const CheckedMethod *declaration;
    Substitutions result_substitutions;
    bool covers_types;
    bool covers_method;
  };
  std::vector<DeclarationObligation> declaration_obligations(const CheckedMethod &method, const std::vector<Type> &arguments,
      const Access &input_access = {}, const Owners &input_owners = {}, const TypeEvidenceState *state = nullptr) const {
    std::vector<DeclarationObligation> obligations;
    const ModuleLookup lookup(module_, method.module);
    if (lexical_methods_.contains(method.source)) return obligations;
    if (!method.family) return obligations;
    for (const auto &declaration : program_.families[*method.family].declarations) {
      if (method.here.has_value() != declaration.here.has_value()) continue;
      auto overlap = declaration_overlap(declaration, arguments);
      if (!overlap) continue;
      if (method.here) {
        const auto bindings = match(method, arguments, {}, true);
        if (!bindings || !same_here_signature(method, declaration, arguments.size(), [&](const Type &left, const Type &right) {
              return substitute(left, method, *bindings, true) == substitute(right, declaration, overlap->substitutions, true);
            })) continue;
      }
      if (!admits_access(declaration, input_access) || match_owners(declaration, input_owners, state) == OwnerMatch::mismatch) continue;
      const auto access = access_coverage(method, declaration);
      if (access == AccessCoverage::disjoint) continue;
      // Method-wide coverage retains independent concrete categories and
      // equalities from repeated method binders, separately from overlap.
      auto domain = arguments;
      std::map<size_t, size_t> category_roles;
      for (size_t i = 0; i < domain.size(); ++i) {
        const auto mode = argument_mode(parameter_at(method, i));
        if (mode == ArgumentMode::value && accepts_exception(domain[i], method.parameters[i])) domain[i] = method.parameters[i];
        // Borrowed inputs retain their exact storage target type. Payload
        // categories vary independently only for copied value inputs.
        if (mode == ArgumentMode::value && domain[i].kind == Type::exception && !domain[i].identity) {
          const auto parameter = binder(method, method.parameters[i]);
          const auto role = parameter ? category_roles.try_emplace(*parameter, i).first->second : i;
          domain[i] = independent_input(domain[i], role, mode);
        }
        if (mode != argument_mode(parameter_at(declaration, i)))
          implementation_gap(method.source->at, "declaration applicability across different parameter passing modes");
      }
      for (size_t i = 0; i < domain.size(); ++i)
        if (numeric(method.parameters[i]) && widens(domain[i], method.parameters[i])) domain[i] = method.parameters[i];
      const bool covers_types = match(declaration, domain, {}, true).has_value();
      // Input coverage does not establish the category of a dependent result.
      // A common view, even in a repeated method binder, cannot replace that
      // unknown category in the declaration's result promise.
      for (const auto index : overlap->unknown_categories) overlap->substitutions[index].reset();
      if (method.source->kind != declaration.source->kind)
        fail(method.source->at, "implementation kind does not satisfy callable declaration");
      for (const auto category : method.throws_categories)
        if (exception_categories[category].checked && !declaration.throws_categories.contains(category))
          fail(method.source->at, "implementation exposes a checked exception outside its callable declaration");
      obligations.push_back({&declaration, std::move(overlap->substitutions), covers_types, covers_types &&
          access == AccessCoverage::contained && implies_owners(method, declaration)});
    }
    return obligations;
  }
  struct DeclaredResult {
    ResultContract contract;
    std::set<std::string> owner_roles;
    bool covers_method;
  };
  std::vector<DeclaredResult> declared_results(const CheckedMethod &method, const std::vector<Type> &arguments,
      const Access &input_access = {}, const Owners &input_owners = {}, const TypeEvidenceState *state = nullptr) const {
    std::vector<DeclaredResult> results;
    for (const auto &[declaration, substitutions, covers_types, covers_method] : declaration_obligations(method, arguments, input_access, input_owners, state)) {
      auto result = declaration->result;
      std::set<std::string> owner_roles;
      result.type = substitute(*result.type, *declaration, substitutions, true);
      if (nested_binder(*declaration, *result.type))
        implementation_gap(method.source->at, "declared result type parameter is not determined by the arguments");
      if (result.borrowed()) {
        for (size_t i = 0; i < arguments.size(); ++i) if (parameter_at(*declaration, i).owner == result.owner) {
          const auto &mapped = method.input_parameters()[i].owner;
          if (!mapped.empty()) owner_roles.insert(mapped);
        }
        if (owner_roles.empty()) implementation_gap(method.source->at, "declared result owner is not established by a parameter");
        // Equalities hold on this declaration's domain. They validate its
        // result promise without changing the method's own owner roles.
        size_t previous;
        do {
          previous = owner_roles.size();
          for (size_t i = 0; i < arguments.size(); ++i) for (size_t j = 0; j < arguments.size(); ++j)
            if (same_owner(*declaration, i, j) && owner_roles.contains(method.input_parameters()[i].owner))
              owner_roles.insert(method.input_parameters()[j].owner);
        } while (owner_roles.size() != previous);
        result.owner = *owner_roles.begin();
      }
      results.push_back({std::move(result), std::move(owner_roles), covers_method});
    }
    return results;
  }
  static ViewEvidence result_view(const CheckedMethod &method, const ResultContract &result, const Owners &owners) {
    ViewEvidence view{result.read_only, result.borrowed(), {}};
    view.nullable = result.nullable();
    if (result.borrowed()) for (size_t i = 0; i < owners.size(); ++i)
      if (parameter_at(method, i).owner == result.owner && owners[i]) view.owner = owners[i];
    return view;
  }
  static bool same_result(const ResultContract &a, const ResultContract &b) {
    return a.type == b.type && a.mode == b.mode && a.owner == b.owner && a.read_only == b.read_only;
  }
  enum class ResultOrigin { inferred, supplied, throw_only_default };
  struct ResultEvidence {
    std::optional<ResultContract> contract;
    ResultOrigin origin = ResultOrigin::inferred;
  };
  static ResultEvidence scope_result(ResultEvidence result, const Type *expected, bool probing_argument = false) {
    if (result.contract && result.origin != ResultOrigin::throw_only_default) return result;
    if (expected) return {ResultContract{*expected}, ResultOrigin::supplied};
    // Operand hints cannot use the standalone default to select a signature.
    return probing_argument ? ResultEvidence{} : result;
  }
  static void check_declared_results(const CheckedMethod &method, const ResultContract &result,
                                     const std::vector<DeclaredResult> &declared) {
    for (const auto &obligation : declared) {
      const auto &contract = obligation.contract;
      const bool compatible = result.mode == contract.mode && (!result.borrowed() || obligation.owner_roles.contains(result.owner)) &&
          (!result.borrowed() ? transfers_value(*result.type, *contract.type)
                            : result.type == contract.type && (!result.read_only || contract.read_only));
      if (!compatible) fail(method.source->at, "implementation result does not satisfy callable declaration");
    }
  }
  size_t demand(size_t method, std::vector<Type> arguments, const Location &at, const std::vector<CheckedCaptureInput> &environment = {},
                const std::vector<Type> &retained = {}, size_t query_environment = 0, const TypeEvidenceState *state = nullptr) {
    auto &definition = program_.methods[method];
    const auto declared = declared_results(definition, arguments);
    arguments = prepared_types(definition, arguments, retained);
    for (const auto id : definition.instances)
      if (program_.instances[id].arguments == arguments && program_.instances[id].environment == environment &&
          program_.instances[id].retained_types == retained && program_.instances[id].query_environment == query_environment) {
        check_declared_results(definition, program_.instances[id].result, declared);
        return id;
      }
    if (program_.instances.size() == instance_limit)
      fail(at, "compiler resource limit: 4096 scalar checking obligations while preparing '" + definition.source->name + "'");
    const auto substitutions = match(definition, arguments, retained);
    if (!substitutions) throw std::logic_error("unchecked method demand");
    const auto &supplied = program_.query_environments[query_environment].result;
    const auto result = supplied ? ResultEvidence{supplied, ResultOrigin::supplied} :
        require_result(method, arguments, retained, at, state);
    check_declared_results(definition, *result.contract, declared);
    definition.contextual_result = definition.contextual_result || supplied.has_value();
    const auto id = program_.instances.size();
    auto context = arguments;
    for (const auto &capture : environment) context.push_back(capture.type);
    context.insert(context.end(), retained.begin(), retained.end());
    program_.instances.push_back({method, arguments, *result.contract, {}, false, {}, {}, {}, environment,
                                  std::move(context), retained, *substitutions, query_environment, {},
                                  result.origin == ResultOrigin::supplied});
    program_.instances.back().effects = initial_effects(definition, arguments);
    if (caller_bindings(definition) && arguments.size() == 2 && arguments[0] == arguments[1] &&
        definition.input_parameters()[0].read_only == definition.input_parameters()[1].read_only)
      distinct_binding_effects_.emplace(id, program_.instances.back().effects);
    definition.instances.push_back(id);
    return id;
  }
  std::vector<size_t> most_specific(const std::vector<size_t> &applicable, const Access &access = {}, const Owners &owners = {},
                                    const TypeEvidenceState *state = nullptr, bool widening = false) const {
    std::vector<size_t> best;
    for (const auto candidate : applicable) {
      const auto &definition = program_.methods[candidate];
      const bool dominated = std::any_of(applicable.begin(), applicable.end(), [&](size_t other) {
        const auto &alternative = program_.methods[other];
        if (!implies(alternative, definition, widening) || !implies_owners(alternative, definition)) return false;
        const bool stricter_owners = !implies_owners(definition, alternative);
        if (stricter_owners && match_owners(alternative, owners, state) != OwnerMatch::satisfied) return false;
        bool strict = !implies(definition, alternative, widening) || stricter_owners;
        for (size_t i = 0; i < alternative.parameters.size(); ++i) {
          const auto exact = alternative.input_parameters()[i].read_only;
          if (exact == definition.input_parameters()[i].read_only) continue;
          if (i >= access.size() || !access[i] || exact != *access[i]) return false;
          strict = true;
        }
        return strict;
      });
      if (!dominated) best.push_back(candidate);
    }
    return best;
  }
  size_t select_method(const std::vector<size_t> &family, const std::vector<Type> &arguments, const Location &at, const std::string &name,
                       const std::vector<Type> &retained = {}, const Access &access = {}, const Owners &owners = {},
                       const TypeEvidenceState *state = nullptr, bool allow_widening = true, bool has_here = false) const {
    const auto candidates = method_candidates(family, arguments, retained, access, owners, state, allow_widening, has_here);
    const auto best = most_specific(candidates.methods, access, owners, state, candidates.widening);
    if (best.empty()) {
      if (candidates.access_match) fail(at, "arguments for one owner role have different actual owners");
      if (candidates.type_match) fail(at, "readonly argument cannot supply a writable borrowed parameter");
      fail(at, "no applicable method for '" + name + "' (argument types or same-T relation do not match)");
    }
    if (!owners.empty() && std::any_of(best.begin(), best.end(), [&](size_t id) {
          return match_owners(program_.methods[id], owners, state) == OwnerMatch::unknown;
        })) implementation_gap(at, "method selection requires an unproved owner equality");
    if (best.size() != 1) ambiguous_methods(at, name, best);
    const auto selected = best.front();
    declaration_obligations(program_.methods[selected], arguments);
    return selected;
  }
  size_t select_family(const std::vector<size_t> &family, const std::vector<Type> &arguments, const Location &at, const std::string &name,
                       const std::vector<CheckedCaptureInput> &environment = {}, const std::vector<Type> &retained = {},
                       const Access &access = {}, const Owners &owners = {}, size_t query_environment = 0) {
    return demand(select_method(family, arguments, at, name, retained, access, owners), arguments, at, environment, retained, query_environment);
  }
  size_t select(const Expression &expr, const std::vector<Type> &arguments, const Access &access, const Owners &owners) {
    const auto family = named_family(expr.text);
    if (family.definition) check_query_binding(expr.text, expr.at);
    if (family.methods.empty()) fail(expr.at, has_declarations(expr.text)
        ? "no applicable implementation for declared callable '" + expr.text + "'" : "unknown callable '" + expr.text + "'");
    const auto method = select_method(family.methods, arguments, expr.at, expr.text, family.retained_types(), access, owners);
    return demand(method, arguments, expr.at,
        family.context ? family.context->environment : family.definition ? family.definition->environment : std::vector<CheckedCaptureInput>{},
        family.retained_types(), family.context ? family.context->query_environment : family.definition ? family.definition->query : 0,
        family.declaration);
  }
  bool applicable_operation(const std::string &name, const std::vector<Type> &arguments, const Access &access = {}, const Owners &owners = {}) const {
    const auto family = module_names().callables.find(name);
    return family != module_names().callables.end() && !method_candidates(family_methods(family->second), arguments, {}, access, owners).methods.empty();
  }
  void finish_named_call(const Expression &expr, const std::vector<Type> &arguments,
                         const std::vector<CheckedOperation> &views, CheckedOperation &operation,
                         bool read, bool discard, bool completes = true, std::optional<size_t> selected = {}) {
    operation.callee = selected ? *selected : select(expr, arguments, access_of(views), owners_of(views));
    const auto &target = program_.instances[*operation.callee];
    const auto &method = program_.methods[target.method];
    const auto &definition = *method.source;
    const auto &parameters = method.input_parameters();
    const bool uses_caller_bindings = caller_bindings(method);
    if (std::any_of(operation.arguments.begin(), operation.arguments.end(), [&](const auto &argument) {
          return argument.binding_slot != uses_caller_bindings;
        })) implementation_gap(expr.at, "selected method requires unproved slot preparation adaptation");
    bool distinct_bindings = false;
    if (uses_caller_bindings && views.size() == 2) {
      const auto left = overlapping_slots(views[0].slots);
      const auto right = physical_slots(views[1].slots);
      distinct_bindings = !left.empty() && !right.empty() && intersect(left, right).empty();
    }
    const auto &effects = read_effects(*operation.callee, distinct_bindings);
    if (uses_caller_bindings) {
      for (size_t i = 0; i < views.size(); ++i) {
        if (&expr == prepared_call_) require_deferred_slots(views[i], expr.at);
        for (const auto slot : physical_slots(views[i].slots)) {
          const auto &binding = bindings_->at(slot);
          if (binding.read_only != parameters[i].read_only && proving_ && validating_) {
            const auto &sources = effects.binding_sources;
            const auto found = sources.find(i);
            const bool preserved = found != sources.end() && !found->second.empty() &&
                std::all_of(found->second.begin(), found->second.end(), [&](size_t input) {
                  if (input == null_target) return true;
                  const auto slots = physical_slots(views.at(input).slots);
                  return !slots.empty() && std::all_of(slots.begin(), slots.end(), [&](size_t source) {
                    return !bindings_->at(source).read_only;
                  });
                });
            if (!preserved) implementation_gap(expr.at, "caller-binding target permission preservation remains unproved");
          }
          if (binding.requires_initialized && argument_mode(parameters[i]) != ArgumentMode::borrowed_ref && proving_ && validating_) {
            const auto &sources = effects.binding_sources;
            const auto found = sources.find(i);
            const bool preserved = found != sources.end() && !found->second.empty() &&
                std::all_of(found->second.begin(), found->second.end(), [&](size_t input) {
                  if (input == null_target) return false;
                  const auto slots = physical_slots(views.at(input).slots);
                  return !slots.empty() && std::all_of(slots.begin(), slots.end(), [&](size_t source) {
                    return bindings_->at(source).requires_initialized;
                  });
                });
            if (!preserved) implementation_gap(expr.at, "caller ref preservation through a non-ref method parameter remains unproved");
          }
        }
      }
    }
    for (size_t i = 0; i < arguments.size(); ++i) {
      const auto mode = argument_mode(parameters[i]);
      if (operation.destination_arguments) {
        operation.arguments[i].mode = mode;
        if (completes) prepare_borrow(views[i], mode, expr.children[i]->at);
      } else if (operation.arguments[i].mode != mode)
        throw std::logic_error("argument preparation disagrees with selected method");
      operation.arguments[i].type = target.arguments[i];
      if (operation.arguments[i].binding_slot && views[i].targets.contains(null_target))
        operation.nullable_arguments.insert(i);
    }
    if (checking_method_ && checking_method_->source->kind == Form::Kind::function && definition.kind == Form::Kind::procedure)
      fail(expr.at, "a function cannot invoke a procedure");
    if (!completes) { operation.type = Type::never; return; }
    const auto contract = callable_type(program_.methods[target.method], target.arguments, target.result,
        access_of(views), owners_of(views), &arguments);
    auto reasons = direct_call_reasons(contract, effects);
    if (proving_ && effects.propagates_here_transfer && (operation.forwards_here || operation.supplied_here))
      reasons.insert(Completion::protected_transfer);
    finish_call(expr, contract, effects, reasons, arguments, views, operation, read, discard);
  }
  struct EagerTypes {
    Type result, operand;
    bool unary = false, failing = false;
    std::string error;
  };
  static std::optional<Type> common_value_type(const std::vector<Type> &values) {
    if (values.empty()) return {};
    auto candidates = values;
    for (const auto kind : {Type::uint8, Type::uint32, Type::uint64, Type::integer, Type::float32, Type::float64})
      candidates.push_back(Type{kind});
    candidates.push_back(exception_type(0));
    std::vector<Type> common;
    for (const auto &candidate : candidates)
      if (std::find(common.begin(), common.end(), candidate) == common.end() &&
          std::all_of(values.begin(), values.end(), [&](const auto &type) { return transfers_value(type, candidate); }))
        common.push_back(candidate);
    std::optional<Type> least;
    for (const auto &candidate : common)
      if (std::all_of(common.begin(), common.end(), [&](const auto &other) { return transfers_value(candidate, other); })) {
        if (least && *least != candidate) return {};
        least = candidate;
      }
    return least;
  }
  static EagerTypes eager_types(std::string_view name, const std::vector<Type> &arguments) {
    EagerTypes types;
    types.unary = arguments.size() == 1 && (name == "+" || name == "-" || name == "!");
    const auto error = [&](const std::string &message) { types.error = message; return types; };
    if (!types.unary && (arguments.size() != 2 || name == "!")) return error("builtin operator argument count does not match");
    const bool completes = std::none_of(arguments.begin(), arguments.end(), [](const Type &type) { return type == Type::never; });
    if (types.unary) {
      const auto &argument = arguments.front();
      if (!completes) { types.result = types.operand = Type::never; return types; }
      if (name == "!") {
        if (argument != Type::boolean) return error("logical negation requires bool");
      } else if (argument != Type::integer && !(name == "-" && floating(argument)))
        return error("numeric unary operation requires i64, or floating point for negation");
      types.result = types.operand = argument;
      types.failing = name == "-" && argument == Type::integer;
    } else {
      const bool equality = name == "==" || name == "!=";
      const bool comparison = equality || name == "<" || name == ">" || name == "<=" || name == ">=";
      std::vector<Type> values;
      for (const auto &type : arguments) if (type != Type::never) values.push_back(type);
      if (values.empty()) { types.result = types.operand = Type::never; return types; }
      if (equality && std::all_of(values.begin(), values.end(), [](const Type &type) { return type == Type::boolean; })) types.operand = Type::boolean;
      else {
        if (!std::all_of(values.begin(), values.end(), numeric)) return error("no applicable numeric operation '" + std::string(name) + "'");
        if (const auto common = common_value_type(values)) types.operand = *common;
        else return error("numeric operands have no lossless common type");
        if (name == "%" && floating(types.operand)) return error("remainder requires integer operands");
      }
      types.result = !completes ? Type{Type::never} : comparison ? Type{Type::boolean} : types.operand;
      types.failing = completes && !comparison && integral(types.operand);
    }
    return types;
  }
  std::set<size_t> builtin_declarations(const Expression &expr, const std::vector<Type> &arguments,
                                      const Type &result, bool destinations = false,
                                      const Access &access = {}, const Owners &owners = {}, const TypeEvidenceState *state = nullptr) const {
    if (expr.intrinsic || !has_declarations(expr.text)) return {};
    // Builtins have established signatures. Declarations constrain these just
    // as they constrain explicit methods; they cannot retarget a builtin result.
    const auto family = module_names().callables.at(expr.text);
    const auto &identity = program_.families[family].operation;
    Form source;
    source.kind = Form::Kind::function; source.name = expr.text; source.at = expr.at; source.operation = true;
    for (size_t i = 0; i < arguments.size(); ++i) {
      Parameter parameter;
      parameter.name = "argument" + std::to_string(i); parameter.type = arguments[i]; parameter.at = expr.at;
      if (destinations) {
        parameter.owner = identity == "<=>" ? "owner" : "owner" + std::to_string(i);
        if (identity == "<=>") {
          parameter.declaration_kind = "ptr";
          parameter.read_only = i < access.size() && access[i].value_or(false);
        }
      }
      source.parameters.push_back(std::move(parameter));
    }
    CheckedMethod method;
    method.source = &source; method.operation = identity; method.module = module_; method.family = family; method.parameters = arguments; method.result.type = result;
    check_declared_results(method, method.result, declared_results(method, arguments, access, owners, state));
    std::set<size_t> failures;
    for (const auto &[declaration, substitutions, covers_types, covers_method] : declaration_obligations(method, arguments, access, owners, state))
      failures.insert(declaration->throws_categories.begin(), declaration->throws_categories.end());
    return failures;
  }
  void check_builtin_declarations(const Expression &expr, const std::vector<Type> &arguments,
                                  CheckedOperation &operation, bool destinations = false,
                                  const std::vector<CheckedOperation> &views = {}) {
    const auto failures = builtin_declarations(expr, arguments, operation.type, destinations, access_of(views), owners_of(views));
    if (failures.empty()) return;
    operation.invocation.try_emplace(0);
    for (const auto category : failures) operation.invocation.try_emplace(3 + category);
    if (&expr != prepared_call_) note_abrupt(expr.at, failures);
  }
  void check_eager(const Expression &expr, std::string_view name, const std::vector<Type> &arguments,
                   const std::vector<CheckedOperation> &views, CheckedOperation &operation) {
    const auto types = eager_types(name, arguments);
    if (!types.error.empty()) fail(expr.at, types.error);
    operation.builtin = types.unary ? BuiltinOperation::unary : BuiltinOperation::binary;
    operation.builtin_name = name;
    operation.type = types.result; operation.operand_type = types.operand;
    if (types.result == Type::never) return;
    if (types.unary && name == "!")
      operation.predicate = predicate(Predicate::Kind::negation, predicate_of(views.front()));
    operation.invocation.emplace(0, CleanupEquations::Effects{});
    check_builtin_declarations(expr, arguments, operation, false, views);
    if (types.failing) {
      operation.invocation.emplace(2, CleanupEquations::Effects{});
      if (&expr != prepared_call_) note_abrupt(expr.at);
    }
  }
  void add_declared_failures(Type &type, const CheckedMethod &method, const std::vector<Type> &arguments,
                             const Access &access = {}, const Owners &owners = {}, bool covering_types_only = false) const {
    for (const auto &[declaration, substitutions, covers_types, covers_method] : declaration_obligations(method, arguments, access, owners)) {
      if (covering_types_only && !covers_types) continue;
      for (const auto &failure : declaration->throws_types)
        if (std::find(type.throws_types.begin(), type.throws_types.end(), failure) == type.throws_types.end())
          type.throws_types.push_back(failure);
    }
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
  Type callable_type(const CheckedMethod &checked, const std::vector<Type> &arguments, const ResultContract &contract,
                     const Access &access = {}, const Owners &owners = {}, const std::vector<Type> *original = nullptr) const {
    const ModuleLookup lookup(module_, checked.module);
    auto type = callable_signature(checked, arguments, contract);
    add_declared_failures(type, checked, arguments, access, owners, original && *original != arguments);
    if (original && *original != arguments) add_declared_failures(type, checked, *original, access, owners);
    return resolve_type(std::move(type));
  }
  struct ResultObligation {
    size_t method;
    std::vector<Type> arguments, retained;
    TypeEvidenceState context;
    std::set<size_t> dependencies, dependents;
    ResultEvidence result;
  };
  mutable std::deque<ResultObligation> result_obligations_;
  mutable std::map<size_t, std::vector<size_t>> method_result_obligations_;
  mutable std::set<size_t> pending_results_;
  mutable std::optional<size_t> result_probe_;
  mutable bool result_graph_changed_ = false;
  struct TypeCallProofs;
  mutable TypeCallProofs *type_call_proofs_ = nullptr;

  static std::optional<Type> common_result(const std::vector<Type> &returns, const Location &at) {
    if (returns.empty()) return {};
    const auto least = common_value_type(returns);
    if (!least) fail(at, "return alternatives have no unique least common result; provide an explicit result contract and valid conversions");
    return least;
  }
  TypeEvidenceState result_context(size_t id, const TypeEvidenceState *supplied) const {
    TypeEvidenceState context;
    const auto &method = *program_.methods[id].source;
    if (!lexical_methods_.contains(&method)) return context;
    const auto &dependencies = definition_dependencies(method);
    const auto available = supplied && supplied->method == id && supplied->declaration ? project_evidence(*supplied->declaration, dependencies)
        : checking_instance_ && checking_instance_->method == id && (!supplied || supplied->method == id) ? evidence_state(true, &dependencies)
        : supplied ? project_evidence(*supplied, dependencies)
        : bindings_ && !lexical_.scopes.empty() ? evidence_state(false, &dependencies) : TypeEvidenceState{};
    const auto retain = [&](const Capture &capture, bool query = false) {
      if (const auto found = available.bindings.find(capture.name); found != available.bindings.end()) {
        context.bindings.emplace(*found);
        context.views.emplace(capture.name, available.views.at(capture.name));
      } else if (query && lexical_self(capture.name, available.method)) {
        context.bindings.emplace(capture.name, callable_query({*available.method}, nullptr, capture.at, available));
        context.views.emplace(capture.name, ViewEvidence{false, false, {}});
      }
      if (const auto found = available.owner_bindings.find(capture.name); found != available.owner_bindings.end())
        context.owner_bindings.emplace(*found);
    };
    const auto &free = free_bindings(method);
    for (const auto &capture : free.captures) retain(capture);
    for (const auto &query : free.queries) retain(query, true);
    for (const auto &symbol : free.symbols)
      if (symbol_shadowed(symbol.name, available)) context.symbol_shadows.insert(symbol.name);
    std::map<EvidenceOwner, size_t> owners;
    const auto owner = [&](const EvidenceOwner &original) {
      return EvidenceOwner::retained(owners.try_emplace(original, owners.size()).first->second);
    };
    for (auto &[name, view] : context.views) view.map_owners(owner);
    for (auto &[name, id] : context.owner_bindings) id = owner(id);
    for (const auto &[original, id] : owners) {
      context.external_owners.insert(EvidenceOwner::retained(id));
      for (const auto &[other, target] : owners)
        context.owner_relations[{EvidenceOwner::retained(id), EvidenceOwner::retained(target)}] = compare_owners(original, other, &available);
    }
    return context;
  }
  TypeEvidenceState body_evidence(size_t id, const std::vector<Type> &arguments, const std::vector<Type> &retained,
                                  const TypeEvidenceState &context) const {
    const auto &method = program_.methods[id];
    auto state = context;
    state.declaration = &context;
    state.method = id;
    state.substitutions = *match(method, arguments, retained);
    state.retained = retained;
    state.full_scope_owner = 0;
    state.invocation_result_owner.reset();
    state.caught_exception.reset();
    // An ordinary retained value has invocation-local storage. A standalone
    // here body has direct access to its supplying activation instead.
    if (!method.source->implicit_scope) for (const auto &capture : free_bindings(*method.source).captures)
      if (const auto found = state.views.find(capture.name);
          found != state.views.end() && found->second.value_capture == true) {
        found->second.owner = EvidenceOwner{};
        found->second.place_owners = {EvidenceOwner{}};
        found->second.storage_kind = PlaceKind::value;
        found->second.view = found->second.value_binding = true;
        found->second.fixed_binding = false;
        found->second.slot_read_only = found->second.read_only;
      }
    for (size_t i = 0; i < method.input_owners().size(); ++i) {
      const auto owner = EvidenceOwner::parameter(i);
      const auto &name = method.input_owners()[i].name;
      state.owner_bindings[name] = owner;
      state.external_owners.insert(owner);
      if (method.result.borrowed() && method.result.owner == name) state.invocation_result_owner = owner;
    }
    if (method.scope_inputs) {
      const auto &profile = *method.scope_inputs;
      std::map<EvidenceOwner, EvidenceOwner> forwarded;
      const auto retain = [&](const std::string &name, std::optional<EvidenceOwner> caller) {
        if (!caller) return;
        const auto role = std::find_if(profile.owners.begin(), profile.owners.end(),
            [&](const auto &owner) { return owner.name == name; });
        if (role != profile.owners.end())
          forwarded.emplace(*caller, EvidenceOwner::parameter(std::distance(profile.owners.begin(), role)));
      };
      for (const auto &parameter : profile.parameters)
        if (!parameter.owner.empty() && state.views.contains(parameter.name))
          retain(parameter.owner, state.views.at(parameter.name).owner);
      for (const auto &[name, access] : profile.place_access)
        if (state.views.contains(name)) retain(profile.parameters[access.input].owner, state.views.at(name).owner);
      for (const auto &[name, role] : profile.owner_names)
        if (context.owner_bindings.contains(name))
          retain(profile.owners[role].name, context.owner_bindings.at(name));
      const auto translate = [&](EvidenceOwner owner) { return forwarded.contains(owner) ? forwarded.at(owner) : owner; };
      for (auto &[name, view] : state.views) view.map_owners(translate);
      for (auto &[name, owner] : state.owner_bindings)
        if (forwarded.contains(owner)) owner = forwarded.at(owner);
      for (const auto &[pair, relation] : context.owner_relations) {
        state.owner_relations[{translate(pair.first), translate(pair.second)}] = relation;
      }
      for (const auto &[name, role] : profile.owner_names)
        state.owner_bindings[name] = EvidenceOwner::parameter(role);
      for (size_t i = 0; i < profile.owners.size(); ++i)
        for (size_t j = 0; j < profile.owners.size(); ++j)
          state.owner_relations[{EvidenceOwner::parameter(i), EvidenceOwner::parameter(j)}] = profile.relations[i][j];
    }
    for (size_t i = 0; i < method.parameters.size(); ++i) {
      const auto &parameter = method.input_parameters()[i];
      const bool value = parameter.owner.empty();
      const bool live_value = method.scope_inputs && method.scope_inputs->live_values.contains(i);
      const bool independent = parameter.declaration_kind == "val" || (value && arguments[i].kind == Type::procedure);
      state.bindings[parameter.name] = arguments[i];
      state.definitions.erase(parameter.name);
      state.views[parameter.name] = {parameter.read_only || (parameter.declaration_kind == "val" && parameter.fixed_binding), true,
                                    live_value || parameter.declaration_kind == "val",
                                    value ? EvidenceOwner{} : state.owner_bindings.at(parameter.owner), false,
                                    parameter.fixed_binding, parameter.read_only, independent || live_value,
                                    !live_value && argument_mode(parameter) == ArgumentMode::borrowed_ptr};
      auto &view = state.views.at(parameter.name);
      view.storage_kind = place_kind(*view.value_binding, parameter.declaration_kind == "ref");
      if (const auto &profile = method.scope_inputs; profile && profile->place_owners.contains(i)) {
        for (const auto role : profile->place_owners.at(i)) view.place_owners.insert(EvidenceOwner::parameter(role));
      } else if (state.full_scope_owner) view.place_owners.insert(*state.full_scope_owner);
    }
    if (method.scope_inputs) for (const auto &[name, access] : method.scope_inputs->place_access) {
      auto view = state.views.at(method.input_parameters()[access.input].name);
      view.read_only = access.read_only;
      view.fixed_binding = access.fixed || view.fixed_binding.value_or(false);
      view.value_capture = false;
      state.bindings[name] = arguments[access.input];
      state.definitions.erase(name);
      state.views.insert_or_assign(name, std::move(view));
    }
    return state;
  }
  ResultEvidence sample_result(size_t id, bool complete = false) const {
    auto &obligation = result_obligations_[id];
    const auto &method = program_.methods[obligation.method];
    auto state = body_evidence(obligation.method, obligation.arguments, obligation.retained, obligation.context);
    TypeEvidenceState::Returns returns;
    state.returns = &returns;
    const auto previous = result_probe_;
    result_probe_ = id;
    const auto completion = type_evidence(*method.source->body, state);
    result_probe_ = previous;
    if (!method.source->requires_explicit_return() && !returns.value_obligation &&
        (completion || !returns.unresolved) && completion != Type::never && state.complete)
      returns.types.push_back(Type::void_type);
    if (method.source->implicit_scope && completion == Type::never && state.complete &&
        returns.types.empty() && !returns.value_obligation && !returns.unresolved &&
        returns.saw_throw && !returns.unresolved_throw)
      return {ResultContract{Type::void_type}, ResultOrigin::throw_only_default};
    if (complete && returns.unresolved) return {};
    if (const auto type = common_result(returns.types, method.source->at)) return {ResultContract{type}};
    return {};
  }
  void invalidate_result(size_t id) const {
    std::vector<size_t> work{id};
    while (!work.empty()) {
      const auto current = work.back(); work.pop_back();
      if (!pending_results_.insert(current).second) continue;
      auto &node = result_obligations_[current];
      node.result = {};
      work.insert(work.end(), node.dependents.begin(), node.dependents.end());
    }
  }
  void solve_results() const {
    // Discover finite type obligations without creating executable instances. Each
    // recursive component first withholds its own results to expose independent anchors.
    do {
      result_graph_changed_ = false;
      for (const auto id : pending_results_) result_obligations_[id].result = {};
      for (const auto id : pending_results_) sample_result(id);
      if (result_graph_changed_) continue;
      struct Visit { size_t index, low; bool active = true; };
      struct Frame { size_t id; std::set<size_t>::const_iterator dependency; };
      std::map<size_t, Visit> visited;
      std::vector<size_t> stack;
      std::vector<Frame> work;
      std::vector<std::vector<size_t>> components;
      const auto enter = [&](size_t id) {
        visited.emplace(id, Visit{visited.size(), visited.size()});
        stack.push_back(id);
        work.push_back({id, result_obligations_[id].dependencies.begin()});
      };
      for (const auto root : pending_results_) {
        if (visited.contains(root)) continue;
        enter(root);
        while (!work.empty()) {
          auto &frame = work.back();
          auto &node = visited.at(frame.id);
          if (frame.dependency != result_obligations_[frame.id].dependencies.end()) {
            const auto dependency = *frame.dependency++;
            if (!pending_results_.contains(dependency)) continue;
            if (!visited.contains(dependency)) enter(dependency);
            else if (const auto &target = visited.at(dependency); target.active) node.low = std::min(node.low, target.index);
            continue;
          }
          const auto id = frame.id;
          work.pop_back();
          if (!work.empty()) {
            auto &parent = visited.at(work.back().id);
            parent.low = std::min(parent.low, node.low);
          }
          if (node.low != node.index) continue;
          auto &component = components.emplace_back();
          for (;;) {
            const auto member = stack.back(); stack.pop_back();
            visited.at(member).active = false; component.push_back(member);
            if (member == id) break;
          }
        }
      }
      for (const auto &component : components) {
        std::vector<ResultEvidence> anchors;
        for (const auto id : component) anchors.push_back(sample_result(id));
        for (size_t i = 0; i < component.size(); ++i) result_obligations_[component[i]].result = std::move(anchors[i]);
        bool changed;
        do {
          changed = false;
          std::vector<std::pair<size_t, ResultEvidence>> established;
          for (const auto id : component) if (!result_obligations_[id].result.contract) {
            auto result = sample_result(id, true);
            if (result.contract) established.emplace_back(id, std::move(result));
          }
          for (auto &[id, result] : established) { result_obligations_[id].result = std::move(result); changed = true; }
        } while (changed);
      }
    } while (result_graph_changed_);
    pending_results_.clear();
  }
  ResultEvidence infer_result(size_t method, std::vector<Type> arguments, const std::vector<Type> &retained,
                             const TypeEvidenceState *state = nullptr) const {
    const auto &definition = program_.methods[method];
    const auto input_declared = declared_results(definition, arguments);
    arguments = prepared_types(definition, arguments, retained);
    auto supplied = definition.result;
    if (supplied.type) supplied.type = substitute(*supplied.type, definition, *match(definition, arguments, retained), true);
    const auto declared = declared_results(definition, arguments);
    if (!supplied.type) {
      for (const auto &obligation : declared) if (obligation.covers_method) {
        if (supplied.type && !same_result(supplied, obligation.contract))
          implementation_gap(definition.source->at, "declarations do not establish one result contract; provide an explicit compatible result");
        supplied = obligation.contract;
      }
    }
    if (supplied.type) {
      check_declared_results(definition, supplied, declared);
      check_declared_results(definition, supplied, input_declared);
      return {supplied, ResultOrigin::supplied};
    }
    // Optional call-effect proofs use established signatures. They must
    // not leave a speculative result-inference obligation in the main graph.
    if (type_call_proofs_) return {};
    const auto context = result_context(method, state);
    auto &candidates = method_result_obligations_[method];
    const auto found = std::find_if(candidates.begin(), candidates.end(), [&](size_t id) {
      const auto &node = result_obligations_[id];
      return node.arguments == arguments && node.retained == retained && node.context.same_definition_inputs(context);
    });
    const auto id = found == candidates.end() ? result_obligations_.size() : *found;
    if (found == candidates.end()) {
      if (id == instance_limit) fail(definition.source->at, "compiler resource limit: 4096 callable result obligations; result remains unproved");
      result_obligations_.push_back({method, arguments, retained, context, {}, {}, {}});
      candidates.push_back(id);
      invalidate_result(id);
      result_graph_changed_ = true;
    }
    if (result_probe_) {
      if (result_obligations_[*result_probe_].dependencies.insert(id).second) {
        result_obligations_[id].dependents.insert(*result_probe_);
        invalidate_result(*result_probe_);
        result_graph_changed_ = true;
      }
    } else if (!pending_results_.empty()) solve_results();
    const auto &result = result_obligations_[id].result;
    if (result.contract && !result_probe_) {
      const auto &type = *result.contract->type;
      if (type != Type::void_type && type.kind != Type::procedure && !scalar_type(type))
        implementation_gap(definition.source->at, "inferred result mode for a non-scalar type; provide an explicit result contract");
      if (swap_operation(definition.operation) && type != Type::void_type)
        fail(definition.source->at, "swap method must return void");
    }
    if (result.contract) check_declared_results(definition, *result.contract, input_declared);
    return result;
  }
  ResultEvidence require_result(size_t method, const std::vector<Type> &arguments, const std::vector<Type> &retained,
                               const Location &at, const TypeEvidenceState *state = nullptr) const {
    if (const auto result = infer_result(method, arguments, retained, state); result.contract) {
      if (nested_binder(program_.methods[method], *result.contract->type))
        implementation_gap(at, "type parameter is not determined by the arguments");
      return result;
    }
    implementation_gap(at, "unproved callable result; provide an independent result contract");
  }

  std::optional<std::vector<Type>> callable_retained_types(size_t id, const TypeEvidenceState *state) const {
    const auto &method = program_.methods[id];
    std::vector<Type> retained;
    const auto parent = state ? state->method : checking_instance_ ? std::optional<size_t>{checking_instance_->method} : std::nullopt;
    if (parent == id) retained = state ? state->retained : checking_instance_->retained_types;
    else if (!method.retained_types.empty()) {
      if (method.parent != lexical_template(parent) || !parent) return {};
      const auto &substitutions = state ? state->substitutions : checking_instance_->substitutions;
      for (const auto index : method.retained_types) {
        const auto &type = substitutions.at(index);
        if (!type) return {};
        retained.push_back(*type);
      }
    }
    return retained;
  }
  std::optional<Type> callable_evidence(size_t id, const TypeEvidenceState *state = nullptr) const {
    const auto &method = program_.methods[id];
    if (!method.source->type_parameters.empty()) return {};
    const auto retained = callable_retained_types(id, state);
    if (!retained) return {};
    const auto bindings = type_context(method, *retained);
    auto arguments = method.parameters;
    for (auto &argument : arguments) argument = substitute(argument, method, bindings, true);
    if (std::any_of(arguments.begin(), arguments.end(), [&](const auto &type) { return nested_binder(method, type); })) return {};
    const auto result = infer_result(id, arguments, *retained, state).contract;
    if (!result || nested_binder(method, *result->type)) return {};
    return complete_callable_type(method, arguments, *result);
  }
  static CheckedOperation literal(const Expression &expr) {
    using K = Expression::Kind;
    CheckedOperation operation;
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
    default: throw std::logic_error("expected scalar literal");
    }
    return operation;
  }
  bool signed_minimum(const Expression &expr, const TypeEvidenceState *state = nullptr) const {
    if (expr.kind != Expression::Kind::unary || expr.text != "-" || expr.children.front()->kind != Expression::Kind::integer) return false;
    auto magnitude = std::string_view(expr.children.front()->text);
    while (magnitude.starts_with('0')) magnitude.remove_prefix(1);
    if (magnitude != "9223372036854775808" || operation_name(expr, state) != "-") return false;
    if (applicable_operation(expr.text, {Type::integer}))
      implementation_gap(expr.at, "signed minimum literal with overloaded unary negation");
    return true;
  }
  static void check_exception_field(const Expression &expr) {
    if (expr.text != "category" && expr.text != "site" && expr.text != "requested" && expr.text != "available" && expr.text != "cause_count")
      fail(expr.at, "unknown intrinsic exception field '" + expr.text + "'");
  }
  void check_query_binding(const std::string &name, const Location &at, bool owner_metadata = false) const {
    if (const auto id = lexical_.lookup(name)) {
      if (!lexical_.bindings_available.contains(*id))
        fail(at, "binding '" + name + "' is not available on every reaching path");
      if (bindings_->at(*id).owner_binding && !owner_metadata) fail(at, "owner binding is not a value in typeof");
    }
  }
  static ViewEvidence join_views(const ViewEvidence &a, const ViewEvidence &b) {
    const auto either = [](std::optional<bool> x, std::optional<bool> y) -> std::optional<bool> {
      if (x.value_or(false) || y.value_or(false)) return true;
      return x && y ? std::optional<bool>{false} : std::nullopt;
    };
    const auto owner = a.view == false ? b.owner : b.view == false ? a.owner : a.owner == b.owner ? a.owner : std::nullopt;
    ViewEvidence result{either(a.read_only, b.read_only), either(a.view, b.view),
                        a.value_binding == b.value_binding ? a.value_binding : std::nullopt,
                        owner, a.mixed_view || b.mixed_view || a.view != b.view,
                        either(a.fixed_binding, b.fixed_binding), a.slot_read_only == b.slot_read_only ? a.slot_read_only : std::nullopt,
                        a.value_capture == b.value_capture ? a.value_capture : std::nullopt, a.nullable || b.nullable};
    if (!a.place_owners.empty() && !b.place_owners.empty()) {
      result.place_owners = a.place_owners;
      result.place_owners.insert(b.place_owners.begin(), b.place_owners.end());
    }
    if (a.storage_kind && b.storage_kind)
      result.storage_kind = a.storage_kind == b.storage_kind ? *a.storage_kind : PlaceKind::mixed;
    return result;
  }
  enum class GroupResult { value, nullable_target, unsupported };
  static GroupResult group_result(const Type &type, bool view, bool nullable, bool mixed) {
    if (nullable)
      return mixed ? GroupResult::unsupported : GroupResult::nullable_target;
    if (!view || scalar_type(type) || type.kind == Type::procedure) return GroupResult::value;
    return GroupResult::unsupported;
  }
  static bool independent_scalar_conditional(const Type &type, std::optional<bool> yes_view,
                                             std::optional<bool> no_view, bool mixed) {
    return scalar_type(type) && yes_view && no_view && (mixed || *yes_view != *no_view);
  }
  static void require_conditional_owner(OwnerMatch relation, const Location &at) {
    if (relation == OwnerMatch::mismatch) fail(at, "conditional reference result joins different owners");
    if (relation == OwnerMatch::unknown) implementation_gap(at, "conditional reference result requires one provable owner");
  }
  static TargetEvidence join_targets(TargetEvidence left, const TargetEvidence &right) {
    left.initialized = (left.initialized || left.presence == TargetPresence::absent) &&
        (right.initialized || right.presence == TargetPresence::absent);
    if (left.presence == TargetPresence::absent) left.places = right.places;
    else if (right.presence != TargetPresence::absent)
      left.places = QuerySelections::join_places(std::move(left.places), right.places);
    if (left.presence != right.presence) left.presence = TargetPresence::unknown;
    if (left.selection.slot != right.selection.slot) left.selection.slot.reset();
    if (left.selection.footprint.empty() || right.selection.footprint.empty()) left.selection.footprint.clear();
    else left.selection.footprint.insert(right.selection.footprint.begin(), right.selection.footprint.end());
    left.branches = QuerySelections::alternatives(std::move(left.branches), right.branches);
    left.presence_proof = QuerySelections::alternatives(std::move(left.presence_proof), right.presence_proof);
    left.contextual_null = left.contextual_null && right.contextual_null;
    left.same_selected = QuerySelections::common_slots(std::move(left.same_selected), right.same_selected);
    return left;
  }
  static void require_present_projection(const ViewEvidence &view, TargetPresence presence, const Location &at) {
    if (view.view != true || !view.nullable || presence == TargetPresence::present) return;
    if (presence == TargetPresence::absent) fail(at, "conditional scalar value requires a proven non-null target");
    implementation_gap(at, "conditional scalar value requires a proven non-null target in typeof");
  }
  static bool return_owner_filter(const Expression &pattern) {
    return pattern.binding.empty() && !pattern.owner_name.empty();
  }
  struct ReturnPatternEvidence {
    std::optional<Type> binding;
    OwnerMatch matches = OwnerMatch::satisfied;
  };
  static ResultMode return_pattern_mode(const Expression &pattern) {
    if (pattern.declaration_kind == "ptr") return ResultMode::borrowed_ptr;
    if (pattern.declaration_kind == "ref" ||
        (pattern.declaration_kind.empty() && !pattern.owner_name.empty())) return ResultMode::borrowed_ref;
    return ResultMode::value;
  }
  static ReturnPatternEvidence borrowed_return_pattern(const Expression &pattern, const std::optional<Type> &annotation,
      const std::optional<Type> &result, const ResultContract &contract, OwnerMatch match, bool required) {
    if (required && annotation == Type::void_type) fail(pattern.at, "cannot bind a void return");
    auto binding = annotation ? annotation : result;
    if (binding == Type::void_type) binding = Type::never;
    if (return_pattern_mode(pattern) != contract.mode || (result && annotation && result != annotation) ||
        (annotation && !pattern.read_only && contract.read_only)) match = OwnerMatch::mismatch;
    return {binding, match};
  }
  static bool bindable_return_value(const Type &type) {
    return scalar_type(type) || (type.kind == Type::procedure && type.pure_callable);
  }
  static ReturnPatternEvidence value_return_pattern(const Expression &pattern, const std::optional<Type> &annotation,
                                                     const std::optional<Type> &result, bool borrowed, bool required) {
    if (pattern.binding.empty() && pattern.owner_name.empty()) return {};
    if (required && pattern.declaration_kind == "val" && !pattern.owner_name.empty())
      fail(pattern.at, "a val return pattern cannot specify an owner");
    const bool value_pattern = !pattern.binding.empty() && pattern.owner_name.empty() &&
        (pattern.declaration_kind.empty() || pattern.declaration_kind == "val");
    if (required && value_pattern && annotation == Type::void_type) fail(pattern.at, "cannot bind a void return");
    if (required && (!value_pattern || (annotation && !bindable_return_value(*annotation)) ||
                     (result && result != Type::void_type && !bindable_return_value(*result))))
      implementation_gap(pattern.at, "this result mode, type or owner in deferred return inspection");
    const bool matches = !borrowed && result != Type::void_type && (!annotation || !result || annotation == result);
    auto binding = annotation ? annotation : result;
    if (binding == Type::void_type) binding = Type::never;
    return {binding, matches ? OwnerMatch::satisfied : OwnerMatch::mismatch};
  }
  ReturnPatternEvidence return_pattern_evidence(const Expression &pattern, const TypeEvidenceState &state) const {
    if (return_owner_filter(pattern)) {
      const auto owner = pattern.owner_name == "here" ? state.full_scope_owner : owner_evidence(pattern.owner_name, state, pattern.at);
      const auto match = state.invocation_result_owner ? compare_owners(state.invocation_result_owner, owner, &state) : OwnerMatch::mismatch;
      return {{}, match};
    }
    const auto annotation = pattern.annotation ? std::optional{evidence_annotation(*pattern.annotation, state)} : std::nullopt;
    const auto *contract = state.borrowed_return;
    if (state.method) {
      const auto &method = program_.methods[*state.method];
      if (method.here_group) return value_return_pattern(pattern, annotation, method.here_group->supplier, false, state.required);
      if (!contract) contract = &method.result;
    }
    if (return_pattern_mode(pattern) != ResultMode::value) {
      auto match = OwnerMatch::satisfied;
      if (!pattern.owner_name.empty()) {
        const auto owner = pattern.owner_name == "here" ? state.full_scope_owner : owner_evidence(pattern.owner_name, state, pattern.at);
        match = state.invocation_result_owner ? compare_owners(state.invocation_result_owner, owner, &state) : OwnerMatch::mismatch;
      }
      return borrowed_return_pattern(pattern, annotation, state.invocation_result, contract ? *contract : ResultContract{}, match, state.required);
    }
    return value_return_pattern(pattern, annotation, state.invocation_result, contract && contract->borrowed(), state.required);
  }
  struct ConditionalEvidence {
    std::optional<Type> yes, no;
    TypeEvidenceState yes_state, no_state;
    bool independent() const {
      return yes && yes == no && independent_scalar_conditional(*yes, yes_state.value.view, no_state.value.view,
          yes_state.value.mixed_view || no_state.value.mixed_view);
    }
  };
  ConditionalEvidence conditional_arms(const Expression &expr, const TypeEvidenceState &state, bool contextual,
                                       const Type *expected, std::vector<Type> *alternatives,
                                       ExpectedTypeUse expected_use, bool reached, EvidenceRead read = EvidenceRead::payload) const {
    ConditionalEvidence arms{{}, {}, state, state};
    const auto &condition = *expr.children.front();
    const bool return_test = condition.kind == Expression::Kind::return_test;
    const auto &proof = state.target.branches;
    const bool yes_reaches = state.selections.reachable(proof, true);
    const bool no_reaches = state.selections.reachable(proof, false);
    arms.yes_state.selections.assume(proof, true);
    arms.no_state.selections.assume(proof, false);
    // A branch predicated on a prepared return cannot provide the initial
    // evidence required to prepare that return.
    if (return_test) arms.yes_state.returns = nullptr;
    const bool binds = return_test && !condition.binding.empty();
    TypeEvidenceExits matching_exits;
    if (binds) {
      auto &yes = arms.yes_state;
      const auto &name = condition.binding;
      const auto mode = return_pattern_mode(condition);
      const bool value = mode == ResultMode::value;
      const bool reference = mode == ResultMode::borrowed_ref;
      const auto pattern = return_pattern_evidence(condition, state);
      const auto *contract = state.borrowed_return;
      if (!contract && state.method) contract = &program_.methods[*state.method].result;
      const bool readonly = condition.read_only || (value && condition.fixed_binding) ||
          (!value && pattern.matches != OwnerMatch::mismatch && contract && contract->read_only);
      yes.bindings[name] = pattern.binding;
      yes.local_declarations[name] = &condition;
      yes.definitions.erase(name);
      yes.owner_bindings.erase(name);
      yes.selections.remove_binding(name);
      const auto owner = EvidenceOwner::for_scope(condition);
      auto target_owner = value ? std::optional{owner} : state.invocation_result_owner;
      if (!value && pattern.matches == OwnerMatch::mismatch) {
        if (!condition.owner_name.empty() && condition.owner_name != "here")
          target_owner = owner_evidence(condition.owner_name, state, condition.at);
        else if (!target_owner || condition.owner_name == "here") target_owner = state.full_scope_owner;
      }
      yes.views[name] = {readonly, true, value, target_owner, false, condition.fixed_binding, readonly, value,
                         mode == ResultMode::borrowed_ptr};
      yes.views.at(name).place_owners.insert(owner);
      yes.views.at(name).storage_kind = place_kind(value, reference);
      if (yes.selections.tracking) {
        if (value) yes.selections.value_places.emplace(name, EvidencePlaces{&condition});
        else {
          TargetEvidence selected;
          if (reference) selected.presence = TargetPresence::present;
          yes.selections.bind(name, &condition, std::move(selected));
        }
      }
      if (state.exits) yes.exits = &matching_exits;
    }
    arms.yes = type_evidence(*expr.children[1], arms.yes_state, contextual, expected, return_test ? nullptr : alternatives,
                            expected_use, reached && yes_reaches, read);
    arms.yes_state.returns = state.returns;
    if (binds) {
      restore_type_binding(arms.yes_state, state, condition.binding);
      arms.yes_state.exits = state.exits;
      for (auto &exit : matching_exits.paths) {
        restore_type_binding(exit.state, state, condition.binding);
        forward_type_exit(*state.exits, exit);
      }
    }
    arms.no = type_evidence(*expr.children[2], arms.no_state, contextual, expected, alternatives, expected_use, reached && no_reaches, read);
    return arms;
  }
  static std::optional<EvidenceOwner> owner_evidence(const std::string &name, const TypeEvidenceState &state, const Location &at) {
    if (const auto found = state.owner_bindings.find(name); found != state.owner_bindings.end()) return found->second;
    if (state.required) fail(at, state.bindings.contains(name) ? "nearest binding '" + name + "' is not an owner" : "unknown owner '" + name + "'");
    return {};
  }
  static TypeEvidenceState merge_evidence(TypeEvidenceState left, const TypeEvidenceState &right, bool right_exits = false) {
    const auto unavailable = [&](const std::string &name) {
      left.bindings[name].reset();
      left.views[name] = {};
      left.definitions.erase(name);
      left.owner_bindings.erase(name);
      left.selections.remove_binding(name);
    };
    // An ending arm can introduce a nearer lexical declaration, but cannot
    // establish that declaration's availability at the normal continuation.
    for (const auto &[name, declaration] : right.local_declarations) {
      const auto [entry, inserted] = left.local_declarations.try_emplace(name, declaration);
      const bool different = !inserted && entry->second != declaration;
      if (different) entry->second = nullptr;
      if (inserted || different) unavailable(name);
    }
    if (right_exits) return left;
    if (left.pending_return != right.pending_return) left.pending_return.reset();
    for (auto &[name, declaration] : left.local_declarations) {
      if (const auto other = right.local_declarations.find(name);
          other == right.local_declarations.end() || !declaration || other->second != declaration) {
        declaration = nullptr;
        unavailable(name);
      }
    }
    std::erase_if(left.definitions, [&](const auto &entry) {
      const auto found = right.definitions.find(entry.first);
      return found == right.definitions.end() || found->second.method != entry.second.method ||
          found->second.declaration != entry.second.declaration;
    });
    std::erase_if(left.owner_bindings, [&](const auto &owner) {
      const auto found = right.owner_bindings.find(owner.first);
      return found == right.owner_bindings.end() || found->second != owner.second;
    });
    for (auto &[name, type] : left.bindings) {
      const auto other = right.bindings.find(name);
      if (other == right.bindings.end() || other->second != type) type.reset();
    }
    for (const auto &[name, type] : right.bindings)
      if (!left.bindings.contains(name)) left.bindings.emplace(name, std::nullopt);
    for (auto &[name, view] : left.views) {
      const auto other = right.views.find(name);
      if (other == right.views.end() || other->second != view) view = {};
    }
    for (const auto &[name, view] : right.views)
      if (!left.views.contains(name)) left.views.emplace(name, ViewEvidence{});
    auto &selection = left.selections;
    const auto &other_selections = right.selections;
    const auto retain_shared = [](auto &names, const auto &other_names) {
      std::erase_if(names, [&](const auto &item) {
        const auto other = other_names.find(item.first);
        return other == other_names.end() || other->second != item.second;
      });
    };
    retain_shared(selection.bindings, other_selections.bindings);
    retain_shared(selection.value_places, other_selections.value_places);
    const auto combine = [](auto &into, const auto &from) {
      for (const auto &[slot, footprint] : from) into[slot].insert(footprint.begin(), footprint.end());
    };
    combine(selection.footprints, other_selections.footprints);
    combine(selection.possible_overlap, other_selections.possible_overlap);
    if (left.deferred && left.deferred == right.deferred)
      left.deferred_queue = left.deferred->join(left, right);
    selection.join_flow(other_selections);
    left.has_deferred_action |= right.has_deferred_action;
    left.complete = left.complete && right.complete;
    return left;
  }
  static TypeEvidenceState merge_reaching_evidence(TypeEvidenceState left, TypeEvidenceState right,
                                                   bool left_exits, bool right_exits) {
    if (left_exits && !right_exits) return merge_evidence(std::move(right), left, true);
    return merge_evidence(std::move(left), right, right_exits && !left_exits);
  }
  static void merge_type_exits(TypeEvidenceExits &flow, std::set<size_t> reasons, const TypeEvidenceState &state,
                               TypeReturned result = {}, bool retained_return = false) {
    if (reasons.empty()) return;
    if (result.borrowed) result.borrowed->initialize(state.selections);
    if (result.boolean) result.advance_boolean(state.selections);
    const auto entry = std::ranges::find_if(flow.paths, [&](const auto &path) {
      return path.reasons == reasons && path.retained_return == retained_return &&
          path.result.saved_selection() == result.saved_selection();
    });
    if (entry == flow.paths.end()) {
      flow.paths.push_back({std::move(reasons), state, std::move(result), retained_return});
      flow.paths.back().state.exits = nullptr;
    } else {
      entry->state = merge_evidence(std::move(entry->state), state);
      entry->result.merge(result);
      entry->result.boolean_revision = entry->state.selections.writes;
    }
  }
  static void note_type_exit(const TypeEvidenceState &state, Completion completion, TypeReturned result = {}) {
    if (state.exits) merge_type_exits(*state.exits, {completion.reason}, state, std::move(result));
  }
  static void note_type_allocation(const TypeEvidenceState &state) {
    note_type_exit(state, Completion::failure(exception_id(ExceptionCategory::out_of_memory)));
  }
  bool has_type_captures(size_t method, const TypeEvidenceState &state) const {
    const auto &source = *program_.methods[method].source;
    if (!lexical_methods_.contains(&source)) return false;
    if (source.explicit_captures) return !source.captures.empty();
    return std::ranges::any_of(free_bindings(source).captures, [&](const Capture &capture) {
      return state.bindings.contains(capture.name) || lexical_self(capture.name, state.method);
    });
  }
  void note_family_type_capture(const TypeEvidenceState &state, const NamedFamily &family) const {
    if (!state.exits) return;
    const auto &declaration = family.declaration ? *family.declaration : state;
    if (std::ranges::any_of(family.methods, [&](size_t method) { return has_type_captures(method, declaration); }))
      note_type_allocation(state);
  }
  template <typename Callable>
  static void note_type_call(const TypeEvidenceState &state, const Callable &callable) {
    if (!state.exits) return;
    auto reasons = call_reasons(callable);
    reasons.erase(0);
    merge_type_exits(*state.exits, std::move(reasons), state);
  }
  static void record_type_invocation(const Expression &expr, TypeEvidenceState &state, std::set<size_t> outcomes,
                                     bool explicit_effects) {
    if (state.preparing(expr)) {
      auto &effects = state.prepared_call->effects;
      effects.outcomes.merge(outcomes);
      effects.opaque |= !explicit_effects;
    } else if (state.exits) {
      outcomes.erase(0);
      merge_type_exits(*state.exits, std::move(outcomes), state);
    }
  }
  template <typename Callable>
  static void record_type_call(const Expression &expr, TypeEvidenceState &state, const Callable &callable,
                               bool explicit_effects) {
    if (state.preparing(expr) || state.exits)
      record_type_invocation(expr, state, call_reasons(callable), explicit_effects);
  }
  bool module_storage_is_closed(size_t root) const {
    if (const auto cached = closed_module_storage_.find(root); cached != closed_module_storage_.end()) return cached->second;
    std::set<size_t> visited;
    std::vector<size_t> pending{root};
    const auto prove = [&] {
      while (!pending.empty()) {
        const auto id = pending.back(); pending.pop_back();
        if (!visited.insert(id).second) continue;
        if (const auto cached = closed_module_storage_.find(id); cached != closed_module_storage_.end()) {
          if (!cached->second) return false;
          continue;
        }
        const auto &method = program_.methods.at(id);
        const auto &source = *method.source;
        if (!source.body || lexical_methods_.contains(&source)) return false;
        if (source.kind == Form::Kind::function) continue;
        const auto &uses = free_bindings(source);
        if (uses.opaque_storage) return false;
        const auto &names = modules_.at(method.module);
        for (const auto *call : uses.runtime_calls) {
          using K = Expression::Kind;
          if (call->intrinsic) continue;
          const auto &name = call->text;
          if (uses.declarations.contains(name) || names.values.contains(name)) return false;
          if (call->kind == K::behave || call->kind == K::perform) {
            const auto symbol = names.symbols.find(name);
            if (symbol == names.symbols.end() || !program_.symbols.at(symbol->second).intrinsic_print) return false;
            continue;
          }
          const auto family = names.callables.find(name);
          if (family == names.callables.end()) {
            if (!eager_operation(name) && !swap_operation(name) && !identity_operation(name) && name != "cause_at")
              return false;
            continue;
          }
          // A body may select any applicable method in its interpretation.
          // Close the whole family without demanding instances from a query.
          const auto *methods = &program_.families.at(family->second).methods;
          if (method.interpretation) {
            const auto view = family_views_.find({family->second, method.interpretation});
            if (view == family_views_.end() || !view->second.unproved.empty()) return false;
            methods = &view->second.methods;
          }
          if (methods->empty()) return false;
          pending.insert(pending.end(), methods->begin(), methods->end());
        }
      }
      return true;
    };
    if (!prove()) {
      closed_module_storage_.emplace(root, false);
      return false;
    }
    for (const auto id : visited) closed_module_storage_.try_emplace(id, true);
    return true;
  }
  bool explicit_type_call_effects(size_t method, const TypeEvidenceState &state, const std::vector<Type> &arguments) const {
    // The caller handles the effects reachable through explicit borrowed
    // arguments and binding slots. Exclude hidden channels into its storage.
    const auto &source = *program_.methods[method].source;
    if (!std::ranges::all_of(arguments, scalar_type)) return false;
    if (source.kind != Form::Kind::function && !module_storage_is_closed(method)) return false;
    const auto &environment = state.declaration && method == state.method ? *state.declaration : state;
    if (!has_type_captures(method, environment)) return true;
    const auto independent = [&](const Capture &capture) {
      const auto type = environment.bindings.find(capture.name);
      const auto view = environment.views.find(capture.name);
      return type != environment.bindings.end() && type->second && scalar_type(*type->second) &&
          view != environment.views.end() && view->second.value_capture == true;
    };
    const auto &free = free_bindings(source).captures;
    return std::ranges::all_of(source.captures, independent) && std::ranges::all_of(free, [&](const Capture &capture) {
      return (!environment.bindings.contains(capture.name) && !lexical_self(capture.name, environment.method)) || independent(capture);
    });
  }
  static std::set<size_t> deferred_outcomes() {
    std::set<size_t> outcomes{0, 2};
    for (size_t category = 1; category < exception_categories.size(); ++category)
      outcomes.insert(Completion::failure(category).reason);
    return outcomes;
  }
  static void register_type_defer(TypeEvidenceState &state, const Expression *entry, bool may_replace,
                                  TypeDeferredEffects effects) {
    note_type_allocation(state);
    state.has_deferred_action = true;
    state.selections.deferred_writes.merge(effects.payloads);
    state.selections.deferred_writes.unknown |= !effects.outcomes.empty() && effects.opaque;
    state.selections.deferred_return |= may_replace;
    if (state.deferred) state.deferred_queue = state.deferred->push(state, entry, may_replace, std::move(effects));
  }
  static void note_type_failure(const TypeEvidenceState &state, const Type &failure) {
    if (!state.exits) return;
    std::set<size_t> reasons;
    for (const auto category : exception_possibilities(failure))
      reasons.insert(Completion::failure(category).reason);
    merge_type_exits(*state.exits, std::move(reasons), state);
  }
  static void note_type_failures(const TypeEvidenceState &state, const std::vector<Type> &failures) {
    for (const auto &failure : failures)
      note_type_failure(state, failure);
  }
  static void require_type_initialized(const TypeEvidenceState &state, const TargetEvidence &target,
                                       std::string_view name, const Location &at) {
    if (!state.checks_initialization() || !state.selections.initialization_reachable ||
        target.places.empty() || state.selections.selection_initialized(target)) return;
    const auto reaching = state.selections.outcome(state.selections.selection_outcomes(target), true);
    for (const auto &place : target.places) if (state.selections.may_read_uninitialized(place, reaching)) {
      if (name.empty()) if (const auto *local = std::get_if<const Expression *>(&place)) name = (*local)->text;
      fail(at, "read of '" + std::string(name.empty() ? "target" : name) +
          "' before definite initialization of every possible target");
    }
  }
  static void read_type_target(TypeEvidenceState &state, const ViewEvidence &view, const TargetEvidence &target) {
    if (!state.selections.tracking || !view.nullable || target.presence == TargetPresence::present) return;
    note_type_exit(state, Completion::failure(exception_id(ExceptionCategory::null_pointer)));
    state.selections.assume(state.selections.selection_outcomes(target), true);
  }
  static TargetEvidence named_type_target(const TypeEvidenceState &state, const std::string &name) {
    TargetEvidence target;
    if (const auto slot = state.selections.bindings.find(name); slot != state.selections.bindings.end())
      target = state.selections.selected_target(slot->second);
    if (const auto place = state.selections.value_places.find(name); place != state.selections.value_places.end())
      target.places = place->second;
    return target;
  }
  static void type_boolean_write(const Expression &expr, TypeEvidenceState &state, const EvidencePlaces &places,
                                 const std::optional<PresenceBranches> &proof = {}) {
    if (state.preparing(expr)) state.prepared_call->effects.payloads.record(places);
    else state.selections.write_boolean(places, proof);
  }
  static void type_slot_write(const Expression &expr, TypeEvidenceState &state, const EvidenceSelection &slot) {
    if (state.preparing(expr)) state.prepared_call->effects.slots.record(slot.footprint);
    else state.selections.assign(slot, {});
  }
  static PresenceBranches snapshot_type_branches(const TypeEvidenceState &state,
                                                   const std::optional<PresenceBranches> &value) {
    auto saved = state.selections.observe(value);
    saved.forget([&](const auto &slot) { return !state.selections.entry_selections.contains(slot); });
    if (state.repeated_observations) saved.forget_observations(*state.repeated_observations);
    return saved;
  }
  static std::optional<EvidenceSlot> prepared_entry_target(const TargetEvidence &target) {
    for (const auto &slot : target.same_selected) if (std::holds_alternative<PreparedEntryTarget>(slot)) return slot;
    return {};
  }
  static void capture_preparation_targets(const Expression &entry, TypeEvidenceState &state) {
    if (!state.deferred || !state.required || !state.checks_initialization() || state.repeated_observations) return;
    auto &selections = state.selections;
    size_t ordinal = 0;
    // Capture once before a fork. Each arm can then use the same target
    // identity even when only one of them registers an action.
    for (const auto &[name, binding] : selections.bindings) {
      if (!binding.slot) continue;
      auto target = selections.selected_target(binding);
      if (target.places.empty() || selections.selection_initialized(target) || prepared_entry_target(target)) continue;
      EvidenceSlot snapshot = PreparedEntryTarget{&entry, ordinal++};
      while (selections.footprints.contains(snapshot)) snapshot = PreparedEntryTarget{&entry, ordinal++};
      selections.save_selection(snapshot, std::move(target));
    }
  }
  static std::optional<EvidenceSlot> save_prepared_type_target(TypeEvidenceState &state,
      const Expression &site, TargetEvidence target) {
    const auto entry = prepared_entry_target(target);
    if (entry || state.repeated_observations) return entry;
    state.selections.save_selection(&site, std::move(target));
    return &site;
  }
  static TargetEvidence snapshot_type_target(const TypeEvidenceState &state, TargetEvidence selected) {
    selected.selection = {};
    std::erase_if(selected.same_selected, [&](const auto &slot) { return !state.selections.entry_selections.contains(slot); });
    selected.presence_proof = snapshot_type_branches(state, selected.presence_proof);
    return selected;
  }
  static void apply_type_builtin(QuerySelections &selections, const PreparedBuiltin &operation) {
    std::visit([&](const auto &effect) {
      using Effect = std::decay_t<decltype(effect)>;
      if constexpr (std::is_same_v<Effect, PreparedTransfer>) {
        if (effect.saved_selection) {
          auto target = selections.selected_target({effect.saved_selection, {*effect.saved_selection}});
          target.places = effect.destination;
          selections.initialize(target);
        } else selections.initialize(effect.destination);
        if (effect.writes_boolean) selections.write_boolean(effect.destination, effect.boolean);
      }
      else if constexpr (std::is_same_v<Effect, PreparedReferenceAssignment>)
        selections.assign(effect.destination, effect.value);
      else if constexpr (std::is_same_v<Effect, PreparedBooleanSwap>)
        selections.exchange_boolean(effect.left, effect.right);
      else if constexpr (std::is_same_v<Effect, PreparedReferenceSwap>)
        selections.exchange(effect.left, effect.right);
    }, operation);
  }
  static void type_input_mutation(const Expression &expr, TypeEvidenceState &state, const Type &argument, const TargetEvidence &target,
                                  ArgumentMode mode, bool readonly, bool writable_slot = false) {
    if (writable_slot) type_slot_write(expr, state, target.selection);
    if (argument == Type::boolean && mode != ArgumentMode::value && !readonly)
      type_boolean_write(expr, state, writable_slot ? EvidencePlaces{} : target.places);
  }
  // Evidence only: no reads, captures, demands, storage or cleanup equations are created here.
  // Operand hints defer validation to ordered checking; required typeof queries validate their bounded source forms here.
  std::optional<Type> type_evidence(const Expression &expr, TypeEvidenceState &state, bool contextual = false,
                                    const Type *expected = nullptr, std::vector<Type> *alternatives = nullptr,
                                    ExpectedTypeUse expected_use = ExpectedTypeUse::materialization, bool reached = true,
                                    EvidenceRead read = EvidenceRead::payload) const {
    using K = Expression::Kind;
    const ModuleLookup lookup(module_, state.method ? program_.methods[*state.method].module : module_);
    const ModuleLookup context(interpretation_, state.method ? program_.methods[*state.method].interpretation : interpretation_);
    ViewEvidence view{false, false, {}};
    TargetEvidence target;
    std::vector<Type> independent;
    auto *const exits = state.exits;
    auto *const deferred = state.deferred;
    const auto deferred_queue = state.deferred_queue;
    const auto initialization = state.initialization;
    if (!reached) {
      state.exits = nullptr;
      state.deferred = nullptr;
      state.initialization = TypeEvidenceState::InitializationCheck::suppressed;
    }
    const auto type = query_evidence(expr, state, view, target, contextual, expected, alternatives ? &independent : nullptr, expected_use, read);
    if (state.checks_initialization() && !target.places.empty() && !target.initialized) {
      const auto reaching = state.selections.outcome(state.selections.selection_outcomes(target), true);
      target.initialized = state.selections.selection_initialized(target) ||
          std::ranges::none_of(target.places, [&](const auto &place) {
            return state.selections.may_read_uninitialized(place, reaching);
          });
    }
    const bool reads_result = read == EvidenceRead::payload && !state.preparing(expr);
    // Bare names still read their payload in statement position. A borrowed
    // result of a call, group or rebind is read only when the result is used.
    if ((reads_result || (read == EvidenceRead::discarded && expr.kind == K::name)) &&
        type && *type != Type::never && *type != Type::null_ref &&
        expr.kind != K::block && expr.kind != K::conditional && expr.kind != K::try_scope) {
      require_type_initialized(state, target, expr.kind == K::name ? expr.text : std::string_view{}, expr.at);
      read_type_target(state, view, target);
    }
    state.exits = exits;
    if (!reached) {
      state.deferred = deferred;
      state.deferred_queue = deferred_queue;
      state.initialization = initialization;
    }
    if (alternatives && type != Type::never) {
      if (type) alternatives->push_back(*type);
      else alternatives->insert(alternatives->end(), independent.begin(), independent.end());
    }
    if (state.required && !type) implementation_gap(expr.at, "typeof requires a known declaration or operand type with a proved result contract");
    state.value = type && type != Type::never ? view : ViewEvidence{};
    if (type && type != Type::never && state.selections.tracking) {
      if (view.view.has_value() && !view.nullable) target.presence = TargetPresence::present;
      if (read == EvidenceRead::payload && type == Type::boolean && !target.branches)
        target.branches = state.selections.read_boolean(target.places);
      state.target = std::move(target);
    } else state.target = {};
    return type;
  }
  Type evidence_annotation(const Type &type, const TypeEvidenceState &state) const {
    return state.method ? substitute(type, program_.methods[*state.method], state.substitutions, true) : resolve_type(type);
  }
  static bool type_loop_control(const Expression *declaration) {
    return declaration && (declaration->kind == Expression::Kind::range_for || declaration->kind == Expression::Kind::fold);
  }
  static void enter_type_defer(TypeEvidenceState &state) {
    for (const auto &[name, declaration] : state.local_declarations)
      if (type_loop_control(declaration)) state.delayed_controls.insert(declaration);
  }
  static void check_type_capture(const TypeEvidenceState &state, const std::string &name, const Location &at) {
    if (!state.required || state.delayed_controls.empty()) return;
    const auto declared = state.local_declarations.find(name);
    if (declared != state.local_declarations.end() && state.delayed_controls.contains(declared->second))
      fail(at, "a deferred block cannot capture loop-control binding '" + name + "'");
  }
  static void prepare_type_readable_argument(const TypeEvidenceState &state, const Expression &operand,
                                             const ViewEvidence &view, const TargetEvidence &target, ArgumentMode mode) {
    if (!state.checks_initialization() || !state.selections.initialization_reachable) return;
    if (mode == ArgumentMode::borrowed_ref && view.nullable &&
        state.selections.reachable(state.selections.selection_outcomes(target), false))
      fail(operand.at, "ref argument requires a proven non-null target");
    const auto name = operand.kind == Expression::Kind::name ? std::string_view{operand.text} : std::string_view{};
    require_type_initialized(state, target, name, operand.at);
  }
  static void check_type_deferred_argument(const TypeEvidenceState &state, const Expression &call, const Expression &operand,
                                           const ViewEvidence &view, ArgumentMode mode) {
    if (!state.required || !state.preparing(call) || mode == ArgumentMode::value || !view.owner) return;
    for (const auto &[name, declaration] : state.local_declarations)
      if (type_loop_control(declaration) && *view.owner == EvidenceOwner::for_scope(*declaration))
        fail(operand.at, "deferred borrowed argument does not outlive its cleanup scope");
  }
  TypeEvidenceState catch_evidence(const Expression &handler, const TypeEvidenceState &entry,
                                   std::set<size_t> &handled) const {
    const auto type = handler.annotation ? evidence_annotation(*handler.annotation, entry) : exception_type(0);
    if (entry.required) check_catch_category(type, handled, handler.at);
    auto caught = entry;
    caught.caught_exception = type;
    caught.bindings[handler.text] = type;
    caught.local_declarations[handler.text] = &handler;
    caught.definitions.erase(handler.text);
    caught.owner_bindings.erase(handler.text);
    caught.selections.remove_binding(handler.text);
    caught.views[handler.text] = {true, true, true, EvidenceOwner::for_scope(handler), false, false, true, true};
    caught.views.at(handler.text).place_owners.insert(EvidenceOwner::for_scope(handler));
    caught.views.at(handler.text).storage_kind = PlaceKind::value;
    return caught;
  }
  static void restore_type_binding(TypeEvidenceState &state, const TypeEvidenceState &entry, const std::string &name) {
    const auto restore = [&](auto &bindings, const auto &outer) {
      if (const auto found = outer.find(name); found != outer.end()) bindings.insert_or_assign(name, found->second);
      else bindings.erase(name);
    };
    restore(state.bindings, entry.bindings);
    restore(state.local_declarations, entry.local_declarations);
    restore(state.definitions, entry.definitions);
    restore(state.owner_bindings, entry.owner_bindings);
    restore(state.views, entry.views);
    restore(state.selections.bindings, entry.selections.bindings);
    restore(state.selections.value_places, entry.selections.value_places);
  }
  static void leave_type_catch(TypeEvidenceState &caught, const TypeEvidenceState &entry, const std::string &name) {
    restore_type_binding(caught, entry, name);
    caught.caught_exception = entry.caught_exception;
  }
  static std::optional<TypeEvidenceState> take_type_catch(TypeEvidenceExits &incoming, const Type &type) {
    std::optional<TypeEvidenceState> selected;
    const auto reason = Completion::failure(type.identity).reason;
    for (auto &path : incoming.paths)
      if (std::erase_if(path.reasons, [&](size_t outcome) { return outcome >= 3 && (!type.identity || outcome == reason); }))
        selected = selected ? merge_evidence(std::move(*selected), path.state) : path.state;
    return selected;
  }
  static void forward_type_exit(TypeEvidenceExits &outgoing, TypeEvidenceExits::Path &path) {
    merge_type_exits(outgoing, std::move(path.reasons), path.state, std::move(path.result), path.retained_return);
  }
  std::optional<Type> complete_type_finalizer(const Expression &handler, TypeEvidenceExits::Path &pending,
                                              TypeEvidenceExits &outgoing, CleanupBoundary boundary) const {
    TypeEvidenceExits exits;
    const auto inherited = pending.state.pending_return;
    if (pending.returned()) pending.state.pending_return = true;
    pending.state.exits = &exits;
    const auto type = type_evidence(handler, pending.state);
    pending.state.exits = nullptr;
    pending.state.pending_return = inherited;
    for (auto &path : exits.paths) {
      path.state.pending_return = inherited;
      if (boundary == CleanupBoundary::deferred && !path.returned() && pending.returned()) {
        path.retained_return = true;
        path.result = pending.result;
      }
      forward_type_exit(outgoing, path);
    }
    return pending.state.selections.initialization_reachable ? type : std::optional<Type>{Type::never};
  }
  TypeEvidenceExits replay_type_defer(const Expression &entry, TypeEvidenceState state, size_t pending_error, bool returned) const {
    using K = Expression::Kind;
    TypeEvidenceExits incoming, outgoing;
    enter_type_defer(state);
    state.pending_return = returned;
    state.exits = &incoming;
    // The current lowering enters any guarded deferred entry with its pending
    // failure. Model this until native cleanup separates the error queue.
    if (entry.children.size() > 1 && pending_error)
      merge_type_exits(incoming, {pending_error}, state);
    else if (type_evidence(*entry.children.front(), state, false, nullptr, nullptr,
                           ExpectedTypeUse::materialization, true, EvidenceRead::discarded) != Type::never &&
             state.selections.initialization_reachable)
      merge_type_exits(outgoing, {0}, state);

    const Expression *finalizer = nullptr;
    std::set<size_t> handled;
    for (size_t i = 1; i < entry.children.size(); ++i) {
      const auto &handler = *entry.children[i];
      if (handler.kind == K::block) {
        finalizer = &handler;
        break;
      }
      const auto type = handler.annotation ? evidence_annotation(*handler.annotation, state) : exception_type(0);
      const auto selected = take_type_catch(incoming, type);
      if (!selected) continue;
      auto caught = catch_evidence(handler, *selected, handled);
      TypeEvidenceExits exits;
      caught.exits = &exits;
      if (type_evidence(*handler.children.front(), caught, false, nullptr, nullptr,
                        ExpectedTypeUse::materialization, true, EvidenceRead::discarded) != Type::never &&
          caught.selections.initialization_reachable)
        merge_type_exits(exits, {0}, caught);
      for (auto &path : exits.paths) {
        leave_type_catch(path.state, *selected, handler.text);
        forward_type_exit(outgoing, path);
      }
    }
    for (auto &path : incoming.paths) forward_type_exit(outgoing, path);
    if (!finalizer) return outgoing;

    TypeEvidenceExits completed;
    for (auto &path : outgoing.paths)
      if (complete_type_finalizer(*finalizer, path, completed, CleanupBoundary::deferred) != Type::never)
        forward_type_exit(completed, path);
    return completed;
  }
  std::optional<Type> project_finalizer(const Expression &handler, TypeEvidenceState &state) const {
    const bool required = state.required;
    auto *const returns = state.returns;
    auto *const exits = state.exits;
    TypeEvidenceState::Returns projected_returns;
    state.required = false;
    state.exits = nullptr;
    if (returns) state.returns = &projected_returns;
    const auto type = type_evidence(handler, state);
    state.required = required;
    state.returns = returns;
    state.exits = exits;
    return type;
  }
  std::optional<PresenceBranches> complete_type_deferred(const TypeDeferredFlow &flow, const TypeEvidenceExits &exits,
                                                        const TypeEvidenceState &body, bool falls_through,
                                                        TypeEvidenceExits *completed) const {
    const bool may_replace = body.selections.deferred_return || std::ranges::any_of(exits.paths,
        [](const auto &path) { return path.state.selections.deferred_return; });
    bool returning = false;
    std::optional<PresenceBranches> proof = PresenceBranches{{}, {}};
    const auto collect = [&](const auto &result) {
      proof = QuerySelections::alternatives(std::move(proof), result);
      returning = true;
    };
    if (!completed && !may_replace && !body.tracks_target_places()) {
      for (const auto &path : exits.paths) if (path.returned()) collect(path.result.boolean);
      return returning ? proof : std::nullopt;
    }

    struct Pending {
      TypeEvidenceState state;
      TypeReturned result;
    };
    const bool guards_errors = completed || body.tracks_target_places() || std::ranges::any_of(flow.entries, [](const auto &entry) {
      const auto *action = std::get_if<TypeDeferredFlow::Action>(&entry);
      return action && action->entry && action->entry->children.size() > 1;
    });
    using WorkKey = std::tuple<size_t, bool, size_t, const Expression *>;
    std::map<WorkKey, Pending> pending, loop_inputs;
    const auto enqueue = [&](size_t queue, bool returned, size_t error, TypeEvidenceState state,
                             TypeReturned result) {
      if (body.checks_initialization() && !state.selections.initialization_reachable) return;
      state.exits = nullptr;
      state.deferred = nullptr;
      if (returned) {
        const auto condition = state.selections.reach_condition();
        if (body.borrowed_return) {
          if (result.borrowed) {
            result.borrowed->restrict_to(condition);
            result.borrowed->initialize(state.selections);
          }
        } else {
          if (!result.boolean) result.boolean.emplace();
          result.advance_boolean(state.selections);
          for (const bool value : {false, true}) result.boolean->at(value).conjoin(condition);
        }
      }
      const auto key = std::tuple{queue, returned, guards_errors ? error : 0, result.saved_selection()};
      if (const auto entry = pending.find(key); entry != pending.end()) {
        entry->second.state = merge_evidence(std::move(entry->second.state), state);
        entry->second.result.merge(result);
        entry->second.result.boolean_revision = entry->second.state.selections.writes;
      } else pending.emplace(key, Pending{std::move(state), std::move(result)});
    };
    for (const auto &path : exits.paths) {
      if (!guards_errors) enqueue(path.state.deferred_queue, path.returned(), 0, path.state, path.result);
      else for (const auto reason : path.reasons)
        enqueue(path.state.deferred_queue, path.returned(), reason >= 2 ? reason : 0, path.state, path.result);
    }
    if (falls_through) enqueue(body.deferred_queue, false, 0, body, {});

    // Reverse registration order visits older tails first within each path.
    // Loop headers also feed earlier iterations until their input stabilizes.
    while (!pending.empty()) {
      auto next = pending.extract(std::prev(pending.end()));
      const auto [queue, returned, error, saved_selection] = next.key();
      auto &[state, result] = next.mapped();
      if (!queue) {
        if (completed) merge_type_exits(*completed, {error ? error : returned ? 1u : 0u}, state, result);
        if (returned && !error && !body.borrowed_return) collect(result.boolean);
        continue;
      }
      const auto &entry = flow.entries.at(queue - 1);
      if (const auto *loop = std::get_if<TypeDeferredFlow::Loop>(&entry)) {
        // A registration decision from one dynamic iteration cannot select
        // a branch in another. Keep the immutable observations from outside.
        state.selections.forget_observations(loop->first_observation);
        result.forget_observations(loop->first_observation);
        const auto [known, inserted] = loop_inputs.try_emplace(next.key(), Pending{state, result});
        auto &input = known->second;
        if (!inserted) {
          auto joined = merge_evidence(input.state, state);
          auto joined_result = input.result;
          joined_result.merge(result);
          joined_result.boolean_revision = joined.selections.writes;
          const auto &previous = input.state.selections;
          const auto &current = joined.selections;
          if (previous.same_flow(current) && previous.footprints == current.footprints &&
              previous.possible_overlap == current.possible_overlap && joined_result == input.result) continue;
          input = {std::move(joined), std::move(joined_result)};
        }
        enqueue(loop->entry, returned, error, input.state, input.result);
        if (loop->backedge) enqueue(loop->backedge, returned, error, input.state, input.result);
        continue;
      }
      if (const auto *choice = std::get_if<TypeDeferredFlow::Choice>(&entry)) {
        for (const auto *arm : {&choice->first, &choice->second}) {
          const PresenceBranches condition{arm->condition, {}};
          if (!state.selections.reachable(condition, true)) continue;
          auto selected = state;
          selected.selections.assume(condition, true);
          for (const auto &slot : arm->initialized_entry_targets)
            selected.selections.initialize_selection(selected.selections.selected_target({slot, {slot}}));
          enqueue(arm->queue, returned, error, std::move(selected), result);
        }
        continue;
      }
      const auto &action = std::get<TypeDeferredFlow::Action>(entry);
      if (!action.entry) {
        const auto &effects = action.effects;
        const bool exact = !std::holds_alternative<std::monostate>(effects.builtin);
        if (exact && effects.outcomes.contains(0)) {
          auto normal = state;
          apply_type_builtin(normal.selections, effects.builtin);
          enqueue(action.earlier, returned, error, std::move(normal), result);
          if (effects.outcomes.size() == 1) continue;
        }
        if (effects.opaque || effects.slots.unknown) {
          const auto condition = state.selections.reach_condition();
          state.selections.forget();
          state.selections.retain_conditions(condition);
        } else {
          if (!effects.slots.places.empty()) state.selections.assign({std::nullopt, effects.slots.places}, {});
          if (effects.payloads.unknown || !effects.payloads.places.empty())
            state.selections.write_boolean(effects.payloads.unknown ? EvidencePlaces{} : effects.payloads.places, {});
        }
        if (guards_errors) {
          const bool prepared = !effects.outcomes.empty();
          for (const auto outcome : prepared ? effects.outcomes : deferred_outcomes()) {
            if (exact && !outcome) continue;
            // An ordinary call cannot consume a failure already pending.
            const auto next_error = prepared && !outcome ? error : outcome;
            auto completed = state;
            if (!outcome) {
              for (const auto &place : effects.initialized) completed.selections.uninitialized.erase(place);
              for (const auto &slot : effects.initialized_selections)
                completed.selections.initialize_if_present(completed.selections.selected_target({slot, {slot}}));
            }
            if (action.may_replace) enqueue(action.earlier, true, next_error, completed, {});
            enqueue(action.earlier, returned, next_error, std::move(completed), result);
          }
        } else {
          if (action.may_replace) enqueue(action.earlier, true, 0, state, {});
          enqueue(action.earlier, returned, 0, std::move(state), std::move(result));
        }
        continue;
      }

      auto invocation = *action.declaration;
      auto bindings = std::move(invocation.selections.bindings);
      auto places = std::move(invocation.selections.value_places);
      invocation.selections = std::move(state.selections);
      invocation.selections.bindings = std::move(bindings);
      invocation.selections.value_places = std::move(places);
      invocation.prepared_call.reset();
      invocation.deferred = nullptr;
      // The declaration supplies names. The queue supplies the live values.
      auto completed = replay_type_defer(*action.entry, std::move(invocation), error, returned);
      for (auto &path : completed.paths) {
        const bool replacement = path.returned();
        const auto &proof = replacement ? path.result : result;
        if (!guards_errors) enqueue(action.earlier, replacement || returned, 0, std::move(path.state), proof);
        else for (const auto reason : path.reasons) {
          // A normal plain entry preserves the existing failure. A guarded
          // entry had the opportunity to handle it as part of its body exit.
          const auto pending_error = reason >= 2 ? reason : reason == 0 && action.entry->children.size() == 1 ? error : 0;
          enqueue(action.earlier, replacement || returned, pending_error, path.state, proof);
        }
      }
    }
    return returning ? proof : std::nullopt;
  }
  struct LocalStorageEvidence {
    std::optional<Type> type;
    std::optional<PresenceBranches> boolean;
    bool completes = true;
  };
  struct TypeCallInputs {
    std::set<size_t> ready;
    // Each group shares an entry payload. Its formal bindings stay independent.
    std::map<size_t, size_t> shared_targets;
    size_t target(size_t input) const {
      const auto shared = shared_targets.find(input);
      return shared == shared_targets.end() ? input : shared->second;
    }
    bool operator==(const TypeCallInputs &) const = default;
  };
  struct TypeCallSummary {
    std::set<size_t> targets, required_inputs, initialized_inputs;
    bool completes = false;
    bool operator==(const TypeCallSummary &) const = default;

    void merge(const TypeCallSummary &other) {
      if (!other.completes) return;
      if (!completes) { *this = other; return; }
      targets.insert(other.targets.begin(), other.targets.end());
      required_inputs.insert(other.required_inputs.begin(), other.required_inputs.end());
      std::erase_if(initialized_inputs, [&](size_t input) { return !other.initialized_inputs.contains(input); });
    }
  };
  struct TypeCallProofs {
    struct Obligation {
      size_t method;
      std::vector<Type> arguments, retained;
      ResultContract contract;
      TypeEvidenceState context;
      TypeCallInputs inputs;
      std::set<size_t> dependents{};
      // Live edges start with no observed normal completion. An unproved
      // body uses the conservative declaration bound instead.
      std::optional<TypeCallSummary> result{TypeCallSummary{}};
    };
    std::deque<Obligation> obligations;
    std::map<size_t, std::vector<size_t>> methods;
    std::set<size_t> pending;
    std::optional<size_t> current;
    size_t remaining_input_structure = instance_limit;

    bool admit_input_structure(const std::vector<Type> &arguments, const std::vector<Type> &retained,
                               const ResultContract &contract, const TypeEvidenceState &context) {
      // Recursive substitutions can grow their type structure long before
      // reaching the ordinary checking-obligation count.
      auto remaining = remaining_input_structure;
      std::vector<const Type *> pending_types{&*contract.type};
      const auto add = [&](const std::vector<Type> &types) {
        for (const auto &type : types) pending_types.push_back(&type);
      };
      add(arguments);
      add(retained);
      for (const auto &[name, type] : context.bindings) if (type) pending_types.push_back(&*type);
      while (!pending_types.empty()) {
        if (pending_types.size() > remaining) return false;
        --remaining;
        const auto &type = *pending_types.back();
        pending_types.pop_back();
        if (type.target) pending_types.push_back(type.target.get());
        for (const auto *nested : {&type.parameters, &type.bounds, &type.owner_bounds, &type.throws_types}) add(*nested);
      }
      remaining_input_structure = remaining;
      return true;
    }
  };
  std::optional<TypeCallSummary> sample_type_call(const TypeCallProofs::Obligation &obligation) const {
    const auto &[id, prepared, retained, contract, context, inputs, dependents, previous] = obligation;
    const auto &method = program_.methods[id];
    try {
      auto body = body_evidence(id, prepared, retained, context);
      body.required = body.validating_body = true;
      body.initialization = TypeEvidenceState::InitializationCheck::body;
      if (contract.borrowed()) body.borrowed_return = &contract;
      body.invocation_result = contract.type;
      body.selections.tracking = true;
      body.selections.observation_decisions = std::make_shared<ObservationDecisions>();
      TypeEvidenceExits exits, completed;
      TypeDeferredFlow deferred;
      body.exits = &exits;
      body.deferred = &deferred;
      TypeCallSummary result;
      std::map<size_t, PresenceBranches> input_presence;
      std::map<size_t, size_t> result_inputs;
      for (size_t i = 0; i < prepared.size(); ++i) {
        const auto &parameter = parameter_at(method, i);
        if (parameter.owner.empty()) {
          if (prepared[i] == Type::boolean) body.selections.value_places.emplace(parameter.name, EvidencePlaces{i});
          continue;
        }
        TargetEvidence input;
        const auto place = inputs.target(i);
        if (argument_mode(parameter) == ArgumentMode::borrowed_ptr) {
          const auto &decisions = body.selections.observation_decisions;
          const auto [observation, first] = input_presence.try_emplace(place);
          if (first) observation->second = PresenceBranches::observed(decisions, decisions->next_observation++);
          input.presence_proof = observation->second;
          if (place != i) input.same_selected.insert(place);
          if (!inputs.ready.contains(place)) {
            body.selections.uninitialized.emplace(place, input.presence_proof->at(true));
            result.initialized_inputs.insert(i);
          }
        } else {
          // Readable arguments establish readiness at preparation. This is
          // an input contract and contributes no callee write guarantee.
          input.presence = TargetPresence::present;
        }
        if (parameter.owner == contract.owner) result_inputs.try_emplace(place, i);
        input.places.insert(place);
        body.selections.bind(parameter.name, i, std::move(input));
        if (prepared[i] == Type::boolean) body.selections.borrowed_booleans.insert(place);
      }
      const auto completion = type_evidence(*method.source->body, body, false, nullptr, nullptr,
                                            ExpectedTypeUse::materialization, true, EvidenceRead::discarded);
      complete_type_deferred(deferred, exits, body, completion != Type::never, &completed);
      if (!body.complete) return {};
      const bool allows_fallthrough = !method.source->completion_requires_return(*contract.type);
      for (const auto &path : completed.paths) {
        if (!path.reasons.contains(1) && !(allows_fallthrough && path.reasons.contains(0))) continue;
        result.completes = true;
        if (!path.state.complete) return {};
        const auto &selections = path.state.selections;
        const auto reaching = selections.reach_condition();
        if (contract.borrowed()) {
          if (!path.result.borrowed) return {};
          const auto &selected = *path.result.borrowed;
          for (const auto &[place, targets] : selected.targets) {
            auto possible = targets;
            possible.conjoin(reaching);
            if (!possible) continue;
            const auto *ordinal = std::get_if<size_t>(&place);
            if (!ordinal || !result_inputs.contains(*ordinal)) return {};
            const auto result_input = result_inputs.at(*ordinal);
            result.targets.insert(result_input);
            if (const auto missing = selected.uninitialized.find(place); missing != selected.uninitialized.end()) {
              auto unwritten = missing->second;
              unwritten.conjoin(reaching);
              if (selections.may_read_uninitialized(place, unwritten)) result.required_inputs.insert(result_input);
            }
          }
        }
        std::erase_if(result.initialized_inputs, [&](size_t input) {
          return selections.may_read_uninitialized(inputs.target(input), reaching);
        });
      }
      if (!result.completes) result.initialized_inputs.clear();
      return result;
    } catch (const std::runtime_error &) {
      // A declaration still supplies the result bound when its body evidence is unavailable.
      return {};
    }
  }
  std::optional<size_t> type_call_obligation(size_t id, const std::vector<Type> &arguments, const std::vector<Type> &retained,
      const ResultContract &contract, const TypeEvidenceState &supplied, const TypeCallInputs &inputs) const {
    auto &proofs = *type_call_proofs_;
    const auto &method = program_.methods[id];
    if (!method.source->body || variadic(method) || caller_bindings(method)) return {};
    auto prepared = prepared_types(method, arguments, retained);
    auto context = result_context(id, &supplied);
    auto &ids = proofs.methods[id];
    const auto known = std::ranges::find_if(ids, [&](size_t index) {
      const auto &obligation = proofs.obligations[index];
      return obligation.arguments == prepared && obligation.retained == retained && obligation.contract == contract &&
          obligation.inputs == inputs && obligation.context.same_definition_inputs(context);
    });
    if (known != ids.end()) return *known;
    if (!proofs.admit_input_structure(prepared, retained, contract, context)) return {};
    const auto index = proofs.obligations.size();
    proofs.obligations.push_back({id, std::move(prepared), retained, contract, std::move(context), inputs});
    ids.push_back(index);
    proofs.pending.insert(index);
    return index;
  }
  std::optional<TypeCallSummary> prove_type_call(size_t id, const std::vector<Type> &arguments,
      const std::vector<Type> &retained, const ResultContract &contract, const TypeEvidenceState &supplied,
      const TypeCallInputs &inputs) const {
    if (auto *proofs = type_call_proofs_) {
      const auto index = type_call_obligation(id, arguments, retained, contract, supplied, inputs);
      if (!index) return {};
      auto &obligation = proofs->obligations[*index];
      if (proofs->current) obligation.dependents.insert(*proofs->current);
      return obligation.result;
    }
    TypeCallProofs proofs;
    type_call_proofs_ = &proofs;
    struct Reset {
      TypeCallProofs *&active;
      ~Reset() { active = nullptr; }
    } reset{type_call_proofs_};
    try {
      const auto root = type_call_obligation(id, arguments, retained, contract, supplied, inputs);
      if (!root) return {};
      while (!proofs.pending.empty()) {
        const auto index = proofs.pending.extract(proofs.pending.begin()).value();
        auto &obligation = proofs.obligations[index];
        if (!obligation.result) continue;
        proofs.current = index;
        auto next = sample_type_call(obligation);
        if (next) next->merge(*obligation.result);
        if (next == obligation.result) continue;
        obligation.result = std::move(next);
        proofs.pending.insert(obligation.dependents.begin(), obligation.dependents.end());
      }
      auto &result = proofs.obligations[*root].result;
      if (result && result->completes) return std::move(result);
    } catch (const std::runtime_error &) {
      // Optional proof preparation follows the same declaration fallback.
    }
    return {};
  }
  std::set<size_t> prove_shared_call_initialization(const Expression &expr, size_t instance,
      const std::vector<CheckedOperation> &views, const std::set<size_t> &established) const {
    const auto &selected = program_.instances[instance];
    const auto &method = program_.methods[selected.method];
    if (variadic(method) || caller_bindings(method) ||
        (*selected.result.type != Type::void_type && !scalar_type(*selected.result.type))) return {};
    TypeCallInputs inputs;
    std::vector<size_t> pointers;
    std::set<size_t> unproved_groups;
    for (size_t i = 0; i < views.size(); ++i) {
      if (argument_mode(parameter_at(method, i)) != ArgumentMode::borrowed_ptr) continue;
      const auto &view = views[i];
      for (const auto prior : pointers) if (same_selected_reference(view, views[prior])) {
        const auto target = inputs.target(prior);
        inputs.shared_targets.emplace(i, target);
        if (!established.contains(i) && !established.contains(prior) && !view.initialized_target() &&
            !views[prior].initialized_target()) unproved_groups.insert(target);
        break;
      }
      if (view.initialized_target()) inputs.ready.insert(inputs.target(i));
      pointers.push_back(i);
    }
    if (unproved_groups.empty()) return {};
    const bool lexical = lexical_methods_.contains(method.source);
    const auto family = lexical ? named_family(expr.text) : NamedFamily{};
    if (lexical && !family.declaration) return {};
    const TypeEvidenceState module_context;
    const auto &context = family.declaration ? *family.declaration : module_context;
    if (!explicit_type_call_effects(selected.method, context, selected.arguments)) return {};
    auto proof = prove_type_call(selected.method, selected.arguments, selected.retained_types, selected.result, context, inputs);
    if (!proof || !proof->completes) return {};
    std::erase_if(proof->initialized_inputs, [&](size_t input) { return !unproved_groups.contains(inputs.target(input)); });
    return std::move(proof->initialized_inputs);
  }
  static void project_type_initialization(TypeEvidenceState &caller, const TypeEvidenceState &completed, size_t first_observation) {
    auto &missing = caller.selections.uninitialized;
    const auto reaching = completed.selections.reach_condition();
    for (auto entry = missing.begin(); entry != missing.end();) {
      const auto observed = completed.selections.uninitialized.find(entry->first);
      if (observed == completed.selections.uninitialized.end()) {
        entry = missing.erase(entry);
        continue;
      }
      auto paths = observed->second;
      paths.conjoin(reaching);
      paths.forget_observations(first_observation);
      if (paths) { entry->second = std::move(paths); ++entry; }
      else entry = missing.erase(entry);
    }
  }
  static PresenceSlotMap project_scope_bindings(QuerySelections &caller, const QuerySelections &body,
      const std::map<EvidenceSlot, EvidenceSelection> &bindings, PresenceSlotMap slots, size_t first_observation) {
    EvidencePlaces changed;
    std::vector<std::pair<EvidenceSelection, TargetEvidence>> updates;
    for (const auto &[local, destination] : bindings) {
      if (!body.changed_since(local, 0)) continue;
      const EvidenceSelection input{local, {local}};
      auto selected = body.selected_target(input);
      for (const auto &[slot, footprint] : caller.footprints)
        if (caller.may_overlap(footprint, destination.footprint)) changed.insert(slot);
      selected.initialized = body.selection_initialized(selected);
      if (selected.places.empty() && selected.presence != TargetPresence::absent && !selected.initialized)
        for (const auto &[place, missing] : caller.uninitialized) selected.places.insert(place);
      updates.emplace_back(destination, std::move(selected));
    }
    // Entry witnesses can describe a caller binding's previous selection.
    // Surviving observations of the invocation's slot describe the output.
    for (auto &[local, exported] : slots)
      std::erase_if(exported, [&](const auto &slot) { return changed.contains(slot); });
    for (const auto &[local, destination] : bindings) if (destination.slot) slots[local].insert(*destination.slot);
    // Publish all selections before reconnecting their equality facts. A slot
    // replaced by another update cannot denote its old selection here.
    for (const auto &[destination, selected] : updates) {
      TargetEvidence target;
      target.presence = selected.presence;
      target.places = selected.places;
      target.initialized = selected.initialized;
      caller.assign(destination, std::move(target));
    }
    for (auto &[destination, selected] : updates) {
      EvidencePlaces equal;
      for (const auto &same : selected.same_selected)
        if (const auto found = slots.find(same); found != slots.end()) equal.insert(found->second.begin(), found->second.end());
      selected.same_selected = std::move(equal);
      if (selected.presence_proof) {
        selected.presence_proof = selected.presence_proof->remap(slots);
        selected.presence_proof->forget_observations(first_observation);
      }
      caller.record_assignment_selection(destination, std::move(selected));
    }
    return slots;
  }
  LocalStorageEvidence local_storage_evidence(const Expression &expr, TypeEvidenceState &state, bool reached = true) const {
    LocalStorageEvidence storage;
    if (expr.annotation) storage.type = evidence_annotation(*expr.annotation, state);
    else {
      auto query = state; query.returns = nullptr; query.exits = nullptr;
      query.borrowed_return = nullptr;
      if (query.checks_initialization()) query.initialization = TypeEvidenceState::InitializationCheck::unevaluated;
      query.delayed_controls.clear();
      storage.type = type_evidence(*expr.children.front(), query);
    }
    if (state.required && storage.type && storage.type->kind == Type::procedure)
      implementation_gap(expr.at, "raw callable storage construction");
    for (size_t i = expr.annotation ? 0 : 1; i < expr.children.size(); ++i) {
      const auto initializer = type_evidence(*expr.children[i], state, false, nullptr, nullptr,
                                             ExpectedTypeUse::materialization, reached && storage.completes);
      if (initializer == Type::never) storage.completes = false;
      if (state.required && storage.type && initializer) check_local_initializer(*initializer, *storage.type, expr.at);
      if (storage.completes && initializer == Type::boolean) storage.boolean = state.target.branches;
    }
    return storage;
  }
  std::optional<Type> scope_callable_evidence(const Expression &callee, TypeEvidenceState &state,
                                              const Type *expected, bool probing_argument, TargetEvidence &returned) const {
    const auto id = lexical_method(callee.definition.get(), state.method);
    const auto retained = callable_retained_types(id, &state);
    const auto result = retained ? scope_result(infer_result(id, {}, *retained, &state), expected, probing_argument) : ResultEvidence{};
    if (!result.contract) {
      if (state.required)
        implementation_gap(callee.at, "typeof requires a known declaration or operand type with a proved result contract");
      return {};
    }
    if (state.returns && result.origin == ResultOrigin::throw_only_default) {
      state.returns->saw_throw = true;
      return Type::never;
    }
    const auto &captures = free_bindings(*callee.definition).captures;
    for (const auto &capture : captures) check_type_capture(state, capture.name, capture.at);
    std::vector<EvidencePlaces> writable_booleans;
    if (state.selections.tracking) for (const auto &capture : captures) {
      const auto binding = state.bindings.find(capture.name);
      const auto view = state.views.find(capture.name);
      if (binding != state.bindings.end() && binding->second == Type::boolean && view != state.views.end() &&
          !view->second.read_only.value_or(false))
        writable_booleans.push_back(named_type_target(state, capture.name).places);
    }
    if (state.required) {
      const auto declaration = result_context(id, &state);
      auto body = body_evidence(id, {}, *retained, declaration);
      body.required = body.validating_body = true;
      if (!state.selections.initialization_reachable)
        body.initialization = TypeEvidenceState::InitializationCheck::suppressed;
      else if (proving_ && validating_ && state.initialization == TypeEvidenceState::InitializationCheck::unevaluated)
        body.initialization = TypeEvidenceState::InitializationCheck::body;
      else body.initialization = state.initialization;
      body.selections.tracking = state.selections.tracking;
      body.selections.observation_decisions = state.selections.observation_decisions;
      if (state.checks_initialization()) {
        body.selections.uninitialized = state.selections.uninitialized;
        body.selections.borrowed_booleans = state.selections.borrowed_booleans;
      }
      PresenceSlotMap exported_slots;
      std::map<EvidenceSlot, EvidenceSelection> live_bindings;
      EvidencePlaces live_value_places;
      if (body.selections.tracking) {
        PresenceSlotMap imported_slots;
        QuerySelections::Observations imported_booleans;
        std::map<EvidenceSlot, EvidenceSlot> local_place_slots;
        const auto retain_witness = [&](const EvidenceSlot &slot) {
          body.selections.footprints.try_emplace(slot, EvidencePlaces{slot});
          body.selections.entry_selections.insert(slot);
          imported_slots[slot].insert(slot);
          exported_slots[slot].insert(slot);
        };
        const auto retain_observed = [&](const PresenceBranches &proof) {
          for (const bool value : {false, true})
            for (const auto &[slot, presence] : proof.at(value).presence) retain_witness(slot);
        };
        for (const auto &capture : captures) {
          const auto &name = capture.name;
          const auto view = body.views.find(name);
          if (view == body.views.end()) continue;
          const auto source = named_type_target(state, name);
          const EvidenceSlot local = std::pair{callee.definition.get(), name};
          if (view->second.value_binding == true) {
            body.selections.value_places.emplace(name, source.places);
            if (const auto &place = source.selection.slot; place && state.checks_initialization()) {
              if (live_value_places.insert(*place).second) {
                auto selected = source;
                selected.initialized = state.selections.selection_initialized(source);
                body.selections.save_selection(*place, std::move(selected));
              }
              body.selections.bindings.emplace(name, EvidenceSelection{*place, {*place}});
            }
          } else if (source.selection.slot) {
            const auto [place, inserted] = local_place_slots.try_emplace(*source.selection.slot, local);
            if (inserted) {
              // Entry observations outlive writes to the invocation's slots.
              for (const auto &slot : source.same_selected) {
                retain_witness(slot);
                imported_slots[slot].insert(local);
              }
              if (source.presence_proof) retain_observed(*source.presence_proof);
              body.selections.bind(name, local, source);
              live_bindings.emplace(local, source.selection);
            } else {
              const auto &slot = place->second;
              body.selections.bindings.emplace(name, EvidenceSelection{slot, {slot}});
            }
          }
          if (body.bindings.at(name) == Type::boolean) state.selections.read_boolean(source.places);
          for (const auto &place : source.places)
            if (const auto found = state.selections.booleans.find(place); found != state.selections.booleans.end())
              imported_booleans.insert(*found);
        }
        for (auto first = live_bindings.begin(); first != live_bindings.end(); ++first)
          for (auto second = std::next(first); second != live_bindings.end(); ++second)
            if (state.selections.may_overlap(first->second.footprint, second->second.footprint)) {
              body.selections.possible_overlap[first->first].insert(second->first);
              body.selections.possible_overlap[second->first].insert(first->first);
            }
        // A captured Boolean can describe a caller slot that this body never
        // accessed, or that it later replaced in its local reference slot.
        for (const auto &[place, proof] : imported_booleans) retain_observed(proof);
        for (const auto &[place, proof] : imported_booleans)
          body.selections.booleans.emplace(place, *body.selections.compact(proof.remap(imported_slots)));
        if (state.selections.conditions)
          body.selections.assume(PresenceBranches{state.selections.conditions->remap(imported_slots), {}}, true);
      }
      body.invocation_result = *result.contract->type;
      body.supplied_invocation_result = result.origin == ResultOrigin::supplied;
      const bool track_return = body.selections.tracking && body.invocation_result == Type::boolean && !result.contract->borrowed();
      const auto first_local_observation = body.selections.observation_decisions ? body.selections.observation_decisions->next_observation : 0;
      TypeEvidenceExits exits;
      TypeDeferredFlow deferred;
      const bool track_completion = state.checks_initialization() || !live_bindings.empty();
      if (track_return || body.checks_initialization() || track_completion) {
        body.deferred = &deferred;
        body.deferred_queue = 0;
      }
      if (state.exits) {
        for (const auto &capture : captures) {
          const auto binding = body.bindings.find(capture.name);
          if (binding != body.bindings.end() && binding->second && binding->second->kind == Type::procedure &&
              state.definitions.contains(capture.name)) {
            note_type_allocation(state);
            break;
          }
        }
      }
      if (state.exits || body.deferred || !writable_booleans.empty()) body.exits = &exits;
      const auto completion = type_evidence(*callee.definition->body, body, false, nullptr, nullptr,
                                            ExpectedTypeUse::materialization, true, EvidenceRead::discarded);
      TypeEvidenceExits completed;
      if (body.deferred) {
        if (auto proof = complete_type_deferred(deferred, exits, body, completion != Type::never,
                track_completion ? &completed : nullptr); track_return && !track_completion && proof) {
          proof->forget_observations(first_local_observation);
          returned.branches = proof->remap(exported_slots);
        }
      }
      for (const auto &places : writable_booleans) {
        const auto written = [&](const QuerySelections &flow) {
          return flow.payload_writes.touches(places) || flow.deferred_writes.touches(places);
        };
        if (written(body.selections) || std::ranges::any_of(exits.paths, [&](const auto &path) { return written(path.state.selections); }))
          state.selections.write_boolean(places, {});
      }
      if (track_completion) {
        std::optional<QuerySelections> normal;
        std::optional<PresenceBranches> boolean_returns = PresenceBranches{{}, {}};
        for (const auto &outcome : completed.paths) {
          auto caller = state;
          if (state.checks_initialization()) {
            project_type_initialization(caller, outcome.state, first_local_observation);
            for (const auto &place : live_value_places)
              if (outcome.state.selections.initialized_selections.contains(place))
                caller.selections.initialized_selections.insert(place);
          }
          const auto exported = project_scope_bindings(caller.selections, outcome.state.selections, live_bindings,
                                                        exported_slots, first_local_observation);
          for (auto reason : outcome.reasons) {
            if (reason == 0 && callee.definition->completion_requires_return(*body.invocation_result))
              reason = Completion::failure(exception_id(ExceptionCategory::missing_return)).reason;
            if (reason <= 1) {
              if (track_return && reason == 1) {
                auto proof = outcome.result.boolean.value_or(PresenceBranches{}).remap(exported);
                proof.forget_observations(first_local_observation);
                boolean_returns = QuerySelections::alternatives(std::move(boolean_returns), proof);
              }
              if (normal) normal->join_flow(caller.selections);
              else normal = caller.selections;
            } else if (state.exits) merge_type_exits(*state.exits, {reason}, caller);
          }
        }
        if (normal) state.selections = std::move(*normal);
        if (track_return) returned.branches = std::move(boolean_returns);
      } else if (state.exits) {
        if (completion != Type::never && callee.definition->completion_requires_return(*body.invocation_result))
          note_type_exit(state, Completion::failure(exception_id(ExceptionCategory::missing_return)));
        const auto registered = [](const TypeEvidenceExits::Path &path) { return path.state.has_deferred_action; };
        if (body.has_deferred_action || std::ranges::any_of(exits.paths, registered))
          note_type_call(state, complete_callable_type(program_.methods[id], {}, *result.contract));
        for (auto &path : exits.paths) {
          path.reasons.erase(1);
          merge_type_exits(*state.exits, std::move(path.reasons), state);
        }
      }
    } else for (const auto &places : writable_booleans) state.selections.write_boolean(places, {});
    return complete_callable_type(program_.methods[id], {}, *result.contract);
  }
  std::optional<Type> query_evidence(const Expression &expr, TypeEvidenceState &state, ViewEvidence &view, TargetEvidence &target_facts,
                                     bool contextual, const Type *expected, std::vector<Type> *alternatives,
                                     ExpectedTypeUse expected_use, EvidenceRead read) const {
    using K = Expression::Kind;
    if (expr.kind == K::name && active_here(expr.text, &state)) {
      if (state.required) fail(expr.at, "a here argument can only be invoked or forwarded");
      return {};
    }
    if (expr.has_here_body() || forwards_here(expr, &state)) {
      if (state.required) implementation_gap(expr.at, "type inspection of a call receiving a here argument");
      return {};
    }
    if (expr.kind == K::call && !expr.intrinsic && active_here(expr.text, &state)) {
      const auto &method = program_.methods[*state.method];
      const auto contract = here_invocation_type(method, state.substitutions, expr.at);
      if (expr.children.size() != contract.parameters.size()) fail(expr.at, "here invocation argument count does not match its contract");
      if (state.preparing(expr)) implementation_gap(expr.at, "prepared deferred here invocation");
      std::vector<TargetEvidence> targets;
      bool completes = true;
      for (size_t i = 0; i < expr.children.size(); ++i) {
        const auto &operand = *expr.children[i];
        const auto mode = argument_mode(contract, i);
        const auto use = mode == ArgumentMode::value ? EvidenceRead::payload : EvidenceRead::target;
        const auto argument = type_evidence(operand, state, false, nullptr, nullptr, expected_type_use(mode), completes, use);
        if (state.required && argument && *argument != Type::never) {
          if (!accepts_argument(*argument, contract.parameters[i], mode))
            fail(operand.at, "argument does not satisfy the here contract");
          const auto &input = method.here->inputs[i];
          if (input.owner) {
            if (state.value.view != true || state.value.mixed_view)
              implementation_gap(operand.at, "borrowed arguments require existing target views");
            if (!input.read_only && state.value.read_only.value_or(false))
              fail(operand.at, "readonly argument cannot supply a writable borrowed parameter");
            const auto role = *input.owner;
            auto owner = EvidenceOwner{};
            if (role) {
              owner = EvidenceOwner::parameter(role - 1);
              if (!state.external_owners.contains(owner)) owner = EvidenceOwner{null_target - role};
            }
            require_here_owner(state.value.owner, owner, operand.at, &state);
            if (completes) prepare_type_readable_argument(state, operand, state.value, state.target, mode);
          }
        }
        targets.push_back(std::move(state.target));
        completes = completes && argument != Type::never;
      }
      if (completes) {
        for (size_t i = 0; i < targets.size(); ++i)
          type_input_mutation(expr, state, contract.parameters[i], targets[i], argument_mode(contract, i), method.here->inputs[i].read_only);
        record_type_call(expr, state, contract, true);
      }
      return completes ? *contract.target : Type{Type::never};
    }
    const auto child = [&](size_t i, bool context = false, bool reached = true, EvidenceRead use = EvidenceRead::payload) {
      return type_evidence(*expr.children.at(i), state, context, nullptr, nullptr, ExpectedTypeUse::materialization, reached, use);
    };
    if ((expr.kind == K::binary || expr.kind == K::call) && identity_operation(expr.text)) {
      if (expr.children.size() != 2) fail(expr.at, "reference identity requires two reference views");
      const auto left = child(0, false, true, EvidenceRead::target); const auto first = state.value;
      auto tested_left = state.target;
      const auto version = state.selections.writes;
      const auto right = child(1, false, left != Type::never, EvidenceRead::target); const auto second = state.value;
      if (state.required) {
        for (const auto &operand : {std::pair{left, first}, std::pair{right, second}})
          if (operand.first != Type::never && (operand.second.view != true || operand.second.mixed_view))
            fail(expr.at, "reference identity requires existing target views, not independent values");
        if (left != Type::never && right != Type::never && !first.owner && !second.owner)
          fail(expr.at, "reference identity requires an owner context; two unqualified nulls have none");
      }
      if (state.selections.tracking) {
        state.selections.forget_changed(tested_left, version);
        const bool unequally = expr.text == "is not";
        const auto owners = (left == Type::null_ref && tested_left.contextual_null) ||
            (right == Type::null_ref && state.target.contextual_null) ? OwnerMatch::satisfied :
            compare_owners(first.owner, second.owner, &state);
        if (owners == OwnerMatch::mismatch) target_facts.branches = PresenceBranches::constant(unequally);
        else if (left == Type::null_ref || right == Type::null_ref) {
          const auto &selected = right == Type::null_ref ? tested_left : state.target;
          if (const auto outcomes = state.selections.selection_outcomes(selected)) {
            auto &proof = target_facts.branches.emplace();
            proof.at(!unequally) = outcomes->at(false);
            // Inequality can also distinguish two nulls with different owners.
            if (owners == OwnerMatch::satisfied) proof.at(unequally) = outcomes->at(true);
          }
        }
      }
      return left == Type::never || right == Type::never ? Type{Type::never} : Type{Type::boolean};
    }
    if (state.required) {
      if (expr.spread) implementation_gap(expr.at, "spread operands in typeof");
      if (signed_minimum(expr, &state)) {
        builtin_declarations(expr, {Type::integer}, Type::integer);
        return Type::integer;
      }
      if (!state.validating_body) switch (expr.kind) {
      case K::integer: case K::real: case K::boolean: case K::return_test: case K::null_ref: case K::name: case K::symbol:
      case K::unary: case K::binary: case K::call: case K::value_call: case K::behave: case K::perform: case K::field:
      case K::conditional: case K::block: case K::grouped: case K::transfer: case K::rebind: case K::typed_expression: break;
      case K::callable:
        if (expr.definition->name.empty()) break;
        [[fallthrough]];
      default: implementation_gap(expr.at, "typeof for this expression form");
      }
    }
    switch (expr.kind) {
    case K::integer:
      if (state.required) return literal(expr).type;
      if (expr.text.ends_with("u8")) return Type::uint8;
      if (expr.text.ends_with("u32")) return Type::uint32;
      if (expr.text.ends_with("u64")) return Type::uint64;
      return Type::integer;
    case K::real:
      if (state.required) return literal(expr).type;
      return expr.text.ends_with("f32") ? Type::float32 : Type::float64;
    case K::boolean:
      if (state.selections.tracking) target_facts.branches = PresenceBranches::constant(expr.text == "true");
      return Type::boolean;
    case K::return_test: {
      const auto pattern = return_pattern_evidence(expr, state);
      if (pattern.matches == OwnerMatch::mismatch || state.pending_return == false)
        target_facts.branches = PresenceBranches::constant(false);
      else if (pattern.matches == OwnerMatch::satisfied && state.pending_return == true)
        target_facts.branches = PresenceBranches::constant(true);
      return Type::boolean;
    }
    case K::null_ref:
      view.view = true;
      view.nullable = true;
      target_facts.presence = TargetPresence::absent;
      target_facts.contextual_null = expr.owner_name.empty();
      if (!expr.owner_name.empty()) view.owner = expr.owner_name == "here" ? state.full_scope_owner : owner_evidence(expr.owner_name, state, expr.at);
      return Type::null_ref;
    case K::symbol: {
      if (!state.complete) return {};
      if (state.validating_body && state.method && program_.methods[*state.method].source->kind == Form::Kind::function)
        fail(expr.at, "a function cannot access dynamic bindings");
      const auto id = resolve_symbol(expr.text, expr.at, symbol_shadowed(expr.text, state), state.method);
      return program_.symbols[id].type;
    }
    case K::procedure_effect: case K::effect: {
      if (state.validating_body && state.method && program_.methods[*state.method].source->kind == Form::Kind::function)
        fail(expr.at, "a function cannot access dynamic bindings");
      const auto operand = expr.children.empty() ? std::optional<Type>{} : child(0);
      if (operand && operand->kind == Type::procedure) note_type_allocation(state);
      return operand == Type::never ? Type{Type::never} : Type{Type::void_type};
    }
    case K::touch: return Type::void_type;
    case K::name: {
      if (!state.complete) return {};
      const auto *expected_callable = callable_expectation(expected);
      check_type_capture(state, expr.text, expr.at);
      if (state.required) {
        if (state.owner_bindings.contains(expr.text)) fail(expr.at, "owner binding is not a value in typeof");
        if (!state.validating_body) check_query_binding(expr.text, expr.at);
      }
      if (const auto found = state.bindings.find(expr.text); found != state.bindings.end() && !state.definitions.contains(expr.text)) {
        view = state.views.at(expr.text);
        target_facts = named_type_target(state, expr.text);
        return found->second;
      }
      if (const auto family = named_family(expr.text, &state); !family.methods.empty()) {
        note_family_type_capture(state, family);
        if (expected_callable) {
          if (program_.methods[family.methods.front()].family && has_declarations(expr.text))
            return declared_callable_query(expr.text, expected_callable, expr.at, state, family.methods);
          return callable_query(family.methods, expected_callable, expr.at, family.declaration ? *family.declaration : state);
        }
        if ((!contextual || state.required) && family.methods.size() == 1)
          return callable_evidence(family.methods.front(), family.declaration ? family.declaration : &state);
      } else if (has_declarations(expr.text) && (expected_callable || !contextual || state.required))
        return declared_callable_query(expr.text, expected_callable, expr.at, state);
      return {};
    }
    case K::grouped: {
      const auto type = type_evidence(*expr.children.front(), state, contextual, expected, alternatives, expected_use, true, EvidenceRead::target);
      if (!type || type == Type::never) return type;
      const auto &source = state.value;
      const auto result = group_result(*type, source.view.value_or(false), source.nullable, source.mixed_view);
      if (result == GroupResult::unsupported) {
        if (state.required) implementation_gap(expr.at, "parenthesized result with no proved value or nullable-target contract");
        return {};
      }
      if (result == GroupResult::nullable_target) view = {source.read_only, true, {}, source.owner, false, {}, {}, {}, true};
      if (result == GroupResult::nullable_target) {
        target_facts = state.target;
        target_facts.selection.footprint.clear();
      } else {
        if (result == GroupResult::value && source.view == true)
          require_type_initialized(state, state.target, {}, expr.at);
        if (type == Type::boolean)
          target_facts.branches = state.target.branches ? state.target.branches : state.selections.read_boolean(state.target.places);
      }
      if (result == GroupResult::value && type->kind == Type::procedure && source.view == true)
        note_type_allocation(state);
      return type;
    }
    case K::typed_expression: {
      if (*expr.annotation != Type::void_type) implementation_gap(expr.at, "non-void expression conversion or assertion");
      const auto operand = child(0, false, true, EvidenceRead::target);
      if (!operand) {
        if (state.returns) state.returns->unresolved = true;
        return {};
      }
      if (state.required && operand == Type::null_ref) fail(expr.at, "explicit discard requires an independently typed operand");
      return operand == Type::never ? Type{Type::never} : Type{Type::void_type};
    }
    case K::block: {
      std::optional<Type> result = Type::void_type;
      bool exits = false;
      for (size_t i = 0; i < expr.children.size(); ++i) {
        const bool last = i + 1 == expr.children.size();
        result = type_evidence(*expr.children[i], state, contextual && last, last ? expected : nullptr,
                               last ? alternatives : nullptr, expected_use, !exits, last ? read : EvidenceRead::discarded);
        exits = exits || result == Type::never;
        if (state.returns && exits) break;
      }
      if (!expr.children.empty()) { view = state.value; target_facts = state.target; }
      return exits ? std::optional<Type>{Type::never} : result;
    }
    case K::pointer_group: {
      bool completes = true;
      for (const auto &declaration : expr.children) {
        if (declaration->kind == K::block) {
          if (type_evidence(*declaration, state, false, nullptr, nullptr, ExpectedTypeUse::materialization, completes) == Type::never)
            completes = false;
          continue;
        }
        const bool value = expr.declaration_kind == "val" || expr.declaration_kind == "func" || expr.declaration_kind == "proc";
        if (state.required && value && !declaration->owner_name.empty())
          fail(declaration->at, "val storage belongs to its lexical scope, not an external owner");
        const auto declared_owner = declaration->owner_name.empty() ? std::optional<EvidenceOwner>{} : owner_evidence(declaration->owner_name, state, declaration->at);
        const auto declared = declaration->annotation ?
            std::optional<Type>{evidence_annotation(*declaration->annotation, state)} : std::nullopt;
        const auto &initializer = *declaration->children.front();
        std::optional<Type> initial, storage_type;
        std::optional<PresenceBranches> boolean;
        if (initializer.kind == K::make_local) {
          // The raw storage type remains known if evaluating its initializer
          // completes the callable before the declaration can finish.
          const auto storage = local_storage_evidence(initializer, state, completes);
          storage_type = storage.type;
          boolean = storage.boolean;
          initial = storage.completes ? storage.type : std::optional<Type>{Type::never};
          state.value = storage.completes ? ViewEvidence{false, false, {}} : ViewEvidence{};
        } else initial = type_evidence(initializer, state, declared.has_value(),
            declared && declared->kind == Type::procedure ? &*declared : nullptr, nullptr,
            ExpectedTypeUse::materialization, completes,
            value || declaration->binding == "<-" ? EvidenceRead::payload : EvidenceRead::target);
        const auto type = declared ? declared : storage_type ? storage_type : initial;
        if (initial == Type::never) completes = false;
        const auto borrowed = value || declaration->binding == "<-" ? std::optional{false} : state.value.view;
        if (completes && type && type->kind == Type::procedure && (borrowed != true || state.value.mixed_view))
          note_type_allocation(state);
        if (state.required) {
          if (*initial != Type::never && declared_owner) {
            const auto relation = compare_owners(state.value.owner, declared_owner, &state);
            check_initializer_owner(borrowed, borrowed == true && state.value.mixed_view, relation == OwnerMatch::mismatch,
                                    declaration->at);
          }
          check_initializer_type(expr, *declaration, *initial, *type, borrowed);
          if (*initial != Type::never && borrowed == true)
            check_initializer_access(*declaration, state.value.read_only.value_or(false));
          if (proving_ && validating_ && expr.declaration_kind == "ref" && *initial != Type::never &&
              borrowed == true && state.value.nullable && state.target.presence != TargetPresence::present)
            fail(initializer.at, "ref requires a proven non-null target");
          if (expr.declaration_kind == "ref" && initializer.kind == K::make_local &&
              initializer.children.size() == (initializer.annotation ? 0u : 1u))
            fail(declaration->at, "ref requires definitely initialized target storage");
        }
        if (completes && borrowed == true && expr.declaration_kind == "ref")
          require_type_initialized(state, state.target, initializer.kind == K::name ? initializer.text : std::string_view{},
                                   initializer.at);
        state.bindings[declaration->text] = type;
        state.local_declarations[declaration->text] = declaration.get();
        state.definitions.erase(declaration->text);
        state.owner_bindings.erase(declaration->text);
        std::optional<bool> readonly = declaration->read_only || (value && expr.fixed_binding);
        if (!*readonly && !value && declaration->binding != "<-") {
          if (state.value.view.value_or(false)) readonly = state.value.read_only;
          else if (!state.value.view && state.value.read_only != false) readonly.reset();
        }
        auto owner = !declaration->owner_name.empty() ? declared_owner :
            value || declaration->binding == "<-" || state.value.view == false ? state.full_scope_owner : state.value.owner;
        if (!value && declaration->binding != "<-" && state.value.mixed_view && owner != state.full_scope_owner) owner.reset();
        state.views[declaration->text] = {readonly, true, value, owner, false, expr.fixed_binding, readonly, value,
                                         expr.declaration_kind == "ptr"};
        if (state.full_scope_owner) state.views.at(declaration->text).place_owners.insert(*state.full_scope_owner);
        state.views.at(declaration->text).storage_kind = place_kind(value, expr.declaration_kind == "ref");
        state.selections.remove_binding(declaration->text);
        EvidencePlaces places;
        if (state.selections.tracking && completes && (type == Type::boolean || state.tracks_target_places())) {
          if (borrowed == true && !state.value.mixed_view) places = state.target.places;
          else if (borrowed == false && !state.value.mixed_view) {
            places.insert(declaration.get());
            if (value) state.selections.value_places.insert_or_assign(declaration->text, places);
            if (state.checks_initialization() && type && scalar_type(*type)) {
              if (initializer.kind == K::make_local && initializer.children.size() == (initializer.annotation ? 0u : 1u))
                state.selections.uninitialized.insert_or_assign(declaration.get(), state.selections.reach_condition());
              else state.selections.initialize(places);
            }
            if (type == Type::boolean) {
              if (initializer.kind != K::make_local)
                boolean = state.target.branches ? state.target.branches : state.selections.read_boolean(state.target.places);
              state.selections.write_boolean(places, boolean);
            }
          }
        }
        if (!value && initial && *initial != Type::never) {
          auto selected = borrowed == true && !state.value.mixed_view ? state.target : TargetEvidence{};
          selected.places = std::move(places);
          selected.presence = expr.declaration_kind != "ptr" || declaration->binding == "<-" || initializer.kind == K::make_local ?
              TargetPresence::present : state.target.presence;
          state.selections.bind(declaration->text, declaration.get(), std::move(selected));
        }
      }
      return completes ? Type{Type::void_type} : Type{Type::never};
    }
    case K::alias: {
      const auto type = child(0, false, true, EvidenceRead::target);
      if (type == Type::never) return Type::never;
      auto selected = state.value;
      if (state.required) {
        if (selected.view == false || type == Type::null_ref) fail(expr.at, "alias requires an existing storage place");
        if (!selected.value_binding || selected.mixed_view) implementation_gap(expr.at, "alias of a non-binding storage place");
        if (selected.storage_kind == PlaceKind::mixed) implementation_gap(expr.at, "alias selection with different storage kinds");
        if (!expr.declaration_kind.empty()) {
          if (!selected.storage_kind) implementation_gap(expr.at, "declared place alias requires an established storage kind");
          const auto kind = place_kind_name(*selected.storage_kind);
          if (kind != expr.declaration_kind) fail(expr.at, "alias kind must match the selected " + kind + " storage");
        }
        if (expr.annotation && type != evidence_annotation(*expr.annotation, state))
          fail(expr.at, "alias annotation requires the exact selected type");
        if (expr.annotation && !expr.read_only && selected.read_only == true)
          fail(expr.at, "readonly source cannot supply a writable alias target");
        if (!expr.owner_name.empty()) {
          const auto owner = expr.owner_name == "here" ? state.full_scope_owner :
              owner_evidence(expr.owner_name, state, expr.at);
          if (selected.place_owners.empty() || std::ranges::any_of(selected.place_owners, [&](const auto &place) {
                return compare_owners(owner, place, &state) != OwnerMatch::satisfied;
              })) fail(expr.at, "alias owner must match the selected binding storage, not its target owner");
        }
      }
      state.bindings[expr.text] = type;
      state.local_declarations[expr.text] = &expr;
      state.definitions.erase(expr.text);
      state.owner_bindings.erase(expr.text);
      if (expr.read_only || (expr.fixed_binding && selected.value_binding.value_or(false))) selected.read_only = true;
      if (expr.fixed_binding) selected.fixed_binding = true;
      state.views[expr.text] = selected;
      state.views[expr.text].value_capture = false;
      state.selections.remove_binding(expr.text);
      if ((type == Type::boolean || state.tracks_target_places()) && selected.value_binding == true && !state.target.places.empty()) {
        state.selections.value_places.emplace(expr.text, state.target.places);
        if (!state.target.selection.slot && state.target.places.size() > 1)
          state.selections.bind(expr.text, &expr, state.target);
      }
      if (selected.nullable || !state.target.selection.footprint.empty())
        state.selections.declare_place(expr.text, &expr, state.target);
      return Type::void_type;
    }
    case K::make_local: {
      const auto storage = local_storage_evidence(expr, state);
      return storage.completes ? storage.type : std::optional<Type>{Type::never};
    }
    case K::transfer: case K::rebind: {
      const auto type = child(0, false, true, EvidenceRead::target); view = state.value;
      auto destination = state.target;
      const auto destination_revision = state.selections.writes;
      if (expr.kind == K::transfer && type != Type::never) read_type_target(state, view, state.target);
      const auto *context = expr.kind == K::transfer && type && type->kind == Type::procedure ? &*type : nullptr;
      const auto source = type_evidence(*expr.children[1], state, context != nullptr, context, nullptr,
                                        ExpectedTypeUse::materialization, type != Type::never,
                                        expr.kind == K::transfer ? EvidenceRead::payload : EvidenceRead::target);
      if (type == Type::never || source == Type::never) return Type::never;
      if (state.required) assignment_destination(view.view == true, view.mixed_view, expr.at);
      if (expr.kind == K::transfer) {
        if (type && type->kind == Type::procedure) note_type_allocation(state);
        if (state.required) {
          if (!view.read_only) implementation_gap(expr.at, "assignment destination access is not established");
          transfer_access(*view.read_only, expr.at);
          transfer_type(*source, *type, expr.at);
        }
        view.read_only = true;
        view.nullable = false;
        target_facts.places = destination.places;
        if (type == Type::boolean) {
          target_facts.branches = state.target.branches;
          type_boolean_write(expr, state, destination.places, target_facts.branches);
        }
        if (!state.preparing(expr) && state.checks_initialization()) {
          state.selections.forget_changed(destination, destination_revision);
          state.selections.initialize(destination);
        }
        else if (state.preparing(expr) && !destination.places.empty() && (type == Type::boolean || state.checks_initialization())) {
          std::optional<EvidenceSlot> saved;
          if (state.checks_initialization() && destination.places.size() > 1) {
            state.selections.forget_changed(destination, destination_revision);
            saved = save_prepared_type_target(state, expr, destination);
          }
          state.prepared_call->effects.builtin = PreparedTransfer{destination.places,
              type == Type::boolean ? std::optional{snapshot_type_branches(state, target_facts.branches)} : std::nullopt,
              type == Type::boolean, saved};
        }
      } else if (state.required) {
        reference_destination(view, expr.at);
        const auto &selected = state.value;
        if (selected.view != true || selected.mixed_view)
          implementation_gap(expr.at, "rebinding from a fresh value without an existing target");
        rebind_type(*type, *source, expr.at);
        if (proving_ && validating_ && !view.nullable && selected.nullable && state.target.presence != TargetPresence::present)
          fail(expr.at, "ref requires a proven non-null target");
        if (!view.nullable) require_type_initialized(state, state.target, {}, expr.at);
        if (!selected.read_only || (!view.slot_read_only && *selected.read_only))
          implementation_gap(expr.at, "reference target access is not established");
        rebind_access(view.slot_read_only.value_or(false), *selected.read_only, expr.at);
        if (*source != Type::null_ref || selected.owner) {
          const auto relation = compare_owners(view.owner, selected.owner, &state);
          if (relation == OwnerMatch::mismatch) fail(expr.at, "reference rebinding cannot change the initializer-fixed owner");
          if (relation == OwnerMatch::unknown) implementation_gap(expr.at, "reference assignment requires an established owner equality");
        }
      }
      if (expr.kind == K::rebind) {
        target_facts = state.target;
        target_facts.selection = {};
        if (state.preparing(expr)) {
          type_slot_write(expr, state, destination.selection);
          if (!destination.selection.footprint.empty())
            state.prepared_call->effects.builtin =
                PreparedReferenceAssignment{destination.selection, snapshot_type_target(state, target_facts)};
        }
        else if (view.nullable || !destination.selection.footprint.empty()) {
          const auto before = state.selections.writes;
          state.selections.assign(destination.selection, target_facts);
          if (destination.selection.slot && !destination.selection.footprint.empty()) {
            const auto assigned = state.selections.selected_target(destination.selection);
            target_facts.selection.slot = assigned.selection.slot;
            target_facts.same_selected = assigned.same_selected;
            target_facts.presence_proof = assigned.presence_proof;
          } else state.selections.forget_changed(target_facts, before);
        }
      }
      if (state.preparing(expr)) record_type_invocation(expr, state, {0}, true);
      view.value_binding.reset(); view.fixed_binding.reset(); view.slot_read_only.reset();
      view.place_owners.clear();
      view.storage_kind.reset();
      return type;
    }
    case K::conditional: {
      const bool observes_return = expr.children[0]->kind == K::return_test;
      const auto condition = child(0);
      if (state.returns && condition == Type::never) return Type::never;
      if (state.required && condition != Type::boolean && condition != Type::never) fail(expr.at, "condition requires bool");
      if (state.deferred) state.selections.observe_choice(state.target.branches);
      const auto condition_proof = state.target.branches;
      const bool reaches_yes = state.selections.reachable(condition_proof, true);
      const bool reaches_no = state.selections.reachable(condition_proof, false);
      if (condition != Type::never && reaches_yes && reaches_no) capture_preparation_targets(expr, state);
      auto arms = conditional_arms(expr, state, contextual, expected, alternatives, expected_use, condition != Type::never, read);
      auto &[yes, no, yes_state, no_state] = arms;
      view = yes == Type::never ? no_state.value : no == Type::never ? yes_state.value : join_views(yes_state.value, no_state.value);
      const bool independent = arms.independent();
      if (independent) {
        if (state.required && proving_ && validating_ && state.checks_reaching_paths() && condition != Type::never) {
          if (reaches_yes) require_present_projection(yes_state.value, yes_state.target.presence, expr.children[1]->at);
          if (reaches_no) require_present_projection(no_state.value, no_state.target.presence, expr.children[2]->at);
        }
        view = {false, false, {}};
      } else target_facts = yes == Type::never ? no_state.target : no == Type::never ? yes_state.target :
          join_targets(yes_state.target, no_state.target);
      auto result = yes == Type::never ? no : no == Type::never || yes == no ? yes : std::nullopt;
      const bool yes_null = yes == Type::null_ref && no_state.value.view == true;
      const bool no_null = no == Type::null_ref && yes_state.value.view == true;
      if (yes_null || no_null) result = yes_null ? no : yes;
      if (result == Type::boolean && state.selections.tracking && read == EvidenceRead::payload) {
        const auto outcome = [&](const TypeEvidenceState &arm, const std::optional<Type> &type, bool branch) {
          if (type == Type::never || !(branch ? reaches_yes : reaches_no)) return PresenceBranches{{}, {}};
          return arm.selections.observe(arm.target.branches);
        };
        target_facts.branches = QuerySelections::alternatives(outcome(yes_state, yes, true), outcome(no_state, no, false));
      }
      if (state.selections.tracking && view.view == true && view.nullable && !view.mixed_view) {
        const auto outcome = [&](const TypeEvidenceState &arm, const std::optional<Type> &type, bool branch) {
          if (type == Type::never || !(branch ? reaches_yes : reaches_no)) return PresenceBranches{{}, {}};
          return arm.selections.observe(arm.selections.selection_outcomes(arm.target));
        };
        target_facts.presence_proof = QuerySelections::alternatives(outcome(yes_state, yes, true), outcome(no_state, no, false));
      }
      if (!independent && result && scalar_type(*result) && !view.mixed_view &&
          yes_state.value.view == true && no_state.value.view == true) {
        auto left = yes_state.value.owner, right = no_state.value.owner;
        if (yes_null && !left) left = right;
        if (no_null && !right) right = left;
        const auto relation = compare_owners(left, right, &state);
        if (state.required) require_conditional_owner(relation, expr.at);
        if (relation == OwnerMatch::satisfied) view.owner = left;
      }
      if (!reaches_yes) yes_state.deferred_queue = no_state.deferred_queue;
      if (!reaches_no) no_state.deferred_queue = yes_state.deferred_queue;
      if (observes_return && condition_proof) {
        if (condition_proof->at(true)) {
          state = merge_evidence(std::move(yes_state), no_state, true);
          return yes;
        }
        state = merge_evidence(std::move(no_state), yes_state, true);
        return no;
      }
      if (state.checks_initialization() && reaches_yes != reaches_no) {
        // The type pass still needs both arms for lexical and result evidence.
        const auto &reaching = reaches_yes ? yes_state : no_state;
        auto missing = reaching.selections.uninitialized;
        auto ready = reaching.selections.initialized_selections;
        const bool continues = reaching.selections.initialization_reachable && (reaches_yes ? yes : no) != Type::never;
        state = merge_reaching_evidence(std::move(yes_state), std::move(no_state), yes == Type::never, no == Type::never);
        state.selections.uninitialized = std::move(missing);
        state.selections.initialized_selections = std::move(ready);
        state.selections.initialization_reachable = continues;
      } else state = condition == Type::never ? merge_evidence(std::move(yes_state), no_state) :
          merge_reaching_evidence(std::move(yes_state), std::move(no_state), yes == Type::never, no == Type::never);
      if (condition == Type::never) return Type::never;
      if (observes_return && state.returns && !state.invocation_result) return no;
      return result;
    }
    case K::deferred:
      if (state.validating_body) {
        const auto &action = *expr.children.front();
        if (action.kind != K::block) {
          auto outer_preparation = std::exchange(state.prepared_call, TypePreparedCall{&action});
          const auto preparation = type_evidence(action, state);
          auto effects = std::move(state.prepared_call->effects);
          state.prepared_call = std::move(outer_preparation);
          if (preparation == Type::never) return Type::never;
          if (effects.outcomes.empty()) {
            effects.outcomes = deferred_outcomes();
            effects.opaque = true;
          }
          register_type_defer(state, nullptr, false, std::move(effects));
          return Type::void_type;
        }
        auto delayed = state;
        delayed.initialization = TypeEvidenceState::InitializationCheck::suppressed;
        TypeEvidenceExits delayed_exits;
        delayed.exits = state.selections.tracking ? &delayed_exits : nullptr;
        delayed.deferred = nullptr;
        delayed.selections.forget();
        delayed.selections.payload_writes = {};
        delayed.selections.deferred_writes = {};
        delayed.selections.deferred_return = false;
        enter_type_defer(delayed);
        const auto delayed_entry = delayed.selections.writes;
        bool may_replace = false;
        TypeDeferredEffects effects;
        const auto include_writes = [&](const TypeEvidenceState &path) {
          effects.payloads.merge(path.selections.payload_writes);
          effects.payloads.merge(path.selections.deferred_writes);
          effects.opaque |= path.selections.writes != delayed_entry;
          may_replace |= path.selections.deferred_return;
        };
        type_evidence(action, delayed);
        std::set<size_t> handled;
        for (size_t i = 1; i < expr.children.size(); ++i) {
          const auto &handler = *expr.children[i];
          if (handler.kind == K::block) { type_evidence(handler, delayed); continue; }
          auto caught = catch_evidence(handler, delayed, handled);
          type_evidence(*handler.children.front(), caught);
          include_writes(caught);
        }
        include_writes(delayed);
        for (const auto &path : delayed_exits.paths) {
          include_writes(path.state);
          may_replace |= path.returned();
        }
        const bool bodyless_catch = expr.binding == "fallback" && expr.children[1]->kind == K::handler;
        register_type_defer(state, bodyless_catch && !state.tracks_target_places() ? nullptr : &expr, may_replace, std::move(effects));
        return Type::void_type;
      }
      if (!state.returns || expr.children.front()->kind != K::block) {
        if (expr.children.front()->kind != K::block && child(0) == Type::never) return Type::never;
        return Type::void_type;
      }
      [[fallthrough]];
    case K::try_scope: {
      const bool guarded = expr.kind == K::try_scope;
      if (guarded) capture_preparation_targets(expr, state);
      const auto before = state;
      const bool has_finalizer = guarded && expr.children.size() > 1 && expr.children.back()->kind == K::block;
      auto *const outer = state.exits;
      const bool track_exits = guarded && ((state.required && (has_finalizer || outer)) || state.selections.tracking);
      TypeEvidenceExits body_exits, outgoing;
      if (track_exits) state.exits = &body_exits;
      auto result = type_evidence(*expr.children.front(), state, contextual, expected, alternatives, expected_use, true, read);
      state.exits = outer;
      view = state.value;
      target_facts = state.target;
      if (track_exits && read == EvidenceRead::payload && result && result->kind == Type::procedure)
        merge_type_exits(body_exits, {Completion::failure(exception_id(ExceptionCategory::out_of_memory)).reason}, state);
      const auto body_state = state;
      std::optional<TypeEvidenceState> finalizer_entry;
      const Expression *finalizer = nullptr;
      std::set<size_t> handled;
      for (size_t i = 1; i < expr.children.size(); ++i) {
        const auto &handler = *expr.children[i];
        if (handler.kind == K::block) {
          finalizer = &handler;
          break;
        }
        std::optional<TypeEvidenceState> incoming;
        if (track_exits) {
          const auto type = handler.annotation ? evidence_annotation(*handler.annotation, before) : exception_type(0);
          incoming = take_type_catch(body_exits, type);
        }
        const bool handler_reaches = !track_exits || incoming.has_value();
        const auto catch_entry = track_exits ? merge_evidence(incoming ? std::move(*incoming) : before, body_state, true)
                                             : merge_evidence(before, body_state);
        auto caught = catch_evidence(handler, catch_entry, handled);
        TypeEvidenceExits exits;
        if (track_exits) caught.exits = handler_reaches ? &exits : nullptr;
        const auto type = type_evidence(*handler.children.front(), caught, contextual, expected, alternatives, expected_use, handler_reaches, read);
        if (state.required) check_guarded_result(*result, *type, handler.at);
        if (read == EvidenceRead::payload && type && type->kind == Type::procedure)
          note_type_allocation(caught);
        const auto caught_view = caught.value;
        const auto caught_target = caught.target;
        leave_type_catch(caught, catch_entry, handler.text);
        caught.exits = outer;
        if (!handler_reaches) {
          state = merge_evidence(std::move(state), caught, true);
          continue;
        }
        for (auto &path : exits.paths) {
          leave_type_catch(path.state, catch_entry, handler.text);
          forward_type_exit(outgoing, path);
        }
        const bool prior_exits = result == Type::never, catch_exits = type == Type::never;
        view = prior_exits ? caught_view : catch_exits ? view : join_views(view, caught_view);
        target_facts = prior_exits ? caught_target : catch_exits ? target_facts : join_targets(target_facts, caught_target);
        if (!track_exits) {
          if (finalizer_entry) *finalizer_entry = merge_evidence(std::move(*finalizer_entry), caught);
          else if (has_finalizer && prior_exits != catch_exits) finalizer_entry = merge_evidence(state, caught);
        }
        state = guarded ? merge_reaching_evidence(std::move(state), std::move(caught), prior_exits, catch_exits) :
            merge_evidence(std::move(state), caught);
        if (prior_exits) result = type;
        else if (!catch_exits && type != result) result.reset();
      }
      for (auto &path : body_exits.paths) forward_type_exit(outgoing, path);
      const bool project_paths = finalizer && outer && state.selections.tracking && !outgoing.paths.empty();
      if (finalizer) {
        const auto before_cleanup = state.selections.writes;
        if (!outgoing.paths.empty()) {
          const auto &joined = finalizer_entry ? *finalizer_entry : state;
          std::optional<TypeEvidenceState> incoming;
          if (result != Type::never) incoming = joined;
          for (const auto &path : outgoing.paths) {
            auto suspended = path.state;
            if (path.returned() && finalizer->inspects_return()) suspended.pending_return = true;
            incoming = incoming ? merge_evidence(std::move(*incoming), suspended) : std::move(suspended);
          }
          // An abandoned normal path contributes lexical declarations only.
          finalizer_entry = merge_evidence(std::move(*incoming), joined, true);
        }
        auto &entry = finalizer_entry ? *finalizer_entry : state;
        entry.exits = project_paths ? nullptr : outer;
        const auto cleanup = type_evidence(*finalizer, entry);
        if (guarded && entry.required && cleanup != Type::void_type && cleanup != Type::never)
          fail(finalizer->at, "finally must complete with void or an explicit return");
        // Check against every incoming completion, then project the effects
        // onto the normal continuation and the still-pending abrupt paths.
        if (project_paths) {
          if (result != Type::never) {
            TypeEvidenceExits::Path normal{{0}, std::move(state), {}};
            if (complete_type_finalizer(*finalizer, normal, *outer, CleanupBoundary::ordinary_finally) == Type::never)
              result = Type::never;
            state = std::move(normal.state);
          } else state = std::move(entry);
          for (auto &path : outgoing.paths)
            if (complete_type_finalizer(*finalizer, path, *outer, CleanupBoundary::ordinary_finally) != Type::never)
              forward_type_exit(*outer, path);
        } else if (finalizer_entry) {
          if (cleanup == Type::never || result == Type::never) state = std::move(*finalizer_entry);
          else if (project_finalizer(*finalizer, state) == Type::never) result = Type::never;
        }
        if (cleanup == Type::never) return Type::never;
        // A saved result selection survives cleanup. Facts about how to
        // reach it through a mutable slot or a Boolean test may have expired.
        state.selections.forget_changed(target_facts, before_cleanup);
      }
      if (outer && !project_paths) for (auto &path : outgoing.paths)
        if (!finalizer || project_finalizer(*finalizer, path.state) != Type::never)
          forward_type_exit(*outer, path);
      if (expr.kind == K::deferred) { state = before; return Type::void_type; }
      return result;
    }
    case K::range_for: case K::fold: {
      const bool fold = expr.kind == K::fold;
      const auto first = child(0), last = child(1, false, first != Type::never);
      const auto index = state.required ? std::optional{range_index_type(*first, *last, expr.at)} :
          first == Type::never ? last : first;
      const bool completes = first != Type::never && last != Type::never;
      const auto accumulator = fold ? child(2, false, completes) : std::optional<Type>{Type::void_type};
      if (fold && state.required) require_value(*accumulator, expr.at);
      if (fold && completes && accumulator && accumulator->kind == Type::procedure) note_type_allocation(state);
      const auto bound = [&](const Expression &operand) -> std::optional<std::uint64_t> {
        if (first && first == last && integral(*first) && operand.kind == K::integer) return literal(operand).integer;
        return {};
      };
      const auto reach = range_reachability(completes && accumulator != Type::never, index,
                                            bound(*expr.children[0]), bound(*expr.children[1]));
      const auto first_iteration_observation = state.selections.observation_decisions ? state.selections.observation_decisions->next_observation : 0;
      if (reach.body) capture_preparation_targets(expr, state);
      auto iteration = state;
      if (reach.repeat && !iteration.repeated_observations) iteration.repeated_observations = first_iteration_observation;
      const auto control_owner = EvidenceOwner::for_scope(expr);
      iteration.bindings[expr.text] = index; iteration.views[expr.text] = {true, true, true, control_owner, false, false, true, true};
      iteration.views.at(expr.text).place_owners.insert(control_owner);
      iteration.views.at(expr.text).storage_kind = PlaceKind::value;
      iteration.local_declarations[expr.text] = &expr;
      iteration.definitions.erase(expr.text);
      iteration.owner_bindings.erase(expr.text);
      iteration.selections.remove_binding(expr.text);
      if (fold) {
        iteration.bindings[expr.binding] = accumulator; iteration.views[expr.binding] = {true, true, true, control_owner, false, false, true, true};
        iteration.views.at(expr.binding).place_owners.insert(control_owner);
        iteration.views.at(expr.binding).storage_kind = PlaceKind::value;
        iteration.local_declarations[expr.binding] = &expr;
        iteration.definitions.erase(expr.binding);
        iteration.owner_bindings.erase(expr.binding);
        iteration.selections.remove_binding(expr.binding);
      }
      const auto &loop_body = *expr.children[fold ? 3 : 2];
      const auto loop_read = fold ? EvidenceRead::payload : EvidenceRead::discarded;
      const auto header = state.deferred && reach.repeat
          ? state.deferred->header(state.deferred_queue, first_iteration_observation) : 0;
      if (header) iteration.deferred_queue = header;
      // Only facts that survive every backedge are available in the body.
      // Each abstract iteration can only remove facts from this finite set.
      if (iteration.selections.tracking && reach.repeat) {
        while (!iteration.selections.facts.empty() || !iteration.selections.booleans.empty() ||
               !iteration.selections.targets.empty() || !iteration.selections.equal_selections.empty() ||
               !iteration.selections.reference_observations.empty()) {
          auto probe = iteration;
          probe.required = false; probe.returns = nullptr; probe.exits = nullptr; probe.deferred = nullptr;
          if (type_evidence(loop_body, probe, false, nullptr, nullptr, ExpectedTypeUse::materialization, true, loop_read) == Type::never) break;
          probe.selections.forget_observations(first_iteration_observation);
          auto invariant = iteration.selections;
          iteration.selections.join_flow(probe.selections);
          if (iteration.selections.same_flow(invariant)) break;
        }
      }
      TypeEvidenceExits iteration_exits;
      if (state.exits) iteration.exits = &iteration_exits;
      const auto body = type_evidence(loop_body, iteration, false, nullptr, nullptr,
                                      ExpectedTypeUse::materialization, reach.body, loop_read);
      if (fold && reach.body && body && body->kind == Type::procedure)
        note_type_allocation(iteration);
      if (header && body != Type::never) state.deferred->backedge(header, iteration.deferred_queue);
      const auto leave_iteration = [&](TypeEvidenceState &path) {
        restore_type_binding(path, state, expr.text);
        if (fold) restore_type_binding(path, state, expr.binding);
        if (reach.repeat) path.selections.forget_observations(first_iteration_observation);
        path.repeated_observations = state.repeated_observations;
      };
      leave_iteration(iteration);
      iteration.exits = state.exits;
      if (state.exits) for (auto &path : iteration_exits.paths) {
        leave_iteration(path.state);
        if (reach.repeat) path.result.forget_observations(first_iteration_observation);
        forward_type_exit(*state.exits, path);
      }
      if (fold && state.required) check_fold_body(*body, *accumulator, expr.at);
      const auto otherwise = !fold && expr.children.size() == 4
          ? type_evidence(*expr.children[3], state, false, nullptr, nullptr, ExpectedTypeUse::materialization,
                          reach.empty, EvidenceRead::discarded)
          : std::optional<Type>{Type::void_type};
      const bool body_exits = !reach.body || body == Type::never;
      const bool empty_exits = !reach.empty || otherwise == Type::never;
      state = merge_reaching_evidence(std::move(state), std::move(iteration), empty_exits, body_exits);
      return body_exits && empty_exits ? std::optional<Type>{Type::never} : accumulator;
    }
    case K::binary:
      if (expr.text == "&&" || expr.text == "||") {
        const auto left = child(0);
        if (state.returns && left == Type::never) return Type::never;
        const bool short_result = expr.text == "||";
        if (state.deferred) state.selections.observe_choice(state.target.branches);
        const auto proof = state.target.branches;
        const auto skip_facts = state.selections.outcome(proof, short_result);
        const auto evaluate_right = state.selections.outcome(proof, !short_result);
        if (left != Type::never && skip_facts && evaluate_right) capture_preparation_targets(expr, state);
        auto skipped = state;
        skipped.selections.assume(proof, short_result);
        state.selections.assume(proof, !short_result);
        const auto right = child(1, false, left != Type::never && static_cast<bool>(evaluate_right));
        if (state.required && ((left != Type::boolean && left != Type::never) ||
                               (right != Type::boolean && right != Type::never)))
          fail(expr.at, "logical operation requires bool operands");
        if (state.selections.tracking && left != Type::never) {
          PresenceBranches skip{{}, {}}, evaluated{{}, {}};
          skip.at(short_result) = skip_facts;
          if (evaluate_right && right != Type::never)
            for (const bool value : {false, true}) evaluated.at(value) = state.selections.outcome(state.target.branches, value);
          target_facts.branches = QuerySelections::alternatives(std::move(skip), evaluated);
        }
        state = merge_reaching_evidence(std::move(skipped), std::move(state), !skip_facts,
                                       left != Type::never && (right == Type::never || !evaluate_right));
        if (state.returns && !left) return {};
        return left == Type::never ? Type{Type::never} : Type{Type::boolean};
      }
      [[fallthrough]];
    case K::unary: case K::call: case K::value_call: case K::behave: case K::perform: {
      const bool invokes = !state.preparing(expr);
      std::optional<Type> target;
      bool local = false, implicit = false, independent_callee = false;
      if (expr.kind == K::behave || expr.kind == K::perform) {
        if (!state.complete) return {};
        if (state.validating_body && state.method && program_.methods[*state.method].source->kind == Form::Kind::function)
          fail(expr.at, "a function cannot invoke a dynamic procedure");
        const auto id = resolve_symbol(expr.text, expr.at, symbol_shadowed(expr.text, state), state.method);
        if (program_.symbols[id].intrinsic_print) {
          if (state.required) print_result(expr);
          bool exits = false;
          for (const auto &item : expr.children) {
            const auto argument = type_evidence(*item, state, false, nullptr, nullptr, ExpectedTypeUse::materialization, !exits);
            exits = exits || argument == Type::never;
            if (state.required) print_result(expr, argument);
          }
          if (!exits && !invokes) record_type_invocation(expr, state, call_reasons(std::set<size_t>{}), true);
          return exits ? Type::never : Type::void_type;
        }
        if (state.required) dynamic_procedure_type(program_.symbols[id], expr.at);
        target = program_.symbols[id].type; local = true;
      } else if (expr.kind == K::value_call) {
        const auto &callee = *expr.children.front();
        const bool scope_expectation = expected_use == ExpectedTypeUse::callable_result;
        implicit = callee.definition && callee.definition->implicit_scope;
        if (!invokes && !implicit) {
          const auto *source = &callee;
          while (source->kind == K::grouped) source = source->children.front().get();
          if (source->definition)
            independent_callee = explicit_type_call_effects(lexical_method(source->definition.get(), state.method), state, {});
        }
        target = implicit ?
            scope_callable_evidence(callee, state, scope_expectation ? expected : nullptr,
                                    scope_expectation && contextual && !state.required, target_facts) : child(0);
        local = true;
      } else if (!expr.intrinsic && state.complete) {
        if (state.required && state.bindings.contains(expr.text)) check_query_binding(expr.text, expr.at);
        if (const auto found = state.bindings.find(expr.text); found != state.bindings.end() && !state.definitions.contains(expr.text)) {
          target = found->second; local = true;
          if (target && target->kind == Type::procedure) {
            const auto selected = named_type_target(state, expr.text);
            require_type_initialized(state, selected, expr.text, expr.at);
            read_type_target(state, state.views.at(expr.text), selected);
          }
        }
      }
      const auto name = local ? std::string_view{} : operation_name(expr, &state);
      const size_t offset = expr.kind == K::value_call ? 1 : 0;
      if (local && !implicit && target && target->kind == Type::procedure)
        note_type_allocation(state);
      if (!local && !expr.intrinsic && state.exits) note_family_type_capture(state, named_family(expr.text, &state));
      auto probe = state;
      probe.required = false;
      probe.exits = nullptr;
      probe.initialization = TypeEvidenceState::InitializationCheck::suppressed;
      std::vector<ArgumentEvidence> evidence(expr.children.size());
      std::vector<ViewEvidence> hinted_views(expr.children.size());
      std::vector<TargetEvidence> hinted_targets;
      std::map<size_t, TypeEvidenceState> unresolved;
      if (state.selections.tracking) hinted_targets.resize(expr.children.size());
      for (size_t i = offset; i < expr.children.size(); ++i) {
        auto before = probe;
        evidence[i].type = type_evidence(*expr.children[i], probe, true, nullptr, nullptr, ExpectedTypeUse::callable_result,
                                         true, EvidenceRead::target);
        if (state.returns && evidence[i].type == Type::never) return Type::never;
        evidence[i].read_only = probe.value.read_only; evidence[i].owner = probe.value.owner;
        hinted_views[i] = probe.value;
        if (!hinted_targets.empty()) hinted_targets[i] = probe.target;
        if (!evidence[i].type) unresolved.emplace(i, std::move(before));
      }
      const bool ordered = state.required || state.selections.tracking;
      if (!ordered) state = probe;
      std::vector<Type> arguments;
      Access access;
      Owners owners;
      std::vector<ViewEvidence> views;
      std::vector<TargetEvidence> operand_targets;
      std::vector<size_t> operand_revisions;
      std::map<size_t, size_t> shared_targets;
      std::vector<ArgumentMode> prepared_modes;
      const auto readable_borrow = [](ArgumentMode mode) {
        return mode == ArgumentMode::borrowed_ref || mode == ArgumentMode::borrowed_read;
      };
      bool complete = true;
      bool exits = target == Type::never;
      for (size_t i = offset; i < expr.children.size(); ++i) {
        ArgumentPreparation parameter;
        if (local && target && target->kind == Type::procedure && target->parameters.size() == expr.children.size() - offset)
          parameter = {argument_mode(*target, i - offset), target->parameters[i - offset]};
        else if (!local && complete && !exits)
          parameter = argument_preparation(expr, name, arguments, evidence, access, owners, state.required, &state);
        const auto *expected_argument = parameter.type ? &*parameter.type : nullptr;
        const auto use = expected_type_use(parameter.mode);
        const auto operand_read = !swap_operation(name) &&
            (parameter.mode == ArgumentMode::value || parameter.mode == ArgumentMode::borrowed_read) ?
                EvidenceRead::payload : EvidenceRead::target;
        auto type = evidence[i].type;
        auto selected = hinted_views[i];
        TargetEvidence selected_target = hinted_targets.empty() ? TargetEvidence{} : std::move(hinted_targets[i]);
        if (ordered) {
          type = type_evidence(*expr.children[i], state, true, expected_argument, nullptr, use, !exits, operand_read);
          selected = state.value;
          selected_target = state.target;
          if (!exits && type && type != Type::never && readable_borrow(parameter.mode))
            prepare_type_readable_argument(state, *expr.children[i], selected, selected_target, parameter.mode);
          if (!exits && type && type->kind == Type::procedure && parameter.mode == ArgumentMode::value)
            note_type_allocation(state);
        } else if (!type && expected_argument) {
          auto prepared = unresolved.at(i);
          type = type_evidence(*expr.children[i], prepared, true, expected_argument, nullptr, use);
          selected = prepared.value;
          selected_target = prepared.target;
        }
        complete = complete && type.has_value();
        exits = exits || type == Type::never;
        arguments.push_back(type.value_or(Type::never)); access.push_back(selected.read_only); owners.push_back(selected.owner);
        views.push_back(selected);
        prepared_modes.push_back(parameter.mode);
        if (state.selections.tracking) {
          if (!local && state.checks_initialization() && parameter.mode == ArgumentMode::borrowed_ptr)
            for (size_t prior = 0; prior < operand_targets.size(); ++prior) {
              if (prepared_modes[prior] != ArgumentMode::borrowed_ptr || arguments[prior] != arguments.back() ||
                  !state.selections.same_prepared_target(operand_targets[prior], operand_revisions[prior], selected_target)) continue;
              shared_targets.emplace(operand_targets.size(), shared_targets.contains(prior) ? shared_targets.at(prior) : prior);
              break;
            }
          operand_targets.push_back(std::move(selected_target));
          operand_revisions.push_back(state.selections.writes);
        }
      }
      if (exits) return Type::never;
      const auto check_borrowed_argument = [&](size_t i, ArgumentMode mode) {
        if (mode == ArgumentMode::value) return;
        if (views[i].view != true || views[i].mixed_view)
          implementation_gap(expr.at, "borrowed parameter requires an existing view");
        const auto &operand = *expr.children[i + offset];
        if (readable_borrow(mode) && state.checks_initialization() && prepared_modes[i] != mode)
          implementation_gap(operand.at, "readable borrowed argument requires an established preparation mode");
        check_type_deferred_argument(state, expr, operand, views[i], mode);
      };
      const auto saved_operand = [&](size_t i) {
        auto operand = operand_targets.at(i);
        state.selections.forget_changed(operand, operand_revisions.at(i));
        return operand;
      };
      const auto ready_input = [&](size_t i) {
        const auto operand = saved_operand(i);
        if (state.selections.selection_initialized(operand) || operand.presence == TargetPresence::absent) return true;
        // Later operands can initialize the prepared target. A rebind cannot
        // make the saved operand follow the binding's new selection.
        const auto reaching = state.selections.outcome(state.selections.selection_outcomes(operand), true);
        return !operand.places.empty() && std::ranges::none_of(operand.places, [&](const auto &place) {
          return state.selections.may_read_uninitialized(place, reaching);
        });
      };
      const auto retain_borrowed_targets = [&](const ViewEvidence &result, const Type &type, const auto &mode,
          const std::optional<TypeCallSummary> &proved = {}) {
        if (!state.tracks_target_places() || result.view != true || !scalar_type(type)) return;
        if (proved) target_facts.initialized = std::ranges::all_of(proved->required_inputs, ready_input);
        for (size_t i = 0; i < operand_targets.size(); ++i) {
          if (proved ? !proved->targets.contains(i) : mode(i) == ArgumentMode::value || arguments[i] != type ||
              compare_owners(result.owner, owners[i], &state) == OwnerMatch::mismatch) continue;
          const auto &operand = operand_targets[i];
          if (state.borrowed_return && operand.places.empty() && operand.presence != TargetPresence::absent) {
            target_facts.places.clear();
            return;
          }
          target_facts.places.insert(operand.places.begin(), operand.places.end());
        }
        if (proved && proved->targets.empty()) target_facts.presence = TargetPresence::absent;
      };
      const auto apply_initialized_inputs = [&](const TypeCallSummary &proof) {
        for (const auto i : proof.initialized_inputs) {
          auto operand = saved_operand(i);
          if (invokes) state.selections.initialize_if_present(operand);
          else if (!operand.places.empty()) {
            if (const auto saved = save_prepared_type_target(state, *expr.children[i + offset], operand))
              state.prepared_call->effects.initialized_selections.insert(*saved);
            if (operand.presence == TargetPresence::present && operand.places.size() == 1)
              state.prepared_call->effects.initialized.insert(*operand.places.begin());
          }
        }
      };
      if (local) {
        if (state.required && (!target || target->kind != Type::procedure || !target->target))
          fail(expr.at, "nearest binding '" + expr.text + "' is not callable");
        if (state.required && target->parameters.size() != arguments.size()) fail(expr.at, "callable argument count does not match its view");
        if (!target || target->kind != Type::procedure || !target->target || target->parameters.size() != arguments.size()) return {};
        if (state.validating_body && state.method && program_.methods[*state.method].source->kind == Form::Kind::function && !target->pure_callable)
          fail(expr.at, "a function cannot invoke a procedure");
        if (state.required) for (size_t i = 0; i < arguments.size(); ++i) {
          const auto mode = target->argument_modes.empty() ? ArgumentMode::value : target->argument_modes[i];
          check_borrowed_argument(i, mode);
          if (!accepts_argument(arguments[i], target->parameters[i], mode))
            fail(expr.children[i + (expr.kind == K::value_call)]->at, "argument does not satisfy the callable view type");
          if (mode != ArgumentMode::value && access[i].value_or(false) && (target->const_views.empty() || !target->const_views[i]))
            fail(expr.at, "readonly argument cannot supply a writable borrowed parameter");
          for (size_t j = 0; j < i; ++j) if (!target->owners.empty() && target->owners[i] && target->owners[i] == target->owners[j]) {
            const auto relation = compare_owners(owners[i], owners[j], &state);
            if (relation == OwnerMatch::mismatch) fail(expr.at, "arguments for one owner role have different actual owners");
            if (relation == OwnerMatch::unknown) implementation_gap(expr.at, "typeof callable view requires an unproved owner equality");
          }
        }
        const bool borrowed = !target->owners.empty() && target->owners.back();
        view = {borrowed && !target->const_views.empty() && target->const_views.back(), borrowed, {}};
        view.nullable = borrowed && !target->nullable_views.empty() && target->nullable_views.back();
        if (borrowed) for (size_t i = 0; i < owners.size(); ++i)
          if (target->owners[i] == target->owners.back() && owners[i]) view.owner = owners[i];
        if (borrowed) retain_borrowed_targets(view, *target->target, [&](size_t i) { return argument_mode(*target, i); });
        if (!implicit) {
          for (size_t i = 0; i < operand_targets.size(); ++i)
            type_input_mutation(expr, state, arguments[i], operand_targets[i], argument_mode(*target, i),
                                !target->const_views.empty() && target->const_views[i]);
          const bool explicit_effects = independent_callee && std::ranges::all_of(arguments, scalar_type);
          record_type_call(expr, state, *target, explicit_effects);
        }
        return *target->target;
      }
      if (!state.complete) return {};
      if (swap_operation(name)) {
        if (!state.required && !state.selections.tracking) return Type::void_type;
        if (arguments.size() != 2) {
          if (state.required) fail(expr.at, "swap requires two destinations");
          return Type::void_type;
        }
        const bool reference = name == "<=>";
        const auto family = expr.intrinsic ? NamedFamily{} : named_family(expr.text, &state);
        const auto methods = method_candidates(family.methods, arguments, family.retained_types(), access, owners, &state);
        if (reference && invokes && state.selections.tracking && methods.methods.empty()) {
          const auto &left = operand_targets[0].selection, &right = operand_targets[1].selection;
          state.selections.exchange(left, right);
        }
        for (const auto id : methods.methods) {
          const auto &method = program_.methods[id];
          for (size_t i = 0; i < operand_targets.size(); ++i) {
            const auto &parameter = method.input_parameters()[i];
            type_input_mutation(expr, state, arguments[i], operand_targets[i], argument_mode(parameter), parameter.read_only, reference);
          }
          record_type_call(expr, state, method, invokes ||
              explicit_type_call_effects(id, family.declaration ? *family.declaration : state, arguments));
          if (!invokes && reference)
            state.prepared_call->effects.outcomes.insert(Completion::failure(exception_id(ExceptionCategory::null_pointer)).reason);
        }
        if (!reference && methods.methods.empty() && operand_targets.size() == 2)
          for (size_t i = 0; i < views.size(); ++i) {
            auto selected = operand_targets[i];
            selected.selection.slot.reset();
            selected.same_selected.clear();
            selected.presence_proof.reset();
            require_type_initialized(state, operand_targets[i], i ? "right swap target" : "left swap target", expr.at);
            read_type_target(state, views[i], selected);
          }
        if (!reference && methods.methods.empty() && operand_targets.size() == 2 && arguments.front() == Type::boolean) {
          const auto &left = operand_targets.front().places, &right = operand_targets.back().places;
          if (invokes) state.selections.exchange_boolean(left, right);
          else {
            type_boolean_write(expr, state, left);
            type_boolean_write(expr, state, right);
            if (!left.empty() && !right.empty()) state.prepared_call->effects.builtin = PreparedBooleanSwap{left, right};
          }
        }
        if (!state.required) return Type::void_type;
        for (const auto &operand : views) {
          if (operand.view != true) fail(expr.at, "swap requires two storage destinations");
          if (operand.mixed_view) implementation_gap(expr.at, "swap through a mixed value/storage view");
          if (reference) reference_destination(operand, expr.at);
        }
        if (!methods.methods.empty()) {
          // Signature inspection retains exact expression access and owner
          // checks. Physical binding preservation requires the invoked body's
          // source summary; a type query neither invokes nor demands that body.
          select_method(family.methods, arguments, expr.at, expr.text, family.retained_types(), access, owners, &state);
          return Type::void_type;
        }
        if (reference) {
          for (size_t i = 0; i < views.size(); ++i) {
            const auto other = 1 - i;
            rebind_type(arguments[i], arguments[other], expr.at);
            if (!views[other].read_only || (!views[i].slot_read_only && *views[other].read_only))
              implementation_gap(expr.at, "reference target access is not established");
            rebind_access(views[i].slot_read_only.value_or(false), *views[other].read_only, expr.at);
          }
          const auto relation = compare_owners(owners[0], owners[1], &state);
          if (relation == OwnerMatch::mismatch) fail(expr.at, "reference rebinding cannot change the initializer-fixed owner");
          if (relation == OwnerMatch::unknown) implementation_gap(expr.at, "reference swap requires an established owner equality");
        } else {
          for (size_t i = 0; i < views.size(); ++i) {
            if (!access[i]) implementation_gap(expr.at, "bits destination access is not established");
            bits_swap_target(arguments[i], *access[i], expr.at);
          }
          if (arguments[0] != arguments[1]) fail(expr.at, "bits swap requires the exact same type");
        }
        const auto declared = builtin_declarations(expr, arguments, Type::void_type, true, access, owners, &state);
        if (!invokes) {
          if (reference && operand_targets.size() == 2) {
            const auto &left = operand_targets.front().selection, &right = operand_targets.back().selection;
            type_slot_write(expr, state, left);
            type_slot_write(expr, state, right);
            if (!left.footprint.empty() && !right.footprint.empty())
              state.prepared_call->effects.builtin = PreparedReferenceSwap{left, right};
          }
          std::set<size_t> outcomes{0};
          for (const auto category : declared) outcomes.insert(Completion::failure(category).reason);
          record_type_invocation(expr, state, std::move(outcomes), true);
        }
        return Type::void_type;
      }
      if (expr.text == "cause_at" && (expr.intrinsic || (!module_names().callables.contains(expr.text) && !has_declarations(expr.text)))) {
        if (state.required && (arguments.size() != 2 || arguments[0].kind != Type::exception || arguments[1] != Type::uint64))
          fail(expr.at, "cause_at requires an exception and a u64 index");
        record_type_invocation(expr, state, {0, Completion::failure(exception_id(ExceptionCategory::index_out_of_bounds)).reason}, true);
        return exception_type(0);
      }
      if (!complete) return {};
      const auto family = expr.intrinsic ? NamedFamily{} : named_family(expr.text, &state);
      const auto &retained = family.retained_types();
      const auto candidates = method_candidates(family.methods, arguments, retained, access, owners, &state);
      if (candidates.methods.empty() && !expr.intrinsic && has_declarations(expr.text) &&
          (!eager_operation(name) || !eager_types(name, arguments).error.empty())) {
        const auto result = inspect_declarations(expr.text, arguments, access, owners, expr.at, state,
            [&](const CheckedMethod &declaration, const Substitutions &, const ResultContract &contract) {
          retain_borrowed_targets(result_view(declaration, contract, owners), *contract.type,
              [&](size_t i) { return argument_mode(parameter_at(declaration, i)); });
          for (size_t i = 0; i < operand_targets.size(); ++i) {
            const auto &parameter = parameter_at(declaration, i);
            type_input_mutation(expr, state, arguments[i], operand_targets[i], argument_mode(parameter), parameter.read_only,
                                caller_bindings(declaration));
          }
          record_type_call(expr, state, declaration, invokes ||
              (declaration.source->kind == Form::Kind::function && std::ranges::all_of(arguments, scalar_type)));
          if (!invokes && caller_bindings(declaration))
            state.prepared_call->effects.outcomes.insert(Completion::failure(exception_id(ExceptionCategory::null_pointer)).reason);
          if (state.required) for (size_t i = 0; i < arguments.size(); ++i) {
            const auto mode = argument_mode(parameter_at(declaration, i));
            check_borrowed_argument(i, mode);
          }
        });
        if (result) view = result->view;
        return result ? std::optional<Type>{result->type} : std::nullopt;
      }
      if (state.required && family.methods.empty() && !eager_operation(name))
        implementation_gap(expr.at, "typeof requires a known callable signature without outer value capture");
      if (candidates.methods.empty() && eager_operation(name)) {
        const auto types = eager_types(name, arguments);
        if (state.required && !types.error.empty()) fail(expr.at, types.error);
        if (types.error.empty()) {
          if (name == "!" && operand_targets.size() == 1 && operand_targets.front().branches) {
            const auto &test = *operand_targets.front().branches;
            target_facts.branches = PresenceBranches{test.when_false, test.when_true};
          }
          const auto failures = builtin_declarations(expr, arguments, types.result, false, access, owners, &state);
          std::set<size_t> outcomes{0};
          for (const auto category : failures) outcomes.insert(Completion::failure(category).reason);
          if (types.failing) outcomes.insert(2);
          record_type_invocation(expr, state, std::move(outcomes), true);
        }
        return types.error.empty() ? std::optional<Type>{types.result} : std::nullopt;
      }
      const auto best = state.required ? std::vector<size_t>{select_method(family.methods, arguments, expr.at, expr.text, retained, access, owners, &state)}
                                       : most_specific(candidates.methods, access, owners, &state, candidates.widening);
      std::optional<Type> result;
      for (const auto id : best) {
        const auto &method = program_.methods[id];
        for (size_t i = 0; i < operand_targets.size(); ++i) {
          const auto &parameter = parameter_at(method, i);
          type_input_mutation(expr, state, arguments[i], operand_targets[i], argument_mode(parameter), parameter.read_only,
                              caller_bindings(method));
        }
        record_type_call(expr, state, method, invokes ||
            explicit_type_call_effects(id, family.declaration ? *family.declaration : state, arguments));
        if (state.validating_body && state.method && program_.methods[*state.method].source->kind == Form::Kind::function &&
            method.source->kind == Form::Kind::procedure)
          fail(expr.at, "a function cannot invoke a procedure");
        if (state.required) for (size_t i = 0; i < arguments.size(); ++i) {
          const auto mode = argument_mode(parameter_at(method, i));
          check_borrowed_argument(i, mode);
        }
        const auto type = infer_result(id, arguments, retained, family.declaration ? family.declaration : &state).contract;
        if (!type || nested_binder(method, *type->type) || (result && result != type->type)) return {};
        if (state.exits || !invokes)
          for (const auto &obligation : declaration_obligations(method, arguments, access, owners, &state)) {
            if (invokes) note_type_failures(state, obligation.declaration->throws_types);
            else for (const auto &failure : obligation.declaration->throws_types)
              for (const auto category : exception_possibilities(failure))
                state.prepared_call->effects.outcomes.insert(Completion::failure(category).reason);
          }
        const auto next = result_view(method, *type, owners);
        std::optional<TypeCallSummary> proved;
        if (state.required && state.checks_initialization() && (scalar_type(*type->type) || type->type == Type::void_type)) {
          TypeCallInputs inputs;
          for (const auto &[input, representative] : shared_targets)
            if (argument_mode(parameter_at(method, input)) == ArgumentMode::borrowed_ptr &&
                argument_mode(parameter_at(method, representative)) == ArgumentMode::borrowed_ptr)
              inputs.shared_targets.emplace(input, representative);
          bool writable_unready_input = false;
          for (size_t i = 0; i < operand_targets.size(); ++i)
            if (const auto &parameter = parameter_at(method, i); argument_mode(parameter) == ArgumentMode::borrowed_ptr) {
              if (ready_input(i)) inputs.ready.insert(inputs.target(i));
              else writable_unready_input |= !parameter.read_only;
            }
          if (type->borrowed() || writable_unready_input)
            proved = prove_type_call(id, arguments, retained, *type, family.declaration ? *family.declaration : state, inputs);
        }
        if (proved && !proved->completes) {
          if (invokes) return Type::never;
          state.prepared_call->effects.outcomes.erase(0);
          proved.reset();
        }
        retain_borrowed_targets(next, *type->type, [&](size_t i) { return argument_mode(parameter_at(method, i)); }, proved);
        if (proved) apply_initialized_inputs(*proved);
        view = !result || view == next ? next : ViewEvidence{};
        result = type->type;
      }
      return result;
    }
    case K::field: {
      const auto type = child(0);
      if (type == Type::never) return Type::never;
      if (state.required && type && type->kind == Type::exception) check_exception_field(expr);
      return type && type->kind == Type::exception ? std::optional<Type>{Type::uint64} : std::nullopt;
    }
    case K::return_tail: case K::return_value: {
      const bool tail = expr.kind == K::return_tail;
      if (tail && !state.checks_reaching_paths())
        return child(0, false, false, EvidenceRead::discarded) == Type::never ? Type{Type::never} : Type{Type::void_type};
      const auto *expected_return = state.invocation_result ? &*state.invocation_result : nullptr;
      const auto expected_use = state.supplied_invocation_result ? ExpectedTypeUse::callable_result : ExpectedTypeUse::materialization;
      std::vector<Type> tail_types;
      auto *evidence = !state.returns ? nullptr : tail ? &tail_types : &state.returns->types;
      const auto result = expr.children.empty() ? std::optional<Type>{Type::void_type} :
          type_evidence(*expr.children.front(), state, false, expected_return, evidence,
                        expected_use, true, state.borrowed_return ? EvidenceRead::target : EvidenceRead::payload);
      if (tail && (result == Type::void_type || result == Type::never)) return result;
      if (tail && state.returns)
        for (const auto &type : tail_types) if (type != Type::void_type) state.returns->types.push_back(type);
      if (!state.borrowed_return && state.validating_body && result && result != Type::never && state.invocation_result &&
          !transfers_value(*result, *state.invocation_result)) {
        if (tail && *state.invocation_result == Type::void_type)
          fail(expr.at, "value-producing tail in a void callable; discard the result with ': void' or end with 'return;'");
        fail(expr.at, "return type does not satisfy the implicit callable result");
      }
      if (state.returns) {
        state.returns->value_obligation |= !expr.children.empty() && result != Type::void_type && result != Type::never;
        state.returns->unresolved |= !result;
        if (expr.children.empty()) state.returns->types.push_back(Type::void_type);
      }
      if (result && result->kind == Type::procedure) note_type_allocation(state);
      if (result != Type::never) {
        TypeReturned returned;
        if (state.borrowed_return) {
          const auto &contract = *state.borrowed_return;
          const auto owner = state.owner_bindings.at(contract.owner);
          const bool valid_owner = compare_owners(state.value.owner, owner, &state) == OwnerMatch::satisfied;
          if (contract.nullable() && result == Type::null_ref && (valid_owner || !state.value.owner)) returned.borrowed.emplace();
          else if (result == contract.type && state.value.view == true && !state.value.mixed_view && valid_owner &&
                   (contract.read_only || state.value.read_only == false) &&
                   (contract.nullable() || !state.value.nullable || state.target.presence == TargetPresence::present) &&
                   (!state.target.places.empty() || state.target.presence == TargetPresence::absent)) {
            if (!contract.nullable()) require_type_initialized(state, state.target, {}, expr.at);
            returned.borrowed = TypeBorrowedReturn::capture(state.target, state.selections);
            if (state.checks_initialization() && !returned.borrowed->uninitialized.empty() &&
                state.target.places.size() > 1 && (!state.repeated_observations || prepared_entry_target(state.target))) {
              state.selections.save_selection(&expr, state.target);
              returned.borrowed->saved_selection = &expr;
            }
          } else if (!contract.nullable() && state.checks_initialization() && state.selections.initialization_reachable) {
            // Every attempted ref return needs a valid prepared result, even
            // when an action will replace it before delivery.
            fail(expr.at, "cannot prove preparation of the borrowed ref result");
          }
        } else if (state.exits && state.selections.tracking && result == Type::boolean) {
          returned.boolean = state.selections.observe(state.target.branches);
          returned.boolean_revision = state.selections.writes;
        }
        note_type_exit(state, {1, true}, std::move(returned));
      }
      return Type::never;
    }
    case K::raise: {
      if (state.validating_body && expr.children.empty() && !state.caught_exception)
        fail(expr.at, "bare throw requires an active catch");
      if (state.returns) {
        state.returns->saw_throw = true;
        if (expr.children.empty() && !state.caught_exception) {
          if (state.method && program_.methods[*state.method].source->implicit_scope)
            fail(expr.at, "bare throw requires an active catch");
          state.returns->unresolved_throw = true;
        }
      }
      auto category = state.caught_exception;
      bool completes = true;
      for (size_t i = 0; i < expr.children.size(); ++i) {
        const auto &item = *expr.children[i];
        const auto type = child(i, false, completes);
        if (!i) category = type;
        completes = completes && type != Type::never;
        if (state.returns && !type) state.returns->unresolved_throw = true;
        if (state.validating_body && type && type->kind != Type::exception && type != Type::never)
          fail(item.at, "throw requires an intrinsic exception value");
      }
      if (completes && expr.children.size() == 2) note_type_allocation(state);
      if (completes && category) note_type_failure(state, *category);
      return Type::never;
    }
    case K::callable: {
      const auto method = lexical_method(expr.definition.get(), state.method);
      if (!expr.definition->name.empty()) {
        auto declaration = std::make_shared<TypeEvidenceState>(project_evidence(state, definition_dependencies(*expr.definition)));
        state.bindings[expr.definition->name] = callable_evidence(method, &state);
        state.local_declarations[expr.definition->name] = &expr;
        state.definitions[expr.definition->name] = {method, std::move(declaration)};
        state.owner_bindings.erase(expr.definition->name);
        state.views[expr.definition->name] = {false, false, {}, {}, false, {}, {}, true};
        return Type::void_type;
      }
      const auto &captures = expr.definition->explicit_captures ? expr.definition->captures : free_bindings(*expr.definition).captures;
      for (const auto &capture : captures) check_type_capture(state, capture.name, capture.at);
      if (state.exits && has_type_captures(method, state)) note_type_allocation(state);
      if (expected && expected->kind == Type::procedure)
        return callable_query({method}, expected, expr.at, state, true);
      return contextual && !state.required ? std::nullopt : callable_evidence(method, &state);
    }
    default:
      state.complete = false;
      return {};
    }
  }
  TypeEvidenceState evidence_state(bool retained_only = false, const std::set<std::string> *names = nullptr,
                                  bool track_selection = false) const {
    TypeEvidenceState state;
    state.selections.tracking = track_selection;
    if (track_selection) state.selections.observation_decisions = std::make_shared<ObservationDecisions>();
    const auto selected = [&](const std::string &name) { return !names || names->contains(name); };
    for (const auto &name : query_environment().symbol_shadows) if (selected(name)) state.symbol_shadows.insert(name);
    if (checking_instance_) {
      state.method = checking_instance_->method; state.substitutions = checking_instance_->substitutions;
      state.retained = checking_instance_->retained_types;
    }
    state.full_scope_owner = cleanup_scope_;
    for (const auto &binding : query_environment().bindings) if (selected(binding.name)) {
      state.bindings[binding.name] = binding.type;
      state.views[binding.name] = binding.view;
      if (binding.owner_binding) state.owner_bindings[binding.name] = *binding.owner_binding;
    }
    const auto retain_binding = [&](const std::string &name, size_t id) {
      const auto &binding = bindings_->at(id);
      state.selections.remove_binding(name);
      state.bindings[name] = binding.type;
      state.definitions.erase(name);
      if (binding.definition) {
        const auto &definition = definitions_.at(id);
        state.definitions[name] = {definition.method, definition.declaration};
        state.bindings[name] = callable_evidence(definition.method, definition.declaration.get());
        state.owner_bindings.erase(name);
        state.views[name] = {false, false, {}, {}, false, {}, {}, true};
        return;
      }
      if (binding.owner_binding) state.owner_bindings[name] = *binding.owner_binding;
      else state.owner_bindings.erase(name);
      const auto found = lexical_.owners.find(binding.alias.value_or(id));
      const auto owner = found != lexical_.owners.end() && found->second.size() == 1 ? std::optional<size_t>{*found->second.begin()} : std::nullopt;
      std::optional<bool> slot_read_only;
      bool fixed = binding.fixed, first = true;
      const auto physical_selection = physical_slots({binding.alias.value_or(id)});
      for (const auto slot : physical_selection) {
        const auto &physical = bindings_->at(slot);
        fixed = fixed || physical.fixed;
        if (first) slot_read_only = physical.read_only;
        else if (slot_read_only != physical.read_only) slot_read_only.reset();
        first = false;
      }
      state.views[name] = {binding.read_only, true, binding.value_binding, owner, false, fixed, slot_read_only,
                           !binding.alias && binding.selection.empty() && (binding.value_binding || binding.callable_value),
                           binding.nullable};
      for (const auto slot : physical_selection)
        for (const auto owner : bindings_->at(slot).place_owners) state.views.at(name).place_owners.insert(owner);
      state.views.at(name).storage_kind = binding.kind();
      if (!track_selection || binding.owner_binding || physical_selection.empty()) return;
      const auto places = [&](size_t slot) {
        EvidencePlaces result;
        if (binding.type != Type::boolean) return result;
        if (const auto targets = lexical_.targets.find(slot); targets != lexical_.targets.end()) {
          for (const auto place : targets->second) {
            if (place == null_target) continue;
            if (storage(place).borrowed) return EvidencePlaces{};
            result.insert(place);
          }
        }
        return result;
      };
      if (binding.value_binding) {
        if (physical_selection.size() == 1) {
          auto known = places(*physical_selection.begin());
          if (!known.empty()) state.selections.value_places.emplace(name, std::move(known));
        }
        return;
      }
      const auto remember = [&](size_t slot) {
        if (const auto targets = lexical_.targets.find(slot); targets != lexical_.targets.end() && !targets->second.empty()) {
          if (!targets->second.contains(null_target)) state.selections.facts[slot] = TargetPresence::present;
          else if (targets->second.size() == 1) state.selections.facts[slot] = TargetPresence::absent;
        }
        const auto known = places(slot);
        if (!known.empty()) state.selections.targets[slot] = known;
      };
      const auto handle = physical_selection.size() == 1 ? *physical_selection.begin() : binding.alias.value_or(id);
      EvidenceSelection selection{handle, {}};
      for (const auto slot : physical_selection) {
        selection.footprint.insert(slot);
        state.selections.footprints[slot] = {slot};
        const auto &overlapping = bindings_->at(slot).overlaps;
        for (const auto other : overlapping) state.selections.possible_overlap[slot].insert(other);
        remember(slot);
      }
      state.selections.footprints[handle] = selection.footprint;
      remember(handle);
      state.selections.bindings[name] = std::move(selection);
    };
    if (retained_only) {
      if (checking_instance_) for (const auto place : checking_instance_->frame.captures) {
        const auto &capture = storage(place);
        if (selected(capture.name)) retain_binding(capture.name, capture.binding);
      }
    } else if (names) {
      for (const auto &name : *names) if (const auto binding = lexical_.lookup(name)) retain_binding(name, *binding);
    } else for (const auto &scope : lexical_.scopes) for (const auto &[name, id] : scope.bindings) retain_binding(name, id);
    const auto conditions = track_selection ? path_conditions(lexical_) : CleanupEquations::ConditionClauses{};
    if (track_selection) {
      std::map<size_t, size_t> selected;
      for (const auto &[slot, footprint] : state.selections.footprints)
        if (const auto id = std::get_if<size_t>(&slot)) {
          const auto identity = lexical_.selection_identities.find(*id);
          if (identity != lexical_.selection_identities.end()) selected.emplace(*id, identity->second);
        }
      for (const auto &group : selection_groups(selected, conditions)) {
        EvidencePlaces witnesses(group.begin(), group.end());
        state.selections.retain_selection(*group.begin(), std::move(witnesses));
      }
    }
    const auto current_slot = [&](const CleanupEquations::BindingSnapshot &snapshot) {
      const auto &[slot, versions] = snapshot;
      const auto current = lexical_.target_versions.find(slot);
      return current != lexical_.target_versions.end() && current->second == versions && state.selections.footprints.contains(slot);
    };
    const auto project_presence = [&](const CleanupEquations::ConditionFacts &clause) {
      PresenceFacts facts;
      for (const auto &binding : clause.bindings)
        if (current_slot(binding)) facts[binding.first] = TargetPresence::present;
      for (const auto &binding : clause.absent_bindings)
        if (current_slot(binding)) facts[binding.first] = TargetPresence::absent;
      return facts;
    };
    if (proving_ && conditions.size() > 1) {
      std::vector<PresenceFacts> paths;
      EvidencePlaces observed;
      for (const auto &clause : conditions) {
        paths.push_back(project_presence(clause));
        for (const auto &[slot, fact] : paths.back())
          if (!state.selections.facts.contains(slot)) observed.insert(slot);
      }
      for (const auto &slot : observed) {
        PresenceBranches branches{{}, {}};
        for (const auto &path : paths) {
          const auto selected = path.find(slot);
          for (const bool present : {false, true}) {
            const auto fact = present ? TargetPresence::present : TargetPresence::absent;
            if (selected != path.end() && selected->second != fact) continue;
          branches.at(present).add({{}, path});
          }
        }
        const auto same = state.selections.same_selected(slot);
        branches.forget([&](const EvidenceSlot &binding) {
          return same.contains(binding) || state.selections.facts.contains(binding);
        });
        state.selections.remember_selection(slot, branches);
      }
    }
    if (track_selection && proving_) {
      std::map<std::pair<BooleanDecision::Origin, size_t>, size_t> observation_ids;
      const auto &decisions = state.selections.observation_decisions;
      const auto observation_id = [&](BooleanDecision::Origin origin, size_t id) {
        const auto [observation, inserted] = observation_ids.try_emplace({origin, id}, decisions->next_observation);
        if (inserted) ++decisions->next_observation;
        return observation->second;
      };
      const auto project_path = [&](const CleanupEquations::ConditionFacts &clause) {
        ObservationPath path;
        for (const auto &[input, value] : clause.inputs)
          path.booleans.emplace(observation_id(BooleanDecision::Origin::input, input), value);
        path.presence = project_presence(clause);
        return path;
      };
      ObservationPaths reaching;
      reaching.decisions = decisions;
      for (const auto &clause : conditions) reaching.add(project_path(clause));
      state.selections.retain_conditions(std::move(reaching));
      for (const auto &[place, proof] : lexical_.predicates) {
        if (!proof || storage(place).borrowed) continue;
        const auto decision = boolean_decision(proof.get());
        if (!proof->complete_results && !decision) continue;
        PresenceBranches branches{{}, {}};
        for (const bool value : {false, true}) branches.at(value).decisions = decisions;
        const auto attach_decision = [&](ObservationPath &path, bool value) {
          if (!decision) return true;
          const bool truth = value == decision->value_when_true;
          const auto [observation, inserted] = path.booleans.emplace(observation_id(decision->origin, decision->id), truth);
          return inserted || observation->second == truth;
        };
        for (const bool value : {false, true}) {
          const auto clauses = proof->results.find(value);
          if (!proof->complete_results) {
            ObservationPath path;
            if (attach_decision(path, value)) branches.at(value).add(std::move(path));
            continue;
          }
          if (clauses == proof->results.end()) continue;
          const auto applicable = conditions.empty() ? clauses->second : CleanupEquations::conjoin_conditions(clauses->second, conditions);
          for (const auto &clause : applicable) {
            auto path = project_path(clause);
            if (attach_decision(path, value)) branches.at(value).add(std::move(path));
          }
        }
        if (branches != PresenceBranches{}) state.selections.booleans.emplace(place, *state.selections.compact(std::move(branches)));
      }
    }
    std::set<EvidenceOwner> owners;
    for (const auto &[name, view] : state.views) {
      if (view.owner) owners.insert(*view.owner);
      owners.insert(view.place_owners.begin(), view.place_owners.end());
    }
    for (const auto &[name, owner] : state.owner_bindings) owners.insert(owner);
    for (const auto &[name, owner] : external_owners_) if (owners.contains(owner)) state.external_owners.insert(owner);
    for (const auto owner : owners) for (const auto other : owners)
      state.owner_relations[{owner, other}] = compare_owners(owner, other);
    return state;
  }
  size_t retain_query_environment(const CheckedMethod &method, const std::optional<ResultContract> &result = {},
                                  const TypeEvidenceState *declaration = nullptr, const std::vector<size_t> &scope_owners = {}) {
    const auto &symbols = free_bindings(*method.source).symbols;
    if (method.queries.empty() && symbols.empty() && !result) return 0;
    QueryEnvironment environment;
    environment.result = result;
    const auto state = declaration ? *declaration : evidence_state();
    if (lexical_methods_.contains(method.source))
      for (const auto &symbol : symbols)
        if (symbol_shadowed(symbol.name, state)) environment.symbol_shadows.insert(symbol.name);
    std::vector<EvidenceOwner> owners;
    const auto retained_owner = [&](EvidenceOwner owner) {
      const auto prior = std::find(owners.begin(), owners.end(), owner);
      const auto id = std::distance(owners.begin(), prior);
      if (prior == owners.end()) owners.push_back(owner);
      return EvidenceOwner::retained(id);
    };
    for (const auto &query : method.queries) {
      check_query_binding(query.name, query.at, true);
      const auto found = state.bindings.find(query.name);
      if (found == state.bindings.end() && lexical_self(query.name, state.method)) {
        const auto type = callable_query({*state.method}, nullptr, query.at, state);
        if (!type) implementation_gap(query.at, "typeof requires a known lexical declaration with a proved result contract");
        environment.bindings.push_back({query.name, *type, {false, false, {}}});
        continue;
      }
      if (found == state.bindings.end() || !found->second)
        implementation_gap(query.at, "typeof requires a known lexical declaration with a proved result contract");
      auto view = state.views.at(query.name);
      view.map_owners(retained_owner);
      std::optional<EvidenceOwner> owner;
      if (const auto named = state.owner_bindings.find(query.name); named != state.owner_bindings.end())
        owner = retained_owner(named->second);
      environment.bindings.push_back({query.name, *found->second, view, owner});
    }
    for (const auto owner : owners) {
      std::vector<OwnerMatch> relations;
      for (const auto other : owners) relations.push_back(compare_owners(owner, other));
      environment.owners.push_back(std::move(relations));
      if (!scope_owners.empty()) {
        auto &incoming = environment.scope_owners.emplace_back();
        for (const auto other : scope_owners) incoming.push_back(compare_owners(owner, other));
      }
    }
    const auto found = std::find(program_.query_environments.begin(), program_.query_environments.end(), environment);
    const auto id = std::distance(program_.query_environments.begin(), found);
    if (found == program_.query_environments.end()) program_.query_environments.push_back(std::move(environment));
    return id;
  }
  std::vector<ArgumentEvidence> argument_evidence(const Expression &expr, TypeEvidenceState state, size_t first = 0) const {
    std::vector<ArgumentEvidence> result(expr.children.size());
    for (size_t i = first; i < expr.children.size(); ++i) {
      result[i].type = type_evidence(*expr.children[i], state, true, nullptr, nullptr, ExpectedTypeUse::callable_result);
      result[i].read_only = state.value.read_only;
      result[i].owner = state.value.owner;
    }
    return result;
  }
  std::vector<ArgumentEvidence> argument_evidence(const Expression &expr) const {
    if (expr.intrinsic || named_family(expr.text).methods.empty()) return std::vector<ArgumentEvidence>(expr.children.size());
    return argument_evidence(expr, evidence_state());
  }
  struct CallableSelection {
    std::vector<Type> retained;
    Type type;
    std::optional<ResultContract> definition_result;
  };
  static ResultContract expected_result(const CheckedMethod &method, const Type &expected, const Location &at) {
    ResultContract result{*expected.target};
    if (expected.value_result) return result;
    const auto role = expected.owners.empty() ? 0 : expected.owners.back();
    if (!role) implementation_gap(at, "expected borrowed result requires an independently established owner");
    for (size_t i = 0; i < expected.parameters.size(); ++i) if (expected.owners[i] == role) {
      const auto &owner = method.input_parameters()[i].owner;
      if (owner.empty() || (!result.owner.empty() && result.owner != owner))
        implementation_gap(at, "expected result owner does not map to one definition owner");
      result.owner = owner;
    }
    if (result.owner.empty()) implementation_gap(at, "expected result owner is not established by a parameter");
    result.mode = !expected.nullable_views.empty() && expected.nullable_views.back() ? ResultMode::borrowed_ptr : ResultMode::borrowed_ref;
    result.read_only = !expected.const_views.empty() && expected.const_views.back();
    return result;
  }
  using InputDomains = std::vector<std::vector<Type>>;
  static InputDomains input_domains(const Type &view, const Location &at) {
    signature_type(CheckedMethod{}, view, at);
    // Cover every admitted input, independently of observed invocations.
    // This finite proof does not define a general family representation.
    InputDomains domains;
    for (size_t i = 0; i < view.parameters.size(); ++i) {
      const auto &parameter = view.parameters[i];
      const auto mode = argument_mode(view, i);
      std::vector<Type> domain;
      if (mode != ArgumentMode::value) {
        domain.push_back(parameter);
      } else if (scalar_type(parameter)) {
        for (const auto kind : {Type::boolean, Type::uint8, Type::uint32, Type::uint64, Type::integer, Type::float32, Type::float64})
          if (transfers_value(kind, parameter)) domain.push_back(kind);
      } else if (parameter.kind == Type::exception) {
        for (const auto category : exception_possibilities(parameter)) domain.push_back(exception_type(category));
      } else if (parameter.kind == Type::procedure) domain.push_back(parameter);
      else implementation_gap(at, "unproved callable family input domain");
      domains.push_back(std::move(domain));
    }
    return domains;
  }
  struct InputPlan {
    InputDomains domains;
    std::optional<CallableSelectionInputs> selection_inputs = {};
    std::optional<std::pair<size_t, Type>> widening_example = {};
  };
  template <typename Validate>
  static void cover_inputs(const Type &view, const Location &at, const InputPlan &plan, const Validate &validate) {
    std::vector<Type> arguments;
    std::vector<size_t> varying;
    arguments.reserve(plan.domains.size());
    for (size_t i = 0; i < plan.domains.size(); ++i) {
      const auto &domain = plan.domains[i];
      if (domain.empty()) return;
      arguments.push_back(domain.front());
      if (domain.size() > 1) varying.push_back(i);
    }
    size_t obligations = 0;
    const auto account = [&] {
      if (++obligations > family_coverage_limit)
        fail(at, "compiler resource limit: " + std::to_string(family_coverage_limit) +
            " callable family coverage steps while covering '" + type_name(view) + "'; coverage remains unproved");
    };
    // Fixed coordinates are present in each checked tuple. Only alternatives
    // require traversal, so signature width does not consume search depth.
    std::vector<size_t> next(varying.size());
    size_t depth = 0;
    account();
    for (;;) {
      if (depth == varying.size()) {
        if (const auto &example = plan.widening_example) {
          // The admitted narrower types precede the exact type in the domain.
          auto widened = arguments;
          widened.at(example->first) = example->second;
          validate(widened);
          account();
        }
        validate(arguments);
      } else {
        const auto input = varying[depth];
        if (next[depth] < plan.domains[input].size()) {
          arguments[input] = plan.domains[input][next[depth]++];
          account();
          ++depth;
          continue;
        }
        next[depth] = 0;
      }
      if (depth == 0) return;
      --depth;
    }
  }
  static const Type &coverage_formal(const CheckedMethod &method, size_t index, const Substitutions &context) {
    const auto &type = method.parameters[index];
    if (const auto binding = binder(method, type); binding && context.at(*binding)) return *context.at(*binding);
    return type;
  }
  static bool disjoint_input_domain(const CheckedMethod &method, const InputDomains &domains,
                                    const std::vector<Type> &retained) {
    if (!accepts_arity(method, domains.size())) return true;
    if (method.retained_types.size() != retained.size()) return false;
    auto context = type_context(method, retained);
    for (size_t i = 0; i < domains.size(); ++i) {
      const auto &domain = domains[i];
      if (domain.empty() || !scalar_type(domain.front())) continue;
      const auto binding = binder(method, method.parameters[parameter_index(method, i)]);
      if (binding && !context.at(*binding)) continue;
      // Scalar inputs cannot bind nested parameters. An already bound formal
      // can match only without changing its substitution.
      if (std::none_of(domain.begin(), domain.end(), [&](const auto &actual) {
        return bind_parameter(method, i, actual, context, true);
      })) return true;
    }
    return false;
  }
  static std::optional<Substitutions> factored_coverage_context(const CheckedMethod &method, const Type &view,
      const InputDomains &domains, const std::vector<Type> &retained) {
    if (variadic(method) || method.retained_types.size() != retained.size() ||
        caller_bindings(method) || method.parameters.size() != domains.size())
      return {};
    auto context = type_context(method, retained);
    for (size_t i = 0; i < domains.size(); ++i) {
      const auto mode = argument_mode(view, i);
      if (argument_mode(parameter_at(method, i)) != mode || domains[i].empty()) return {};
      // Complete callable signatures and borrowed target types stay exact.
      // Whole-tuple checking still binds their types and proves their owners.
      if (view.parameters[i].kind == Type::procedure) continue;
      if (mode != ArgumentMode::value) {
        if (!scalar_type(view.parameters[i])) return {};
        continue;
      }
      const auto binding = binder(method, method.parameters[i]);
      const bool unresolved = binding && !context.at(*binding);
      if ((!unresolved && !scalar_type(coverage_formal(method, i, context))) ||
          std::any_of(domains[i].begin(), domains[i].end(), [](const auto &actual) { return !scalar_type(actual); }))
        return {};
    }
    return context;
  }
  static bool fixed_preparation(const CheckedMethod &method, const Type &view, const InputDomains &domains,
                                const std::vector<Type> &retained = {}) {
    // Each input prepares independently to the same fixed formal.
    // Declaration evidence is fixed per materialization across these inputs.
    const auto context = factored_coverage_context(method, view, domains, retained);
    if (!context || std::any_of(context->begin(), context->end(), [](const auto &type) { return !type; })) return false;
    for (size_t i = 0; i < domains.size(); ++i) for (const auto &actual : domains[i]) {
      auto substitutions = *context;
      if (!bind_parameter(method, i, actual, substitutions, true)) return false;
    }
    return true;
  }
  bool fixed_method_preparation(const CheckedMethod &method, const Type &view, const InputDomains &domains,
                                const std::vector<Type> &retained = {}) const {
    if (!fixed_preparation(method, view, domains, retained)) return false;
    if (method.family && !lexical_methods_.contains(method.source))
      for (const auto &declaration : program_.families[*method.family].declarations)
        if (!fixed_preparation(declaration, view, domains)) return false;
    return true;
  }
  InputPlan plan_inputs(const Type &view, const InputDomains &domains, std::vector<const CheckedMethod *> contracts,
                        const std::vector<Type> &retained = {}) const {
    InputPlan plan{domains};
    const auto methods = contracts.size();
    std::set<size_t> declarations;
    for (size_t i = 0; i < methods; ++i) {
      const auto &method = *contracts[i];
      if (method.family && !lexical_methods_.contains(method.source) && declarations.insert(*method.family).second)
        for (const auto &declaration : program_.families[*method.family].declarations) contracts.push_back(&declaration);
    }
    // Remove only proven non-overlaps from the projection proof. The
    // coverage callbacks still select and check against the original family.
    std::erase_if(contracts, [&](const auto *contract) { return disjoint_input_domain(*contract, domains, retained); });
    if (contracts.empty()) return plan;
    std::vector<Substitutions> contexts;
    for (const auto *contract : contracts) {
      auto context = factored_coverage_context(*contract, view, domains, retained);
      if (!context) return plan;
      contexts.push_back(std::move(*context));
    }
    // Owner and access requirements can affect dominance when more candidates
    // enter the lossless phase. Generic candidates can distinguish it too.
    const bool phase_sensitive = !view.owners.empty() ||
        std::any_of(contracts.begin(), contracts.end(), [&](const auto *contract) {
          return !contract->type_parameters.empty() || !same_access(*contract, *contracts.front());
        });
    CallableSelectionInputs selection;
    bool always_widens = false;
    for (size_t i = 0; i < domains.size(); ++i) {
      if (argument_mode(view, i) != ArgumentMode::value || view.parameters[i].kind == Type::procedure) continue;
      const auto &formal = coverage_formal(*contracts.front(), i, contexts.front());
      bool same = scalar_type(formal);
      for (size_t j = 1; same && j < contracts.size(); ++j)
        same = coverage_formal(*contracts[j], i, contexts[j]) == formal;
      const bool prepares = same && std::all_of(domains[i].begin(), domains[i].end(), [&](const auto &actual) {
        auto substitutions = contexts.front();
        return bind_parameter(*contracts.front(), i, actual, substitutions, true);
      });
      if (!prepares) {
        selection.positions.push_back(i);
        continue;
      }
      plan.domains[i] = {view.parameters[i]};
      if (!phase_sensitive) continue;
      if (formal != view.parameters[i]) {
        always_widens = true;
      } else if (const auto narrower = std::find_if(domains[i].begin(), domains[i].end(),
          [&](const auto &actual) { return actual != formal; }); narrower != domains[i].end()) {
        selection.widening_positions.push_back(i);
        if (!plan.widening_example) plan.widening_example = std::pair{i, *narrower};
      }
    }
    // One witness covers any widening at a shared coordinate. It rules out
    // exact matching for every candidate without affecting their substitutions.
    if (always_widens) {
      selection.widening_positions.clear();
      plan.widening_example.reset();
    }
    if (selection.positions.size() != domains.size()) plan.selection_inputs = std::move(selection);
    return plan;
  }
  struct CallableInputEvidence {
    Access access;
    Owners owners;
    TypeEvidenceState proof;
  };
  static CallableInputEvidence callable_input_evidence(const Type &view) {
    CallableInputEvidence evidence;
    for (size_t i = 0; i < view.parameters.size(); ++i) {
      // The view fixes target access. Distinct formal owner roles remain
      // symbolic: they can denote equal or different actual owners.
      evidence.access.push_back(!view.const_views.empty() && view.const_views[i]);
      const auto role = view.owners.empty() ? 0 : view.owners[i];
      evidence.owners.push_back(role ? std::optional<size_t>{role} : std::nullopt);
      if (role) evidence.proof.external_owners.insert(role);
    }
    return evidence;
  }
  template <typename Validate>
  std::optional<CallableSelectionInputs> cover_family(const std::vector<size_t> &family, const Type &view, const Location &at, const Validate &validate,
                    const std::vector<Type> &retained = {}) const {
    const auto evidence = callable_input_evidence(view);
    const auto domains = input_domains(view, at);
    const auto check = [&](const std::vector<Type> &arguments) {
      validate(select_method(family, arguments, at, "callable family", retained, evidence.access, evidence.owners, &evidence.proof), arguments);
    };
    if (family.size() == 1 && fixed_method_preparation(program_.methods[family.front()], view, domains, retained)) {
      check(view.parameters);
      return CallableSelectionInputs{};
    }
    std::vector<const CheckedMethod *> contracts;
    for (const auto id : family) contracts.push_back(&program_.methods[id]);
    const auto plan = plan_inputs(view, domains, std::move(contracts), retained);
    cover_inputs(view, at, plan, check);
    return plan.selection_inputs;
  }
  Type complete_callable_type(const CheckedMethod &method, const std::vector<Type> &arguments, const ResultContract &result) const {
    auto type = callable_type(method, arguments, result);
    if (method.family && !program_.families[*method.family].declarations.empty()) {
      const auto domains = input_domains(type, method.source->at);
      if (fixed_method_preparation(method, type, domains)) add_declared_failures(type, method, type.parameters);
      else {
        const auto plan = plan_inputs(type, domains, {&method});
        cover_inputs(type, method.source->at, plan,
            [&](const std::vector<Type> &inputs) { add_declared_failures(type, method, inputs); });
      }
    }
    return resolve_type(std::move(type));
  }
  static bool satisfies_callable_view(Type contract, const Type &view, const Location &at) {
    std::set<size_t> allowed;
    for (const auto &failure : view.throws_types) {
      const auto categories = exception_possibilities(failure); allowed.insert(categories.begin(), categories.end());
    }
    for (const auto &failure : contract.throws_types) for (const auto category : exception_possibilities(failure))
      if (exception_categories[category].checked && !allowed.contains(category))
        fail(at, "callable view omits a required checked exception");
    contract.throws_types = view.throws_types;
    // Semantic selection precedes value preparation; borrowed targets remain exact.
    contract.parameters = view.parameters;
    if (contract.value_result && view.value_result && transfers_value(*contract.target, *view.target))
      contract.target = view.target;
    return contract == view;
  }
  void check_family_contract(const CheckedMethod &method, const std::vector<Type> &actual,
      const std::vector<Type> &prepared, const ResultContract &result, const Type &view, const Location &at) const {
    if (caller_bindings(method))
      implementation_gap(at, "caller-binding method values require an unproved callable slot contract");
    for (size_t i = 0; i < actual.size(); ++i)
      if (!view.argument_modes.empty() && view.argument_modes[i] != ArgumentMode::value && prepared[i] != actual[i])
        implementation_gap(at, "unproved callable family borrowed target adaptation");
    if (!satisfies_callable_view(callable_type(method, prepared, result, {}, {}, &actual), view, at))
      implementation_gap(at, "selected method does not satisfy the callable view contract");
  }
  bool concrete_callable_result(const CheckedMethod &method, const ResultContract &result, const Location &at) const {
    if (!nested_binder(method, *result.type)) return true;
    if (result_probe_) return false;
    implementation_gap(at, "type parameter is not determined by the arguments");
  }
  std::vector<Type> definition_retained(size_t id, const TypeEvidenceState &state) const {
    const auto &definition = program_.methods[id];
    if (state.method == id) return state.retained;
    std::vector<Type> retained;
    if (!definition.retained_types.empty()) {
      if (definition.parent != lexical_template(state.method))
        implementation_gap(definition.source->at, "callable definition lacks its enclosing type context");
      for (const auto index : definition.retained_types) {
        const auto &type = state.substitutions.at(index);
        if (!type) implementation_gap(definition.source->at, "enclosing type parameter is not determined by the arguments");
        retained.push_back(*type);
      }
    }
    return retained;
  }
  std::optional<CallableSelection> try_select_callable(const std::vector<size_t> &family, const Type *expected, const Location &at,
                                                        const TypeEvidenceState *state = nullptr, bool definition_site = false) const {
    if (family.empty()) fail(at, "no applicable implementation for declared callable value");
    if (std::any_of(family.begin(), family.end(), [&](size_t id) { return caller_bindings(program_.methods[id]); }))
      implementation_gap(at, "caller-binding method values require an unproved callable slot contract");
    const auto &definition = program_.methods[family.front()];
    if (expected) signature_type(CheckedMethod{}, *expected, at);
    if (!expected && (family.size() != 1 || !program_.methods[family.front()].source->type_parameters.empty()))
      implementation_gap(at, "overloaded or generic callable values require an explicit concrete view type");
    std::vector<Type> retained;
    const auto parent = state ? state->method : checking_instance_ ? std::optional<size_t>{checking_instance_->method} : std::nullopt;
    if (parent == family.front()) retained = state ? state->retained : checking_instance_->retained_types;
    else if (!definition.retained_types.empty()) {
      if (!parent || definition.parent != lexical_template(parent)) return {};
      const auto &substitutions = state ? state->substitutions : checking_instance_->substitutions;
      for (const auto index : definition.retained_types) {
        const auto &type = substitutions.at(index);
        if (!type) implementation_gap(at, "enclosing type parameter is not determined by the arguments");
        retained.push_back(*type);
      }
    }
    auto arguments = expected ? expected->parameters : definition.parameters;
    if (!expected) for (auto &type : arguments) type = substitute(type, definition, type_context(definition, retained));
    Access access;
    if (expected) for (size_t i = 0; i < expected->parameters.size(); ++i)
      access.push_back(!expected->const_views.empty() && expected->const_views[i]);
    const auto selected = select_method(family, arguments, at, "callable value", retained, access, {}, state);
    const auto &method = program_.methods[selected];
    const bool contextual_result = definition_site && expected && !method.result.type && method.source->name.empty();
    const auto supplied = contextual_result ? std::optional<ResultContract>{expected_result(method, *expected, at)} : std::nullopt;
    const auto result = supplied ? supplied : infer_result(selected, arguments, retained, state).contract;
    if (!result) return {};
    if (!concrete_callable_result(method, *result, at)) return {};
    auto type = expected ? callable_type(method, prepared_types(method, arguments, retained), *result, {}, {}, &arguments)
                         : complete_callable_type(method, arguments, *result);
    signature_type(definition, type, at);
    if (expected) {
      if (!satisfies_callable_view(type, *expected, at))
        fail(at, "selected method does not satisfy the callable view contract");
      type = *expected;
    }
    return CallableSelection{std::move(retained), std::move(type), supplied};
  }
  CallableSelection select_callable(const std::vector<size_t> &family, const Type *expected, const Location &at, bool definition_site = false,
                                    const TypeEvidenceState *state = nullptr) const {
    if (const auto selected = try_select_callable(family, expected, at, state, definition_site)) return *selected;
    implementation_gap(at, "unproved callable result; provide an independent result contract");
  }
  struct InspectedDeclaration {
    Type type;
    ResultMode mode;
    ViewEvidence view;
  };
  template <typename Validate>
  std::optional<InspectedDeclaration> inspect_declarations(const std::string &name, const std::vector<Type> &arguments,
      const Access &access, const Owners &owners, const Location &at, const TypeEvidenceState &state, const Validate &validate) const {
    std::optional<InspectedDeclaration> result;
    for (const auto &declaration : declarations(name)) {
      const auto substitutions = declaration_match(declaration, arguments);
      if (!substitutions || !admits_access(declaration, access)) continue;
      const auto equality = match_owners(declaration, owners, &state);
      if (equality == OwnerMatch::mismatch) continue;
      if (equality == OwnerMatch::unknown) {
        if (state.required) implementation_gap(at, "declared signature requires an unproved owner equality");
        return {};
      }
      auto contract = declaration.result;
      contract.type = substitute(*contract.type, declaration, *substitutions, true);
      const auto view = result_view(declaration, contract, owners);
      if (nested_binder(declaration, *contract.type) ||
          (result && (result->type != contract.type || result->view != view || result->mode != contract.mode))) {
        if (state.required) implementation_gap(at, "declarations do not establish one inspected result contract");
        return {};
      }
      validate(declaration, *substitutions, contract);
      result = InspectedDeclaration{*contract.type, contract.mode, view};
    }
    if (state.required && !result) fail(at, "no applicable declared signature for '" + name + "'");
    return result;
  }
  std::optional<Type> declared_callable_query(const std::string &name, const Type *expected, const Location &at,
                                             const TypeEvidenceState &state, const std::vector<size_t> &methods = {}) const {
    std::optional<Type> view;
    if (expected) view = *expected;
    else for (const auto &declaration : declarations(name)) {
      if (variadic(declaration) || !declaration.type_parameters.empty()) return {};
      const auto signature = resolve_type(callable_signature(declaration, declaration.parameters, declaration.result));
      if (view && *view != signature) return {};
      view = signature;
    }
    if (!view || view->kind != Type::procedure) return {};
    auto evidence = callable_input_evidence(*view);
    evidence.proof.required = state.required;
    bool resolved = true;
    const auto domains = input_domains(*view, at);
    const auto validate = [&](const std::vector<Type> &arguments) {
      const auto candidates = method_candidates(methods, arguments, {}, evidence.access, evidence.owners, &evidence.proof);
      if (!candidates.methods.empty()) {
        const auto selected = select_method(methods, arguments, at, name, {}, evidence.access, evidence.owners, &evidence.proof);
        if (!inspect_method_contract(selected, arguments, {}, *view, at, state)) resolved = false;
        return;
      }
      const auto result = inspect_declarations(name, arguments, evidence.access, evidence.owners, at, evidence.proof,
          [&](const CheckedMethod &declaration, const Substitutions &substitutions, const ResultContract &contract) {
        std::vector<Type> prepared;
        for (size_t i = 0; i < arguments.size(); ++i) {
          const auto type = substitute(declaration.parameters[parameter_index(declaration, i)], declaration, substitutions);
          if (argument_mode(parameter_at(declaration, i)) != ArgumentMode::value && type != arguments[i])
            implementation_gap(at, "unproved callable family borrowed target adaptation");
          prepared.push_back(type);
        }
        if (!satisfies_callable_view(resolve_type(callable_signature(declaration, prepared, contract)), *view, at))
          implementation_gap(at, "declared signature does not satisfy the callable view contract");
      });
      resolved = resolved && result.has_value();
    };
    const auto contracts = declarations(name);
    const bool fixed_contracts = std::all_of(contracts.begin(), contracts.end(),
        [&](const auto &contract) { return fixed_preparation(contract, *view, domains); });
    const bool fixed_method = methods.size() == 1 && fixed_method_preparation(program_.methods[methods.front()], *view, domains);
    if (fixed_contracts && (fixed_method || (methods.empty() && !contracts.empty()))) validate(view->parameters);
    else {
      std::vector<const CheckedMethod *> candidates;
      for (const auto &contract : contracts) candidates.push_back(&contract);
      for (const auto id : methods) candidates.push_back(&program_.methods[id]);
      const auto plan = plan_inputs(*view, domains, std::move(candidates));
      cover_inputs(*view, at, plan, validate);
    }
    return resolved ? view : std::nullopt;
  }
  bool inspect_method_contract(size_t selected, const std::vector<Type> &arguments, const std::vector<Type> &retained,
      const Type &view, const Location &at, const TypeEvidenceState &state, const std::optional<ResultContract> &supplied = {}) const {
    const auto result = supplied ? supplied : infer_result(selected, arguments, retained, &state).contract;
    if (!result) return false;
    const auto &method = program_.methods[selected];
    if (!concrete_callable_result(method, *result, at)) return false;
    check_family_contract(method, arguments, prepared_types(method, arguments, retained), *result, view, at);
    return true;
  }
  std::optional<Type> callable_query(const std::vector<size_t> &family, const Type *expected, const Location &at,
                                   const TypeEvidenceState &state, bool definition_site = false) const {
    if (expected && !family.empty() && program_.methods[family.front()].family) {
      bool resolved = true;
      cover_family(family, *expected, at, [&](size_t selected, const std::vector<Type> &arguments) {
        if (!inspect_method_contract(selected, arguments, {}, *expected, at, state)) resolved = false;
      });
      return resolved ? std::optional<Type>{*expected} : std::nullopt;
    }
    const auto selection = try_select_callable(family, expected, at, &state, definition_site);
    if (!selection) return {};
    bool resolved = true;
    cover_family(family, selection->type, at, [&](size_t selected, const std::vector<Type> &arguments) {
      if (!inspect_method_contract(selected, arguments, selection->retained, selection->type, at, state, selection->definition_result)) resolved = false;
    }, selection->retained);
    return resolved ? std::optional<Type>{selection->type} : std::nullopt;
  }
  size_t intern_callable(CheckedCallable value) {
    const auto found = std::find_if(program_.callables.begin(), program_.callables.end(), [&](const auto &prior) {
      return prior.family == value.family && prior.type == value.type && prior.dispatch == value.dispatch &&
          prior.uniform_target == value.uniform_target && prior.selection_inputs == value.selection_inputs;
    });
    const auto id = std::distance(program_.callables.begin(), found);
    if (found == program_.callables.end()) program_.callables.push_back(std::move(value));
    return id;
  }
  CheckedOperation materialized_value(size_t id, std::vector<CheckedOperation> captures, const Location &at) {
    std::set<size_t> symbols;
    for (const auto target : program_.callables[id].implementations()) {
      const auto &required = read_effects(target).symbols;
      symbols.insert(required.begin(), required.end());
    }
    if (!symbols.empty() && checking_method_ && checking_method_->source->kind == Form::Kind::function)
      fail(at, "a function cannot capture dynamic bindings");
    for (const auto symbol : symbols) require_symbol(lexical_, symbol, at);
    if (!captures.empty() || !symbols.empty()) note_abrupt(at, {exception_id(ExceptionCategory::out_of_memory)});
    CheckedOperation value; value.type = program_.callables[id].type; value.callable = id; value.callable_captures = std::move(captures);
    return value;
  }
  void warn_captures(const CheckedMethod &definition) {
    if (validating_ && definition.source->explicit_captures && warned_captures_.insert(definition.source).second)
      for (const auto &capture : definition.captures)
        if (std::none_of(free_bindings(*definition.source).captures.begin(), free_bindings(*definition.source).captures.end(),
                         [&](const auto &used) { return used.name == capture.name; }))
          program_.warnings.push_back({Warning::unused_capture, capture.at, "unused callable capture '" + capture.name + "' remains retained"});
  }
  void require_value_capture_source(std::optional<size_t> binding, const Location &at) const {
    if (!binding) return;
    const auto &source = bindings_->at(*binding);
    if (source.alias || !source.selection.empty())
      implementation_gap(at, "capture of a declared place alias requires selected-place lifetime and transport proof");
    if (!source.value_binding && !source.callable_value && !source.definition)
      implementation_gap(at, "capture of a lexical reference requires selected-target lifetime and transport proof");
  }
  void define_callable(size_t method, const Location &at) {
    auto &definition = program_.methods[method];
    resolve_captures(definition);
    auto evidence = std::make_shared<TypeEvidenceState>(evidence_state(false, &definition_dependencies(*definition.source)));
    DefinitionContext context{method, evidence, {}, {}, definition_retained(method, *evidence)};
    for (const auto &capture : definition.captures) {
      const auto binding = lexical_.lookup(capture.name);
      require_value_capture_source(binding, capture.at);
      const auto family = binding ? NamedFamily{} : named_family(capture.name);
      context.captures.push_back({binding, family.methods, family.context != nullptr});
      Expression name{}; name.kind = Expression::Kind::name; name.at = capture.at; name.text = capture.name;
      auto state = *evidence;
      const auto type = type_evidence(name, state);
      if (!type) implementation_gap(capture.at, "callable capture requires a proved concrete view type");
      if (!lexical_value_type(*type))
        implementation_gap(capture.at, "capture requires an admitted independent value contract");
      const auto access = evidence->views.find(capture.name);
      context.environment.push_back({*type, access != evidence->views.end() && access->second.read_only.value_or(false)});
    }
    context.query = retain_query_environment(definition, {}, evidence.get());
    warn_captures(definition);
    const auto id = bind(definition.source->name, Type::void_type, {}, true, true, true, at);
    bindings_->at(id).definition = method;
    const auto &bound = definitions_.emplace(id, std::move(context)).first->second;
    // Body checking is independent of reading captures or constructing a value.
    if (definition.source->type_parameters.empty()) {
      const auto selection = select_callable({method}, nullptr, at, false, evidence.get());
      cover_family({method}, selection.type, at, [&](size_t selected, const std::vector<Type> &arguments) {
        demand(selected, arguments, at, bound.environment, bound.retained, bound.query, evidence.get());
      }, bound.retained);
    }
  }
  std::vector<CheckedOperation> capture_values(const NamedFamily &family, const Location &at) {
    const auto &definition = program_.methods[family.methods.front()];
    std::vector<CheckedOperation> captures;
    for (size_t i = 0; i < definition.captures.size(); ++i) {
      const auto &capture = definition.captures[i];
      Expression name{}; name.kind = Expression::Kind::name; name.at = at; name.text = capture.name;
      CheckedOperation value;
      if (family.context) {
        const auto binding = storage(family.context->frame.captures.at(i)).binding;
        require_value_capture_source(binding, capture.at);
        value = read_binding(name, true, binding);
      } else if (family.definition) {
        const auto &source = family.definition->captures[i];
        if (source.binding) {
          require_value_capture_source(source.binding, capture.at);
          value = named_binding(name, *source.binding);
        }
        else value = materialize_definition(NamedFamily{source.methods, source.enclosing ? checking_instance_ : nullptr}, nullptr, at);
      } else {
        require_value_capture_source(lexical_.lookup(capture.name), capture.at);
        value = named_value(name);
      }
      if (!lexical_value_type(value.type))
        implementation_gap(capture.at, "capture requires an admitted independent value contract");
      captures.push_back(std::move(value));
    }
    return captures;
  }
  CheckedOperation materialize_definition(const NamedFamily &family, const Type *expected, const Location &at, bool definition_site = false) {
    if (family.methods.empty()) fail(at, "no applicable implementation for declared callable value");
    if (std::ranges::any_of(family.methods, [&](size_t id) { return program_.methods[id].here.has_value(); }))
      implementation_gap(at, "function views containing a here parameter");
    auto &definition = program_.methods[family.methods.front()];
    if (lexical_methods_.contains(definition.source)) resolve_captures(definition);
    if (definition.family) {
      const auto view = expected ? *expected : select_callable(family.methods, nullptr, at).type;
      return materialize_family(*definition.family, view, at);
    }
    const auto &dependencies = definition_dependencies(*definition.source);
    const auto state = family.declaration ? *family.declaration : evidence_state(family.context != nullptr, &dependencies);
    const auto selection = select_callable(family.methods, expected, at, definition_site, &state);
    auto captures = capture_values(family, at);
    std::vector<CheckedCaptureInput> environment;
    for (const auto &capture : captures) environment.push_back({capture.type, capture.read_only});
    warn_captures(definition);
    CheckedCallable value{selection.type};
    const auto query = family.context ? family.context->query_environment : family.definition ? family.definition->query
        : retain_query_environment(definition, selection.definition_result, &state);
    const auto selection_inputs = cover_family(family.methods, selection.type, at, [&](size_t selected, const std::vector<Type> &arguments) {
      const auto target = demand(selected, arguments, at, environment, selection.retained, query, &state);
      const auto &instance = program_.instances[target];
      check_family_contract(program_.methods[selected], arguments, instance.arguments, instance.result, selection.type, at);
      value.dispatch.emplace_back(arguments, target);
    }, selection.retained);
    if (selection_inputs && selection_inputs->uniform()) {
      value.uniform_target = value.dispatch.front().second; value.dispatch.clear();
    } else value.selection_inputs = selection_inputs;
    return materialized_value(intern_callable(std::move(value)), std::move(captures), at);
  }
  CheckedOperation materialize(const std::vector<size_t> &family, const Type *expected, const Location &at, bool definition_site = false) {
    return materialize_definition(NamedFamily{family}, expected, at, definition_site);
  }
  CheckedOperation materialize_symbol(size_t symbol, const Type &view, const Location &at) {
    if (view.kind != Type::procedure || view.pure_callable)
      fail(at, "a symbolic procedure family requires a procedure view");
    const auto selected = module_names().families.find(symbol);
    if (selected == module_names().families.end()) fail(at, "callable family has no applicable implementation");
    return materialize_family(selected->second, view, at);
  }
  CheckedOperation materialize_family(size_t family_id, const Type &view, const Location &at) {
    const auto &family = program_.families[family_id].methods;
    if (family.empty()) fail(at, "callable family has no applicable implementation");
    const auto found = std::find_if(program_.callables.begin(), program_.callables.end(), [&](const auto &value) {
      return value.family == family_id && value.type == view;
    });
    const auto id = std::distance(program_.callables.begin(), found);
    if (found == program_.callables.end()) {
      CheckedCallable value{view, family_id};
      const auto selection_inputs = cover_family(family, view, at, [&](size_t selected, const std::vector<Type> &arguments) {
        const auto target = demand(selected, arguments, at);
        const auto &instance = program_.instances[target];
        check_family_contract(program_.methods[selected], arguments, instance.arguments, instance.result, view, at);
        value.dispatch.emplace_back(arguments, target);
      });
      if (selection_inputs && selection_inputs->uniform()) {
        value.uniform_target = value.dispatch.front().second; value.dispatch.clear();
      } else value.selection_inputs = selection_inputs;
      program_.callables.push_back(std::move(value));
    }
    return materialized_value(id, {}, at);
  }
  const Expression &scope_operand(size_t method, const Capture &capture) {
    const auto key = std::pair{method, capture.name};
    if (const auto found = scope_operand_names_.find(key); found != scope_operand_names_.end()) return *found->second;
    auto operand = std::make_unique<Expression>();
    operand->kind = Expression::Kind::name; operand->at = capture.at; operand->text = capture.name;
    const auto &stored = *program_.scope_operands.emplace_back(std::move(operand));
    scope_operand_names_.emplace(key, &stored);
    return stored;
  }
  size_t scope_profile(CheckedMethod::ScopeInputs inputs, ResultContract result = {}, std::vector<Type> failures = {},
                       std::optional<CheckedMethod::HereGroupResults> group = {}) {
    auto &profiles = scope_profiles_[inputs.lexical_method];
    std::set<size_t> throws_categories;
    for (const auto &failure : failures) {
      const auto categories = exception_possibilities(failure);
      throws_categories.insert(categories.begin(), categories.end());
    }
    for (const auto id : profiles) {
      const auto &method = program_.methods[id];
      if (method.result != result || method.throws_categories != throws_categories || method.here_group != group) continue;
      const auto &candidate = *method.scope_inputs;
      if (candidate.owner_names != inputs.owner_names || candidate.relations != inputs.relations ||
          candidate.here_input_owners != inputs.here_input_owners || candidate.here_snapshots != inputs.here_snapshots ||
          candidate.nonnull != inputs.nonnull || candidate.live_values != inputs.live_values ||
          candidate.live_bindings != inputs.live_bindings ||
          candidate.place_access != inputs.place_access || candidate.place_owners != inputs.place_owners ||
          candidate.overlapping_input_bindings != inputs.overlapping_input_bindings ||
          candidate.boolean_conditions != inputs.boolean_conditions ||
          candidate.reference_conditions != inputs.reference_conditions ||
          candidate.disjoint_input_targets != inputs.disjoint_input_targets ||
          candidate.parameters.size() != inputs.parameters.size()) continue;
      if (std::equal(candidate.parameters.begin(), candidate.parameters.end(), inputs.parameters.begin(),
          [](const auto &a, const auto &b) {
            return a.name == b.name && a.owner == b.owner && a.declaration_kind == b.declaration_kind &&
                a.fixed_binding == b.fixed_binding && a.read_only == b.read_only;
          })) return id;
    }
    auto method = program_.methods[inputs.lexical_method];
    method.instances.clear(); method.captures.clear();
    method.result = std::move(result);
    method.throws_categories = std::move(throws_categories);
    method.throws_types = std::move(failures);
    method.here_group = std::move(group);
    for (auto &parameter : inputs.parameters) {
      const auto name = "$scope.input." + std::to_string(method.parameters.size());
      parameter.type = Type{Type::named, name};
      method.parameters.push_back(parameter.type);
      method.type_parameters.push_back(name);
    }
    method.scope_inputs = std::move(inputs);
    const auto id = program_.methods.size();
    program_.methods.push_back(std::move(method));
    profiles.push_back(id);
    return id;
  }
  size_t scope_owner_role(CheckedMethod::ScopeInputs &inputs, std::vector<size_t> &owners, size_t actual, const Location &at) const {
    if (lexical_.live && !owner_alive(actual)) fail(at, "full-scope input owner has ended");
    const auto found = std::ranges::find(owners, actual);
    const auto role = std::distance(owners.begin(), found);
    if (found == owners.end()) {
      owners.push_back(actual);
      inputs.owners.push_back({"$scope.owner." + std::to_string(role), {}, at});
    }
    return role;
  }
  void scope_owner_relations(CheckedMethod::ScopeInputs &inputs, const std::vector<size_t> &owners) const {
    for (const auto a : owners) {
      auto &relations = inputs.relations.emplace_back();
      for (const auto b : owners) relations.push_back(compare_owners(a, b));
    }
  }
  static std::set<std::set<size_t>> selection_groups(const std::map<size_t, size_t> &selected,
                                                     const CleanupEquations::ConditionClauses &conditions) {
    std::set<std::set<size_t>> groups;
    if (selected.size() < 2) return groups;
    for (auto left = selected.begin(); left != selected.end(); ++left)
      for (auto right = std::next(left); right != selected.end(); ++right)
        if (left->second == right->second || (!conditions.empty() && std::ranges::all_of(conditions, [&](const auto &facts) {
              return facts.same_selection(left->second, right->second);
            }))) groups.insert({left->first, right->first});
    CleanupEquations::ConditionFacts::close_equalities(groups);
    return groups;
  }
  void scope_input_observations(CheckedMethod::ScopeInputs &inputs, const std::vector<CheckedOperation> &views,
                                const Location &at) {
    for (const auto left : inputs.live_bindings) {
      const auto possible = overlapping_slots(views[left].slots);
      for (const auto right : inputs.live_bindings)
        if (right > left && !intersect(possible, physical_slots(views[right].slots)).empty())
          inputs.overlapping_input_bindings.emplace(left, right);
    }
    // Deferred selection bounds are complete after target discovery.
    if (proving_ || !delayed_) for (size_t i = 0; i < views.size(); ++i) {
      const auto &a = views[i];
      if (inputs.parameters[i].owner.empty() || a.targets.empty()) continue;
      for (size_t j = i + 1; j < views.size(); ++j) {
        const auto &b = views[j];
        if (inputs.parameters[j].owner.empty() || b.targets.empty() || a.type != b.type ||
            compare_owners(*a.owners.begin(), *b.owners.begin()) == OwnerMatch::mismatch) continue;
        const bool may_share = std::ranges::any_of(a.targets, [&](size_t left) {
          return std::ranges::any_of(b.targets, [&](size_t right) {
            return payload_targets_may_share(left, right);
          });
        });
        if (!may_share) inputs.disjoint_input_targets.emplace(i, j);
      }
    }
    std::map<size_t, CleanupEquations::BindingSnapshot> selections;
    std::map<size_t, size_t> identities;
    for (size_t p = 0; p < views.size(); ++p) {
      if (inputs.parameters[p].declaration_kind == "ptr" && !inputs.nonnull[p] && views[p].binding)
        selections.emplace(p, CleanupEquations::BindingSnapshot{*views[p].binding, views[p].selection_versions});
      if (!inputs.parameters[p].owner.empty() && views[p].selection_identity)
        identities.emplace(p, *views[p].selection_identity);
    }
    const auto conditions = path_conditions(lexical_);
    CleanupEquations::ConditionFacts captured;
    captured.same_inputs = selection_groups(identities, conditions);
    if (!selections.empty()) for (const auto &clause : conditions) {
      auto facts = captured;
      for (const auto &[p, selected] : selections) {
        if (clause.bindings.contains(selected)) facts.present.insert(p);
        if (clause.absent_bindings.contains(selected)) facts.absent.insert(p);
      }
      CleanupEquations::add_condition(inputs.reference_conditions, std::move(facts));
    }
    if (inputs.reference_conditions.empty() && !captured.same_inputs.empty())
      inputs.reference_conditions.insert(std::move(captured));
    if (inputs.reference_conditions == CleanupEquations::ConditionClauses{CleanupEquations::ConditionFacts{}})
      inputs.reference_conditions.clear();
    if (selections.empty()) return;
    std::map<size_t, std::shared_ptr<const Predicate>> booleans;
    std::map<size_t, std::vector<std::pair<size_t, bool>>> captured_inputs;
    for (size_t i = 0; i < views.size(); ++i) {
      if (views[i].type != Type::boolean) continue;
      const auto predicate = inputs.parameters[i].owner.empty() ? predicate_of(views[i]) : stored_predicate(views[i]);
      if (!predicate) continue;
      booleans.emplace(i, predicate);
      if (const auto decision = boolean_decision(predicate.get()); decision && decision->origin == BooleanDecision::Origin::input)
        captured_inputs[decision->id].emplace_back(i, decision->value_when_true);
    }
    const auto no_conditions = CleanupEquations::ConditionClauses{CleanupEquations::ConditionFacts{}};
    for (const auto &[input, predicate] : booleans) {
      CleanupEquations::BooleanValue proof;
      for (const auto &[truth, clauses] : snapshot_boolean(predicate, lexical_, at)) for (const auto &clause : clauses) {
        CleanupEquations::ConditionFacts facts;
        for (const auto &[other, value] : clause.inputs) if (const auto captured = captured_inputs.find(other); captured != captured_inputs.end())
          for (const auto &[ordinal, when_true] : captured->second) if (ordinal != input)
            facts.inputs.emplace(ordinal, value == when_true);
        for (const auto &[ordinal, selected] : selections) {
          if (clause.bindings.contains(selected)) facts.present.insert(ordinal);
          if (clause.absent_bindings.contains(selected)) facts.absent.insert(ordinal);
        }
        CleanupEquations::add_condition(proof[truth], std::move(facts));
      }
      if (std::ranges::any_of(proof, [&](const auto &outcome) { return outcome.second != no_conditions; }))
        inputs.boolean_conditions.emplace(input, std::move(proof));
    }
  }
  void check_scope_call(const Expression &expr, size_t method_id, CheckedInstance *instance,
      std::map<const Expression *, CheckedOperation> &operations, CheckedOperation &operation, bool read, bool discard,
      const Type *expected) {
    auto &method = program_.methods[method_id];
    resolve_captures(method);
    const auto &dependencies = definition_dependencies(*method.source);
    const auto state = evidence_state(false, &dependencies);
    const auto retained = definition_retained(method_id, state);
    CheckedMethod::ScopeInputs inputs{};
    inputs.lexical_method = method_id;
    operation.argument_sources.emplace();
    const auto owner_role = [&](size_t actual, const Location &at) {
      return scope_owner_role(inputs, operation.scope_owners, actual, at);
    };
    std::vector<Type> arguments;
    std::vector<CheckedOperation> views;
    std::map<size_t, size_t> place_inputs;
    for (const auto &capture : method.captures) {
      const auto binding = lexical_.lookup(capture.name);
      const auto *source = binding ? &bindings_->at(*binding) : nullptr;
      if (source && source->owner_binding) {
        inputs.owner_names.emplace(capture.name, owner_role(named_owner(capture.name, capture.at), capture.at));
        continue;
      }
      const bool independent = !source || source->definition;
      const bool live_value = !independent && (source->value_binding || source->callable_value);
      const bool live_binding = !independent && !live_value;
      const bool declared_place = !independent && (source->alias || !source->selection.empty());
      Parameter parameter; parameter.name = capture.name; parameter.at = capture.at;
      parameter.declaration_kind = independent ? "val" : !live_value && source->requires_initialized ? "ref" : "ptr";
      const auto &operand = scope_operand(method_id, capture);
      const auto type = check(operand, instance, operations, independent);
      auto view = operations.at(&operand);
      if (!lexical_value_type(type))
        implementation_gap(capture.at, "full-scope input requires an admitted target or independent value type");
      parameter.read_only = view.read_only;
      if (!independent) {
        const auto [input, inserted] = place_inputs.try_emplace(*view.binding, arguments.size());
        const auto ordinal = input->second;
        if (declared_place)
          inputs.place_access.emplace(capture.name, CheckedMethod::ScopeInputs::PlaceAccess{ordinal, view.read_only, view.fixed_binding});
        if (!inserted) {
          if (!declared_place) inputs.parameters[ordinal].name = capture.name;
          continue;
        }
        if (declared_place) parameter.name = "$scope.place." + std::to_string(ordinal);
        if (view.owners.size() != 1)
          implementation_gap(capture.at, "full-scope reference input requires one statically provable owner");
        require_borrow_lifetime(view, capture.at, false);
        parameter.owner = inputs.owners[owner_role(*view.owners.begin(), capture.at)].name;
        const auto slots = physical_slots(view.slots);
        parameter.read_only = std::ranges::all_of(slots, [&](size_t slot) { return bindings_->at(slot).read_only; });
        parameter.fixed_binding = std::ranges::any_of(slots, [&](size_t slot) { return bindings_->at(slot).fixed; });
        for (const auto slot : slots) for (const auto owner : bindings_->at(slot).place_owners) {
          if (lexical_.live && !owner_alive(owner)) fail(capture.at, "selected storage place's lexical owner has ended");
          inputs.place_owners[ordinal].insert(owner_role(owner, capture.at));
        }
        // The outlined body applies each lexical name's access restrictions.
        view.read_only = parameter.read_only;
      } else if (type.kind == Type::procedure) {
        note_abrupt(capture.at, {exception_id(ExceptionCategory::out_of_memory)});
      }
      const auto mode = argument_mode(parameter);
      prepare_borrow(view, mode, capture.at);
      inputs.nonnull.push_back(!independent && !view.targets.empty() && !view.targets.contains(null_target));
      if (live_value) inputs.live_values.insert(arguments.size());
      if (live_binding) inputs.live_bindings.insert(arguments.size());
      inputs.parameters.push_back(std::move(parameter));
      operation.argument_sources->push_back(&operand);
      operation.arguments.push_back({mode, type, live_binding});
      arguments.push_back(type);
      views.push_back(view);
    }
    scope_owner_relations(inputs, operation.scope_owners);
    scope_input_observations(inputs, views, expr.at);
    const auto profile = scope_profile(std::move(inputs));
    std::optional<ResultContract> supplied;
    if (expected) {
      const auto result = scope_result(infer_result(profile, arguments, retained, &state), expected);
      if (result.origin == ResultOrigin::supplied) supplied = result.contract;
    }
    const auto query = retain_query_environment(program_.methods[profile], supplied, &state, operation.scope_owners);
    const auto target = demand(profile, arguments, expr.at, {}, retained, query, &state);
    operation.callee = target;
    const auto &selected = program_.instances[target];
    auto contract = callable_type(program_.methods[profile], selected.arguments, selected.result);
    const auto &effects = read_effects(target);
    for (const auto failure : effects.scope_checked_failures) contract.throws_types.push_back(exception_type(failure));
    if (deferred_action_ && !effects.scope_checked_failures.empty())
      deferred_actions_with_checked_scopes_.insert(*deferred_action_);
    finish_call(expr, contract, effects, direct_call_reasons(contract, effects), arguments, views, operation, read, discard, 0, true);
  }
  void check_value_call(const Expression &expr, const Type &contract, size_t offset, CheckedInstance *instance,
      std::map<const Expression *, CheckedOperation> &operations, CheckedOperation &operation, bool read, bool discard) {
    const auto *choices = operation.invokes_here ? nullptr : observed_callables(expr);
    if (choices) operation.callable_choices = *choices;
    if (!operation.invokes_here && (!proving_ || !capture_free_callables(choices)))
      note_abrupt(expr.at, {exception_id(ExceptionCategory::out_of_memory)});
    if (checking_method_ && checking_method_->source->kind == Form::Kind::function && !contract.pure_callable)
      fail(expr.at, "a function cannot invoke a procedure");
    if (expr.children.size() - offset != contract.parameters.size()) fail(expr.at, "callable argument count does not match its view");
    operation.callable_call = !operation.invokes_here; operation.operand_type = contract;
    if (offset) operation.arguments.push_back({ArgumentMode::value, contract});
    bool completes = true;
    std::vector<Type> arguments;
    std::vector<CheckedOperation> views;
    for (size_t i = offset; i < expr.children.size(); ++i) {
      const auto &parameter = contract.parameters[i - offset];
      const auto mode = argument_mode(contract, i - offset);
      const auto argument = check(*expr.children[i], instance, operations, mode == ArgumentMode::value, false,
                                  &parameter, expected_type_use(mode));
      if (mode == ArgumentMode::value && argument.kind == Type::procedure)
        note_abrupt(expr.children[i]->at, {exception_id(ExceptionCategory::out_of_memory)});
      completes = completes && argument != Type::never;
      if (argument != Type::never && !accepts_argument(argument, parameter, mode)) {
        fail(expr.children[i]->at, "argument does not satisfy the callable view type");
      }
      arguments.push_back(argument); views.push_back(operations.at(expr.children[i].get()));
      if (argument != Type::never) prepare_borrow(views.back(), mode, expr.children[i]->at);
      operation.arguments.push_back({mode, operation.invokes_here || argument == Type::never ? parameter : argument});
    }
    if (!completes) { operation.type = Type::never; return; }
    if (operation.invokes_here && validating_ && proving_ && lexical_.live)
      for (size_t i = 0; i < views.size(); ++i) {
        if (const auto role = checking_method_->here->inputs[i].owner; role)
          require_here_owner(one_owner(views[i]), here_formal_owner(*role), expr.children[i + offset]->at);
      }
    auto [effects, closed] = callable_effects(contract, arguments,
        operation.invokes_here ? std::optional{std::set<size_t>{}} : operation.callable_choices);
    if (expr.kind == Expression::Kind::perform) effects.symbols.clear();
    auto reasons = closed ? direct_call_reasons(contract, effects) : call_reasons(contract);
    if (operation.invokes_here && proving_ && checking_method_->here_argument_instance) {
      const auto callback = *checking_method_->here_argument_instance;
      const auto &body = read_effects(callback);
      const bool protected_here = program_.methods[program_.instances[callback].method].here_group.has_value();
      if (lexical_.live && body.requires_deferred_failure_delivery) observed_.requires_deferred_failure_delivery = true;
      if (protected_here || !body.requires_deferred_failure_delivery) std::erase_if(reasons, [&](size_t reason) {
        const auto completion = Completion{reason};
        const bool checked = completion.exception() && exception_categories.at(reason - 3).checked;
        return completion.failed() && !checked && (protected_here || !body.escaping_failures.contains(reason));
      });
      if (protected_here && (body.propagates_here_transfer || !body.escaping_failures.empty() || body.requires_deferred_failure_delivery))
        reasons.insert(Completion::protected_transfer);
    }
    finish_call(expr, contract, effects, reasons, arguments, views,
                operation, read, discard, offset);
  }
  bool active_here(const std::string &name, const TypeEvidenceState *state = nullptr) const {
    const auto *method = state ? (state->method ? &program_.methods[*state->method] : nullptr) : checking_method_;
    return method && method->here && method->source->parameters.front().name == name &&
           !(state ? state->bindings.contains(name) || state->definitions.contains(name) : lexical_.lookup(name).has_value());
  }
  bool forwards_here(const Expression &expr, const TypeEvidenceState *state = nullptr) const {
    return expr.kind == Expression::Kind::call && !expr.intrinsic && !active_here(expr.text, state) && !expr.children.empty() &&
           expr.children.front()->kind == Expression::Kind::name && active_here(expr.children.front()->text, state);
  }
  size_t here_formal_owner(size_t role) const {
    return role ? external_owners_.at(checking_method_->input_owners().at(role - 1).name) : lexical_.scopes.front().id;
  }
  void require_here_owner(std::optional<EvidenceOwner> actual, EvidenceOwner expected, const Location &at,
                          const TypeEvidenceState *state = nullptr) const {
    const auto equality = compare_owners(actual, expected, state);
    if (equality == OwnerMatch::mismatch) fail(at, "borrowed here input has a different actual owner from its contract");
    if (equality == OwnerMatch::unknown) implementation_gap(at, "borrowed here input requires an unproved fixed owner equality");
  }
  std::optional<size_t> here_receiver_owner(const CheckedMethod &receiver, size_t role,
      const std::vector<CheckedOperation> &views, const Location &at) const {
    if (!role) return {}; // The receiver's own activation will supply this owner.
    const auto &name = receiver.input_owners().at(role - 1).name;
    const auto formals = receiver.input_parameters();
    const auto found = std::ranges::find(formals, name, &Parameter::owner);
    if (found != formals.end())
      if (const auto actual = one_owner(views.at(std::distance(formals.begin(), found)))) return actual;
    implementation_gap(at, "here contract requires an owner established by an ordinary borrowed argument");
  }
  // Reuse the ordinary call equations for arguments and outcomes.
  // The here capability itself stays outside the language's value carriers.
  Type here_invocation_type(const CheckedMethod &receiver, const Substitutions &bindings, const Location &at) const {
    const auto &here = *receiver.here;
    if (here.labelled || here.feedback || here.result.owner)
      implementation_gap(at, "labelled or borrowed here invocation contracts");
    const ModuleLookup lookup(module_, receiver.module);
    Type type{Type::procedure};
    type.pure_callable = type.value_result = true;
    std::map<size_t, size_t> roles;
    for (const auto &component : here.inputs) {
      const auto parameter = substitute(component.type, receiver, bindings, true);
      if (!scalar_type(parameter) ||
          (component.mode != ArgumentMode::value && component.mode != ArgumentMode::borrowed_ref))
        implementation_gap(at, "here invocation currently supports scalar value and initialized ref inputs");
      type.parameters.push_back(parameter);
      type.const_views.push_back(component.read_only);
      type.argument_modes.push_back(component.mode);
      type.owners.push_back(component.owner ? roles.try_emplace(*component.owner, roles.size() + 1).first->second : 0);
    }
    if (roles.empty()) {
      type.owners.clear();
      type.argument_modes.clear();
    } else type.owners.push_back(0);
    if (std::ranges::none_of(type.const_views, std::identity{})) type.const_views.clear();
    const auto result = substitute(here.result.type, receiver, bindings, true);
    if (result != Type::void_type && !scalar_type(result))
      implementation_gap(at, "here invocation requires an independently established scalar or void result");
    type.target = std::make_shared<Type>(result);
    for (const auto category : here.throws_categories) type.throws_types.push_back(exception_type(category));
    return type;
  }
  std::pair<CheckedOperation::SuppliedHere, std::optional<size_t>> demand_supplied_here(const Expression &body, const Type &contract,
      std::vector<std::optional<size_t>> owners, const ResultContract &receiving,
      CheckedInstance *supplier, std::map<const Expression *, CheckedOperation> &operations) {
    if (!body.definition) implementation_gap(body.at, "this receiving pattern for a here body");
    const auto &definition = *body.definition;
    std::optional<CheckedMethod::HereGroupResults> group;
    if (definition.supplied_group) {
      if (checking_method_ && checking_method_->here_group) implementation_gap(body.at, "a single-brace here body supplied inside a here group");
      if (body.receiving->from_for && receiving.type != Type::void_type)
        fail(body.at, "a generator loop requires a receiver with a void result");
      if (receiving.borrowed() || (receiving.type != Type::void_type && !scalar_type(*receiving.type)) ||
          (supplier && (supplier->result.borrowed() || (supplier->result.type != Type::void_type && !scalar_type(*supplier->result.type)))))
        implementation_gap(body.at, "single-brace here completion with a non-scalar supplier or receiving result");
      group = CheckedMethod::HereGroupResults{supplier ? supplier->result.type : std::nullopt, *receiving.type, {}};
      if (definition.body->inspects_return()) group->supplier_return = false;
      group->binds_return_payload = definition.body->inspects_return(true);
    }
    const auto &parameters = definition.parameters;
    if (parameters.size() != contract.parameters.size()) fail(body.at, "here body header arity does not match the selected contract");
    const auto base = lexical_method(&definition);
    if (!program_.methods[base].type_parameters.empty())
      implementation_gap(body.at, "retained generic parameters in a supplied here body");
    CheckedMethod::ScopeInputs inputs{};
    inputs.lexical_method = base;
    inputs.parameters = parameters;
    inputs.nonnull.resize(parameters.size(), false);
    inputs.here_input_owners = owners.size();
    const auto add_owner = [&](const Location &at) {
      inputs.owners.push_back({"$scope.owner." + std::to_string(inputs.owners.size()), {}, at});
    };
    for (size_t i = 0; i < owners.size(); ++i) add_owner(body.at);
    for (size_t i = 0; i < parameters.size(); ++i) {
      const auto &parameter = parameters[i];
      const auto &item = body.receiving->items[i];
      const auto mode = argument_mode(contract, i);
      const bool receives_value = parameter.declaration_kind == "val" || item.operation == "<-";
      const bool borrowed = mode != ArgumentMode::value;
      const bool binds_ref = borrowed && !receives_value;
      const bool snapshot = borrowed && receives_value;
      if (!parameter.owner.empty() || parameter.declaration_kind == "ptr" || parameter.declaration_kind == "func")
        implementation_gap(parameter.at, "this receiving declaration for a here body");
      if (!borrowed && parameter.declaration_kind == "ref")
        implementation_gap(parameter.at, "supplier-owned materialization of a scalar here input");
      if (parameter.type != Type::void_type) {
        const auto declared = supplier ? substitute(parameter.type, program_.methods[supplier->method], supplier->substitutions)
                                       : resolve_type(parameter.type);
        if (declared != contract.parameters[i]) fail(parameter.at, "here body header type does not match the selected contract");
      }
      const auto readonly = !contract.const_views.empty() && contract.const_views[i];
      if (!snapshot && (parameter.type != Type::void_type || parameter.read_only) &&
          parameter.read_only != readonly && !(binds_ref && parameter.read_only))
        fail(parameter.at, "here body header access does not match the selected contract");
      auto &input = inputs.parameters[i];
      input.declaration_kind = binds_ref ? "ref" : "val";
      input.read_only = parameter.read_only || (!snapshot && readonly);
      if (binds_ref) input.owner = inputs.owners.at(contract.owners[i] - 1).name;
      if (snapshot) inputs.here_snapshots.insert(i);
    }
    auto arguments = contract.parameters;
    CheckedOperation::SuppliedHere supplied{};
    const auto capture_owner = [&](size_t actual, const Location &at) {
      if (lexical_.live && !owner_alive(actual)) fail(at, "here access requires a live supplier owner");
      const auto found = std::ranges::find(owners, std::optional{actual});
      const auto role = std::distance(owners.begin(), found);
      if (found == owners.end()) {
        add_owner(at);
        owners.push_back(actual);
        supplied.owners.push_back(actual);
      }
      return role;
    };
    const auto &dependencies = free_bindings(definition);
    std::map<std::string, Capture> dependencies_by_name;
    for (const auto *names : {&dependencies.captures, &dependencies.queries}) for (const auto &name : *names)
      dependencies_by_name.try_emplace(name.name, name);
    for (const auto &[name, capture] : dependencies_by_name) {
      if (active_here(name)) implementation_gap(capture.at, "nested access to a here formal from a supplied body");
      const auto binding = lexical_.lookup(name);
      if (!binding) continue;
      const auto &source = bindings_->at(*binding);
      if (source.owner_binding) {
        inputs.owner_names.emplace(name, capture_owner(named_owner(name, capture.at), capture.at));
        continue;
      }
      if (!source.value_binding || source.alias || !source.selection.empty() || !numeric(source.type))
        implementation_gap(capture.at, "here access requires a local numeric val slot with stable storage");
      const auto &operand = scope_operand(base, capture);
      const auto type = check(operand, supplier, operations, false);
      const auto view = operations.at(&operand);
      if (view.targets.size() != 1 || view.owners.size() != 1 || !view.place || storage(*view.place).borrowed)
        implementation_gap(capture.at, "here access requires a local numeric val slot with stable storage");
      // A receiver may enter the body zero or several times. A live numeric
      // val stays initialized, and its value is always read from that slot.
      if (validating_ && proving_ && lexical_.live && !view.initialized_target() &&
          !missing_initialization(lexical_, *view.place, selected_target(view, *view.place, true)).empty())
        implementation_gap(capture.at, "here access to a slot without definite initialization at supply");
      const auto role = capture_owner(*view.owners.begin(), capture.at);
      Parameter input{name, type, capture.at, false, {}};
      input.declaration_kind = "ptr";
      input.owner = inputs.owners.at(role).name;
      input.read_only = view.read_only;
      input.fixed_binding = view.fixed_binding;
      const auto ordinal = arguments.size();
      inputs.live_values.insert(ordinal);
      inputs.place_owners[ordinal].insert(role);
      inputs.nonnull.push_back(true);
      inputs.parameters.push_back(std::move(input));
      arguments.push_back(type);
      supplied.places.push_back(&operand);
    }
    for (const auto a : owners) {
      auto &relations = inputs.relations.emplace_back();
      for (const auto b : owners)
        relations.push_back(a && b ? compare_owners(a, b) : a == b ? OwnerMatch::satisfied : OwnerMatch::mismatch);
    }
    const auto failures = group ? std::vector<Type>{} : contract.throws_types;
    std::optional<size_t> returning;
    if (group && group->supplier_return.has_value()) {
      group->supplier_return = true;
      const auto profile = scope_profile(inputs, ResultContract{*contract.target}, failures, group);
      returning = demand(profile, arguments, body.at);
      group->supplier_return = false;
    }
    const auto profile = scope_profile(std::move(inputs), ResultContract{*contract.target}, failures, std::move(group));
    supplied.instance = demand(profile, std::move(arguments), body.at);
    return {std::move(supplied), returning};
  }
  size_t demand_here_receiver(size_t receiver, size_t callback, const std::vector<Type> &arguments, const Location &at) {
    const auto key = std::pair{receiver, callback};
    const auto [profile, inserted] = here_receiver_profiles_.try_emplace(key, program_.methods.size());
    if (inserted) {
      auto method = program_.methods[receiver];
      method.instances.clear();
      method.here_argument_instance = callback;
      program_.methods.push_back(std::move(method));
    }
    return demand(profile->second, arguments, at);
  }
  bool deliver_here_completion(const Expression &expr, const CheckedOperation &operation) {
    const auto &body = read_effects(operation.supplied_here->instance);
    const auto receiver = lexical_;
    for (const auto reason : body.escaping_failures) {
      const auto completion = Completion{reason};
      note_abrupt(expr.at, completion.exception() ? std::set<size_t>{reason - 3} : std::set<size_t>{}, completion);
    }
    if (body.returns_from_supplier) {
      lexical_.returned = {};
      if (checking_instance_ && checking_instance_->result.type == Type::boolean)
        lexical_.returned.boolean = CleanupEquations::unknown_boolean();
      note_abrupt(expr.at, {}, {1, true});
    }
    lexical_ = receiver;
    return body.breaks_here_call;
  }
  void check_here_receiver(const Expression &expr, CheckedInstance *supplier,
      std::map<const Expression *, CheckedOperation> &operations, CheckedOperation &operation, bool read, bool discard) {
    if (expr.kind != Expression::Kind::call || expr.intrinsic || &expr == prepared_call_)
      implementation_gap(expr.at, "here supply through computed, symbolic or deferred calls");
    if (active_here(expr.text)) implementation_gap(expr.at, "supplying a body to a here formal");
    const auto supplied = expr.has_here_body();
    const auto forward = forwards_here(expr);
    if (supplied && forward) fail(expr.at, "the first here parameter has both a forwarded and a trailing argument");
    const auto family = named_family(expr.text);
    if (family.context || family.definition)
      implementation_gap(expr.at, "supplying here arguments to lexical receiver definitions");
    const auto count = expr.children.size() - supplied - forward;
    const auto written = std::span<const Expr>{expr.children}.subspan(forward, count);
    const auto bind_written = [&](const CheckedMethod &method) -> std::optional<std::vector<size_t>> {
      const auto parameters = method.input_parameters();
      std::vector<size_t> slots;
      std::set<size_t> occupied;
      for (const auto &operand : written) {
        size_t slot = slots.size();
        if (operand->kind == Expression::Kind::named_argument) {
          const auto found = std::ranges::find(parameters, operand->text, &Parameter::name);
          if (found == parameters.end()) return {};
          slot = std::distance(parameters.begin(), found);
        }
        if (!occupied.insert(slot).second) return {};
        slots.push_back(slot);
      }
      return slots;
    };
    std::vector<size_t> candidates;
    std::optional<std::vector<size_t>> slots;
    std::vector<ArgumentMode> modes;
    bool same_arity = false;
    for (const auto id : family.methods) {
      const auto &method = program_.methods[id];
      if (!method.here || method.parameters.size() != count) continue;
      same_arity = true;
      auto bound = bind_written(method);
      if (!bound) continue;
      if (slots && *slots != *bound) implementation_gap(expr.at, "overloads with different named here argument bindings");
      slots = std::move(bound);
      std::vector<ArgumentMode> candidate_modes;
      for (const auto &parameter : method.input_parameters()) candidate_modes.push_back(argument_mode(parameter));
      if (!candidates.empty() && modes != candidate_modes)
        implementation_gap(expr.at, "here receiver overloads with different ordinary argument preparation");
      modes = std::move(candidate_modes);
      candidates.push_back(id);
    }
    if (candidates.empty() && same_arity) fail(expr.at, "written arguments must bind distinct ordinary parameters of the here receiver");
    if (candidates.empty()) fail(expr.at, "no receiver with a matching first here parameter for '" + expr.text + "'");
    auto &operands = operation.argument_sources.emplace();
    operands.resize(count);
    operation.arguments.resize(count);
    std::vector<Type> arguments(count);
    std::vector<CheckedOperation> views(count);
    for (size_t i = 0; i < count; ++i) {
      const auto &source = *written[i];
      const bool named = source.kind == Expression::Kind::named_argument;
      if (named && source.children.empty()) implementation_gap(source.at, "default arguments to a here receiver");
      const auto &operand = named ? *source.children.front() : source;
      const auto slot = slots->at(i);
      const auto mode = modes.at(slot);
      const auto type = check(operand, supplier, operations, mode == ArgumentMode::value, false, nullptr, expected_type_use(mode));
      if (type == Type::never) implementation_gap(operand.at, "abrupt ordinary arguments to a here receiver");
      if (!scalar_type(type)) implementation_gap(operand.at, "here receiver ordinary arguments require scalar inputs");
      arguments[slot] = type;
      views[slot] = operations.at(&operand);
      prepare_borrow(views[slot], mode, operand.at);
      operation.arguments[slot] = {mode, type};
      operands[slot] = &operand;
    }
    if (!std::ranges::is_sorted(*slots)) operation.argument_evaluation_order = std::move(*slots);
    const auto selected = select_method(candidates, arguments, expr.at, expr.text, {}, access_of(views), owners_of(views), nullptr, true, true);
    const auto &receiver = program_.methods[selected];
    if (checking_method_ && checking_method_->source->kind == Form::Kind::function && receiver.source->kind != Form::Kind::function)
      fail(expr.at, "a function cannot invoke a procedure");
    const auto contract = here_invocation_type(receiver, *match(receiver, arguments, {}, true), expr.at);
    std::vector<std::optional<size_t>> owners;
    for (const auto i : here_owner_inputs(contract))
      owners.push_back(here_receiver_owner(receiver, *receiver.here->inputs[i].owner, views, expr.at));
    auto callee = demand(selected, arguments, expr.at);
    if (forward) {
      const auto incoming = here_invocation_type(*checking_method_, supplier->substitutions, expr.at);
      if (incoming != contract) fail(expr.at, "forwarded here argument does not match the selected contract");
      const auto owner_inputs = here_owner_inputs(incoming);
      for (size_t i = 0; i < owner_inputs.size(); ++i) {
        if (!owners[i]) fail(expr.at, "forwarded here argument cannot use the new receiver's invocation owner");
        require_here_owner(owners[i], here_formal_owner(*checking_method_->here->inputs[owner_inputs[i]].owner), expr.at);
      }
      operation.forwards_here = true;
      if (checking_method_->here_argument_instance)
        callee = demand_here_receiver(selected, *checking_method_->here_argument_instance, arguments, expr.at);
      if (proving_ && lexical_.live && read_effects(callee).protected_defer_failure)
        observed_.protected_defer_failure = true;
      finish_named_call(expr, arguments, views, operation, read, discard, true, callee);
      return;
    }
    auto [body, returning] = demand_supplied_here(*expr.children.back(), contract, std::move(owners),
        program_.instances[callee].result, supplier, operations);
    const auto finish_here = [&](CheckedOperation &call, const CheckedOperation::SuppliedHere &body) {
      const auto receiver = demand_here_receiver(selected, body.instance, arguments, expr.at);
      const auto &effects = read_effects(body.instance);
      if (validating_ && !effects.symbols.empty())
        implementation_gap(expr.at, "dynamic binding requirements at a here invocation");
      if (program_.methods[program_.instances[body.instance].method].here_group) {
        if (validating_ && read_effects(receiver).protected_defer_failure)
          implementation_gap(expr.at, "residual receiver defer failures across a protected here transfer");
        for (const auto failure : effects.scope_checked_failures) {
          if (failure) note_abrupt(expr.at, {failure});
          else note_declared(declared_requirement(expr));
        }
      }
      call.supplied_here = body;
      finish_named_call(expr, arguments, views, call, read, discard, true, receiver);
      if (effects.breaks_here_call) call.predicate.reset();
    };
    if (!returning) { finish_here(operation, body); return; }

    const auto before = lexical_;
    auto with_return = operation;
    auto returning_body = body;
    returning_body.instance = *returning;
    refine_return_input(true);
    finish_here(with_return, returning_body);
    const auto returned = lexical_;
    lexical_ = before;
    refine_return_input(false);
    finish_here(operation, body);
    const auto normal = lexical_;
    join(normal, false, returned, false, expr.at);
    if (operation.type != with_return.type) {
      throw std::logic_error("supplied here return state changed the receiving result contract");
    }
    operation.supplied_here->returning = CheckedOperation::SuppliedHere::ReturningProfile{*returning, *with_return.callee};
    operation.predicate.reset();
  }
  CheckedOperation check_guarded(const Expression &expr, CheckedInstance *instance,
      std::map<const Expression *, CheckedOperation> &operations, bool read = true, bool discard = false, const Type *expected = nullptr,
      std::optional<size_t> action = {}, ExpectedTypeUse expected_use = ExpectedTypeUse::materialization) {
    const auto before = lexical_;
    auto *outer = abrupt_;
    AbruptFlow incoming{lexical_.scopes.size(), {}};
    if (expr.kind == Expression::Kind::deferred) {
      const bool observes_return = !return_inputs_.present.empty();
      const auto arrive = [&](Completion reason, const ExceptionFacts &exceptions) {
        for (const bool returned : {false, true}) {
          if (returned && !observes_return) break;
          const auto input = CleanupEquations::deferred_input({reason.reason, returned}, true, observes_return);
          const auto normal = CleanupEquations::deferred_input({0, returned}, true, observes_return);
          auto state = before;
          state.inputs = {input};
          state.input_facts = {{input, before.input_facts.at(normal)}};
          (returned ? return_inputs_.present : return_inputs_.absent).insert(input);
          incoming.paths.push_back({std::move(state), exceptions, expr.at, reason});
        }
      };
      for (size_t category = 1; category < exception_categories.size(); ++category)
        arrive(Completion::failure(category), ExceptionFacts::arrival(action).restricted({category}));
      arrive({2, false}, {});
    }
    abrupt_ = &incoming;
    const auto body = check(*expr.children[0], instance, operations, read, discard, expected, expected_use);
    if (read && !discard && body.kind == Type::procedure) note_abrupt(expr.at, {exception_id(ExceptionCategory::out_of_memory)});
    const auto body_state = lexical_;
    std::vector<AbruptPath> control_inputs;
    for (const auto &path : incoming.paths) if (path.exceptions.empty()) control_inputs.push_back(path);
    std::vector<LexicalState> normals;
    std::vector<const CheckedOperation *> views;
    if (body != Type::never) { normals.push_back(body_state); views.push_back(&operations.at(expr.children[0].get())); }
    Type result_type = body;
    auto result_exceptions = body == Type::never ? ExceptionFacts{} : operations.at(expr.children[0].get()).exceptions;
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
      check_catch_category(type, handled, handler.at);
      if (!type.identity) incoming.declared.clear();
      std::vector<LexicalState> entries;
      ExceptionFacts categories;
      std::map<size_t, CompletionInputs> category_inputs;
      bool resumes_protected_transfer = false;
      for (auto &path : incoming.paths) {
        auto selected = type.identity ? path.exceptions.restricted({type.identity}) : path.exceptions;
        if (selected.empty()) continue;
        auto entry = path.state; entry.exception_conditions.insert(selected);
        entries.push_back(std::move(entry));
        resumes_protected_transfer |= path.receiver_transfer_pending;
        categories.merge(selected);
        for (const auto category : selected.possibilities())
          category_inputs[category].insert(path.state.inputs.begin(), path.state.inputs.end());
        path.exceptions = path.exceptions.without(selected.possibilities());
      }
      lexical_ = body_state;
      if (entries.empty()) join(body_state, true, before, false, handler.at);
      else {
        join(body_state, true, entries.front(), false, handler.at);
        for (size_t p = 1; p < entries.size(); ++p) { const auto joined = lexical_; join(joined, false, entries[p], false, handler.at); }
      }
      lexical_.returned = {};
      checked_scope_owners_[&handler] = next_scope_;
      lexical_.scopes.push_back({next_scope_++, {}});
      const auto id = declare(handler.text, type, true, true, handler.at);
      lexical_.scopes.back().declarations = false;
      initialize(id);
      assign_exceptions(id, categories.empty() ? exception_possibilities(type) : categories);
      auto active = caught_; caught_ = id; catch_bindings_.insert(id);
      caught_inputs_[id] = std::move(category_inputs);
      AbruptFlow handler_exits{before.scopes.size(), {}};
      abrupt_ = &handler_exits;
      const auto handled_type = check(*handler.children.front(), instance, operations, read, discard, expected, expected_use);
      if (read && !discard && handled_type.kind == Type::procedure) note_abrupt(handler.at, {exception_id(ExceptionCategory::out_of_memory)});
      caught_ = active; catch_bindings_.erase(id);
      caught_inputs_.erase(id);
      leave_scope();
      CheckedOperation fact; fact.type = type; fact.place = id; fact.exceptions = categories;
      operations[&handler] = fact;
      if (handled_type != Type::never) {
        if (expr.kind != Expression::Kind::deferred) check_guarded_result(result_type, handled_type, handler.at);
        result_type = handled_type;
        if (!entries.empty()) {
          normals.push_back(lexical_);
          views.push_back(&operations.at(handler.children.front().get()));
          const auto &values = operations.at(handler.children.front().get()).exceptions;
          result_exceptions.merge(values);
          if (resumes_protected_transfer && lexical_.live)
            control_inputs.push_back({lexical_, {}, handler.at, {Completion::protected_transfer}});
        }
      }
      if (!entries.empty()) {
        outgoing.paths.insert(outgoing.paths.end(), handler_exits.paths.begin(), handler_exits.paths.end());
        if (resumes_protected_transfer) for (auto path : handler_exits.paths) {
          // A receiver catch can recover its local failure while the
          // supplier's transfer still has to leave the receiver.
          if (path.completion.failed()) path.receiver_transfer_pending = true;
          else path.completion.reason = Completion::protected_transfer;
          (path.exceptions.empty() ? control_inputs : outgoing.paths).push_back(std::move(path));
        }
        outgoing.declared.insert(handler_exits.declared.begin(), handler_exits.declared.end());
      }
    }
    for (auto &path : incoming.paths) if (!path.exceptions.empty()) outgoing.paths.push_back(path);
    // Empty-category paths include returns and terminal numeric failures; catches cannot handle them.
    std::vector<LexicalState> continuations;
    Type cleanup = Type::void_type;
    if (finalizer) {
      struct SuspendedCompletion {
        const LexicalState *state;
        const AbruptPath *abrupt;
        std::map<size_t, size_t> routes;
        bool returned() const { return abrupt && abrupt->completion.returned; }
      };
      std::vector<SuspendedCompletion> pending;
      for (const auto &normal : normals) pending.push_back({&normal, nullptr, {}});
      for (const auto *paths : {&outgoing.paths, &control_inputs})
        for (const auto &path : *paths) pending.push_back({&path.state, &path, {}});

      const auto inherited = return_inputs_;
      std::map<size_t, size_t> split;
      const auto prepares_observed_return = [&](const SuspendedCompletion &entry) {
        if (checking_method_ && checking_method_->here_group)
          return entry.abrupt && entry.abrupt->here_exit == HereExit::return_supplier;
        return entry.returned();
      };
      if (finalizer->inspects_return()) {
        return_inputs_ = {};
        bool bounded_selection = true;
        for (const auto &entry : pending) if (entry.state->live) {
          const bool prepared = prepares_observed_return(entry);
          const auto &observed = prepared ? entry.state->returned.borrowed_selection : inherited.borrowed_selection;
          if (prepared || std::ranges::any_of(entry.state->inputs, [&](size_t input) { return inherited.present.contains(input); })) {
            if (!observed) bounded_selection = false;
            else if (return_inputs_.borrowed_selection) return_inputs_.borrowed_selection->merge(*observed);
            else return_inputs_.borrowed_selection = observed;
          }
          for (const auto input : entry.state->inputs) {
            if (prepared || inherited.present.contains(input)) return_inputs_.present.insert(input);
            else return_inputs_.absent.insert(input);
          }
        }
        if (!bounded_selection) return_inputs_.borrowed_selection.reset();
        // A normal continuation and a local return may reach this finally
        // from the same deferred entry. Give their effects separate routes.
        for (const auto input : intersect(return_inputs_.present, return_inputs_.absent)) {
          const auto local_return = next_finalizer_input_++;
          split.emplace(input, local_return);
          return_inputs_.present.erase(input);
          return_inputs_.present.insert(local_return);
        }
      }

      lexical_ = body_state;
      bool live_entry = false;
      for (auto &entry : pending) {
        for (const auto input : entry.state->inputs) {
          const auto local_return = split.find(input);
          const auto local = prepares_observed_return(entry) && local_return != split.end() ? local_return->second : input;
          entry.routes.emplace(input, local);
        }
        const auto state = split.empty() ? *entry.state : remap_completion_inputs(*entry.state, entry.routes);
        const auto joined = lexical_;
        join(joined, !live_entry, state, false, expr.at);
        live_entry = live_entry || state.live;
      }

      std::map<size_t, size_t> watched_bindings;
      std::set<size_t> new_observations;
      for (const auto &entry : pending) for (const auto binding : entry.state->returned.selection_bindings) {
        const auto identity = lexical_.selection_identities.find(binding);
        if (identity == lexical_.selection_identities.end() || !lexical_.bindings_available.contains(binding)) continue;
        watched_bindings.emplace(binding, identity->second);
        if (lexical_.observed_saved_selections.insert(identity->second).second) new_observations.insert(identity->second);
      }
      refresh_input_facts(lexical_);
      lexical_.returned = {};
      for (const auto input : lexical_.inputs) lexical_.input_facts.at(input).queues.at(cleanup_scope_) = 0;

      // A rethrow of an enclosing catch keeps that catch's category routes.
      const auto enclosing_catches = split.empty() ? decltype(caught_inputs_){} : caught_inputs_;
      if (!split.empty()) for (auto &[binding, categories] : caught_inputs_)
        for (auto &[category, inputs] : categories)
          for (const auto &[parent, child] : split) if (inputs.contains(parent)) inputs.insert(child);

      AbruptFlow final_exits{before.scopes.size(), {}};
      abrupt_ = &final_exits;
      cleanup = check(*finalizer, instance, operations);
      const auto final_state = lexical_;
      abrupt_ = outer;
      return_inputs_ = inherited;
      if (!split.empty()) caught_inputs_ = enclosing_catches;
      if (cleanup != Type::void_type && cleanup != Type::never)
        fail(expr.at, "finally must complete with void or an explicit return");
      for (const auto requirement : final_exits.declared) note_declared(requirement);

      const auto resume = [&](const SuspendedCompletion &entry, const LexicalState &completed, bool retain_return) {
        if (!entry.state->live || !completed.live) return false;
        std::map<size_t, size_t> outward;
        for (const auto &[parent, local] : entry.routes)
          if (completed.inputs.contains(local)) outward.emplace(local, parent);
        if (outward.empty()) return false;
        lexical_ = remap_completion_inputs(completed, outward);

        const auto &previous = *entry.state;
        lexical_.bindings_available.insert(previous.bindings_available.begin(), previous.bindings_available.end());
        lexical_.available.insert(previous.available.begin(), previous.available.end());
        CleanupEquations::combine_initialization(lexical_.initialized, lexical_.uninitialized,
            previous.initialized, previous.uninitialized, false);
        for (const auto input : lexical_.inputs) {
          auto &facts = lexical_.input_facts.at(input);
          const auto &prior = previous.input_facts.at(input);
          auto &queue = facts.queues.at(cleanup_scope_);
          queue = cleanup_.append(prior.queues.at(cleanup_scope_), queue);
          facts.initialized.insert(prior.initialized.begin(), prior.initialized.end());
          facts.initialized.insert(lexical_.initialized.begin(), lexical_.initialized.end());
          facts.present.insert(prior.present.begin(), prior.present.end());
        }
        refresh_input_facts(lexical_);
        auto returned = retain_return ? previous.returned : CleanupEquations::Returned{};
        if (std::ranges::all_of(lexical_.inputs, [&](size_t input) {
              const auto &ready = lexical_.input_facts.at(input).initialized_saved_selections;
              return std::ranges::any_of(returned.selection_bindings, [&](size_t binding) {
                const auto found = watched_bindings.find(binding);
                return found != watched_bindings.end() && ready.contains(found->second);
              });
            })) returned.initialize_saved_selection();
        std::erase_if(returned.selection_bindings, [&](size_t binding) {
          const auto saved = watched_bindings.find(binding);
          const auto current = lexical_.selection_identities.find(binding);
          return saved == watched_bindings.end() || current == lexical_.selection_identities.end() ||
              saved->second != current->second;
        });
        returned.then(lexical_.returned);
        lexical_.returned = std::move(returned);
        lexical_.returned.initialize(lexical_.initialized, lexical_.selections);
        for (const auto identity : new_observations) {
          lexical_.observed_saved_selections.erase(identity);
          for (const auto input : lexical_.inputs) lexical_.input_facts.at(input).initialized_saved_selections.erase(identity);
        }
        return true;
      };

      for (const auto &replacement : final_exits.paths) {
        const bool retain = expr.kind == Expression::Kind::deferred && replacement.completion.reason != 1;
        using Outcome = std::tuple<size_t, bool, bool>;
        std::map<Outcome, LexicalState> outcomes;
        for (const auto &entry : pending) {
          if (!resume(entry, replacement.state, retain)) continue;
          const bool returned = replacement.completion.returned || (retain && entry.returned());
          const bool carries_transfer = entry.abrupt && checking_method_ && checking_method_->here &&
              (entry.abrupt->completion.protected_here() || entry.abrupt->receiver_transfer_pending);
          const bool displaced = carries_transfer && replacement.completion.failed();
          const auto reason = carries_transfer && !displaced ? Completion::protected_transfer : replacement.completion.reason;
          const auto [found, fresh] = outcomes.try_emplace(Outcome{reason, returned, displaced}, lexical_);
          if (!fresh) {
            const auto resumed = lexical_;
            join(found->second, false, resumed, false, replacement.at);
            found->second = lexical_;
          }
        }
        for (auto &[outcome, state] : outcomes) {
          const auto [reason, returned, displaced] = outcome;
          lexical_ = std::move(state);
          note_abrupt(replacement.at, replacement.exceptions, {reason, returned}, replacement.immediate_scope_failure,
                      replacement.here_exit, displaced || replacement.receiver_transfer_pending);
        }
      }
      for (const auto &entry : pending) {
        if (cleanup == Type::never || !resume(entry, final_state, true)) continue;
        if (entry.abrupt)
          forward_abrupt(*entry.abrupt);
        else continuations.push_back(lexical_);
      }
      if (continuations.empty()) {
        join(final_state, true, body_state, true, expr.at);
        lexical_.live = false;
      }
    } else {
      abrupt_ = outer;
      for (const auto *paths : {&outgoing.paths, &control_inputs}) for (const auto &path : *paths) {
        lexical_ = path.state;
        refresh_input_facts(lexical_);
        forward_abrupt(path);
      }
      continuations = normals;
      lexical_ = body_state;
    }
    if (cleanup != Type::never || expr.kind == Expression::Kind::deferred) {
      for (const auto requirement : incoming.declared) note_declared(requirement);
      for (const auto requirement : outgoing.declared) note_declared(requirement);
    }
    for (size_t i = 0; i < continuations.size(); ++i) {
      const auto joined = lexical_;
      join(joined, i == 0, continuations[i], false, expr.at);
    }
    if (cleanup == Type::never || continuations.empty()) lexical_.live = false;
    CheckedOperation result;
    result.type = cleanup == Type::never || normals.empty() ? Type{Type::never} :
        expr.kind == Expression::Kind::deferred ? Type{Type::void_type} : result_type;
    if (!views.empty()) {
      result.predicate = predicate_of(*views.front());
      for (size_t i = 1; i < views.size(); ++i)
        result.predicate = predicate(Predicate::Kind::alternatives, result.predicate, predicate_of(*views[i]));
      result.place = views.front()->place;
      result.binding = views.front()->binding;
      result.selection_identity = views.front()->selection_identity;
      result.slots = views.front()->slots;
      result.initialization = CleanupEquations::Initialization{};
      for (const auto *view : views) {
        result.selection_identity = merge_selection_identities(result.selection_identity, view->selection_identity);
        result.view = result.view || view->view;
        result.nullable_view = result.nullable_view || view->nullable_view;
        result.read_only = result.read_only || view->read_only;
        result.fixed_binding = result.fixed_binding || view->fixed_binding;
        if (result.place != view->place) result.place.reset();
        if (result.binding != view->binding) result.binding.reset();
        if (view->slots.empty()) result.slots.clear();
        else if (!result.slots.empty()) result.slots.insert(view->slots.begin(), view->slots.end());
        result.targets.insert(view->targets.begin(), view->targets.end());
        result.null_sources.insert(view->null_sources.begin(), view->null_sources.end());
        result.owners.insert(view->owners.begin(), view->owners.end());
        result.origins.insert(view->origins.begin(), view->origins.end());
        result.selection_versions.insert(view->selection_versions.begin(), view->selection_versions.end());
        if (view->view) result.constrain_initialization(view->initialization);
      }
      result.mixed_view = std::any_of(views.begin(), views.end(), [](const auto *view) { return !view->view || view->mixed_view; });
      select_targets(result, result.targets);
      for (size_t i = 0; i < views.size(); ++i) merge_nullness(result, *views[i], normals[i], expr.at);
    }
    result.exceptions = std::move(result_exceptions);
    return result;
  }
  CheckedOperation read_binding(const Expression &expr, bool read = true, std::optional<size_t> selected_binding = {}) {
    CheckedOperation operation;
    const auto binding = selected_binding ? selected_binding : lexical_.lookup(expr.text);
    if (!binding) fail(expr.at, "unknown scalar value '" + expr.text + "'");
    if (!lexical_.bindings_available.contains(*binding)) fail(expr.at, "binding '" + expr.text + "' is not available on every reaching path");
    const auto &slot = (*bindings_)[*binding];
    if (slot.owner_binding) fail(expr.at, "owner binding '" + expr.text + "' is not a scalar value");
    const auto selected = slot.alias.value_or(*binding);
    require_slots_live({selected}, expr.at, "aliased binding's lexical scope has ended");
    if (lexical_.live && lexical_.expired_bindings.contains(selected))
      fail(expr.at, "binding '" + expr.text + "' refers to storage whose lexical owner has ended or may expire across a loop backedge");
    if (lexical_.live) for (const auto owner : lexical_.owners.at(selected))
      if (!owner_alive(owner))
        fail(expr.at, "binding '" + expr.text + "' refers to storage whose lexical owner has ended");
    operation.binding = selected;
    operation.slots = {selected};
    operation.owners = lexical_.owners.at(selected);
    operation.selection_versions = lexical_.target_versions.at(selected);
    operation.selection_identity = lexical_.selection_identities.at(selected);
    operation.null_sources = lexical_.null_sources.at(selected);
    select_targets(operation, lexical_.targets.at(selected));
    operation.origins = {selected};
    operation.type = slot.type;
    operation.nullable_view = slot.nullable;
    operation.read_only = slot.read_only;
    operation.fixed_binding = slot.fixed;
    operation.view = true;
    operation.initialization = selection_initialization(lexical_, selected);
    if (read) dereference(operation, expr.at);
    for (const auto place : operation.targets) {
      if (place == null_target) continue;
      if (delayed_controls_.contains(place)) fail(expr.at, "a deferred block cannot capture loop-control binding '" + expr.text + "'");
      const bool captured = capture_catch(place);
      if (captured) operation.captured_place = place;
      if (read && !operation.initialized_target())
        require_initialized(place, expr.at, expr.text, selected, selected_target(operation, place, true));
    }
    if (operation.type.kind == Type::exception) operation.exceptions = stored_exceptions(operation);
    if (read) operation.predicate = stored_predicate(operation);
    return operation;
  }
  CheckedOperation named_binding(const Expression &expr, size_t binding, const Type *expected = nullptr, bool read = true) {
    if (!lexical_.bindings_available.contains(binding)) fail(expr.at, "binding '" + expr.text + "' is not available on every reaching path");
    if (const auto found = definitions_.find(binding); found != definitions_.end()) {
      const auto &definition = found->second;
      return materialize_definition(NamedFamily{{definition.method}, nullptr, {}, &definition, definition.declaration.get()}, expected, expr.at);
    }
    return read_binding(expr, read, binding);
  }
  CheckedOperation named_value(const Expression &expr, const Type *expected = nullptr, bool read = true) {
    if (const auto binding = lexical_.lookup(expr.text)) return named_binding(expr, *binding, expected, read);
    const auto family = named_family(expr.text);
    if (!family.methods.empty()) return materialize_definition(family, expected, expr.at);
    if (has_declarations(expr.text)) fail(expr.at, "no applicable implementation for declared callable '" + expr.text + "'");
    if (module_names().symbols.contains(expr.text))
      implementation_gap(expr.at, "materialization of a compile-time symbol object");
    return read_binding(expr, read);
  }
  Type check(const Expression &expr, CheckedInstance *instance,
             std::map<const Expression *, CheckedOperation> &operations, bool read = true, bool discard = false,
             const Type *expected = nullptr, ExpectedTypeUse expected_use = ExpectedTypeUse::materialization) {
    using K = Expression::Kind;
    CheckedOperation operation;
    if (checking_method_ && checking_method_->here_group &&
        (expr.kind == K::deferred || expr.kind == K::make_local || expr.kind == K::make_region ||
         expr.kind == K::allocate || (expr.kind != K::return_test && expr.owner_name == "here")))
      implementation_gap(expr.at, "supplier-owned allocation, owner selection or defer from a here group");
    if (expr.spread) implementation_gap(expr.at, "spread arguments");
    if (expr.has_here_body() || forwards_here(expr)) {
      check_here_receiver(expr, instance, operations, operation, read, discard);
      return operations.insert_or_assign(&expr, std::move(operation)).first->second.type;
    }
    if (active_here(expr.text)) {
      if (expr.kind == K::name) fail(expr.at, "a here argument can only be invoked or forwarded");
      if (expr.kind == K::call && !expr.intrinsic) {
        if (&expr == prepared_call_) implementation_gap(expr.at, "prepared deferred here invocation");
        operation.invokes_here = true;
        const auto contract = here_invocation_type(*checking_method_, instance->substitutions, expr.at);
        check_value_call(expr, contract, 0, instance, operations, operation, read, discard);
        return operations.insert_or_assign(&expr, std::move(operation)).first->second.type;
      }
    }
    const auto child = [&](size_t i, bool read_value = true, bool discarded = false) { return check(*expr.children.at(i), instance, operations, read_value, discarded); };
    const auto view = [&](const CheckedOperation &source) {
      operation.predicate = predicate_of(source);
      operation.nullness = source.nullness;
      operation.selection_identity = source.selection_identity;
      operation.place = source.place;
      operation.binding = source.binding;
      operation.slots = source.slots;
      operation.targets = source.targets;
      operation.null_sources = source.null_sources;
      operation.owners = source.owners;
      operation.origins = source.origins;
      operation.selection_versions = source.selection_versions;
      operation.initialization = source.initialization;
      operation.mixed_view = source.mixed_view;
      operation.view = source.view;
      operation.nullable_view = source.nullable_view;
      operation.read_only = source.read_only;
      operation.fixed_binding = source.fixed_binding;
    };
    const auto &method = instance ? program_.methods[instance->method].source : nullptr;
    if (instance && !(lexical_methods_.contains(method) && expr.text == method->name) &&
        !lexical_.lookup(expr.text) && module_names().values.contains(expr.text) &&
        (expr.kind == K::name || expr.kind == K::call || expr.kind == K::perform || expr.kind == K::behave))
      implementation_gap(expr.at, "module-local binding lookup and capture in callable bodies");
    if ((expr.kind == K::binary || expr.kind == K::call) && identity_operation(expr.text)) {
      if (expr.children.size() != 2) fail(expr.at, "reference identity requires two reference views");
      const auto left_type = child(0, false), right_type = child(1, false);
      auto &left = operations.at(expr.children[0].get());
      auto &right = operations.at(expr.children[1].get());
      operation.arguments = {{ArgumentMode::borrowed_ptr, left_type}, {ArgumentMode::borrowed_ptr, right_type}};
      operation.builtin = BuiltinOperation::identity;
      for (const auto *operand : {&left, &right}) if (operand->type != Type::never) {
        if (!operand->view || operand->mixed_view)
          fail(expr.at, "reference identity requires existing target views, not independent values");
        require_borrow_lifetime(*operand, expr.at, &expr == prepared_call_);
      }
      operation.type = left_type == Type::never || right_type == Type::never ? Type{Type::never} : Type{Type::boolean};
      if (operation.type != Type::never) {
        if (left.type == Type::null_ref && left.owners.empty()) left.owners = right.owners;
        if (right.type == Type::null_ref && right.owners.empty()) right.owners = left.owners;
        if (left.owners.size() != 1 || right.owners.size() != 1)
          fail(expr.at, "reference identity requires an owner context; two unqualified nulls have none");
        operation.predicate = identity_predicate(left, right);
        if (expr.text == "is not") operation.predicate = predicate(Predicate::Kind::negation, operation.predicate);
      } else lexical_.live = false;
      operations[&expr] = operation;
      return operation.type;
    }
    switch (expr.kind) {
    case K::integer: case K::real: operation = literal(expr); break;
    case K::boolean:
      operation.type = Type::boolean; operation.integer = expr.text == "true";
      operation.predicate = constant_predicate(operation.integer);
      break;
    case K::return_test: {
      operation.type = Type::boolean;
      const auto &group = checking_method_->here_group;
      if (return_owner_filter(expr)) {
        const auto owner = expr.owner_name == "here" ? cleanup_scope_ : named_owner(expr.owner_name, expr.at);
        if (group || !instance->result.borrowed()) operation.return_filter.can_match = false;
        else {
          const auto result = external_owners_.at(instance->result.owner);
          const auto match = compare_owners(result, owner);
          operation.return_filter.can_match = match != OwnerMatch::mismatch;
          if (match == OwnerMatch::unknown) operation.return_filter.owner_comparison = std::pair{result, owner};
        }
        break;
      }
      const auto annotation = expr.annotation ?
          std::optional{substitute(*expr.annotation, *checking_method_, instance->substitutions, true)} : std::nullopt;
      const auto &inspected = group ? group->supplier : instance->result.type;
      if (!group && return_pattern_mode(expr) != ResultMode::value) {
        const auto result_owner = instance->result.borrowed() ?
            std::optional{external_owners_.at(instance->result.owner)} : std::nullopt;
        const auto expected_owner = expr.owner_name.empty() ? result_owner :
            std::optional{expr.owner_name == "here" ? cleanup_scope_ : named_owner(expr.owner_name, expr.at)};
        const auto match = result_owner && expected_owner ? compare_owners(*result_owner, *expected_owner) : OwnerMatch::mismatch;
        const auto pattern = borrowed_return_pattern(expr, annotation, inspected, instance->result, match, true);
        operation.operand_type = pattern.binding.value_or(Type::never);
        operation.return_filter.can_match = pattern.matches != OwnerMatch::mismatch;
        if (pattern.matches == OwnerMatch::unknown)
          operation.return_filter.owner_comparison = std::pair{*result_owner, *expected_owner};
        operation.owners.insert(operation.return_filter.can_match ? *result_owner : expected_owner.value_or(cleanup_scope_));
        break;
      }
      const auto pattern = value_return_pattern(expr, annotation, inspected, !group && instance->result.borrowed(), true);
      operation.operand_type = pattern.binding.value_or(Type::void_type);
      operation.return_filter.can_match = pattern.matches != OwnerMatch::mismatch;
      break;
    }
    case K::null_ref:
      if (read) implementation_gap(expr.at, "untyped null in a payload expression; bind a typed ptr view first");
      operation.type = Type::null_ref; operation.view = operation.nullable_view = true; select_targets(operation, {null_target});
      if (!expr.owner_name.empty()) operation.owners = {expr.owner_name == "here" ? cleanup_scope_ : named_owner(expr.owner_name, expr.at)};
      break;
    case K::name: operation = named_value(expr, callable_expectation(expected), read); break;
    case K::callable: {
      const auto method = lexical_method(expr.definition.get());
      if (expr.definition->name.empty())
        operation = materialize(std::vector<size_t>{method}, callable_expectation(expected), expr.at, true);
      else { define_callable(method, expr.at); operation.type = Type::void_type; }
      break;
    }
    case K::unary: {
      if (signed_minimum(expr)) {
        operation.type = operation.operand_type = Type::integer;
        operation.minimum_literal = true;
        check_builtin_declarations(expr, {Type::integer}, operation);
        break;
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
        operation.predicate = predicate(expr.text == "&&" ? Predicate::Kind::conjunction : Predicate::Kind::disjunction,
            predicate_of(operations.at(expr.children[0].get())), predicate_of(operations.at(expr.children[1].get())));
        break;
      }
      [[fallthrough]];
    }
    case K::value_call: case K::call: {
      const auto name = expr.kind != K::value_call ? operation_name(expr) : std::string_view{};
      const bool eager = eager_operation(name);
      if (expr.kind == K::call && swap_operation(name)) {
        if (expr.children.size() != 2) fail(expr.at, "swap requires two destinations");
        const bool reference = name == "<=>";
        child(0, false);
        auto left = operations.at(expr.children[0].get());
        child(1, false);
        auto right = operations.at(expr.children[1].get());
        for (const auto *operand : {&left, &right}) if (operand->type != Type::never) {
          if (!operand->view) fail(expr.at, "swap requires two storage destinations");
          if (operand->mixed_view) implementation_gap(expr.at, "swap through a mixed value/storage view");
          if (reference) reference_destination(*operand, expr.at);
        }
        operation.destination_arguments = true;
        operation.arguments = {{ArgumentMode::borrowed_ptr, left.type, reference}, {ArgumentMode::borrowed_ptr, right.type, reference}};
        const auto check_bits_destinations = [&] {
          for (const auto *operand : {&left, &right}) if (operand->type != Type::never) {
            bits_swap_target(operand->type, operand->read_only, expr.at);
          }
        };
        if (reference) operation.builtin = BuiltinOperation::reference_swap;
        if (left.type == Type::never || right.type == Type::never) {
          if (!reference && (expr.intrinsic || named_family(expr.text).methods.empty())) check_bits_destinations();
          operation.type = Type::never; break;
        }
        if (reference) {
          left = current_selection(left, expr.at);
          right = current_selection(right, expr.at);
        }
        const std::vector<CheckedOperation> views{left, right};
        const std::vector<Type> arguments{left.type, right.type};
        if (!expr.intrinsic && applicable_operation(expr.text, arguments, access_of(views), owners_of(views))) {
          operation.builtin = BuiltinOperation::none;
          finish_named_call(expr, arguments, views, operation, read, discard);
          break;
        }
        operation.builtin = reference ? BuiltinOperation::reference_swap : BuiltinOperation::bits_swap;
        if (reference) {
          check_rebind(left, right, expr.at);
          check_rebind(right, left, expr.at);
          check_builtin_declarations(expr, arguments, operation, true, views);
          if (&expr == prepared_call_) prepare_reference_swap(left, right, operation, expr.at);
          else if (!left.binding || left.binding != right.binding) {
            const auto before_left = snapshot_selection(left, expr.at), before_right = snapshot_selection(right, expr.at);
            rebind(left, right, before_right);
            rebind(right, left, before_left);
          }
        } else {
          check_bits_destinations();
          if (left.type != right.type) fail(expr.at, "bits swap requires the exact same type");
          require_borrow_lifetime(left, expr.children[0]->at, &expr == prepared_call_);
          require_borrow_lifetime(right, expr.children[1]->at, &expr == prepared_call_);
          read_view(left, expr.at, "left swap target"); read_view(right, expr.at, "right swap target");
          for (auto &argument : operation.arguments) argument.mode = ArgumentMode::borrowed_read;
          check_builtin_declarations(expr, arguments, operation, true, views);
          if (&expr != prepared_call_) {
            const auto left_exceptions = stored_exceptions(left), right_exceptions = stored_exceptions(right);
            const auto left_predicate = stored_predicate(left), right_predicate = stored_predicate(right);
            write_exceptions(left, right_exceptions); write_exceptions(right, left_exceptions);
            write_predicate(left, right_predicate, expr.at); write_predicate(right, left_predicate, expr.at);
          } else {
            record_payload_write(left, true); record_payload_write(right, true);
            const auto incoming = incoming_payloads();
            auto payloads = incoming;
            const auto left_exceptions = stored_exceptions(left, incoming), right_exceptions = stored_exceptions(right, incoming);
            write_exception_payloads(payloads, left, right_exceptions);
            write_exception_payloads(payloads, right, left_exceptions);
            operation.invocation[0].payloads = payload_changes(incoming, payloads);
            if (left.type == Type::boolean) {
              auto booleans = incoming_booleans();
              const auto left_value = stored_boolean(left, booleans), right_value = stored_boolean(right, booleans);
              write_boolean_payloads(booleans, left, right_value); write_boolean_payloads(booleans, right, left_value);
              for (const auto place : payload_targets(left)) operation.invocation[0].booleans[place] = booleans.at(place);
              for (const auto place : payload_targets(right)) operation.invocation[0].booleans[place] = booleans.at(place);
            }
            const auto left_written = payload_targets(left), right_written = payload_targets(right);
            auto &written = operation.invocation[0].payload_writes;
            written.insert(left_written.begin(), left_written.end()); written.insert(right_written.begin(), right_written.end());
          }
        }
        break;
      }
      if (expr.kind == K::value_call || (!expr.intrinsic && lexical_.lookup(expr.text) && named_family(expr.text).methods.empty())) {
        const size_t offset = expr.kind == K::value_call ? 1 : 0;
        const auto *definition = offset ? expr.children.front()->definition.get() : nullptr;
        if (definition && definition->implicit_scope) {
          check_scope_call(expr, lexical_method(definition), instance, operations, operation, read, discard,
                          expected_use == ExpectedTypeUse::callable_result ? expected : nullptr);
          break;
        }
        CheckedOperation target;
        if (offset) { child(0); target = operations.at(expr.children.front().get()); }
        else target = read_binding(expr);
        if (target.type.kind != Type::procedure || !target.type.target)
          fail(expr.at, "nearest binding '" + expr.text + "' is not callable");
        if (!offset) {
          operation.callable_binding = target.binding; operation.callable_place = target.place;
          operation.callable_nullable = target.targets.contains(null_target);
          operation.callable_targets = target.targets;
        }
        check_value_call(expr, target.type, offset, instance, operations, operation, read, discard);
        break;
      }
      if (expr.intrinsic && !eager && expr.text != "cause_at")
        implementation_gap(expr.at, "explicit intrinsic has no implemented builtin '" + expr.text + "'");
      std::vector<Type> arguments;
      std::vector<CheckedOperation> views;
      bool completes = true;
      const bool declared_builtin = eager && !expr.intrinsic && named_family(expr.text).methods.empty() && has_declarations(expr.text);
      const bool builtin_signature = eager && (expr.intrinsic ||
          (named_family(expr.text).methods.empty() && !has_declarations(expr.text)));
      const auto family = named_family(expr.text);
      if (!family.methods.empty() && (family.context || family.definition)) {
        operation.callable_captures = capture_values(family, expr.at);
        if (!operation.callable_captures.empty()) note_abrupt(expr.at, {exception_id(ExceptionCategory::out_of_memory)});
      }
      auto evidence = argument_evidence(expr);
      for (size_t i = 0; i < expr.children.size(); ++i) {
        auto preparation = argument_preparation(expr, name, arguments, evidence, access_of(views), owners_of(views));
        const auto mode = preparation.mode;
        // A builtin's passing mode is established. Retain context for missing
        // input evidence without pre-analyzing every normally producing operand.
        auto before = declared_builtin && !preparation.type ? std::optional{evidence_state()} : std::nullopt;
        const auto argument = check(*expr.children[i], instance, operations, mode == ArgumentMode::value, false,
            preparation.type ? &*preparation.type : nullptr, expected_type_use(mode));
        if (argument == Type::never) {
          if (before) {
            evidence = argument_evidence(expr, std::move(*before), i);
            preparation = argument_preparation(expr, name, arguments, evidence, access_of(views), owners_of(views));
          }
          if (!preparation.type && !builtin_signature) {
            if (!eager && !expr.intrinsic && expr.text != "cause_at" && named_family(expr.text).methods.empty())
              fail(expr.at, "unknown callable '" + expr.text + "'");
            implementation_gap(expr.children[i]->at, "cannot establish a parameter type for an abruptly completing operand");
          }
          // A known method signature supplies a checking type; a builtin needs no
          // invented payload type for an operand that cannot produce a value.
          arguments.push_back(preparation.type.value_or(Type{Type::never}));
          completes = false;
        } else {
          arguments.push_back(argument);
          require_value(argument, expr.at);
        }
        operation.arguments.push_back({mode, arguments.back()});
        if (mode == ArgumentMode::value && argument.kind == Type::procedure)
          note_abrupt(expr.children[i]->at, {exception_id(ExceptionCategory::out_of_memory)});
        views.push_back(operations.at(expr.children[i].get()));
        if (argument != Type::never) prepare_borrow(views.back(), mode, expr.children[i]->at);
      }
      for (size_t i = 0; i < arguments.size(); ++i)
        if (views[i].type != Type::never && ((evidence[i].type && *evidence[i].type != arguments[i]) ||
            (evidence[i].read_only && *evidence[i].read_only != views[i].read_only) ||
            (evidence[i].owner && checked_owner(*evidence[i].owner) != one_owner(views[i]))))
          implementation_gap(expr.children[i]->at, "operand type/access/owner evidence depends on completion or contextual inference");
      if (expr.text == "cause_at" && (expr.intrinsic || (!module_names().callables.contains(expr.text) && !has_declarations(expr.text)))) {
        if (arguments.size() != 2 || arguments[0].kind != Type::exception || arguments[1] != Type::uint64)
          fail(expr.at, "cause_at requires an exception and a u64 index");
        operation.type = completes ? exception_type(0) : Type{Type::never};
        operation.exceptions = exception_possibilities(operation.type); operation.builtin = BuiltinOperation::cause_at;
        if (completes) {
          const auto bounds = exception_id(ExceptionCategory::index_out_of_bounds);
          operation.invocation.try_emplace(0);
          operation.invocation.try_emplace(3 + bounds);
          if (&expr != prepared_call_) note_abrupt(expr.at, {bounds});
        }
        break;
      }
      if (eager && (expr.intrinsic || !applicable_operation(expr.text, arguments, access_of(views), owners_of(views)))) {
        if (std::any_of(operation.arguments.begin(), operation.arguments.end(), [](const auto &argument) { return argument.mode != ArgumentMode::value; }))
          implementation_gap(expr.at, "builtin fallback requires value preparation before later operand effects");
        check_eager(expr, name, arguments, views, operation);
        if (!completes) operation.type = Type::never;
        break;
      }
      finish_named_call(expr, arguments, views, operation, read, discard, completes);
      break;
    }
    case K::symbol: case K::procedure_effect: case K::effect: case K::touch: {
      operation.symbol = resolve_symbol(expr.text, expr.at,
          lexical_.lookup(expr.text).has_value() || active_here(expr.text) || query_environment().symbol_shadows.contains(expr.text),
          instance ? std::optional<size_t>{instance->method} : std::nullopt);
      const auto &symbol = program_.symbols[*operation.symbol];
      if (!symbol.type || !lexical_value_type(*symbol.type))
        implementation_gap(expr.at, "runtime symbol payload outside supported bits and complete-callable values");
      if (method && method->kind == Form::Kind::function)
        fail(expr.at, "a function cannot access dynamic bindings");
      if (expr.kind == K::effect || expr.kind == K::procedure_effect) {
        const bool cancelled = !instance && cancels_module_installation(*operation.symbol) &&
            std::ranges::none_of(lexical_.scopes, [](const auto &scope) { return scope.symbol_entry.has_value(); });
        operation.publishes_effect = !cancelled;
        if (expr.children.empty()) {
          if (symbol.type->kind != Type::procedure) fail(expr.at, "a scalar symbol installation requires an explicit value");
          const auto id = *operation.symbol;
          operation = materialize_symbol(id, *symbol.type, expr.at);
          operation.symbol = id; operation.operand_type = operation.type; operation.type = Type::void_type;
          operation.publishes_effect = !cancelled;
          if (!cancelled && lexical_.live) lexical_.bound_symbols.insert(id);
          break;
        }
        if (symbol.type->kind == Type::procedure) {
          if (expr.binding != "=") fail(expr.at, "procedure symbols require effect '=' installation");
        } else if (expr.binding != "<-") fail(expr.at, "bits symbols require effect '<-' transfer");
        const auto &operand = *expr.children[0];
        Type source;
        if (symbol.type->kind == Type::procedure && operand.kind == K::name &&
            !lexical_.lookup(operand.text) && module_names().symbols.contains(operand.text)) {
          const auto selected = resolve_symbol(operand.text, operand.at,
              query_environment().symbol_shadows.contains(operand.text), instance ? std::optional<size_t>{instance->method} : std::nullopt);
          operations[&operand] = materialize_symbol(selected, *symbol.type, operand.at);
          source = operations[&operand].type;
        } else source = check(operand, instance, operations, true, false,
                              symbol.type->kind == Type::procedure ? &*symbol.type : nullptr);
        if (source != Type::never && !transfers_value(source, *symbol.type))
          fail(expr.at, "effect value does not satisfy the declared symbol type");
        operation.operand_type = *symbol.type;
        operation.type = source == Type::never ? Type{Type::never} : Type{Type::void_type};
        if (source.kind == Type::procedure) note_abrupt(expr.at, {exception_id(ExceptionCategory::out_of_memory)});
        if (!cancelled && source != Type::never && lexical_.live) lexical_.bound_symbols.insert(*operation.symbol);
      } else {
        require_symbol(lexical_, *operation.symbol, expr.at);
        operation.type = expr.kind == K::touch ? Type{Type::void_type} : *symbol.type;
        operation.exceptions = exception_possibilities(operation.type);
      }
      break;
    }
    case K::behave: case K::perform: {
      operation.symbol = resolve_symbol(expr.text, expr.at,
          lexical_.lookup(expr.text).has_value() || active_here(expr.text) || query_environment().symbol_shadows.contains(expr.text),
          instance ? std::optional<size_t>{instance->method} : std::nullopt);
      if (method && method->kind == Form::Kind::function) fail(expr.at, "a function cannot invoke a dynamic procedure");
      const auto &symbol = program_.symbols[*operation.symbol];
      if (!symbol.intrinsic_print) {
        const auto &type = dynamic_procedure_type(symbol, expr.at);
        require_symbol(lexical_, *operation.symbol, expr.at);
        check_value_call(expr, type, 0, instance, operations, operation, read, discard);
        break;
      }
      print_result(expr);
      const auto argument = child(0);
      operation.arguments.push_back({ArgumentMode::value, argument});
      operation.type = print_result(expr, argument);
      if (argument == Type::never) break;
      operation.builtin = BuiltinOperation::print;
      break;
    }
    case K::grouped: {
      const auto &operand = *expr.children.front();
      operation.type = check(operand, instance, operations, false, discard, expected, expected_use);
      if (operation.type == Type::never) break;
      const auto &source = operations.at(&operand);
      const auto result = group_result(operation.type, source.view, source.nullable_view, source.mixed_view);
      if (result == GroupResult::unsupported)
        implementation_gap(expr.at, "parenthesized result with no proved value or nullable-target contract");
      operation.exceptions = source.exceptions;
      if (result == GroupResult::nullable_target) {
        if (read && operation.type == Type::null_ref)
          implementation_gap(expr.at, "untyped null in a payload expression; bind a typed ptr view first");
        view(source);
        operation.slots.clear();
        operation.fixed_binding = false;
      }
      if (source.view && (result == GroupResult::value || (read && !discard))) {
        require_live_view(source, expr.at);
        read_view(source, expr.at, "parenthesized result");
        operation.predicate = projected_predicate(source);
      } else operation.predicate = predicate_of(source);
      if (result == GroupResult::value && operation.type.kind == Type::procedure && source.view)
        note_abrupt(expr.at, {exception_id(ExceptionCategory::out_of_memory)});
      break;
    }
    case K::typed_expression: {
      if (*expr.annotation != Type::void_type) implementation_gap(expr.at, "non-void expression conversion or assertion");
      const auto &operand = *expr.children.front();
      const auto type = check(operand, instance, operations, false);
      if (type == Type::null_ref) fail(expr.at, "explicit discard requires an independently typed operand");
      require_live_view(operations.at(&operand), operand.at);
      operation.type = type == Type::never ? Type{Type::never} : Type{Type::void_type};
      break;
    }
    case K::conditional: {
      const auto &test = *expr.children.front();
      const bool return_test = test.kind == K::return_test;
      // When the joined scalar result needs a value, read a selected
      // reference after that arm's cleanup has finished.
      const bool value_join = read && !discard && [&] {
        auto probe = evidence_state();
        if (return_test && instance) {
          probe.invocation_result = instance->result.type;
          if (instance->result.borrowed()) {
            probe.borrowed_return = &instance->result;
            probe.invocation_result_owner = external_owners_.at(instance->result.owner);
          }
        }
        const auto condition = type_evidence(test, probe);
        if (return_test && probe.target.branches && !probe.target.branches->at(true)) return false;
        return condition != Type::never && conditional_arms(expr, probe, false, expected, nullptr, expected_use, true).independent();
      }();
      const auto condition = child(0);
      if (condition != Type::boolean && condition != Type::never) fail(expr.at, "if condition requires bool");
      const auto before = lexical_;
      refine(test, true, operations);
      const bool binds_return = return_test && !test.binding.empty();
      if (binds_return) {
        auto &pattern = operations.at(&test);
        lexical_.scopes.push_back({next_scope_++, {}});
        const auto mode = return_pattern_mode(test);
        if (mode != ResultMode::value) {
          const bool nullable = mode == ResultMode::borrowed_ptr;
          const auto owner = *pattern.owners.begin();
          std::set<size_t> targets;
          const auto &observed = return_inputs_.borrowed_selection;
          const bool concrete = observed && observed->entries.empty();
          if (pattern.return_filter.can_match) {
            if (concrete) targets = observed->places;
            else for (const auto &input : instance->frame.inputs) {
              const auto &candidate = storage(input.place);
              if (candidate.borrowed && candidate.scope == owner && candidate.type == pattern.operand_type)
                targets.insert(input.place);
            }
          }
          if (nullable && (!concrete || observed->nullable())) targets.insert(null_target);
          const bool readonly = test.read_only || (pattern.return_filter.can_match && instance->result.read_only);
          const auto id = bind(test.binding, pattern.operand_type, targets, readonly, false, test.fixed_binding, test.at);
          lexical_.owners[id].insert(owner);
          bindings_->at(id).nullable = nullable;
          bindings_->at(id).requires_initialized = !nullable;
          if (deferred_action_ && observed && observed->single_entry() == CleanupEquations::pending_return_selection)
            return_selection_actions_.emplace(lexical_.selection_identities.at(id), *deferred_action_);
          // Ref preparation already proved the selected payload initialized.
          if (!nullable) initialize_selection(lexical_, id);
          pattern.binding = id;
        } else {
          const auto id = declare(test.binding, pattern.operand_type, test.read_only || test.fixed_binding, true, test.at);
          auto &binding = bindings_->at(storage(id).binding);
          binding.fixed = test.fixed_binding;
          binding.callable_value = pattern.operand_type.kind == Type::procedure;
          if (binding.callable_value && proving_ && lexical_.live && !capture_free_callables(observed_callables(test))) {
            // Check the prepared value on the cleanup path that actually matches.
            const auto read = read_obligation({}, {test.at,
                "micro-dispatch implementation gap: return pattern needs proof of a capture-free function value; this is not a language restriction"});
            CleanupEquations::ReadRequirement proof;
            proof.when = path_conditions(lexical_);
            add_requirement(lexical_, read, lexical_.inputs, proof);
          }
          initialize(id);
          pattern.place = id;
        }
        lexical_.scopes.back().declarations = false;
      }
      const auto checked_yes = check(*expr.children[1], instance, operations, read && !value_join, discard, expected, expected_use);
      const auto yes = return_test && !operations.at(&test).return_filter.can_match ? Type{Type::never} : checked_yes;
      auto yes_state = lexical_;
      lexical_ = before;
      refine(test, false, operations);
      const auto no = check(*expr.children[2], instance, operations, read && !value_join, discard, expected, expected_use);
      auto no_state = lexical_;
      auto &yes_fact = operations.at(expr.children[1].get()), &no_fact = operations.at(expr.children[2].get());
      const bool independent = condition != Type::never && yes == no && independent_scalar_conditional(yes, yes_fact.view, no_fact.view,
          yes_fact.mixed_view || no_fact.mixed_view);
      if (value_join && !independent)
        implementation_gap(expr.at, "scalar conditional classification changes during ordered checking");
      if (independent) {
        const auto project = [&](const Expression &arm, CheckedOperation &fact, LexicalState &state) {
          if (!fact.view || !state.live) return;
          if (proving_ && validating_ && fact.targets.contains(null_target))
            fail(arm.at, "conditional scalar value requires a proven non-null target");
          const bool needs_read = !read || discard || value_join;
          if (!needs_read && fact.type != Type::boolean) return;
          lexical_ = std::move(state);
          if (needs_read) read_view(fact, arm.at, "conditional scalar result");
          if (fact.type == Type::boolean) fact.predicate = projected_predicate(fact);
          state = std::move(lexical_);
        };
        project(*expr.children[1], yes_fact, yes_state);
        project(*expr.children[2], no_fact, no_state);
      }
      if (binds_return) retire_scope(yes_state);
      join(yes_state, yes == Type::never, no_state, no == Type::never, expr.at);
      if (yes == Type::never) operation.type = no;
      else if (no == Type::never || yes == no) operation.type = yes;
      else if (yes == Type::null_ref && operations.at(expr.children[2].get()).view) operation.type = no;
      else if (no == Type::null_ref && operations.at(expr.children[1].get()).view) operation.type = yes;
      else implementation_gap(expr.at, "conditional join with different scalar result types");
      operation.exceptions = yes == Type::never ? no_fact.exceptions : yes_fact.exceptions;
      if (yes != Type::never && no != Type::never) operation.exceptions.merge(no_fact.exceptions);
      if (yes == Type::never) view(no_fact);
      else if (no == Type::never) view(yes_fact);
      else {
        if (operation.type == Type::boolean && read && !discard) {
          auto proof = snapshot_boolean(predicate_of(yes_fact), yes_state, expr.at);
          CleanupEquations::merge_boolean(proof, snapshot_boolean(predicate_of(no_fact), no_state, expr.at));
          operation.predicate = boolean_predicate(proof);
        } else operation.predicate = predicate(Predicate::Kind::alternatives, predicate_of(yes_fact), predicate_of(no_fact));
        if (independent) {
          operation.scalar_value_join = true;
          break;
        }
        operation.selection_identity = conditional_selection(operations.at(expr.children[0].get()), yes_fact, no_fact);
        operation.view = yes_fact.view || no_fact.view;
        operation.nullable_view = yes_fact.nullable_view || no_fact.nullable_view;
        operation.mixed_view = yes_fact.mixed_view || no_fact.mixed_view || yes_fact.view != no_fact.view;
        operation.read_only = yes_fact.read_only || no_fact.read_only;
        operation.fixed_binding = yes_fact.fixed_binding || no_fact.fixed_binding;
        if (yes_fact.view || no_fact.view) {
          auto targets = yes_fact.targets;
          targets.insert(no_fact.targets.begin(), no_fact.targets.end());
          operation.null_sources = yes_fact.null_sources;
          operation.null_sources.insert(no_fact.null_sources.begin(), no_fact.null_sources.end());
          select_targets(operation, targets);
        }
        operation.origins = yes_fact.origins;
        operation.origins.insert(no_fact.origins.begin(), no_fact.origins.end());
        operation.owners = yes_fact.owners;
        operation.owners.insert(no_fact.owners.begin(), no_fact.owners.end());
        if (scalar_type(operation.type) && operation.view && !operation.mixed_view && operation.owners.size() > 1) {
          const auto owner = *operation.owners.begin();
          bool common = true;
          for (const auto other : operation.owners) {
            const auto relation = compare_owners(owner, other);
            if (proving_ && validating_) require_conditional_owner(relation, expr.at);
            common = common && relation == OwnerMatch::satisfied;
          }
          if (common) operation.owners = {owner};
        }
        operation.initialization = CleanupEquations::Initialization{};
        if (yes_fact.view) operation.constrain_initialization(yes_fact.initialization);
        if (no_fact.view) operation.constrain_initialization(no_fact.initialization);
        if (yes_fact.binding == no_fact.binding) operation.binding = yes_fact.binding;
        if (!yes_fact.slots.empty() && !no_fact.slots.empty()) {
          operation.slots = yes_fact.slots;
          operation.slots.insert(no_fact.slots.begin(), no_fact.slots.end());
        }
        operation.selection_versions = yes_fact.selection_versions;
        operation.selection_versions.insert(no_fact.selection_versions.begin(), no_fact.selection_versions.end());
        merge_nullness(operation, yes_fact, yes_state, expr.at);
        merge_nullness(operation, no_fact, no_state, expr.at);
      }
      if (condition == Type::never) operation.type = Type::never;
      break;
    }
    case K::block: {
      bool exits = false;
      for (size_t i = 0; i < expr.children.size(); ++i) {
        const bool last = i + 1 == expr.children.size();
        operation.type = check(*expr.children[i], instance, operations, last ? read : true, last ? discard : true,
                               last ? expected : nullptr, expected_use);
        exits = exits || operation.type == Type::never;
        if (exits) lexical_.live = false;
      }
      if (!expr.children.empty()) view(operations.at(expr.children.back().get()));
      if (!expr.children.empty()) operation.exceptions = operations.at(expr.children.back().get()).exceptions;
      if (exits) operation.type = Type::never;
      break;
    }
    case K::range_for: case K::fold: {
      const bool fold = expr.kind == K::fold;
      const auto first = child(0), last = child(1);
      const auto index = range_index_type(first, last, expr.at);
      const bool completes = first != Type::never && last != Type::never;
      Type accumulator;
      if (fold) { accumulator = child(2); require_value(accumulator, expr.at); if (accumulator.kind == Type::procedure) note_abrupt(expr.at, {exception_id(ExceptionCategory::out_of_memory)}); }
      const auto before = lexical_;
      const auto start = constant_integer(*expr.children[0], operations), end = constant_integer(*expr.children[1], operations);
      const auto [body_possible, empty_possible, repeat_possible] = range_reachability(completes, index, start, end);
      std::map<size_t, size_t> headers;
      for (const auto input : lexical_.inputs) {
        auto &queue = lexical_.input_facts.at(input).queues.at(cleanup_scope_);
        headers[input] = queue = cleanup_.header(queue);
      }
      const auto iteration_scope = next_scope_;
      if (repeat_possible) {
        if (!proving_) loop_states_[&expr] = {iteration_scope, iteration_scope, before.bindings_available, {}, {}, {}};
        auto &loop = loop_states_.at(&expr);
        refresh_targets(lexical_, true, &loop.rebound);
        if (proving_) {
          retain_loop_targets(loop, before);
          // Assignments that are restored before the backedge do not select
          // the next iteration's target. Keep only entry and backedge bounds.
          for (const auto &[binding, selected] : loop.header_targets) {
            lexical_.targets[binding] = selected.places;
            if (selected.nullable()) lexical_.targets[binding].insert(null_target);
            lexical_.null_sources[binding] = selected.null_sources;
            lexical_.owners[binding] = before.owners.at(binding);
            assign_selection(lexical_, binding, selected);
          }
          retain_loop_initialization(loop, before);
          for (const auto &[binding, required] : loop.initialized) assign_initialization(lexical_, binding, required);
          lexical_.expired_bindings.insert(loop.expired.begin(), loop.expired.end());
        }
        for (const auto input : lexical_.inputs) for (auto &[id, categories] : lexical_.input_facts[input].exceptions)
          if (!proving_ || loop.writes.places.contains(id)) categories = exception_possibilities(storage(id).type);
        if (!proving_) lexical_.predicates.clear();
        else for (const auto place : loop.writes.places) lexical_.predicates.erase(place);
        active_loops_.push_back(&expr);
      }
      checked_scope_owners_[&expr] = iteration_scope;
      lexical_.live = lexical_.live && body_possible;
      refresh_input_facts(lexical_);
      lexical_.scopes.push_back({next_scope_++, {}, true, lexical_.bound_symbols});
      operation.place = declare(expr.text, index, true, true, expr.at);
      initialize(*operation.place);
      loop_controls_.insert(*operation.place);
      if (fold) {
        operation.accumulator = declare(expr.binding, accumulator, true, true, expr.at);
        initialize(*operation.accumulator);
        assign_exceptions(*operation.accumulator, exception_possibilities(accumulator));
        loop_controls_.insert(*operation.accumulator);
      }
      const auto body = child(fold ? 3 : 2, true, !fold);
      if (fold && body.kind == Type::procedure) note_abrupt(expr.at, {exception_id(ExceptionCategory::out_of_memory)});
      if (fold) check_fold_body(body, accumulator, expr.at);
      leave_scope();
      if (repeat_possible) {
        active_loops_.pop_back();
        if (!proving_) loop_states_.at(&expr).last = next_scope_;
      }
      const auto body_state = lexical_;
      if (proving_ && repeat_possible && body != Type::never && lexical_.live) {
        retain_loop_targets(loop_states_.at(&expr), body_state);
        retain_loop_initialization(loop_states_.at(&expr), body_state);
      }
      if (repeat_possible && body != Type::never && lexical_.live) for (const auto input : lexical_.inputs) {
        const auto &facts = lexical_.input_facts.at(input);
        cleanup_.backedge(headers.at(input), cleanup_.known(facts.queues.at(cleanup_scope_), facts.initialized,
            lexical_.uninitialized, CleanupEquations::stable_input_conditions(path_conditions(lexical_))));
      }
      lexical_ = before;
      lexical_.live = lexical_.live && empty_possible;
      lexical_.scopes.push_back({next_scope_++, {}, true, lexical_.bound_symbols});
      const auto otherwise = !fold && expr.children.size() == 4 ? child(3, true, true) : Type{Type::void_type};
      leave_scope();
      const auto empty_state = lexical_;
      const bool body_exits = !body_possible || body == Type::never;
      const bool empty_exits = !empty_possible || otherwise == Type::never;
      join(body_state, body_exits, empty_state, empty_exits, expr.at);
      operation.type = body_exits && empty_exits ? Type{Type::never} : fold ? accumulator : Type{Type::void_type};
      operation.operand_type = index;
      operation.exceptions = exception_possibilities(operation.type);
      break;
    }
    case K::pointer_group: {
      const bool callable_value = expr.declaration_kind == "func" || expr.declaration_kind == "proc";
      const bool value = expr.declaration_kind == "val" || callable_value;
      for (const auto &declaration : expr.children) {
        if (declaration->kind == K::block) {
          if (check(*declaration, instance, operations, true, true) == Type::never) operation.type = Type::never;
          continue;
        }
        std::optional<size_t> owner;
        if (!declaration->owner_name.empty()) {
          if (value) fail(declaration->at, "val storage belongs to its lexical scope, not an external owner");
          owner = named_owner(declaration->owner_name, declaration->at);
        }
        const auto &initializer = *declaration->children.front();
        const bool copy = value || declaration->binding == "<-";
        Type initial_type;
        bool initialized = true;
        bool completes = true;
        if (initializer.kind == K::make_local) {
          if (initializer.annotation) initial_type = resolve_local_type(*initializer.annotation, instance, initializer.at);
          else {
            auto state = evidence_state(false, nullptr, true);
            state.required = true;
            initial_type = *type_evidence(*initializer.children.front(), state);
          }
          const size_t value_index = initializer.annotation ? 0 : 1;
          if (initial_type.kind == Type::procedure) implementation_gap(initializer.at, "raw callable storage construction");
          initialized = initializer.children.size() > value_index;
          if (initialized) {
            const auto value_type = check(*initializer.children[value_index], instance, operations);
            completes = value_type != Type::never;
            check_local_initializer(value_type, initial_type, initializer.at);
          }
          CheckedOperation allocation;
          allocation.type = completes ? initial_type : Type{Type::never};
          allocation.initializes = initialized;
          if (initialized) {
            const auto &value = operations.at(initializer.children[value_index].get());
            allocation.exceptions = value.exceptions; allocation.predicate = predicate_of(value);
          }
          operations[&initializer] = allocation;
        } else {
          const auto annotation = declaration->annotation ? resolve_local_type(*declaration->annotation, instance, declaration->at) : Type{};
          initial_type = check(initializer, instance, operations, copy, false, annotation.kind == Type::procedure ? &annotation : nullptr);
          completes = initial_type != Type::never;
        }
        const auto type = declaration->annotation ? resolve_local_type(*declaration->annotation, instance, declaration->at) : initial_type;
        const auto &source = operations.at(&initializer);
        const bool borrowed = !copy && source.view;
        const bool mixed = borrowed && source.mixed_view;
        if (completes && owner)
          check_initializer_owner(borrowed, mixed, std::any_of(source.owners.begin(), source.owners.end(),
              [&](size_t actual) { return actual != *owner; }), declaration->at);
        if ((!borrowed || mixed) && type.kind == Type::procedure) note_abrupt(declaration->at, {exception_id(ExceptionCategory::out_of_memory)});
        check_initializer_type(expr, *declaration, completes ? initial_type : Type{Type::never}, type, borrowed);
        if (borrowed && completes) {
          auto owners = source.owners;
          if (mixed) owners.insert(declaration_scope(declaration->text, declaration->at).id);
          if (owners.size() > 1) fail(initializer.at, "reference initializer joins different owners");
          if (owners.empty() && !owner) fail(initializer.at, "reference initializer requires one statically provable owner; qualify null with 'in here' or 'in O'");
          if (lexical_.live) for (const auto owner : owners)
            if (!owner_alive(owner))
              fail(initializer.at, "reference initializer's lexical owner has ended");
          if (expr.declaration_kind == "ref") {
            if (proving_ && validating_ && lexical_.live && source.targets.contains(null_target)) fail(initializer.at, "ref requires a proven non-null target");
            require_initialized(source, initializer.at, declaration->text);
          }
        }
        if (completes && borrowed) check_initializer_access(*declaration, source.read_only);
        if (!borrowed && !initialized && expr.declaration_kind == "ref")
          fail(declaration->at, "ref requires definitely initialized target storage");
        const bool read_only = declaration->read_only || (value && expr.fixed_binding) || (completes && borrowed && source.read_only);
        std::optional<size_t> place;
        if (borrowed) {
          auto selected = completes ? source : CheckedOperation{};
          size_t id;
          if (mixed) {
            place = declare(declaration->text, type, read_only, false, declaration->at);
            id = storage(*place).binding;
            bindings_->at(id).fixed = expr.fixed_binding;
            // The hidden place is reachable only after its value arm initializes it.
            initialize(*place);
            if (type.kind == Type::exception) assign_exceptions(*place, source.exceptions);
            if (type == Type::boolean && source.predicate) lexical_.predicates[*place] = source.predicate;
            selected.targets.insert(*place);
            selected.owners.insert(storage(*place).scope);
          } else id = bind(declaration->text, type, selected.targets, read_only, false, expr.fixed_binding, declaration->at);
          assign_targets(id, selected, snapshot_selection(selected, initializer.at));
          if (owner) lexical_.owners[id].insert(*owner);
        } else {
          place = declare(declaration->text, type, read_only, value, declaration->at);
          bindings_->back().fixed = expr.fixed_binding;
          if (initialized && completes) initialize(*place);
          if (type.kind == Type::exception) assign_exceptions(*place, initialized ? source.exceptions : exception_possibilities(type));
          if (type == Type::boolean && initialized && completes) {
            CheckedOperation target; target.type = type; target.place = place; target.targets = {*place};
            write_predicate(target, predicate_of(source), declaration->at);
          }
        }
        bindings_->back().requires_initialized = expr.declaration_kind == "ref";
        bindings_->back().nullable = expr.declaration_kind == "ptr";
        bindings_->back().callable_value = callable_value;
        CheckedOperation binding;
        binding.binding = *lexical_.lookup(declaration->text);
        binding.place = place;
        binding.operand_type = type;
        binding.view = borrowed;
        binding.mixed_view = mixed;
        binding.initializes = !borrowed && initialized;
        if (!completes) binding.type = operation.type = Type::never;
        operations[declaration.get()] = binding;
      }
      break;
    }
    case K::alias: {
      const auto owner = expr.owner_name.empty() ? std::optional<size_t>{} :
          std::optional<size_t>{expr.owner_name == "here" ? cleanup_scope_ : named_owner(expr.owner_name, expr.at)};
      const auto type = child(0, false);
      const auto declared = expr.annotation ? std::optional{resolve_local_type(*expr.annotation, instance, expr.at)} : std::nullopt;
      if (type == Type::never) {
        declaration_scope(expr.text, expr.at);
        operation.type = Type::never;
        break;
      }
      auto source = operations.at(expr.children.front().get());
      if (!source.view || type == Type::null_ref) fail(expr.at, "alias requires an existing storage place");
      if (source.slots.empty() || source.mixed_view) implementation_gap(expr.at, "alias of a non-binding storage place");
      source = selection_state(std::move(source), expr.at);
      const auto slots = physical_slots(source.slots);
      const auto selected = bindings_->at(*slots.begin());
      for (const auto slot : slots) {
        const auto &binding = bindings_->at(slot);
        if (!expr.declaration_kind.empty() && expr.declaration_kind != place_kind_name(binding.kind()))
          fail(expr.at, "alias kind must match the selected " + place_kind_name(binding.kind()) + " storage");
        if (binding.kind() != selected.kind()) implementation_gap(expr.at, "alias selection with different storage kinds");
        if (owner && std::ranges::any_of(binding.place_owners, [&](size_t place_owner) {
              return compare_owners(*owner, place_owner) != OwnerMatch::satisfied;
            })) fail(expr.at, "alias owner must match the selected binding storage, not its target owner");
      }
      if (source.owners.size() != 1) fail(expr.at, "alias selection requires one statically provable target owner");
      if (declared && *declared != type)
        fail(expr.at, "alias annotation requires the exact selected type");
      if (expr.annotation && !expr.read_only && source.read_only)
        fail(expr.at, "readonly source cannot supply a writable alias target");
      const auto canonical = source.binding ? source.binding : slots.size() == 1 ? std::optional<size_t>{*slots.begin()} : std::nullopt;
      const auto id = bind(expr.text, type, canonical ? std::set<size_t>{} : source.targets,
          source.read_only || expr.read_only || (selected.value_binding && expr.fixed_binding),
          selected.value_binding, source.fixed_binding || expr.fixed_binding, expr.at, canonical);
      bindings_->at(id).requires_initialized = selected.requires_initialized;
      bindings_->at(id).nullable = selected.nullable;
      bindings_->at(id).callable_value = std::all_of(slots.begin(), slots.end(),
          [&](size_t slot) { return bindings_->at(slot).callable_value; });
      if (!canonical) {
        bindings_->at(id).selection = slots;
        assign_targets(id, source, snapshot_selection(source, expr.at));
        operation.binding = id;
      }
      break;
    }
    case K::transfer: case K::rebind: {
      const bool prepared = &expr == prepared_call_;
      operation.arguments = {{ArgumentMode::borrowed_ptr, Type::never, expr.kind == K::rebind},
                             {expr.kind == K::transfer ? ArgumentMode::value : ArgumentMode::borrowed_ptr, Type::never}};
      const auto destination = child(0, false);
      operation.arguments[0].type = destination;
      if (destination == Type::never) {
        child(1, expr.kind == K::transfer);
        operation.type = Type::never;
        break;
      }
      const auto target = operations.at(expr.children[0].get());
      assignment_destination(target.view, target.mixed_view, expr.at);
      for (const auto owner : target.owners) if (lexical_.live)
        if (!owner_alive(owner))
          fail(expr.at, "assignment destination's lexical owner has ended");
      if (expr.kind == K::rebind) {
        reference_destination(target, expr.at);
        if (prepared) require_deferred_slots(target, expr.at);
        if (child(1, false) == Type::never) { operation.type = Type::never; break; }
        const auto &selected = operations.at(expr.children[1].get());
        check_rebind(target, selected, expr.at);
        operation.arguments[1].type = selected.type;
        if (prepared) {
          require_borrow_lifetime(selected, expr.children[1]->at, true);
          const auto targets = snapshot_selection(selected, expr.at);
          const auto before = lexical_;
          lexical_.selections = {};
          for (auto &[input, facts] : lexical_.input_facts) facts.selections = {};
          auto snapshot = selected;
          // The saved operand is independent of current slot selections at invocation.
          snapshot.selection_identity.reset();
          if (snapshot.initialization)
            snapshot.initialization = CleanupEquations::capture_initialization(*snapshot.initialization, initialization_frame());
          rebind(target, snapshot, targets, true);
          operation.invocation[0].selections = lexical_.selections;
          lexical_ = before;
          operation.type = destination;
          break;
        }
        if (read && !discard) require_initialized(selected, expr.at, expr.children[0]->text);
        rebind(target, selected, snapshot_selection(selected, expr.at));
        operation.type = destination;
        operation.targets = selected.targets;
        operation.place = selected.place;
        operation.null_sources = selected.null_sources;
        operation.initialization = selected.initialization;
        operation.captured_place = selected.captured_place;
        operation.exceptions = stored_exceptions(operation);
        operation.predicate = stored_predicate(operation);
        operation.view = true;
        operation.nullable_view = target.nullable_view;
        operation.binding = target.binding;
        operation.selection_identity = selected.selection_identity;
        operation.owners.clear(); operation.selection_versions.clear();
        for (const auto id : target.slots) {
          const auto &owners = lexical_.owners.at(id), &versions = lexical_.target_versions.at(id);
          operation.owners.insert(owners.begin(), owners.end());
          operation.selection_versions.insert(versions.begin(), versions.end());
        }
        operation.origins = target.slots;
        operation.read_only = target.read_only;
        operation.fixed_binding = target.fixed_binding;
        if (read && !discard) dereference(operation, expr.at);
        break;
      }
      transfer_access(target.read_only, expr.at);
      dereference(target, expr.at);
      const auto source = check(*expr.children[1], instance, operations, true, false, destination.kind == Type::procedure ? &destination : nullptr);
      if (source == Type::never) { operation.type = Type::never; break; }
      transfer_type(source, destination, expr.at);
      operation.arguments[1].type = destination;
      auto written = target.targets; written.erase(null_target);
      auto written_target = target; select_targets(written_target, written);
      if (prepared) {
        require_borrow_lifetime(target, expr.children[0]->at, true);
        record_payload_write(written_target, true);
        CallEffects effects;
        effects.initialized[0] = {0};
        operation.invocation = invocation_effects(effects, {0}, {target}, expr.at, true);
        operation.type = operation.operand_type = destination;
        operation.targets = target.targets;
        operation.targets.erase(null_target);
        const auto incoming = incoming_payloads();
        auto payloads = incoming;
        write_exception_payloads(payloads, written_target, operations.at(expr.children[1].get()).exceptions);
        operation.invocation[0].payloads = payload_changes(incoming, payloads);
        if (destination == Type::boolean) {
          auto booleans = incoming_booleans();
          write_boolean_payloads(booleans, written_target, snapshot_boolean(predicate_of(operations.at(expr.children[1].get())), lexical_, expr.at));
          for (const auto place : payload_targets(target)) operation.invocation[0].booleans[place] = booleans.at(place);
        }
        operation.invocation[0].payload_writes = payload_targets(target);
        break;
      }
      if (destination.kind == Type::procedure && (!proving_ || !capture_free_callables(observed_callables(expr))))
        note_abrupt(expr.at, {exception_id(ExceptionCategory::out_of_memory)});
      if (written.size() == 1) initialize(*written.begin());
      else {
        initialize_targets(written_target, lexical_.initialized, lexical_.uninitialized);
        for (const auto input : lexical_.inputs)
          lexical_.input_facts[input].initialized.insert(lexical_.initialized.begin(), lexical_.initialized.end());
        refresh_input_facts(lexical_);
      }
      if (const auto binding = current_binding(target)) initialize_selection(lexical_, *binding);
      operation.type = operation.operand_type = destination;
      select_targets(operation, written);
      operation.owners = target.owners;
      operation.selection_identity = target.selection_identity;
      operation.origins = target.origins;
      operation.read_only = operation.view = true;
      operation.initialization = CleanupEquations::Initialization{};
      operation.exceptions = operations.at(expr.children[1].get()).exceptions;
      write_exceptions(written_target, operation.exceptions);
      operation.predicate = predicate_of(operations.at(expr.children[1].get()));
      write_predicate(written_target, operation.predicate, expr.at);
      break;
    }
    case K::break_value: case K::continue_value: case K::return_tail: case K::return_value: {
      if (!instance) fail(expr.at, "return requires an enclosing callable");
      const auto &group = checking_method_->here_group;
      const bool tail = expr.kind == K::return_tail;
      if (tail && !lexical_.live) {
        operation.type = child(0, true, true) == Type::never ? Type{Type::never} : Type{Type::void_type};
        break;
      }
      if (!tail && expr.kind != K::return_value && !group) fail(expr.at, "here control requires a single-brace here body");
      if (group && expr.kind != K::continue_value) {
        const bool breaking = expr.kind == K::break_value;
        const auto expected = breaking ? std::optional{group->receiving_call} : group->supplier;
        if (!expected) fail(expr.at, "return requires an enclosing supplier callable");
        if (breaking && *expected == Type::void_type && !expr.children.empty())
          fail(expr.at, "void receiving calls require bare break");
        const auto value = expr.children.empty() ? Type{Type::void_type} :
            check(*expr.children.front(), instance, operations, true, false, &*expected);
        if (value != Type::never && !transfers_value(value, *expected))
          fail(expr.at, std::string(breaking ? "break" : "supplier return") + " requires '" + type_name(*expected) + "'");
        operation.type = Type::never;
        operation.operand_type = *expected;
        operation.here_exit = breaking ? HereExit::break_call : HereExit::return_supplier;
        if (value != Type::never) note_abrupt(expr.at, {}, {Completion::protected_transfer}, false, operation.here_exit);
        break;
      }
      const auto &contract = instance->result;
      if (expr.kind == K::continue_value && contract.type == Type::void_type && !expr.children.empty())
        fail(expr.at, "void here feedback requires bare continue");
      const bool borrowed = contract.borrowed();
      const auto use = instance->result_supplied && !borrowed
          ? ExpectedTypeUse::callable_result : ExpectedTypeUse::materialization;
      const auto value = expr.children.empty() ? Type{Type::void_type} :
          check(*expr.children[0], instance, operations, !borrowed, false, &*contract.type, use);
      if (tail && (value == Type::void_type || value == Type::never)) {
        operation.type = value;
        break;
      }
      if (!borrowed && value.kind == Type::procedure && (!proving_ || !capture_free_callables(observed_callables(expr))))
        note_abrupt(expr.at, {exception_id(ExceptionCategory::out_of_memory)});
      if (borrowed && value != Type::never) {
        if (expr.children.empty()) fail(expr.at, "borrowed result requires an explicit target view");
        const auto &selected = operations.at(expr.children.front().get());
        if (!selected.view || selected.mixed_view) implementation_gap(expr.at, "fresh ordinary borrowed results require a separate storage contract");
        if (value != Type::null_ref && value != *instance->result.type) fail(expr.at, "borrowed return requires the exact target type");
        if (!contract.read_only && selected.read_only) fail(expr.at, "readonly target cannot supply a writable borrowed result");
        const auto owner = external_owners_.at(contract.owner);
        if (!selected.owners.empty() && selected.owners != std::set<size_t>{owner})
          fail(expr.at, "borrowed result does not retain its declared external owner");
        if (!contract.nullable() && proving_ && validating_ && lexical_.live && selected.targets.contains(null_target))
          fail(expr.at, "borrowed ref result requires a proven non-null target");
        if (!contract.nullable()) require_initialized(selected, expr.at, "borrowed result");
        operation.borrowed_result = true;
        lexical_.returned = {false, selected.targets, {}, selected.null_sources};
        lexical_.returned.initialization = selected.initialization;
        if (lexical_.returned.initialization)
          lexical_.returned.initialization = CleanupEquations::capture_initialization(*lexical_.returned.initialization, initialization_frame());
        lexical_.returned.selection_sources = selection_sources(selected.selection_identity);
        if (contract.nullable()) lexical_.returned.nullness = snapshot_boolean(null_predicate(selected), lexical_, expr.at);
        const auto observed = selected.selection_identity ? return_selection_actions_.find(*selected.selection_identity) : return_selection_actions_.end();
        if (observed != return_selection_actions_.end() && observed->second == deferred_action_)
          lexical_.returned.borrowed_selection = CleanupEquations::Targets{{}, {}, {CleanupEquations::pending_return_selection}};
        else lexical_.returned.borrowed_selection = selection_targets(selected.targets, selected.null_sources);
        if (contract.nullable() && !selected.initialized_target()) for (const auto place : selected.targets)
          if (place != null_target && !lexical_.initialized.contains(place)) lexical_.returned.uninitialized.insert(place);
        if (lexical_.returned.needs_initialization() && selected.selection_identity) {
          lexical_.returned.saved_capture = selected.selection_identity;
          for (const auto &[binding, identity] : lexical_.selection_identities)
            if (identity == *selected.selection_identity && lexical_.bindings_available.contains(binding))
              lexical_.returned.selection_bindings.insert(binding);
        }
      } else if (value != Type::never) {
        if (tail && *contract.type == Type::void_type)
          fail(expr.at, "value-producing tail in a void callable; discard the result with ': void' or end with 'return;'");
        if (!transfers_value(value, *contract.type))
          fail(expr.at, std::string(expr.kind == K::continue_value ? "continue" : tail ? "tail result" : "return") + " type '" +
               type_name(value) + "' does not satisfy '" + type_name(*instance->result.type) + "'");
        lexical_.returned = {false, {}, {}, {}};
        if (value.kind == Type::exception) lexical_.returned.value = operations.at(expr.children.front().get()).exceptions;
        if (value == Type::boolean) {
          lexical_.returned.boolean = snapshot_boolean(predicate_of(operations.at(expr.children.front().get())), lexical_, expr.at);
        }
      }
      operation.type = Type::never;
      operation.operand_type = *instance->result.type;
      note_abrupt(expr.at, {}, {1, true});
      break;
    }
    case K::field: {
      const auto source = child(0);
      if (source.kind != Type::exception && source != Type::never) implementation_gap(expr.at, "non-exception field selection");
      check_exception_field(expr);
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
      if (lexical_.live && type.kind == Type::exception && !type.identity) note_declared(declared_requirement(expr));
      const auto binding = expr.children.empty() ? caught_ : expr.children[0]->kind == K::name ? operations.at(expr.children[0].get()).place : std::optional<size_t>{};
      // An empty provisional result summary is not a non-returning contract.
      auto thrown = operation.exceptions;
      if (thrown.empty()) thrown = exception_possibilities(type);
      const auto categories = thrown.possibilities();
      if (binding && caught_inputs_.contains(*binding)) {
        const auto inputs = lexical_.inputs;
        for (const auto category : categories) {
          lexical_.inputs = intersect(inputs, caught_inputs_.at(*binding)[category]);
          if (!lexical_.inputs.empty()) note_abrupt(expr.at, thrown.restricted({category}));
        }
        lexical_.inputs = inputs;
      } else note_abrupt(expr.at, thrown);
      operation.type = Type::never;
      break;
    }
    case K::try_scope: operation = check_guarded(expr, instance, operations, read, discard, expected, {}, expected_use); break;
    case K::deferred: {
      const auto action = cleanup_.action();
      CleanupEquations::Action summary;
      if (expr.children.front()->kind == K::block) {
        const auto before = lexical_;
        // Delayed bodies contribute effects to the cleanup sequence. Writes
        // made when it drains are outside the registering loop's backedge.
        auto registering_loops = std::exchange(active_loops_, {});
        auto caught_inputs = std::move(caught_inputs_);
        caught_inputs_.clear();
        const auto controls = delayed_controls_;
        CatchCapture captures{captures_, catch_bindings_, {}};
        captures_ = &captures;
        const auto was_delayed = delayed_;
        const auto previous_inputs = deferred_inputs_;
        const auto previous_action = deferred_action_;
        const auto previous_return_inputs = return_inputs_;
        auto *outer = abrupt_;
        delayed_controls_.insert(loop_controls_.begin(), loop_controls_.end());
        delayed_ = true;
        deferred_action_ = action;
        lexical_.bound_symbols.clear();
        lexical_.required_symbols.clear();
        deferred_inputs_ = before.available;
        if (proving_) for (size_t place = 0; place < catalog_storage_.size(); ++place)
          if (std::any_of(before.scopes.begin(), before.scopes.end(), [&](const auto &scope) { return scope.id == catalog_storage_[place].scope; }))
            deferred_inputs_.insert(place);
        refresh_targets(lexical_);
        lexical_.predicates.clear();
        lexical_.boolean_payloads.clear();
        lexical_.selection_conditions = {};
        lexical_.exception_conditions.clear();
        lexical_.uninitialized.clear();
        lexical_.predicate_conditions = {CleanupEquations::ConditionFacts{}};
        const auto boolean_entry = incoming_booleans(action);
        for (const auto &[place, value] : boolean_entry) lexical_.predicates[place] = boolean_predicate(value);
        lexical_.payload_writes.clear();
        const auto incoming = incoming_payloads(action);
        lexical_.exceptions = incoming;
        summary.observes_return = expr.inspects_return();
        return_inputs_ = {{}, {0}};
        lexical_.inputs = {0};
        if (summary.observes_return) {
          lexical_.inputs.insert(CleanupEquations::deferred_return_offset);
          return_inputs_.present.insert(CleanupEquations::deferred_return_offset);
          if (instance && instance->result.borrowed())
            return_inputs_.borrowed_selection = CleanupEquations::Targets{{}, {}, {CleanupEquations::pending_return_selection}};
        }
        lexical_.selections = {};
        lexical_.input_facts = {{0, {lexical_.initialized, {}, incoming, {{cleanup_scope_, 0}}, lexical_.present}}};
        if (summary.observes_return)
          lexical_.input_facts.emplace(CleanupEquations::deferred_return_offset, lexical_.input_facts.at(0));
        lexical_.requirements.clear();
        lexical_.returned = {};
        lexical_.live = true;
        AbruptFlow flow{lexical_.scopes.size(), {}};
        abrupt_ = &flow;
        summary.guarded = expr.children.size() > 1;
        const auto completed = summary.guarded ? check_guarded(expr, instance, operations, true, true, nullptr, action).type : child(0, true, true);
        if (completed != Type::never && lexical_.live) flow.paths.push_back({lexical_, {}, expr.at, {0, false}});
        for (const auto &path : flow.paths) {
          for (const auto input : path.state.inputs) {
            const auto queue = path.state.input_facts.at(input).queues.at(cleanup_scope_);
            CleanupEquations::Effects effects{{}, path.state.input_facts.at(input).initialized, path.state.returned, path.state.input_facts.at(input).selections};
            effects.action_reaching = CleanupEquations::boolean_conditions(path_conditions(path.state));
            for (const auto place : path.state.available) if (!effects.initialized.contains(place)) {
              auto missing = missing_initialization(path.state, place);
              if (missing != CleanupEquations::ConditionClauses{CleanupEquations::ConditionFacts{}})
                effects.uninitialized.emplace(place, std::move(missing));
            }
            effects.symbols = path.state.required_symbols;
            effects.payload_writes = path.state.payload_writes;
            effects.present = path.state.input_facts.at(input).present;
            effects.declared_failures = flow.declared;
            for (const auto place : path.state.payload_writes) if (boolean_entry.contains(place)) {
              const auto proof = path.state.predicates.find(place);
              effects.booleans[place] = snapshot_boolean(proof == path.state.predicates.end() ? nullptr : proof->second, path.state, expr.at);
            }
            effects.returned.initialize(effects.initialized, effects.selections);
            for (const auto &[target, value] : payload_changes(incoming, path.state.input_facts.at(input).exceptions))
              if (incoming.contains(target)) effects.payloads.emplace(target, value);
            for (const auto &[read, requirement] : path.state.requirements) if (requirement.inputs.contains(input))
              effects.reads.emplace(read, requirement.before);
            if (path.exceptions.empty()) summary.paths.push_back({input, path.completion, queue, effects});
            else for (const auto category : path.exceptions.possibilities()) {
              auto failed = effects;
              failed.thrown = path.exceptions.restricted({category});
              summary.paths.push_back({input, {3 + category, path.completion.returned}, queue, std::move(failed)});
            }
          }
        }
        lexical_ = before;
        active_loops_ = std::move(registering_loops);
        caught_inputs_ = std::move(caught_inputs);
        delayed_controls_ = controls;
        operation.captures = std::move(captures.required);
        captures_ = captures.parent;
        delayed_ = was_delayed;
        deferred_inputs_ = previous_inputs;
        deferred_action_ = previous_action;
        return_inputs_ = previous_return_inputs;
        abrupt_ = outer;
      } else {
        const auto *prepared = prepared_call_;
        prepared_call_ = expr.children.front().get();
        const auto completed = child(0);
        prepared_call_ = prepared;
        const auto &call = operations.at(expr.children.front().get());
        if (completed == Type::never) { operation.type = Type::never; break; }
        if (!call.invocation.empty()) {
          for (const auto &[reason, effects] : call.invocation) {
            auto invoked = effects;
            if (reason >= 3) invoked.thrown = {reason - 3};
            summary.paths.push_back({0, {reason, false}, 0, std::move(invoked)});
          }
        } else summary.paths.push_back({0, {0, false}, 0});
      }
      summary.enforce_checked_boundary = (checking_method_ && checking_method_->source->implicit_scope) ||
          deferred_actions_with_checked_scopes_.contains(action);
      cleanup_.define(action, std::move(summary));
      note_abrupt(expr.at, {exception_id(ExceptionCategory::out_of_memory)});
      if (lexical_.live) {
        const auto reaching = CleanupEquations::stable_input_conditions(path_conditions(lexical_));
        for (const auto input : lexical_.inputs) {
          auto &queue = lexical_.input_facts.at(input).queues.at(cleanup_scope_);
          queue = cleanup_.push(queue, action, reaching);
        }
      }
      operation.type = Type::void_type;
      break;
    }
    default: implementation_gap(expr.at, "source expression '" + expr.text + "' is outside the scalar-method foundation");
    }
    if (operation.type == Type::never) lexical_.live = false;
    if (operation.view && !operation.selection_identity) operation.selection_identity = new_selection_identity();
    operations[&expr] = operation;
    return operation.type;
  }

  CallableChoices reaching_callables() const {
    CallableFlow flow;
    using Node = CallableFlow::Node;
    std::vector<Node> symbols;
    for (size_t i = 0; i < program_.symbols.size(); ++i) symbols.push_back(flow.node());
    using Facts = std::map<const Expression *, CheckedOperation>;
    struct Context {
      const Facts *facts;
      const std::vector<CheckedStorage> *storage;
      std::map<const Expression *, Node> expressions, observations;
      std::vector<Node> places;
      Node result;
    };
    std::vector<Context> contexts;
    const auto context = [&](const Facts &facts, const std::vector<CheckedStorage> &storage) {
      Context result{&facts, &storage, {}, {}, {}, flow.node()};
      for (size_t i = 0; i < storage.size(); ++i) result.places.push_back(flow.node());
      return result;
    };
    contexts.push_back(context(program_.entry, program_.entry_storage));
    for (const auto &instance : program_.instances) contexts.push_back(context(instance.operations, instance.storage));
    const auto expression = [&](size_t c, const Expression *expr) {
      auto &nodes = contexts[c].expressions;
      if (const auto found = nodes.find(expr); found != nodes.end()) return found->second;
      return nodes.emplace(expr, flow.node()).first->second;
    };
    std::function<void(size_t, const CheckedOperation &, Node)> value;
    value = [&](size_t c, const CheckedOperation &fact, Node node) {
      if (fact.callable) {
        flow.seed(node, *fact.callable);
        if (!fact.callable_captures.empty()) for (const auto target : program_.callables[*fact.callable].implementations()) {
          const auto &frame = program_.instances[target].frame;
          for (size_t i = 0; i < fact.callable_captures.size(); ++i)
            value(c, fact.callable_captures[i], contexts[target + 1].places.at(frame.captures.at(i)));
        }
      }
      if (fact.type.kind == Type::procedure)
        for (const auto place : fact.targets) if (place != null_target)
          flow.link(contexts[c].places.at(place), node);
    };
    const auto invocation = [&](size_t c, const Expression &expr, const CheckedOperation &fact, size_t target, size_t offset) {
      CallableFlow::Edges edges;
      const auto &instance = program_.instances[target - 1];
      for (size_t i = 0; i < instance.arguments.size(); ++i) {
        if (instance.arguments[i].kind != Type::procedure) continue;
        const auto *argument = &fact.argument(expr, i + offset);
        const auto &input = instance.frame.inputs.at(i);
        if (input.mode == ArgumentMode::value)
          edges.emplace_back(expression(c, argument), contexts[target].places.at(input.place));
        else {
          for (const auto place : fact.callable_argument_targets.at(i)) if (place != null_target) {
            edges.emplace_back(contexts[c].places.at(place), contexts[target].places.at(input.place));
            edges.emplace_back(contexts[target].places.at(input.place), contexts[c].places.at(place));
          }
        }
      }
      if (instance.result.type->kind == Type::procedure) edges.emplace_back(contexts[target].result, expression(c, &expr));
      if (fact.callee && !instance.environment.empty()) {
        for (size_t i = 0; i < instance.environment.size(); ++i)
          value(c, fact.callable_captures.at(i), contexts[target].places.at(instance.frame.captures.at(i)));
      }
      return edges;
    };
    // A family view admits every compatible complete callable in this closed
    // artifact, even when no observed call supplies that value to this target.
    for (const auto &family : program_.callables)
      for (const auto target : family.implementations()) {
        const auto &instance = program_.instances[target];
        for (size_t i = 0; i < instance.arguments.size(); ++i) if (instance.arguments[i].kind == Type::procedure)
          for (size_t id = 0; id < program_.callables.size(); ++id)
            if (program_.callables[id].type == instance.arguments[i])
              flow.seed(contexts[target + 1].places.at(instance.frame.inputs.at(i).place), id);
      }
    using K = Expression::Kind;
    for (size_t c = 0; c < contexts.size(); ++c) {
      for (const auto &[expr, fact] : *contexts[c].facts) {
        const auto node = expression(c, expr);
        value(c, fact, node);
        if ((expr->kind == K::effect || expr->kind == K::procedure_effect) && fact.publishes_effect &&
            fact.operand_type.kind == Type::procedure)
          flow.link(fact.callable ? node : expression(c, expr->children.front().get()), symbols.at(*fact.symbol));
        if (expr->kind == K::symbol && fact.type.kind == Type::procedure)
          flow.link(symbols.at(*fact.symbol), node);
        if ((expr->kind == K::pointer || expr->kind == K::callable) && fact.place &&
            contexts[c].storage->at(*fact.place).type.kind == Type::procedure) {
          flow.link(expr->kind == K::callable ? node : expression(c, expr->children.front().get()), contexts[c].places.at(*fact.place));
        }
        if (expr->kind == K::transfer && fact.operand_type.kind == Type::procedure) {
          const auto source = expression(c, expr->children[1].get());
          contexts[c].observations.emplace(expr, source);
          for (const auto place : fact.targets) if (place != null_target)
            flow.link(source, contexts[c].places.at(place));
        }
        if ((expr->kind == K::return_value || expr->kind == K::return_tail) && c && !expr->children.empty()) {
          const auto source = expression(c, expr->children.front().get());
          flow.link(source, contexts[c].result);
          if (fact.operand_type.kind == Type::procedure && !fact.borrowed_result)
            contexts[c].observations.emplace(expr, source);
        }
        if (expr->kind == K::return_test && fact.return_filter.can_match && fact.place && fact.operand_type.kind == Type::procedure) {
          flow.link(contexts[c].result, contexts[c].places.at(*fact.place));
          contexts[c].observations.emplace(expr, contexts[c].result);
        }
        if (fact.callee && fact.type != Type::never) {
          for (const auto &[from, to] : invocation(c, *expr, fact, *fact.callee + 1, 0)) flow.link(from, to);
        } else if (fact.callable_call && fact.type != Type::never) {
          const auto callee = flow.node(); contexts[c].observations.emplace(expr, callee);
          const size_t offset = expr->kind == K::value_call ? 1 : 0;
          if (offset) flow.link(expression(c, expr->children.front().get()), callee);
          else if (fact.symbol) flow.link(symbols.at(*fact.symbol), callee);
          else for (const auto place : fact.callable_targets) if (place != null_target)
            flow.link(contexts[c].places.at(place), callee);
          for (size_t id = 0; id < program_.callables.size(); ++id) {
            const auto &candidate = program_.callables[id];
            if (candidate.type == fact.operand_type) {
              std::vector<Type> arguments;
              for (size_t i = offset; i < fact.arguments.size(); ++i) arguments.push_back(fact.arguments[i].type);
              for (const auto &[inputs, target] : candidate.alternatives(arguments))
                flow.when(callee, id, invocation(c, *expr, fact, target + 1, offset));
            }
          }
        }
        if (fact.type.kind != Type::procedure || fact.callable || fact.callee || fact.callable_call) continue;
        const auto input = [&](const Expression *child) { flow.link(expression(c, child), node); };
        switch (expr->kind) {
        case K::grouped: input(expr->children.front().get()); break;
        case K::block:
          if (!expr->children.empty()) input(expr->children.back().get());
          break;
        case K::conditional:
          input(expr->children[1].get()); input(expr->children[2].get()); break;
        case K::try_scope:
          input(expr->children.front().get());
          for (const auto &child : expr->children) if (child->kind == K::handler) input(child->children.front().get());
          break;
        case K::fold:
          if (fact.accumulator) {
            const auto accumulator = contexts[c].places.at(*fact.accumulator);
            flow.link(expression(c, expr->children[2].get()), accumulator);
            flow.link(expression(c, expr->children[3].get()), accumulator);
            flow.link(accumulator, node);
          }
          break;
        case K::name: case K::symbol: case K::transfer: case K::rebind: break;
        default:
          // An uncovered value source must not narrow a call's implementation set.
          for (size_t id = 0; id < program_.callables.size(); ++id)
            if (program_.callables[id].type == fact.type) flow.seed(node, id);
          break;
        }
      }
    }
    flow.solve();
    CallableChoices result(contexts.size());
    for (size_t c = 0; c < contexts.size(); ++c)
      for (const auto &[expr, node] : contexts[c].observations)
        if (!flow.values(node).empty()) result[c].emplace(expr, flow.values(node));
    return result;
  }

  void begin_check(std::vector<CheckedStorage> &storage, std::vector<CheckedBinding> &bindings,
                   std::map<const Expression *, CheckedOperation> &operations, bool proving) {
    proving_ = proving; observed_ = {};
    storage_ = &storage; bindings_ = &bindings;
    storage.clear(); bindings.clear(); operations.clear(); definitions_.clear();
    if (!proving) {
      possible_targets_.clear(); possible_null_sources_.clear(); target_dependencies_.clear(); catalog_storage_.clear();
      rebound_.clear(); payload_writes_ = {}; loop_states_.clear();
    }
    lexical_ = {}; cleanup_ = {}; scope_storage_.clear(); checked_scope_owners_.clear(); read_sites_.clear(); external_owners_.clear();
    declared_sites_.clear(); declared_requirements_.clear();
    deferred_actions_with_checked_scopes_.clear();
    loop_controls_.clear(); delayed_controls_.clear(); deferred_inputs_.clear(); active_loops_.clear();
    caught_.reset(); catch_bindings_.clear(); caught_inputs_.clear();
    captures_ = nullptr; delayed_ = false; deferred_action_.reset(); return_inputs_ = {}; prepared_call_ = nullptr;
    next_finalizer_input_ = 2 * CleanupEquations::deferred_return_offset;
    cleanup_scope_ = 0; next_scope_ = 1;
    next_target_version_ = 0;
    next_selection_identity_ = 0; selection_sources_.clear(); incoming_selections_.clear(); return_selection_actions_.clear();
    next_boolean_observation_ = 0; conditional_selections_.clear();
  }

  template <typename Check> void check_flow(const Check &check) {
    const bool validating = validating_;
    validating_ = false;
    check(false);
    solve_targets();
    for (auto &[expr, loop] : loop_states_)
      for (const auto binding : intersect(loop.bindings, loop.rebound)) loop.initialized[binding] = {};
    if (!loop_states_.empty()) {
      // Entry and reaching backedges establish the header's possible targets
      // and sufficient initialization conditions. Nested loops share the
      // finite fixed point before read validation.
      do {
        loop_evidence_changed_ = false;
        check(true);
      } while (loop_evidence_changed_);
    }
    validating_ = validating;
    if (validating || loop_states_.empty()) {
      loop_evidence_changed_ = false;
      check(true);
      if (loop_evidence_changed_) throw std::logic_error("loop evidence changed during validation");
    }
  }

  void check_entry() {
    checking_context_ = 0;
    checking_method_ = nullptr;
    checking_instance_ = nullptr;
    check_flow([&](bool proving) {
      begin_check(program_.entry_storage, program_.entry_bindings, program_.entry, proving);
      AbruptFlow entry_flow{1, {}};
      abrupt_ = &entry_flow;
      std::vector<std::map<std::string, size_t>> bindings(modules_.size());
      for (const auto &[module, expr] : program_.initializers) {
        const ModuleLookup lookup(module_, module);
        lexical_.scopes.front().bindings.swap(bindings[module_]);
        if (check(*expr, nullptr, program_.entry, true, true) == Type::never) lexical_.live = false;
        for (const auto &[name, id] : lexical_.scopes.front().bindings) module_names().values.insert(name);
        lexical_.scopes.front().bindings.swap(bindings[module_]);
      }
      CheckedOperation entry_result; entry_result.type = lexical_.live ? Type{Type::void_type} : Type{Type::never};
      finish_scope(entry_result, entry_flow, nullptr, 0, program_.initializers.empty() ? Location{} : program_.initializers.front().expression->at);
      if (validating_ && lexical_.live)
        for (const auto &[symbol, extension] : entry_extensions_)
          if (!lexical_.bound_symbols.contains(symbol))
            implementation_gap(extension->at, "cannot prove an entry-scope installation for extended callable '@" + extension->name + "'");
    });
  }
  void observe_outcome(size_t reason, const LexicalState &state) {
    const auto completion = Completion{reason};
    if (completion.failed()) observed_.escaping_failures.insert(reason);
    if (completion.protected_here()) observed_.propagates_here_transfer = true;
    std::set<size_t> known;
    for (const auto id : state.initialized) if (storage(id).borrowed) known.insert(input_ordinal(id));
    CleanupEquations::MissingInitialization missing;
    for (size_t p = 0; p < checking_instance_->frame.inputs.size(); ++p) {
      const auto place = checking_instance_->frame.inputs[p].place;
      if (!storage(place).borrowed || known.contains(p)) continue;
      auto clauses = external_conditions(missing_initialization(state, place));
      if (clauses.empty()) known.insert(p);
      else if (clauses != CleanupEquations::ConditionClauses{CleanupEquations::ConditionFacts{}})
        missing.emplace(p, std::move(clauses));
    }
    const auto [found, inserted] = observed_.initialized.emplace(reason, known);
    if (inserted) observed_.uninitialized[reason] = std::move(missing);
    else CleanupEquations::combine_initialization(found->second, observed_.uninitialized[reason], known, missing, true);
    const auto present = input_ordinals(state.present);
    const auto [presence, first_presence] = observed_.present.emplace(reason, present);
    if (!first_presence) presence->second = intersect(presence->second, present);
    for (size_t p = 0; p < checking_instance_->frame.inputs.size(); ++p) {
      const auto place = checking_instance_->frame.inputs[p].place;
      if (storage(place).borrowed && storage(place).type == Type::boolean) {
        const auto proof = state.predicates.find(place);
        auto value = snapshot_boolean(proof == state.predicates.end() ? nullptr : proof->second, state, checking_method_->source->at);
        CleanupEquations::merge_boolean(observed_.booleans[reason][p], external_boolean(value));
      }
      if (!storage(place).borrowed || storage(place).type.kind != Type::exception) continue;
      const auto value = state.exceptions.find(place);
      observed_.payloads[reason][p].merge(value == state.exceptions.end()
          ? ExceptionFacts{exception_possibilities(storage(place).type)} : value->second);
    }
    for (size_t p = 0; p < checking_instance_->frame.inputs.size(); ++p) {
      const auto &input = checking_instance_->frame.inputs[p];
      if (!input.binding_slot) continue;
      const auto binding = storage(input.place).binding;
      if (checking_method_->scope_inputs && !rebound_.contains(binding)) continue;
      const auto [result, first] = observed_.bindings[reason].try_emplace(p);
      auto &targets = result->second;
      const bool initialized = state.selections.initialized.contains(binding) ||
          std::all_of(state.targets.at(binding).begin(), state.targets.at(binding).end(),
              [&](size_t place) { return place == null_target || state.initialized.contains(place); });
      targets.initialized = initialized && (first || targets.initialized);
      for (const auto place : state.targets.at(binding)) if (place != null_target) {
        if (!storage(place).borrowed) throw std::logic_error("caller binding retained local storage");
        targets.targets.insert(input_ordinal(place));
      }
      if (state.targets.at(binding).contains(null_target))
        for (const auto source : state.null_sources.at(binding))
          targets.null_sources.insert(source == null_target ? null_target : input_ordinal(source));
    }
    if (!reason && checking_instance_->result.borrowed()) {
      auto returned = state.returned;
      returned.enter_initialization(initialization_frame());
      returned.initialize(state.initialized, state.selections);
      returned.selection_bindings.clear();
      returned.saved_capture.reset();
      returned.borrowed_selection.reset();
      returned.initialization = external_initialization(returned.initialization);
      returned.targets = input_ordinals(returned.targets);
      returned.uninitialized = input_ordinals(returned.uninitialized);
      returned.null_sources = input_ordinals(returned.null_sources);
      for (auto &[null, clauses] : returned.nullness)
        clauses = CleanupEquations::conjoin_conditions(clauses, path_conditions(state));
      returned.nullness = external_boolean(returned.nullness);
      observed_.returned.merge(returned);
    }
    if (!reason && checking_instance_->result.type->kind == Type::exception) {
      if (checking_instance_->result.borrowed()) {
        for (const auto place : state.returned.targets) if (place != null_target) {
          const auto found = state.exceptions.find(place);
          observed_.returned.value.merge(found == state.exceptions.end()
              ? ExceptionFacts{exception_possibilities(storage(place).type)} : found->second);
        }
      } else observed_.returned.value.merge(state.returned.value);
    }
    if (!reason && !checking_instance_->result.borrowed() && *checking_instance_->result.type == Type::boolean) {
      CleanupEquations::Returned returned;
      for (const auto &[result, clauses] : state.returned.boolean)
        returned.boolean[result] = CleanupEquations::conjoin_conditions(clauses, path_conditions(state));
      returned.boolean = external_boolean(returned.boolean);
      observed_.returned.merge(returned);
    }
    for (const auto &[read, requirement] : state.requirements)
      if (!requirement.before.nonnull && std::ranges::any_of(requirement.inputs,
            [&](size_t input) { return state.inputs.contains(input); }))
        if (const auto place = cleanup_.read_target(read)) observe_read(*place, requirement.before);
  }
  CallEffects check_instance(size_t i, bool distinct_bindings = false) {
    checking_context_ = i + 1;
    auto &instance = program_.instances[i];
    checking_instance_ = &instance;
    const ModuleLookup lookup(module_, program_.methods[instance.method].module);
    const ModuleLookup context(interpretation_, program_.methods[instance.method].interpretation);
    const auto &method = *program_.methods[instance.method].source;
    checking_method_ = &program_.methods[instance.method];
    check_flow([&](bool proving) {
      begin_check(instance.storage, instance.bindings, instance.operations, proving);
      if (const auto &group = checking_method_->here_group; group && group->supplier_return.has_value())
        (*group->supplier_return ? return_inputs_.present : return_inputs_.absent).insert(0);
      instance.frame = {};
      // Entry bindings select fresh proof places. Seed their contract facts
      // together before deriving any facts for the body.
      auto &entry = lexical_.input_facts.at(0);
      for (size_t i = 0; i < checking_method_->input_owners().size(); ++i)
        external_owners_[checking_method_->input_owners()[i].name] = null_target - 1 - i;
      for (const auto &owner : checking_method_->input_owners()) {
        const auto id = bind(owner.name, Type::void_type, {}, true, true, true, owner.at, {}, true, &entry);
        bindings_->at(id).owner_binding = external_owners_.at(owner.name);
      }
      if (checking_method_->scope_inputs) {
        const auto &profile = *checking_method_->scope_inputs;
        for (const auto &[name, role] : profile.owner_names) {
          const auto &owner = profile.owners.at(role);
          const auto actual = external_owners_.at(owner.name);
          const auto id = bind(name, Type::void_type, {}, true, true, true, owner.at, {}, false, &entry);
          bindings_->at(id).owner_binding = actual;
          external_owners_.emplace(name, actual);
        }
      }
      for (size_t c = 0; c < instance.environment.size(); ++c) {
        const auto &capture = checking_method_->captures[c];
        const auto &input = instance.environment[c];
        const auto id = declare(capture.name, input.type, input.read_only, true, capture.at, false, &entry);
        instance.frame.captures.push_back(id);
        bindings_->back().callable_value = input.type.kind == Type::procedure;
        entry.initialized.insert(id);
      }
      for (size_t p = 0; p < checking_method_->input_parameters().size(); ++p) {
        const auto &parameter = checking_method_->input_parameters()[p];
        const bool value = parameter.declaration_kind == "val";
        const bool live_value = checking_method_->scope_inputs && checking_method_->scope_inputs->live_values.contains(p);
        const bool live_binding = checking_method_->scope_inputs && checking_method_->scope_inputs->live_bindings.contains(p);
        const auto mode = argument_mode(parameter);
        const auto id = declare(parameter.name, instance.arguments[p], parameter.read_only || (value && parameter.fixed_binding),
                                value || live_value, parameter.at, !checking_method_->scope_inputs, &entry);
        const auto binding = storage_->at(id).binding;
        instance.frame.inputs.push_back({id, mode, live_binding || caller_bindings(*checking_method_)});
        if (const auto &profile = checking_method_->scope_inputs; profile && profile->place_owners.contains(p)) {
          auto &owners = bindings_->at(binding).place_owners;
          owners.clear();
          for (const auto role : profile->place_owners.at(p)) owners.insert(null_target - 1 - role);
        }
        bindings_->back().fixed = parameter.fixed_binding;
        bindings_->back().requires_initialized = parameter.declaration_kind == "ref";
        bindings_->back().nullable = !live_value && mode == ArgumentMode::borrowed_ptr;
        bindings_->back().callable_value = instance.arguments[p].kind == Type::procedure && (parameter.owner.empty() || live_value);
        if (!parameter.owner.empty()) {
          const auto owner = external_owners_.at(parameter.owner);
          instance.frame.owners.try_emplace(owner, p);
          storage_->at(id).scope = owner; storage_->at(id).borrowed = true;
          selection_sources_.at(lexical_.selection_identities.at(binding)) = {p};
          incoming_selections_.emplace(id, lexical_.selection_identities.at(binding));
          std::erase(scope_storage_[0], id);
          lexical_.owners[binding] = {owner};
          const bool nonnull = checking_method_->scope_inputs && checking_method_->scope_inputs->nonnull.at(p);
          if (parameter.declaration_kind == "ptr" && !nonnull) {
            lexical_.targets[binding].insert(null_target);
            lexical_.null_sources[binding] = {id};
            entry.selections.assign(binding, selection_targets(lexical_.targets[binding], lexical_.null_sources.at(binding)));
            if (!proving) { possible_targets_[binding].insert(null_target); possible_null_sources_[binding] = {id}; }
          }
          if (nonnull || mode != ArgumentMode::borrowed_ptr) entry.present.insert(id);
        }
        if (mode != ArgumentMode::borrowed_ptr || (live_value && method.supplied_scope)) entry.initialized.insert(id);
        if (instance.arguments[p].kind == Type::exception)
          entry.exceptions.emplace(id, ExceptionFacts::input(p, exception_possibilities(instance.arguments[p])));
        if (instance.arguments[p] == Type::boolean) {
          auto proof = std::make_shared<Predicate>();
          proof->kind = Predicate::Kind::presence;
          for (const bool value : {false, true}) {
            CleanupEquations::ConditionFacts facts; facts.inputs.emplace(p, value);
            proof->results[value].insert(std::move(facts));
          }
          lexical_.predicates[id] = std::move(proof);
        }
      }
      if (checking_method_->scope_inputs) {
        for (const auto &[name, access] : checking_method_->scope_inputs->place_access) {
          const auto input = storage(instance.frame.inputs.at(access.input).place).binding;
          const auto canonical = bindings_->at(input);
          const auto &at = checking_method_->input_parameters()[access.input].at;
          const auto binding = bind(name, canonical.type, {}, access.read_only, canonical.value_binding,
                                    access.fixed, at, input, false, &entry);
          bindings_->at(binding).requires_initialized = canonical.requires_initialized;
          bindings_->at(binding).nullable = canonical.nullable;
          bindings_->at(binding).callable_value = canonical.callable_value;
        }
        for (const auto &owner : checking_method_->input_owners())
          instance.frame.owners.try_emplace(external_owners_.at(owner.name), null_target);
        for (const auto [a, b] : checking_method_->scope_inputs->overlapping_input_bindings) {
          const auto left = storage(instance.frame.inputs.at(a).place).binding;
          const auto right = storage(instance.frame.inputs.at(b).place).binding;
          bindings_->at(left).overlaps.insert(right);
          bindings_->at(right).overlaps.insert(left);
        }
        const auto binding_snapshot = [&](size_t ordinal) {
          const auto binding = storage(instance.frame.inputs.at(ordinal).place).binding;
          return CleanupEquations::BindingSnapshot{binding, lexical_.target_versions.at(binding)};
        };
        CleanupEquations::ConditionClauses captured;
        for (const auto &condition : checking_method_->scope_inputs->reference_conditions) {
          CleanupEquations::ConditionFacts facts;
          for (const auto ordinal : condition.present) facts.bindings.insert(binding_snapshot(ordinal));
          for (const auto ordinal : condition.absent) facts.absent_bindings.insert(binding_snapshot(ordinal));
          for (const auto &group : condition.same_inputs) {
            std::set<size_t> identities;
            std::set<CleanupEquations::BindingSnapshot> bindings;
            for (const auto ordinal : group) {
              auto selected = binding_snapshot(ordinal);
              identities.insert(lexical_.selection_identities.at(selected.first));
              bindings.insert(std::move(selected));
            }
            facts.same_bindings.insert(std::move(bindings));
            facts.same_selections.insert(std::move(identities));
          }
          CleanupEquations::add_condition(captured, std::move(facts));
        }
        if (!captured.empty()) lexical_.predicate_conditions = CleanupEquations::conjoin_conditions(lexical_.predicate_conditions, captured);
        for (const auto &[input, observations] : checking_method_->scope_inputs->boolean_conditions) {
          CleanupEquations::BooleanValue proof;
          for (const auto &[truth, clauses] : observations) for (const auto &clause : clauses) {
            CleanupEquations::ConditionFacts facts;
            facts.inputs = clause.inputs;
            facts.inputs.emplace(input, truth);
            for (const auto ordinal : clause.present) facts.bindings.insert(binding_snapshot(ordinal));
            for (const auto ordinal : clause.absent) facts.absent_bindings.insert(binding_snapshot(ordinal));
            CleanupEquations::add_condition(proof[truth], std::move(facts));
          }
          lexical_.predicates[instance.frame.inputs.at(input).place] = boolean_predicate(proof);
        }
      }
      if (!distinct_bindings && caller_bindings(*checking_method_)) for (const auto &a : instance.frame.inputs)
        for (const auto &b : instance.frame.inputs) {
          const auto left = storage(a.place).binding, right = storage(b.place).binding;
          if (left != right && bindings_->at(left).type == bindings_->at(right).type &&
              bindings_->at(left).read_only == bindings_->at(right).read_only)
            bindings_->at(left).overlaps.insert(right);
        }
      refresh_input_facts(lexical_);
      AbruptFlow flow{1, {}}, outcomes{1, {}};
      abrupt_ = &flow;
      const auto incoming = lexical_;
      CheckedOperation completed;
      const bool has_feedback = checking_method_->here_group && instance.result.type != Type::void_type;
      const auto end = check(*method.body, &instance, instance.operations, true, !has_feedback,
          has_feedback ? &*instance.result.type : nullptr);
      if (has_feedback && end != Type::never && lexical_.live && !transfers_value(end, *instance.result.type))
        fail(method.body->at, "here body fallthrough requires feedback of type '" + type_name(*instance.result.type) + "'");
      completed.type = end == Type::never ? Type{Type::never} : Type{Type::void_type};
      const auto completion = finish_scope(completed, flow, &outcomes, 0, method.at).type;
      const bool requires_return = method.completion_requires_return(*instance.result.type);
      if (validating_ && proving && requires_return && completion != Type::never &&
          warned_missing_return_.insert(instance.method).second)
        program_.warnings.push_back({Warning::missing_return, method.at, method.requires_explicit_return()
            ? "procedure may finish without an explicit return" : "callable may finish without a result"});
      if (proving) {
        abrupt_ = nullptr;
        // Complete capture preparation can fail before the body registers any
        // cleanup or establishes output facts. Retain this incoming outcome
        // for deferred invocation as well as ordinary calls, independently of
        // whether a particular native body can elide the acquisition.
        if (std::any_of(instance.environment.begin(), instance.environment.end(),
              [](const auto &capture) { return capture.type.kind == Type::procedure; }))
          observe_outcome(3 + exception_id(ExceptionCategory::out_of_memory), incoming);
        for (const auto requirement : outcomes.declared) note_declared(requirement);
        if (completion != Type::never && lexical_.live)
          observe_outcome(requires_return ? 3 + exception_id(ExceptionCategory::missing_return) : 0, lexical_);
        const auto normal = lexical_;
        for (const auto &path : outcomes.paths) {
          observe_outcome(path.completion.reason == 1 ? 0 : path.completion.reason, path.state);
          lexical_ = path.state; abrupt_ = nullptr;
          forward_abrupt(path);
        }
        lexical_ = normal;
      }
    });
    instance.complete = validating_;
    for (const auto place : payload_writes_.places)
      if (storage(place).borrowed) observed_.written_inputs.insert(input_ordinal(place));
    std::erase_if(observed_.uninitialized, [](const auto &entry) { return entry.second.empty(); });
    for (const auto reason : call_reasons(*checking_method_)) observed_.initialized.try_emplace(reason);
    for (const auto reason : call_reasons(*checking_method_)) observed_.present.try_emplace(reason);
    if (!instance.result.borrowed() && *instance.result.type == Type::boolean && observed_.returned.boolean.empty())
      observed_.returned.boolean = CleanupEquations::unknown_boolean();
    if (instance.result.nullable() && observed_.returned.nullness.empty())
      observed_.returned.nullness = CleanupEquations::unknown_boolean();
    auto initial = initial_effects(*checking_method_, instance.arguments);
    for (const auto &[reason, values] : initial.booleans)
      for (const auto &[parameter, value] : values) observed_.booleans[reason].try_emplace(parameter, CleanupEquations::unknown_boolean());
    for (const auto &[reason, payloads] : initial.payloads)
      for (const auto &[parameter, value] : payloads) observed_.payloads[reason].try_emplace(parameter, value);
    if (caller_bindings(*checking_method_) || checking_method_->scope_inputs) {
      for (size_t parameter = 0; parameter < instance.frame.inputs.size(); ++parameter) {
        const auto &input = instance.frame.inputs[parameter];
        if (!input.binding_slot || (checking_method_->scope_inputs && !rebound_.contains(storage(input.place).binding))) continue;
        auto &sources = observed_.binding_sources[parameter];
        for (const auto place : possible_targets_.at(storage(input.place).binding)) if (place != null_target) {
          if (!storage(place).borrowed)
            implementation_gap(method.at, "caller-binding access source without input provenance");
          sources.insert(input_ordinal(place));
        }
        if (possible_null_sources_.at(storage(input.place).binding).contains(null_target)) sources.insert(null_target);
      }
      // Recursive closure retains conservative absent outcomes, while a slot
      // with no possible mutation preserves its entry selection on every exit.
      for (auto &[reason, targets] : initial.bindings) {
        for (auto &[parameter, result] : targets) {
          const auto &input = instance.frame.inputs.at(parameter);
          if (!rebound_.contains(storage(input.place).binding)) {
            result.targets = {parameter};
            result.null_sources = input.mode == ArgumentMode::borrowed_ptr ? std::set<size_t>{parameter} : std::set<size_t>{};
            result.initialized = input.mode != ArgumentMode::borrowed_ptr;
          }
        }
        observed_.bindings.try_emplace(reason, std::move(targets));
      }
    }
    if (observed_.returned.value.unresolved())
      throw std::logic_error("unresolved cleanup exception origin in callable result");
    for (const auto &[reason, payloads] : observed_.payloads) for (const auto &[parameter, value] : payloads)
      if (value.unresolved())
        throw std::logic_error("unresolved cleanup exception origin in borrowed payload");
    const auto resolved_boolean = [](const auto &value) {
      for (const auto &[truth, clauses] : value) for (const auto &facts : clauses)
        if (!facts.payloads.empty() || !facts.bindings.empty() || !facts.absent_bindings.empty() || !facts.same_bindings.empty() || !facts.same_selections.empty() || !facts.different_selections.empty() || !facts.conditions.terms.empty() ||
            std::any_of(facts.exceptions.begin(), facts.exceptions.end(), [](const auto &value) { return value.unresolved(); }))
        throw std::logic_error("unresolved cleanup boolean origin in callable summary");
    };
    resolved_boolean(observed_.returned.boolean);
    resolved_boolean(observed_.returned.nullness);
    for (const auto &[reason, values] : observed_.booleans)
      for (const auto &[parameter, value] : values) resolved_boolean(value);
    return observed_;
  }

  static bool merge_effects(CallEffects &known, const CallEffects &effects, bool reads) {
    bool changed = false;
    if (reads) {
      if (known.reads != effects.reads) { known.reads = effects.reads; changed = true; }
      if (known.symbols != effects.symbols) { known.symbols = effects.symbols; changed = true; }
    } else {
      const auto written_count = known.written_inputs.size();
      known.written_inputs.insert(effects.written_inputs.begin(), effects.written_inputs.end());
      changed = known.written_inputs.size() != written_count;
      const auto failure_count = known.escaping_failures.size();
      known.escaping_failures.insert(effects.escaping_failures.begin(), effects.escaping_failures.end());
      changed |= known.escaping_failures.size() != failure_count;
      if (effects.requires_deferred_failure_delivery && !known.requires_deferred_failure_delivery) {
        known.requires_deferred_failure_delivery = true;
        changed = true;
      }
      if (known.initialized != effects.initialized) { known.initialized = effects.initialized; changed = true; }
      if (known.uninitialized != effects.uninitialized) { known.uninitialized = effects.uninitialized; changed = true; }
      if (known.present != effects.present) { known.present = effects.present; changed = true; }
      if (known.scope_checked_failures != effects.scope_checked_failures) {
        known.scope_checked_failures = effects.scope_checked_failures;
        changed = true;
      }
      if (known.breaks_here_call != effects.breaks_here_call || known.returns_from_supplier != effects.returns_from_supplier) {
        known.breaks_here_call = effects.breaks_here_call;
        known.returns_from_supplier = effects.returns_from_supplier;
        changed = true;
      }
      if (effects.propagates_here_transfer && !known.propagates_here_transfer) {
        known.propagates_here_transfer = true;
        changed = true;
      }
      if (known.protected_defer_failure != effects.protected_defer_failure) {
        known.protected_defer_failure = effects.protected_defer_failure;
        changed = true;
      }
      if (known.booleans != effects.booleans) { known.booleans = effects.booleans; changed = true; }
      auto returned = effects.returned;
      returned.value.merge(known.returned.value);
      if (known.returned != returned) { known.returned = std::move(returned); changed = true; }
      if (known.bindings != effects.bindings) { known.bindings = effects.bindings; changed = true; }
      if (known.binding_sources != effects.binding_sources) { known.binding_sources = effects.binding_sources; changed = true; }
      auto payloads = effects.payloads;
      for (const auto &[reason, values] : known.payloads)
        for (const auto &[parameter, value] : values) payloads[reason][parameter].merge(value);
      if (known.payloads != payloads) { known.payloads = std::move(payloads); changed = true; }
    }
    return changed;
  }
  bool close_effects(bool reads) {
    std::deque<size_t> pending;
    std::set<size_t> queued;
    const auto enqueue = [&](size_t context) {
      if (queued.insert(context).second) pending.push_back(context);
    };
    enqueue(0);
    size_t instances = 0, callables = program_.callables.size();
    const auto discover = [&] {
      while (instances < program_.instances.size()) enqueue(++instances);
      if (callables != program_.callables.size()) {
        callables = program_.callables.size();
        for (const auto context : callable_effect_consumers_) enqueue(context);
      }
    };
    discover();
    while (!pending.empty()) {
      const auto context = pending.front(); pending.pop_front(); queued.erase(context);
      if (!context) check_entry();
      else {
        const auto instance = context - 1;
        bool distinct_changed = false;
        if (auto it = distinct_binding_effects_.find(instance); it != distinct_binding_effects_.end()) {
          const auto effects = check_instance(instance, true);
          distinct_changed = merge_effects(it->second, effects, reads);
        }
        const auto effects = check_instance(instance);
        if (merge_effects(program_.instances[instance].effects, effects, reads) || distinct_changed)
          for (const auto dependent : effect_dependents_[instance]) enqueue(dependent);
      }
      // New obligations must establish their write guarantees before any read
      // closure can rely on them. Callable choices can also change guarantees.
      if (reads && (instances != program_.instances.size() || callables != program_.callables.size())) return false;
      discover();
    }
    return true;
  }

  size_t adapt_method(size_t method, size_t receiver, std::optional<size_t> family = {}, std::optional<size_t> parent = {}) {
    const auto &original = program_.methods[method];
    if (original.module == receiver) return method;
    if (original.interpretation && interpretations_[original.interpretation].receiver == receiver &&
        (!family || family == original.family) && (!parent || parent == original.parent)) return method;
    const auto key = std::tuple{method, receiver, family.value_or(null_target), parent.value_or(null_target)};
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
    if (family) adapted.family = family;
    if (parent) adapted.parent = parent;
    adapted.instances.clear(); adapted.nested.clear();
    adapted.captures.clear(); adapted.queries.clear(); adapted.captures_resolved = false;
    const auto id = program_.methods.size();
    program_.methods.push_back(std::move(adapted));
    adapted_methods_.emplace(key, id);
    for (const auto child : original.nested) {
      const auto nested = adapt_method(child, receiver, {}, id);
      program_.methods[id].nested.push_back(nested);
      lexical_variants_.emplace(std::pair{program_.methods[nested].source, id}, nested);
    }
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

  const FamilyView &prepare_family_view(size_t family, size_t context) {
    const auto key = std::pair{family, context};
    if (const auto found = family_views_.find(key); found != family_views_.end()) return found->second;
    if (!context) return family_views_.emplace(key, FamilyView{family, program_.families[family].methods, {}}).first->second;
    const auto interpretation = interpretations_[context];
    const auto inherited = prepare_family_view(family, interpretation.parent);
    const auto &receiver = modules_.at(interpretation.receiver);
    std::set<size_t> selected;
    for (const auto &[name, id] : receiver.callables) selected.insert(id);
    FamilyView view{inherited.identity, {}, inherited.unproved};
    for (const auto method : inherited.methods) view.methods.push_back(adapt_method(method, interpretation.receiver));
    std::optional<size_t> replacement;
    // An explicit receiver import is the receiver's interpretation of that identity.
    // A separately named using view retains its own identity and context.
    if (const auto imported = receiver.imports.find(inherited.identity); imported != receiver.imports.end()) {
      view.identity = imported->second;
      replacement = view.identity;
    } else if (selected.contains(inherited.identity)) replacement = inherited.identity;
    if (replacement) for (const auto method : program_.families[*replacement].methods) {
      const auto previous = std::find_if(view.methods.begin(), view.methods.end(), [&](size_t candidate) {
        return program_.methods[candidate].definition == program_.methods[method].definition;
      });
      if (previous == view.methods.end()) view.methods.push_back(method);
      else *previous = method;
    }
    std::map<size_t, size_t> definitions;
    for (const auto method : view.methods) definitions.emplace(program_.methods[method].definition, method);
    const auto retained = definitions;
    for (const auto id : selected) {
      if (program_.families[id].origin != program_.families[inherited.identity].origin) continue;
      for (const auto method : program_.families[id].methods) {
        const auto definition = program_.methods[method].definition;
        if (retained.contains(definition)) continue;
        const auto [found, added] = definitions.emplace(definition, method);
        if (added) view.methods.push_back(method);
        else if (found->second != method)
          view.unproved = "multiple selected interpretations of one inherited definition require an independent equivalence proof";
      }
    }
    return family_views_.emplace(key, std::move(view)).first->second;
  }

  void prepare_interpretations() {
    // Resolve source names by identity before checking bodies. This closure adds
    // interpretation records, never source definitions or runtime registrations.
    for (size_t id = 0; id < program_.methods.size(); ++id) {
      const auto &method = program_.methods[id];
      if (!method.interpretation) continue;
      const auto &names = modules_.at(method.module).callables;
      const std::function<void(const Expression &)> visit = [&](const Expression &expr) {
        if (const auto found = names.find(expr.text); found != names.end())
          prepare_family_view(found->second, method.interpretation);
        for (const auto &child : expr.children) visit(*child);
      };
      visit(*method.source->body);
      for (const auto &capture : method.source->captures)
        if (const auto found = names.find(capture.name); found != names.end()) prepare_family_view(found->second, method.interpretation);
    }
  }

  void publish(const std::string &name, const ModuleNames::Resource &resource, const Location &at) {
    const auto [published, inserted] = module_names().exports.emplace(name, resource);
    if (!inserted && published->second != resource) fail(at, "conflicting public alias '" + name + "'");
  }

  void select_module_resource(const Form &form, size_t dependency) {
    using Resource = ModuleNames::Resource;
    const bool importing = form.kind == Form::Kind::import_resource;
    if (form.kind != Form::Kind::using_resource && form.kind != Form::Kind::symbol_from && !importing)
      implementation_gap(form.at, "mixin body redefinition remains unproved");
    const auto &exports = modules_.at(dependency).exports;
    const auto found = exports.find(form.name);
    if (found == exports.end()) fail(form.at, "source module does not export '" + form.name + "'");
    auto resource = found->second;
    if (form.symbolic != (resource.kind == Resource::Kind::symbol))
      fail(form.at, "selected resource does not match the requested symbol or lexical category");
    if (resource.kind == Resource::Kind::symbol) {
      if (form.kind == Form::Kind::using_resource && !form.using_symbol)
        implementation_gap(form.at, "symbolic destination selection; select its identity with 'symbol from' and install explicitly");
      if (importing) {
        const auto destination = module_names().symbols.find(form.alias);
        if (destination == module_names().symbols.end())
          fail(form.at, "select the destination symbol identity before importing its callable family");
        if (destination->second != resource.symbol)
          implementation_gap(form.at, "callable import into a different symbol identity; use explicit effect installation for a value");
        if (!resource.family) fail(form.at, "symbol does not export a callable family");
      }
      const auto [alias, inserted] = module_names().symbols.emplace(form.alias, resource.symbol);
      if (!inserted && alias->second != resource.symbol) fail(form.at, "conflicting symbol alias '" + form.alias + "'");
      bool receiver_import = false;
      if (resource.family) {
        const auto source = *resource.family;
        const auto family = module_names().families.try_emplace(resource.symbol, source).first;
        const auto previous = module_names().imports.find(source);
        receiver_import = previous != module_names().imports.end() && previous->second == family->second;
        if (family->second != source && !receiver_import)
          implementation_gap(form.at, "composition of distinct selected interpretations for one symbol");
        if (importing) {
          resource.family = import_family(source, form.at);
          family->second = *resource.family;
          receiver_import = true;
        }
      }
      if (receiver_import) module_names().selected_symbols.erase(resource.symbol);
      else module_names().selected_symbols.insert(resource.symbol);
    } else if (resource.kind == Resource::Kind::exception) {
      const auto [alias, inserted] = module_names().exception_types.emplace(form.alias, resource.type);
      if (!inserted && alias->second != resource.type) fail(form.at, "conflicting type alias '" + form.alias + "'");
    } else {
      if (importing) resource.family = import_family(*resource.family, form.at);
      const auto [alias, inserted] = module_names().callables.emplace(form.alias, *resource.family);
      if (!inserted && alias->second != *resource.family)
        implementation_gap(form.at, "same-alias callable composition remains unproved");
      if (importing) module_names().imported_aliases.insert(form.alias);
      else module_names().selected_callables.insert(form.alias);
    }
    if (form.exporting) publish(form.alias, resource, form.at);
  }

  void publish_module(const SourceModule &source) {
    using Resource = ModuleNames::Resource;
    for (const auto &form : source.forms) if (form->kind == Form::Kind::export_resource) {
      Resource resource{};
      if (form->symbolic) {
        const auto found = module_names().symbols.find(form->name);
        if (found == module_names().symbols.end()) fail(form->at, "unknown exported symbol '" + form->name + "'");
        resource.kind = Resource::Kind::symbol; resource.symbol = found->second;
        if (const auto family = module_names().families.find(resource.symbol); family != module_names().families.end()) resource.family = family->second;
      } else if (const auto found = module_names().callables.find(form->name); found != module_names().callables.end()) {
        resource.kind = Resource::Kind::callable; resource.family = found->second;
      } else if (const auto found = module_names().exception_types.find(form->name); found != module_names().exception_types.end()) {
        resource.kind = Resource::Kind::exception; resource.type = found->second;
      } else fail(form->at, "unknown exported resource '" + form->name + "'");
      publish(form->name, resource, form->at);
    }
  }

  bool cancels_module_installation(size_t symbol) const {
    return module_ != 0 && entry_extensions_.contains(symbol);
  }
  void check_dependency_lifetime(const Expression &expr, bool iteration = false) const {
    using K = Expression::Kind;
    bool retained_effect = !iteration && (expr.kind == K::effect || expr.kind == K::procedure_effect);
    if (retained_effect) {
      const auto symbol = module_names().symbols.find(expr.text);
      if (symbol != module_names().symbols.end()) retained_effect = !cancels_module_installation(symbol->second);
    }
    if (expr.kind == K::deferred || expr.kind == K::pointer_group || expr.kind == K::alias ||
        expr.kind == K::region_binding || (expr.kind == K::callable && !expr.definition->name.empty()) || retained_effect)
      implementation_gap(expr.at, "retained dependency initialization scope remains unproved; use an explicit full scope for resources that end during initialization");
    for (size_t i = 0; i < expr.children.size(); ++i) {
      const bool loop_body = (expr.kind == K::range_for && i == 2) || (expr.kind == K::fold && i == 3);
      check_dependency_lifetime(*expr.children[i], iteration || loop_body);
    }
  }

  void plan_entry_installations() {
    if (entry_extensions_.empty()) return;
    using K = Expression::Kind;
    const auto &symbols = modules_.front().symbols;
    std::set<size_t> explicit_installs;
    const std::function<void(const Expression &)> collect = [&](const Expression &expr) {
      if (expr.kind == K::callable || expr.kind == K::deferred) return;
      if (expr.kind == K::effect || expr.kind == K::procedure_effect)
        if (const auto symbol = symbols.find(expr.text); symbol != symbols.end()) explicit_installs.insert(symbol->second);
      const size_t begin = expr.kind == K::make_local && !expr.annotation ? 1 : 0;
      const auto end = expr.kind == K::range_for ? 2 : expr.kind == K::fold ? 3 : expr.children.size();
      for (size_t i = begin; i < end; ++i) collect(*expr.children[i]);
    };
    for (const auto &[module, expression] : program_.initializers)
      if (!module) collect(*expression);
    for (const auto &[symbol, extension] : entry_extensions_)
      if (!explicit_installs.contains(symbol))
        fail(extension->at, "entry module extends callable '@" + extension->name + "' without an explicit installation");
  }

  void plan_initialization(const SourceProgram &source) {
    std::set<size_t> incorporated;
    struct Pending { size_t module, next_form = 0; bool execution_started = false; };
    std::vector<Pending> pending;
    const auto incorporate = [&](size_t module) {
      if (!incorporated.insert(module).second) return;
      program_.execution_order.push_back(module);
      pending.push_back({module});
    };
    incorporate(0);
    while (!pending.empty()) {
      auto &current = pending.back();
      const auto module = current.module;
      const auto &unit = source.modules[module];
      if (current.next_form == unit.forms.size()) { pending.pop_back(); continue; }
      const auto &form = unit.forms[current.next_form++];
      if (const auto dependency = unit.dependencies.find(form.get()); dependency != unit.dependencies.end()) {
        if (current.execution_started)
          implementation_gap(form->at, "source selections after execution expressions require first-reference incorporation analysis");
        incorporate(dependency->second);
        continue;
      }
      if (form->kind != Form::Kind::expression) continue;
      current.execution_started = true;
      if (module) {
        const ModuleLookup lookup(module_, module);
        check_dependency_lifetime(*form->body);
      }
      program_.initializers.push_back({module, form->body.get()});
    }
    plan_entry_installations();
  }

  void register_module(const SourceModule &source, std::optional<size_t> &print_symbol,
                       std::optional<Location> &unsupported_here) {
    const auto &forms = source.forms;
    for (const auto &form : forms)
      if (const auto dependency = source.dependencies.find(form.get()); dependency != source.dependencies.end())
        select_module_resource(*form, dependency->second);
    for (const auto &form : forms) {
      if ((form->kind != Form::Kind::using_resource && form->kind != Form::Kind::import_resource) || form->path != "std:exceptions" || form->using_symbol) continue;
      const auto found = std::find_if(exception_categories.begin(), exception_categories.end(), [&](const ExceptionDescriptor &entry) { return form->name == entry.name; });
      if (found == exception_categories.end()) fail(form->at, "unknown intrinsic exception selection '" + form->name + "'");
      if (!module_names().exception_types.emplace(form->alias, exception_type(exception_id(found->category))).second) fail(form->at, "duplicate exception type selection");
      if (form->exporting) {
        ModuleNames::Resource resource{};
        resource.kind = ModuleNames::Resource::Kind::exception;
        resource.type = exception_type(exception_id(found->category));
        publish(form->alias, resource, form->at);
      }
      program_.exception_values = true;
    }
    std::vector<const Form *> definitions;
    for (const auto &form : forms) {
      if (source.dependencies.contains(form.get())) continue;
      if (form->kind == Form::Kind::expression || form->kind == Form::Kind::export_resource) continue;
      if ((form->kind == Form::Kind::using_resource || form->kind == Form::Kind::import_resource) && form->path == "std:exceptions" && !form->using_symbol) continue;
      if (form->kind == Form::Kind::using_resource && form->using_symbol && form->path == "std:io" && form->name == "print") {
        if (!print_symbol) {
          print_symbol = program_.symbols.size();
          program_.symbols.push_back({{}, true, "print"});
        }
        if (!module_names().symbols.emplace(form->alias, *print_symbol).second) fail(form->at, "duplicate print symbol binding");
        continue;
      }
      if (form->kind == Form::Kind::symbol) {
        if (form->symbol_owner) implementation_gap(form->at, "regional symbol ownership contracts");
        if (module_names().exception_types.contains(form->name)) implementation_gap(form->at, "same-scope type and symbol declarations");
        const auto type = resolve_type(*form->result_type);
        CheckedMethod contract; contract.source = form.get();
        signature_type(contract, type, form->at);
        if (!module_names().symbols.emplace(form->name, program_.symbols.size()).second)
          fail(form->at, "duplicate symbol binding '" + form->name + "'");
        program_.symbols.push_back({type, false, form->name});
        continue;
      }
      if (form->kind != Form::Kind::function && form->kind != Form::Kind::procedure)
        implementation_gap(form->at, "module declaration or selection '" + form->name + "'");
      definitions.push_back(form.get());
    }
    std::map<const Form *, const Form *> parents;
    std::map<const Form *, size_t> method_ids;
    const std::function<void(const Expression &, const Form *)> collect = [&](const Expression &expr, const Form *parent) {
      if (expr.kind == Expression::Kind::here_body && !expr.definition) {
        if (!unsupported_here) unsupported_here = expr.at;
        return;
      }
      if (expr.definition) {
        if (expr.kind == Expression::Kind::procedure_effect) {
          if (parent) implementation_gap(expr.at, "symbolic family definitions inside callable bodies");
        } else lexical_methods_.emplace(expr.definition.get(), 0);
        if (parent) parents.emplace(expr.definition.get(), parent);
        definitions.push_back(expr.definition.get());
        if (expr.definition->body) collect(*expr.definition->body, expr.definition.get());
      }
      for (const auto &child : expr.children) collect(*child, parent);
    };
    for (const auto &form : forms) if (form->body)
      collect(*form->body, form->kind == Form::Kind::expression ? nullptr : form.get());
    for (const auto *form : definitions) {
      const auto *here = !form->parameters.empty() && form->parameters.front().here ? &form->parameters.front() : nullptr;
      std::string operation = form->operation ? form->name : "";
      if (!form->symbolic && !lexical_methods_.contains(form))
        if (const auto family = module_names().callables.find(form->name); family != module_names().callables.end())
          operation = program_.families[family->second].operation;
      if (identity_operation(operation))
        fail(form->at, "reference identity is an intrinsic operation and cannot have user methods");
      if ((!operation.empty() && !eager_operation(operation) && !swap_operation(operation)) || form->conversion || (!lexical_methods_.contains(form) && !form->captures.empty()) || (!form->body && lexical_methods_.contains(form)))
        implementation_gap(form->at, "callable captures, effects, declarations, operators, owners or throws contracts");
      if (!operation.empty()) {
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
      if (here) prepared.here = here_contract(prepared, *here);
      if (form->result_type) prepared.result.type = resolve_signature(*form->result_type);
      prepared.result.owner = form->result_owner;
      if (!form->result_owner.empty()) {
        prepared.result.mode = form->result_kind == "ptr" ? ResultMode::borrowed_ptr : ResultMode::borrowed_ref;
        prepared.result.read_only = form->result_const;
      }
      if (form->result_kind == "val" && prepared.result.type && !scalar_type(*prepared.result.type))
        implementation_gap(form->at, "explicit val results for non-scalar or generic types");
      if (prepared.result.type && !prepared.result.borrowed() && *prepared.result.type != Type::void_type && prepared.result.type->kind != Type::procedure && !scalar_type(*prepared.result.type)) {
        if (!form->result_const) implementation_gap(form->at, "default result mode for non-scalar or generic types");
      }
      if (prepared.result.type) signature_type(prepared, *prepared.result.type, form->at, true);
      for (const auto &type : form->throws_types) {
        const auto resolved = resolve_type(type);
        if (resolved.kind != Type::exception) implementation_gap(form->at, "throws contracts require selected intrinsic exception types");
        const auto categories = exception_possibilities(resolved);
        prepared.throws_categories.insert(categories.begin(), categories.end());
        prepared.throws_types.push_back(resolved);
      }
      std::set<std::string> names;
      if (here) names.insert(here->name);
      for (const auto &parameter : prepared.input_parameters()) {
        if (!names.insert(parameter.name).second) fail(parameter.at, "duplicate parameter name");
        if (form->supplied_scope) continue; // The selected receiver supplies this signature.
        if (!parameter.owner.empty() && !owners.contains(parameter.owner)) fail(parameter.at, "unknown parameter owner '" + parameter.owner + "'");
        if (parameter.variadic && form->body) implementation_gap(parameter.at, "variadic method bodies");
        if (parameter.automatic_conversion) implementation_gap(parameter.at, "automatic parameter conversion contract");
        prepared.parameters.push_back(resolve_signature(parameter.type));
        signature_type(prepared, prepared.parameters.back(), parameter.at);
      }
      for (const auto &parameter : form->type_parameters)
        if (parameter.bound) implementation_gap(parameter.at, "bounded type parameters");
      std::optional<size_t> symbol;
      if (form->symbolic) {
        if (here) implementation_gap(here->at, "here signatures of symbolic callable families");
        if (module_names().exception_types.contains(form->name)) implementation_gap(form->at, "same-scope type and symbol declarations");
        if (const auto found = module_names().symbols.find(form->name); found != module_names().symbols.end()) symbol = found->second;
        else {
          if (!prepared.type_parameters.empty() || !prepared.result.type)
            implementation_gap(form->at, "implicit symbolic declaration requires an independently established concrete signature");
          const auto type = callable_type(prepared, prepared.parameters, prepared.result);
          signature_type(prepared, type, form->at);
          symbol = program_.symbols.size(); module_names().symbols.emplace(form->name, *symbol);
          program_.symbols.push_back({type, false, form->name});
        }
        const auto &declared = program_.symbols[*symbol];
        if (!declared.type || declared.type->kind != Type::procedure || declared.type->pure_callable)
          fail(form->at, "symbolic procedure definition requires a procedure symbol");
      }
      const bool lexical = lexical_methods_.contains(form);
      if (symbol && module_names().selected_symbols.contains(*symbol))
        fail(form->at, "using and symbol selection do not grant callable extension permission");
      if (!symbol && !lexical && module_names().selected_callables.contains(form->name))
        fail(form->at, "using does not grant callable extension permission");
      const auto new_family = [&] {
        const auto family = program_.families.size();
        program_.families.push_back({symbol, module_, family, form->at, {}, {}, {}, operation});
        return family;
      };
      if (symbol) {
        if (!module_names().families.contains(*symbol)) module_names().families.emplace(*symbol, new_family());
        prepared.family = module_names().families.at(*symbol);
        const auto &family = program_.families[*prepared.family];
        if (form->body && family.module == module_ && family.imported_from) {
          module_names().extensions[*prepared.family].emplace(form->name, form->at);
          if (!module_) entry_extensions_.try_emplace(*symbol, form);
        }
      } else if (!lexical) {
        if (!module_names().callables.contains(form->name)) module_names().callables.emplace(form->name, new_family());
        prepared.family = module_names().callables.at(form->name);
        if (module_names().imported_aliases.contains(form->name))
          module_names().extensions[*prepared.family].emplace(form->name, form->at);
      }
      if (!form->body) {
        program_.families[*prepared.family].declarations.push_back(std::move(prepared));
        continue;
      }
      const auto id = program_.methods.size();
      prepared.definition = id;
      std::vector<size_t> local_family;
      auto &family = prepared.family ? program_.families[*prepared.family].methods : local_family;
      for (const auto other : lexical ? std::vector<size_t>{} : family) {
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
      if (const auto parent = program_.methods.back().parent) program_.methods[*parent].nested.push_back(id);
      method_ids.emplace(form, id);
      if (lexical) lexical_methods_[form] = id;
      else family.push_back(id);
    }
    for (const auto &[name, id] : module_names().symbols)
      if (module_names().callables.contains(name)) {
        if (!program_.symbols[id].intrinsic_print)
          implementation_gap(program_.families[module_names().callables.at(name)].at, "same-scope callable and symbol declarations");
        fail(program_.families[module_names().callables.at(name)].at, "callable conflicts with symbol '" + name + "'");
      }
    for (const auto &[name, type] : module_names().exception_types)
      if (module_names().symbols.contains(name) || module_names().callables.contains(name))
        implementation_gap({source.path.string()}, "same-scope type and callable/symbol bindings");
    for (const auto &[family, aliases] : module_names().extensions) if (aliases.size() > 1) {
      std::string message = "one callable identity is extended through multiple aliases:";
      for (const auto &[name, at] : aliases) message += " '" + name + "' at " + location_text(at);
      program_.warnings.push_back({Warning::multi_alias_extension, aliases.begin()->second, std::move(message)});
    }
    publish_module(source);
  }

public:
  CheckedProgram check(const SourceProgram &source) {
    modules_.resize(source.modules.size());
    std::optional<size_t> print_symbol;
    std::optional<Location> unsupported_here;
    for (const auto module : source.construction_order) {
      const ModuleLookup lookup(module_, module);
      register_module(source.modules[module], print_symbol, unsupported_here);
    }
    for (const auto &method : program_.methods)
      if (method.here && method.result.type && method.type_parameters.empty())
        check_declared_results(method, method.result, declared_results(method, method.parameters));
    if (unsupported_here) implementation_gap(*unsupported_here, "this receiving pattern for a here body");
    prepare_interpretations();
    plan_initialization(source);
    for (size_t i = 0; i < program_.methods.size(); ++i) {
      const auto &method = *program_.methods[i].source;
      if (program_.methods[i].definition != i || !method.type_parameters.empty() || lexical_methods_.contains(&method)) continue;
      demand(i, program_.methods[i].parameters, method.at);
    }
    check_entry();
    // Write guarantees use a greatest fixed point, with returned selections
    // and value categories joining before read requirements close. Narrower
    // callees require fresh assumptions for recursive cycles.
    for (;;) {
      effect_dependents_.clear(); callable_effect_consumers_.clear();
      for (auto &instance : program_.instances) instance.effects = initial_effects(program_.methods[instance.method], instance.arguments);
      for (auto &[id, effects] : distinct_binding_effects_) effects = program_.instances[id].effects;
      close_effects(false);
      if (!close_effects(true)) continue;
      auto choices = reaching_callables();
      if (choices == callable_choices_) break;
      callable_choices_ = std::move(choices);
    }
    validating_ = true;
    check_entry();
    for (size_t i = 0; i < program_.instances.size(); ++i) {
      if (const auto it = distinct_binding_effects_.find(i); it != distinct_binding_effects_.end()) {
        const auto effects = check_instance(i, true);
        auto known = it->second;
        if (merge_effects(known, effects, false) || merge_effects(known, effects, true))
          throw std::logic_error("distinct binding call effects changed after validation");
      }
      const auto effects = check_instance(i);
      auto known = program_.instances[i].effects;
      if (merge_effects(known, effects, false) || merge_effects(known, effects, true))
        throw std::logic_error("call effects changed after validation");
    }
    if (reaching_callables() != callable_choices_) throw std::logic_error("callable flow changed after validation");
    for (auto &method : program_.methods) if (!method.instances.empty()) {
      // Carry definition-result evidence only when ordinary type context cannot
      // distinguish the checked bodies. Fixed contracts need no extra native input.
      method.contextual_result = std::any_of(method.instances.begin(), method.instances.end(), [&](size_t a) {
        return std::any_of(method.instances.begin(), method.instances.end(), [&](size_t b) {
          return program_.instances[a].context == program_.instances[b].context &&
                 !same_result(program_.instances[a].result, program_.instances[b].result);
        });
      });
      const auto &first = program_.instances[method.instances.front()];
      for (const auto id : method.instances) {
        const auto &instance = program_.instances[id];
        if (instance.bindings.size() != first.bindings.size())
          implementation_gap(method.source->at, "generic instances with different lexical binding layouts");
        for (size_t i = 0; i < instance.bindings.size(); ++i)
          if (instance.bindings[i].value_binding != first.bindings[i].value_binding || instance.bindings[i].alias != first.bindings[i].alias ||
              instance.bindings[i].selection.empty() != first.bindings[i].selection.empty())
            implementation_gap(method.source->at, "generic instances with different lexical binding layouts");
      }
    }
    return std::move(program_);
  }
};

} // namespace cod::language
