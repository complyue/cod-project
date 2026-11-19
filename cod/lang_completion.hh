//===--- lang_completion.hh - Finite deferred completion equations ---===//
#pragma once

#include "lang_exceptions.hh"

#include <algorithm>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <utility>
#include <vector>

namespace cod::language {

inline constexpr size_t null_target = std::numeric_limits<size_t>::max();

struct SelectionCondition {
  std::optional<size_t> action;
  size_t binding, target;
  bool equal;
  // Immutable reference values survive changes to their original binding slots.
  bool immutable = false;
  auto operator<=>(const SelectionCondition &) const = default;
};
struct SelectionConditions {
  std::set<SelectionCondition> terms;
  bool possible = true;
  auto operator<=>(const SelectionConditions &) const = default;
  void add(SelectionCondition term) {
    for (const auto &known : terms) if (known.immutable == term.immutable && known.action == term.action && known.binding == term.binding &&
        ((known.target == term.target && known.equal != term.equal) ||
         (known.target != term.target && known.equal && term.equal))) possible = false;
    terms.insert(term);
  }
  void append(const SelectionConditions &other) {
    possible = possible && other.possible;
    for (const auto &term : other.terms) add(term);
  }
  void merge(const SelectionConditions &other) {
    if (!other.possible) return;
    if (!possible) { *this = other; return; }
    std::erase_if(terms, [&](const auto &term) { return !other.terms.contains(term); });
  }
  bool implies(const SelectionConditions &other) const {
    return !possible || (other.possible && std::includes(terms.begin(), terms.end(), other.terms.begin(), other.terms.end()));
  }
  SelectionConditions enter(size_t action) const {
    SelectionConditions result; result.possible = possible;
    for (auto term : terms) { if (term.action == action) term.action.reset(); result.add(term); }
    return result;
  }
  template <typename Resolve> SelectionConditions through(const Resolve &resolve, std::optional<size_t> action = {}) const {
    SelectionConditions result; result.possible = possible;
    for (auto term : terms) {
      if (term.immutable || term.action) { result.add(term); continue; }
      const auto selected = resolve(term.binding);
      if (!selected) continue;
      if (const auto entry = selected->single_entry()) {
        term.action = action; term.binding = *entry; result.add(term);
      } else if (selected->entries.empty()) {
        const bool null = term.target == null_target;
        if (null && selected->places.empty() && !selected->nullable()) continue;
        const bool contains = null ? selected->nullable() : selected->places.contains(term.target);
        const bool exact = contains && (null ? selected->places.empty() :
            selected->places.size() == 1 && !selected->nullable());
        if ((!contains && term.equal) || (exact && !term.equal)) result.possible = false;
      }
      // Wider selections cannot establish this equality or its negation.
    }
    return result;
  }
};

// Finite value provenance. Conditions describe immutable entry selections;
// they are resolved with the same relation used by boolean payload evidence.
struct ExceptionFacts {
  struct Origin {
    enum class Kind { constant, input, arrival, payload } kind = Kind::constant;
    std::optional<size_t> action = {};
    size_t index = 0;
    SelectionConditions conditions{};
    auto operator<=>(const Origin &) const = default;
  };
  std::map<Origin, std::set<size_t>> terms;
  ExceptionFacts(std::set<size_t> categories = {}) { if (!categories.empty()) terms[{}] = std::move(categories); }
  ExceptionFacts(std::initializer_list<size_t> categories) : ExceptionFacts(std::set<size_t>{categories}) {}
  bool operator==(const ExceptionFacts &) const = default;
  auto operator<=>(const ExceptionFacts &) const = default;
  bool empty() const { return terms.empty(); }
  void clear() { terms.clear(); }
  void add(Origin origin, const std::set<size_t> &categories) {
    if (!origin.conditions.possible || categories.empty()) return;
    auto &known = terms[std::move(origin)]; known.insert(categories.begin(), categories.end());
  }
  static ExceptionFacts input(size_t ordinal, const std::set<size_t> &categories) {
    ExceptionFacts result; result.add({Origin::Kind::input, {}, ordinal}, categories); return result;
  }
  static ExceptionFacts payload(size_t place, std::optional<size_t> action = {}) {
    ExceptionFacts result;
    for (size_t category = 1; category < exception_categories.size(); ++category)
      result.add({Origin::Kind::payload, action, place}, {category});
    return result;
  }
  static ExceptionFacts arrival(std::optional<size_t> action = {}) {
    ExceptionFacts result;
    for (size_t category = 1; category < exception_categories.size(); ++category)
      result.add({Origin::Kind::arrival, action}, {category});
    return result;
  }
  void merge(const ExceptionFacts &other) { for (const auto &[origin, categories] : other.terms) add(origin, categories); }
  std::set<size_t> possibilities() const {
    std::set<size_t> result;
    for (const auto &[origin, categories] : terms) result.insert(categories.begin(), categories.end());
    return result;
  }
  template <typename Keep> ExceptionFacts select(const Keep &keep) const {
    ExceptionFacts result;
    for (const auto &[origin, categories] : terms) {
      auto retained = categories; std::erase_if(retained, [&](size_t category) { return !keep(category); });
      result.add(origin, retained);
    }
    return result;
  }
  ExceptionFacts restricted(const std::set<size_t> &possible) const {
    return select([&](size_t category) { return possible.contains(category); });
  }
  ExceptionFacts without(const std::set<size_t> &possible) const {
    return select([&](size_t category) { return !possible.contains(category); });
  }
  ExceptionFacts guarded(const SelectionConditions &conditions) const {
    ExceptionFacts result;
    for (const auto &[origin, categories] : terms) {
      auto next = origin; next.conditions.append(conditions); result.add(std::move(next), categories);
    }
    return result;
  }
  template <typename Transform> ExceptionFacts transform_conditions(const Transform &transform) const {
    ExceptionFacts result;
    for (const auto &[origin, categories] : terms) {
      auto next = origin; next.conditions = transform(next.conditions); result.add(std::move(next), categories);
    }
    return result;
  }
  ExceptionFacts substitute(const std::vector<ExceptionFacts> &arguments, bool conservative = false) const {
    ExceptionFacts result;
    for (const auto &[origin, categories] : terms) {
      if (origin.kind == Origin::Kind::input) {
        const auto &value = arguments.at(origin.index);
        result.merge((conservative && value.empty() ? ExceptionFacts{categories} : value).restricted(categories).guarded(origin.conditions));
      }
      else result.add(origin, categories);
    }
    return result;
  }
  // Empty provisional summaries cannot exclude a conditional read path.
  ExceptionFacts through(const std::map<size_t, ExceptionFacts> &values, const ExceptionFacts &failure,
                         bool closed = false, bool conservative = false) const {
    ExceptionFacts result;
    for (const auto &[origin, categories] : terms) {
      if (origin.kind == Origin::Kind::arrival && !origin.action) result.merge((conservative && failure.empty() ? ExceptionFacts{categories} : failure).restricted(categories).guarded(origin.conditions));
      else if (origin.kind == Origin::Kind::payload && !origin.action) {
        const auto found = values.find(origin.index);
        if (found != values.end()) result.merge((conservative && found->second.empty() ? ExceptionFacts{categories} : found->second).restricted(categories).guarded(origin.conditions));
        else if (closed) result.merge(ExceptionFacts{categories}.guarded(origin.conditions));
        else result.add(origin, categories);
      } else result.add(origin, categories);
    }
    return result;
  }
  ExceptionFacts enter(size_t action, const ExceptionFacts &failure) const {
    ExceptionFacts result;
    for (const auto &[origin, categories] : terms) {
      auto next = origin; next.conditions = next.conditions.enter(action);
      if (origin.kind == Origin::Kind::arrival && origin.action == action)
        result.merge(failure.restricted(categories).guarded(next.conditions));
      else {
        if (origin.kind == Origin::Kind::payload && origin.action == action) next.action.reset();
        result.add(std::move(next), categories);
      }
    }
    return result;
  }
  bool unresolved() const {
    return std::any_of(terms.begin(), terms.end(), [](const auto &term) {
      return term.first.kind == Origin::Kind::arrival || term.first.kind == Origin::Kind::payload ||
          !term.first.conditions.terms.empty();
    });
  }
  bool reads_unknown(const std::map<size_t, ExceptionFacts> &values) const {
    return std::any_of(terms.begin(), terms.end(), [&](const auto &term) {
      const auto &origin = term.first;
      const auto found = values.find(origin.index);
      return origin.kind == Origin::Kind::payload && !origin.action && found != values.end() && found->second.empty();
    });
  }
};

struct Completion {
  // Normal, enclosing return, terminal numeric failure, or an intrinsic category.
  size_t reason = 0;
  bool returned = false;
  static Completion failure(size_t category) { return {3 + category, false}; }
  size_t index() const { return 2 * reason + returned; }
  static Completion from(size_t index) { return {index / 2, bool(index % 2)}; }
};

class CleanupEquations {
  static_assert(exception_categories.size() < 64);
public:
  using BooleanPayload = std::pair<std::optional<size_t>, size_t>;
  using BindingSnapshot = std::pair<size_t, std::set<size_t>>;
  // Necessary path facts shared by value summaries and read obligations.
  struct ConditionFacts {
    std::set<size_t> present;
    // Necessary truths of immutable incoming boolean payloads.
    std::set<std::pair<size_t, bool>> inputs;
    std::set<std::pair<BooleanPayload, bool>> payloads{};
    // Non-null selections are valid only for these binding versions.
    std::set<BindingSnapshot> bindings{};
    SelectionConditions conditions{};
    std::set<ExceptionFacts> exceptions{};
    // Absence of immutable borrowed inputs, independently of later rebinding.
    std::set<size_t> absent{};
    std::set<BindingSnapshot> absent_bindings{};
    // Equal observed reference values do not alias their mutable binding slots.
    std::set<std::set<BindingSnapshot>> same_bindings{};
    // Local immutable selections, normalized to formal inputs across calls.
    std::set<std::set<size_t>> same_selections{}, same_inputs{};
    std::set<std::pair<size_t, size_t>> different_selections{}, different_inputs{};
    bool same_selection(size_t left, size_t right) const {
      return left == right || std::any_of(same_selections.begin(), same_selections.end(), [&](const auto &group) {
        return group.contains(left) && group.contains(right);
      });
    }
    bool operator==(const ConditionFacts &) const = default;
    auto operator<=>(const ConditionFacts &) const = default;
    void append(const ConditionFacts &other) {
      present.insert(other.present.begin(), other.present.end());
      inputs.insert(other.inputs.begin(), other.inputs.end());
      payloads.insert(other.payloads.begin(), other.payloads.end());
      bindings.insert(other.bindings.begin(), other.bindings.end());
      conditions.append(other.conditions);
      exceptions.insert(other.exceptions.begin(), other.exceptions.end());
      absent.insert(other.absent.begin(), other.absent.end());
      absent_bindings.insert(other.absent_bindings.begin(), other.absent_bindings.end());
      same_bindings.insert(other.same_bindings.begin(), other.same_bindings.end());
      same_selections.insert(other.same_selections.begin(), other.same_selections.end());
      same_inputs.insert(other.same_inputs.begin(), other.same_inputs.end());
      different_selections.insert(other.different_selections.begin(), other.different_selections.end());
      different_inputs.insert(other.different_inputs.begin(), other.different_inputs.end());
    }
    template <typename T> static void close_equalities(std::set<std::set<T>> &relations) {
      std::set<std::set<T>> groups;
      for (auto group : relations) {
        for (auto it = groups.begin(); it != groups.end();) {
          if (std::none_of(group.begin(), group.end(), [&](const auto &value) { return it->contains(value); })) { ++it; continue; }
          group.insert(it->begin(), it->end()); it = groups.erase(it);
        }
        if (group.size() > 1) groups.insert(std::move(group));
      }
      relations = std::move(groups);
    }
    void resolve_bindings() {
      close_equalities(same_bindings);
      close_equalities(same_selections);
      close_equalities(same_inputs);
      const auto resolve_differences = [&](auto &different, const auto &same) {
        const auto canonical = [&](size_t identity) {
          for (const auto &group : same) if (group.contains(identity)) return *group.begin();
          return identity;
        };
        std::set<std::pair<size_t, size_t>> retained;
        for (const auto &[left, right] : different) {
          const auto lhs = canonical(left), rhs = canonical(right);
          if (lhs == rhs) conditions.possible = false;
          retained.emplace(std::min(lhs, rhs), std::max(lhs, rhs));
        }
        different = std::move(retained);
      };
      resolve_differences(different_selections, same_selections);
      resolve_differences(different_inputs, same_inputs);
      SelectionConditions selected;
      selected.possible = conditions.possible;
      for (auto term : conditions.terms) {
        if (term.immutable) for (const auto &group : same_selections)
          if (group.contains(term.binding)) { term.binding = *group.begin(); break; }
        selected.add(term);
      }
      SelectionConditions excluded;
      for (const auto &[left, right] : different_selections)
        for (auto term : selected.terms) if (term.immutable && term.equal && (term.binding == left || term.binding == right)) {
          term.binding = term.binding == left ? right : left;
          term.equal = false;
          excluded.add(term);
        }
      selected.append(excluded);
      conditions = std::move(selected);
      for (const auto &group : same_bindings) {
        if (std::any_of(group.begin(), group.end(), [&](const auto &value) { return bindings.contains(value); }))
          bindings.insert(group.begin(), group.end());
        if (std::any_of(group.begin(), group.end(), [&](const auto &value) { return absent_bindings.contains(value); }))
          absent_bindings.insert(group.begin(), group.end());
      }
    }
    bool implies(const ConditionFacts &other) const {
      const auto includes = [](const auto &a, const auto &b) { return std::includes(a.begin(), a.end(), b.begin(), b.end()); };
      const auto implies_equalities = [&](const auto &known, const auto &required) {
        return std::all_of(required.begin(), required.end(), [&](const auto &group) {
          return std::any_of(known.begin(), known.end(), [&](const auto &facts) { return includes(facts, group); });
        });
      };
      return includes(present, other.present) && includes(inputs, other.inputs) &&
          includes(payloads, other.payloads) && includes(bindings, other.bindings) && conditions.implies(other.conditions) &&
          includes(exceptions, other.exceptions) && includes(absent, other.absent) && includes(absent_bindings, other.absent_bindings) &&
          implies_equalities(same_bindings, other.same_bindings) && includes(different_selections, other.different_selections) &&
          includes(different_inputs, other.different_inputs) &&
          implies_equalities(same_selections, other.same_selections) && implies_equalities(same_inputs, other.same_inputs);
    }
  };
  // Keep finite alternatives until entry payloads have been substituted.
  // Intersecting them sooner loses preserve-or-write relationships in queues.
  using ConditionClauses = std::set<ConditionFacts>;
  using BooleanValue = std::map<bool, ConditionClauses>;
  using BooleanValues = std::map<size_t, BooleanValue>;
  static void add_condition(ConditionClauses &clauses, ConditionFacts facts) {
    const auto contradictory = [](const auto &truths) {
      return std::adjacent_find(truths.begin(), truths.end(), [](const auto &a, const auto &b) {
        return a.first == b.first && a.second != b.second;
      }) != truths.end();
    };
    if (!facts.conditions.possible || contradictory(facts.inputs) || contradictory(facts.payloads)) return;
    facts.resolve_bindings();
    if (!facts.conditions.possible) return;
    if (std::any_of(facts.absent.begin(), facts.absent.end(), [&](size_t input) { return facts.present.contains(input); })) return;
    if (std::any_of(facts.absent_bindings.begin(), facts.absent_bindings.end(), [&](const auto &binding) { return facts.bindings.contains(binding); })) return;
    // Filters constrain one immutable origin. Constant alternatives and
    // different sources do not establish an identity correlation.
    std::map<ExceptionFacts::Origin, std::set<size_t>> categories;
    std::set<ExceptionFacts> retained;
    for (const auto &value : facts.exceptions) {
      if (value.empty()) return;
      const auto &[origin, possible] = *value.terms.begin();
      if (value.terms.size() != 1 || origin.kind == ExceptionFacts::Origin::Kind::constant) {
        retained.insert(value); continue;
      }
      const auto [found, inserted] = categories.emplace(origin, possible);
      if (!inserted) std::erase_if(found->second, [&](size_t category) { return !possible.contains(category); });
      if (found->second.empty()) return;
    }
    for (auto &[origin, possible] : categories) {
      ExceptionFacts value; value.add(origin, possible); retained.insert(std::move(value));
    }
    facts.exceptions = std::move(retained);
    if (std::any_of(clauses.begin(), clauses.end(), [&](const auto &known) { return facts.implies(known); })) return;
    std::erase_if(clauses, [&](const auto &known) { return known.implies(facts); });
    clauses.insert(std::move(facts));
  }
  static void add_reaching_condition(ConditionClauses &clauses, ConditionFacts facts) {
    // Otherwise identical control-flow paths can share their common facts
    // when they cover both values of an immutable formal input.
    const auto combine = [&] {
      auto test = facts;
      for (const auto &literal : facts.inputs) {
        const auto opposite = std::pair{literal.first, !literal.second};
        test.inputs.erase(literal);
        test.inputs.insert(opposite);
        if (clauses.contains(test)) {
          test.inputs.erase(opposite);
          facts = std::move(test);
          return true;
        }
        test.inputs.erase(opposite);
        test.inputs.insert(literal);
      }
      return false;
    };
    if (!clauses.empty()) while (!facts.inputs.empty() && combine()) {}
    add_condition(clauses, std::move(facts));
  }
  static ConditionClauses conjoin_conditions(const ConditionClauses &left, const ConditionClauses &right) {
    ConditionClauses result;
    for (const auto &a : left) for (const auto &b : right) {
      auto facts = a; facts.append(b); add_condition(result, std::move(facts));
    }
    return result;
  }
  static bool unconditional(const ConditionClauses &clauses) {
    return clauses.size() == 1 && *clauses.begin() == ConditionFacts{};
  }
  static ConditionClauses stable_input_conditions(const ConditionClauses &clauses) {
    ConditionClauses result;
    for (const auto &path : clauses) {
      ConditionFacts stable;
      stable.inputs = path.inputs;
      stable.present = path.present;
      stable.absent = path.absent;
      add_condition(result, std::move(stable));
    }
    return result;
  }
  static ConditionClauses boolean_conditions(const ConditionClauses &clauses) {
    ConditionClauses result;
    for (const auto &path : clauses) {
      ConditionFacts boolean;
      boolean.inputs = path.inputs;
      boolean.payloads = path.payloads;
      add_reaching_condition(result, std::move(boolean));
      if (unconditional(result)) break;
    }
    return result;
  }
  static BooleanValue unknown_boolean() { return {{false, {ConditionFacts{}}}, {true, {ConditionFacts{}}}}; }
  static BooleanValue boolean_payload(size_t place, std::optional<size_t> action = {}) {
    BooleanValue result;
    for (const bool value : {false, true}) {
      ConditionFacts facts; facts.payloads.emplace(BooleanPayload{action, place}, value);
      result[value].insert(std::move(facts));
    }
    return result;
  }
  static void merge_boolean(BooleanValue &known, const BooleanValue &other) {
    for (const auto &[value, clauses] : other) for (const auto &facts : clauses) add_condition(known[value], facts);
  }
  template <typename Transform> static BooleanValue transform_boolean_conditions(const BooleanValue &value, const Transform &transform) {
    BooleanValue output;
    for (const auto &[truth, clauses] : value) for (auto facts : clauses) {
      facts.conditions = transform(facts.conditions); add_condition(output[truth], std::move(facts));
    }
    return output;
  }
  static BooleanValue guard_boolean(const BooleanValue &value, const SelectionConditions &conditions) {
    return transform_boolean_conditions(value, [&](auto known) { known.append(conditions); return known; });
  }
  template <typename Resolve>
  static ConditionClauses substitute_boolean_payloads(const ConditionClauses &clauses, const Resolve &resolve, bool closed,
                                                      bool boolean_only = false) {
    ConditionClauses output;
    for (const auto &facts : clauses) {
      auto retained = facts;
      std::erase_if(retained.payloads, [&](const auto &condition) {
        return !condition.first.first && (closed || resolve(condition.first.second));
      });
      ConditionClauses substituted{std::move(retained)};
      for (const auto &[payload, truth] : facts.payloads) if (!payload.first) {
        const auto *value = resolve(payload.second);
        if (!value) continue;
        if (const auto known = value->find(truth); known != value->end())
          substituted = conjoin_conditions(substituted, boolean_only ? boolean_conditions(known->second) : known->second);
        else substituted.clear();
      }
      for (const auto &known : substituted) add_condition(output, known);
    }
    return output;
  }
  static ConditionClauses through_conditions(const ConditionClauses &clauses, const BooleanValues &payloads, bool closed = false) {
    return substitute_boolean_payloads(clauses, [&](size_t place) -> const BooleanValue * {
      const auto found = payloads.find(place);
      return found == payloads.end() ? nullptr : &found->second;
    }, closed);
  }
  static BooleanValue through_boolean(const BooleanValue &value, const BooleanValues &payloads, bool closed = false) {
    BooleanValue output;
    for (const auto &[truth, clauses] : value) output[truth] = through_conditions(clauses, payloads, closed);
    return output;
  }
  static ConditionClauses enter_conditions(const ConditionClauses &clauses, size_t action) {
    ConditionClauses output;
    for (auto facts : clauses) {
      facts.conditions = facts.conditions.enter(action);
      const auto before = facts.payloads; facts.payloads.clear();
      for (auto [payload, truth] : before) {
        if (payload.first == action) payload.first.reset();
        facts.payloads.emplace(std::move(payload), truth);
      }
      add_condition(output, std::move(facts));
    }
    return output;
  }
  struct Targets {
    // Physical cells only; absence cannot justify initializing a cell.
    std::set<size_t> places;
    // Immutable nullable inputs, or null_target for independent absence.
    std::set<size_t> null_sources;
    // Targets selected by these bindings on entry to the summarized sequence.
    std::set<size_t> entries;
    // Observations of direct alternatives expressed in immutable input facts.
    // Entry selections contribute their own observations when substituted.
    std::optional<BooleanValue> nullness{};
    bool operator==(const Targets &) const = default;
    bool nullable() const { return !null_sources.empty(); }
    std::optional<size_t> single_entry() const {
      if (entries.size() == 1 && places.empty() && !nullable()) return *entries.begin();
      return {};
    }
    BooleanValue null_observations() const {
      if (nullness) return *nullness;
      BooleanValue result;
      for (const auto source : null_sources) {
        ConditionFacts absent;
        if (source != null_target) absent.absent.insert(source);
        add_condition(result[true], std::move(absent));
      }
      if (!places.empty()) {
        ConditionFacts present;
        if (places.size() == 1 && null_sources.contains(*places.begin()))
          present.present.insert(*places.begin());
        result[false].insert(std::move(present));
      }
      return result;
    }
    void merge(const Targets &other) {
      if (nullness || other.nullness) {
        auto proof = null_observations();
        merge_boolean(proof, other.null_observations());
        nullness = std::move(proof);
      }
      places.insert(other.places.begin(), other.places.end());
      null_sources.insert(other.null_sources.begin(), other.null_sources.end());
      entries.insert(other.entries.begin(), other.entries.end());
    }
  };
  struct InitializationInput {
    size_t id;
    // Absent means the incoming selection of this relation. A captured frame
    // remains independent of slot mutations while nested relations compose.
    std::optional<size_t> frame;
    bool physical = false;
    InitializationInput(size_t id, std::optional<size_t> frame = {}, bool physical = false) : id(id), frame(frame), physical(physical) {}
    static InitializationInput payload(size_t place) { return {place, {}, true}; }
    auto operator<=>(const InitializationInput &) const = default;
  };
  using Initialization = std::set<InitializationInput>;
  static size_t action_frame(size_t action) { return action + 1; }
  static Initialization capture_initialization(const Initialization &conditions, size_t frame) {
    Initialization result;
    for (auto input : conditions) { if (!input.physical && !input.frame) input.frame = frame; result.insert(input); }
    return result;
  }
  static Initialization enter_initialization(const Initialization &conditions, size_t frame) {
    Initialization result;
    for (auto input : conditions) { if (input.frame == frame) input.frame.reset(); result.insert(input); }
    return result;
  }
  // A selection fact describes the current target, so rebinding can invalidate it.
  struct Selections {
    std::set<size_t> initialized, invalidated;
    // Prepared operands carry immutable selection identities within a body.
    std::set<size_t> initialized_captures;
    // Sufficient entry-target initialization for a current selected target.
    std::map<size_t, Initialization> initialized_if;
    // Initialization of entry targets survives changes to the current selection.
    std::set<size_t> entry_initialized, entry_nonnull;
    // Actual branch conditions are separate from conditional callee guarantees.
    std::set<size_t> null_inputs, nonnull_inputs;
    bool possible = true;
    std::map<size_t, Targets> targets;
    bool operator==(const Selections &) const = default;
    bool nonnull(size_t id) const {
      if (const auto target = targets.find(id); target != targets.end())
        return !target->second.nullable() && (!target->second.places.empty() || !target->second.entries.empty()) &&
            std::all_of(target->second.entries.begin(), target->second.entries.end(), [&](size_t entry) { return entry_nonnull.contains(entry); });
      return !invalidated.contains(id) && entry_nonnull.contains(id);
    }
    void initialize(size_t id) {
      initialized.insert(id);
      initialized_if.erase(id);
      if (!invalidated.contains(id)) entry_initialized.insert(id);
      else if (const auto target = targets.find(id); target != targets.end())
        if (const auto entry = target->second.single_entry()) entry_initialized.insert(*entry);
    }
    std::optional<Initialization> initialization_requirements(size_t id) const {
      if (initialized.contains(id)) return Initialization{};
      if (const auto found = initialized_if.find(id); found != initialized_if.end()) return found->second;
      if (!invalidated.contains(id)) return Initialization{id};
      if (const auto found = targets.find(id); found != targets.end()) {
        const auto &target = found->second;
        if (target.places.empty() && target.entries.empty() && !target.nullable()) return {};
        Initialization required(target.entries.begin(), target.entries.end());
        for (const auto place : target.places) required.insert(InitializationInput::payload(place));
        return required;
      }
      return {};
    }
    std::optional<Initialization> initialization_requirements(const Initialization &entries) const {
      Initialization required;
      for (const auto entry : entries) {
        if (entry.frame || entry.physical) { required.insert(entry); continue; }
        const auto condition = initialization_requirements(entry.id);
        if (!condition) return {};
        required.insert(condition->begin(), condition->end());
      }
      return required;
    }
    void enter_initialization(size_t frame) {
      for (auto &[binding, conditions] : initialized_if)
        conditions = CleanupEquations::enter_initialization(conditions, frame);
    }
    void forget_initialization(size_t id, bool physical = false) {
      if (!physical) initialized.erase(id);
      std::erase_if(initialized_if, [&](const auto &entry) {
        return (!physical && entry.first == id) || std::any_of(entry.second.begin(), entry.second.end(),
            [&](const auto &input) { return input.physical == physical && input.id == id; });
      });
    }
    void refine(size_t id, bool null) {
      // Null never reads a payload.
      if (null) initialize(id);
      if (!null && !invalidated.contains(id)) entry_nonnull.insert(id);
      if (auto target = targets.find(id); target != targets.end()) {
        if (!null && target->second.places.empty() && target->second.entries.size() == 1)
          entry_nonnull.insert(*target->second.entries.begin());
        auto &selected = target->second;
        if (null) {
          if (!selected.nullable() || !selected.entries.empty()) {
            selected.null_sources.insert(null_target);
            selected.nullness.reset();
          }
          if (selected.nullness) selected.nullness->erase(false);
          selected.places.clear();
          selected.entries.clear();
        } else if (!selected.places.empty() || !selected.entries.empty()) {
          selected.null_sources.clear();
          if (selected.nullness) selected.nullness->erase(true);
        }
      }
    }
    void test(size_t id, bool null) {
      if (!invalidated.contains(id)) {
        (null ? null_inputs : nonnull_inputs).insert(id);
        if ((null ? nonnull_inputs : null_inputs).contains(id)) possible = false;
      }
      if (const auto target = targets.find(id); target != targets.end()) {
        if (target->second.entries.empty() && (null ? !target->second.nullable() : target->second.places.empty())) possible = false;
        if (target->second.places.empty() && target->second.entries.size() == 1 && (!null || !target->second.nullable())) {
          const auto entry = *target->second.entries.begin();
          (null ? null_inputs : nonnull_inputs).insert(entry);
          if ((null ? nonnull_inputs : null_inputs).contains(entry)) possible = false;
        }
      }
      refine(id, null);
    }
    void constrain(const Selections &next) {
      possible = possible && next.possible;
      for (const auto id : next.null_inputs) test(id, true);
      for (const auto id : next.nonnull_inputs) test(id, false);
    }
    void invalidate(size_t id) { initialized.erase(id); initialized_if.erase(id); invalidated.insert(id); targets.erase(id); }
    std::optional<Targets> substitute(const Targets &next) const {
      Targets result{next.places, next.null_sources, {}, next.nullness};
      for (const auto entry : next.entries) {
        if (const auto previous = targets.find(entry); previous != targets.end()) result.merge(previous->second);
        else if (invalidated.contains(entry)) return {};
        else result.entries.insert(entry);
      }
      return result;
    }
    void assign(size_t id, Targets next) {
      const auto selected = substitute(next);
      invalidate(id);
      if (selected) targets.emplace(id, *selected);
    }
    void resolve(std::set<size_t> &known) {
      bool changed;
      do {
        changed = false;
        for (const auto &[id, target] : targets) {
          if (std::all_of(target.places.begin(), target.places.end(), [&](size_t place) { return known.contains(place); })) {
            Initialization required(target.entries.begin(), target.entries.end());
            std::erase_if(required, [&](const auto &entry) { return entry_initialized.contains(entry.id); });
            if (required.empty()) changed = initialized.insert(id).second || changed;
            else if (!initialized.contains(id)) {
              const auto found = initialized_if.find(id);
              if (found == initialized_if.end() || std::includes(found->second.begin(), found->second.end(), required.begin(), required.end()))
                initialized_if[id] = std::move(required);
            }
          }
          if (target.entries.empty() && !target.nullable() && target.places.size() == 1 && initialized.contains(id))
            changed = known.insert(*target.places.begin()).second || changed;
        }
        for (const auto &[id, required] : initialized_if)
          if (std::all_of(required.begin(), required.end(), [&](const auto &entry) {
                return entry.physical ? known.contains(entry.id) : !entry.frame && entry_initialized.contains(entry.id);
              }))
            changed = initialized.insert(id).second || changed;
      } while (changed);
      for (const auto id : initialized) initialized_if.erase(id);
    }
    std::set<size_t> substitute_entries(const std::set<size_t> &entries, const std::set<size_t> &nonnull,
                                        std::set<size_t> &known) const {
      std::set<size_t> preserved;
      for (const auto id : entries) {
        if (const auto target = targets.find(id); target != targets.end()) {
          if (!target->second.nullable() || nonnull.contains(id)) {
            if (target->second.entries.empty() && target->second.places.size() == 1)
              known.insert(*target->second.places.begin());
            if (target->second.places.empty() && target->second.entries.size() == 1)
              preserved.insert(*target->second.entries.begin());
          }
        } else if (!invalidated.contains(id)) preserved.insert(id);
      }
      return preserved;
    }
    void then(const Selections &next, std::set<size_t> &known) {
      constrain(next);
      auto before = *this;
      for (const auto id : next.entry_nonnull) { before.refine(id, false); refine(id, false); }
      const auto entries = before.substitute_entries(next.entry_initialized, next.entry_nonnull, known);
      entry_initialized.insert(entries.begin(), entries.end());
      for (const auto id : next.invalidated) invalidate(id);
      for (const auto &[id, update] : next.targets) {
        if (std::all_of(update.entries.begin(), update.entries.end(), [&](size_t entry) {
              return before.initialized.contains(entry) || next.entry_initialized.contains(entry);
            }) && std::all_of(update.places.begin(), update.places.end(), [&](size_t place) { return known.contains(place); }))
          initialized.insert(id);
        if (const auto target = before.substitute(update)) targets[id] = *target;
      }
      for (const auto &[id, required] : next.initialized_if) {
        auto remaining = required;
        std::erase_if(remaining, [&](const auto &entry) { return !entry.physical && !entry.frame && next.entry_initialized.contains(entry.id); });
        auto incoming = before.initialization_requirements(remaining);
        if (incoming) {
          if (incoming->empty()) initialized.insert(id);
          else initialized_if[id] = std::move(*incoming);
        }
      }
      initialized.insert(next.initialized.begin(), next.initialized.end());
      initialized_captures.insert(next.initialized_captures.begin(), next.initialized_captures.end());
      resolve(known);
    }
    void merge(const Selections &other) {
      if (!other.possible) return;
      if (!possible) { *this = other; return; }
      const auto before = *this;
      auto affected = invalidated;
      affected.insert(other.invalidated.begin(), other.invalidated.end());
      for (const auto id : affected) {
        auto left = before.initialization_requirements(id);
        const auto right = other.initialization_requirements(id);
        initialized_if.erase(id);
        if (left && right) {
          left->insert(right->begin(), right->end());
          if (!left->empty()) initialized_if[id] = std::move(*left);
        }
      }
      std::erase_if(initialized, [&](size_t id) { return !other.initialized.contains(id); });
      std::erase_if(initialized_captures, [&](size_t id) { return !other.initialized_captures.contains(id); });
      std::erase_if(entry_initialized, [&](size_t id) { return !other.entry_initialized.contains(id); });
      std::erase_if(entry_nonnull, [&](size_t id) { return !other.entry_nonnull.contains(id); });
      std::erase_if(null_inputs, [&](size_t id) { return !other.null_inputs.contains(id); });
      std::erase_if(nonnull_inputs, [&](size_t id) { return !other.nonnull_inputs.contains(id); });
      invalidated.insert(other.invalidated.begin(), other.invalidated.end());
      for (auto it = targets.begin(); it != targets.end();) {
        if (const auto next = other.targets.find(it->first); next != other.targets.end()) {
          it->second.merge(next->second); ++it;
        } else if (other.invalidated.contains(it->first)) it = targets.erase(it);
        else { it->second.entries.insert(it->first); ++it; }
      }
      for (const auto &[id, target] : other.targets) if (!before.targets.contains(id) && !before.invalidated.contains(id)) {
        auto merged = target; merged.entries.insert(id);
        targets.emplace(id, std::move(merged));
      }
    }
  };
  // Missing paths are necessary conditions: no entry means no write guarantee.
  using MissingInitialization = std::map<size_t, ConditionClauses>;
  static ConditionClauses missing_initialization(size_t place, const std::set<size_t> &initialized,
                                                 const MissingInitialization &missing) {
    if (initialized.contains(place)) return {};
    const auto found = missing.find(place);
    return found == missing.end() ? ConditionClauses{ConditionFacts{}} : found->second;
  }
  static void combine_initialization(std::set<size_t> &initialized, MissingInitialization &missing,
      const std::set<size_t> &next_initialized, const MissingInitialization &next_missing, bool alternatives) {
    auto places = initialized;
    places.insert(next_initialized.begin(), next_initialized.end());
    for (const auto &[place, clauses] : missing) places.insert(place);
    for (const auto &[place, clauses] : next_missing) places.insert(place);
    std::set<size_t> ready;
    MissingInitialization retained;
    for (const auto place : places) {
      auto clauses = missing_initialization(place, initialized, missing);
      const auto next = missing_initialization(place, next_initialized, next_missing);
      if (alternatives) for (const auto &facts : next) add_condition(clauses, facts);
      else clauses = conjoin_conditions(clauses, next);
      if (clauses.empty()) ready.insert(place);
      else if (clauses != ConditionClauses{ConditionFacts{}}) retained.emplace(place, std::move(clauses));
    }
    initialized = std::move(ready); missing = std::move(retained);
  }
  static void resolve_initialization(std::set<size_t> &initialized, MissingInitialization &missing) {
    std::erase_if(missing, [&](const auto &entry) {
      if (!entry.second.empty()) return false;
      initialized.insert(entry.first); return true;
    });
  }
  struct ReadRequirement {
    std::set<size_t> initialized;
    std::optional<size_t> selection = {};
    // A sufficient cover of the actual non-null read target.
    std::optional<std::set<size_t>> targets = {};
    std::set<size_t> entry_initialized = {};
    std::set<size_t> entry_nonnull = {};
    std::optional<std::set<size_t>> nonnull = {};
    bool nonnull_unproved = false;
    // Registration-path guards apply only while the original binding versions remain current.
    std::map<size_t, std::set<size_t>> nonnull_versions = {};
    // Necessary conditions on paths that still require this read to be initialized.
    ConditionClauses when{ConditionFacts{}};
    MissingInitialization uninitialized{};
    // Sufficient initialization of the sequence's incoming selected targets.
    std::optional<Initialization> initialized_selections = {};
    bool operator==(const ReadRequirement &) const = default;
    bool satisfied(std::optional<size_t> place, const std::set<size_t> &known, const Selections &selections) const {
      if (when.empty()) return true;
      if (!place) return false;
      if (nonnull) return !nonnull_unproved && std::all_of(nonnull->begin(), nonnull->end(),
          [&](size_t id) { return selections.nonnull(id); });
      const auto ready = [&](size_t id) { return initialized.contains(id) || known.contains(id); };
      return ready(*place) || (selection && selections.initialized.contains(*selection)) ||
          (initialized_selections && std::all_of(initialized_selections->begin(), initialized_selections->end(),
              [&](const auto &entry) { return entry.physical ? ready(entry.id) : !entry.frame && selections.initialized.contains(entry.id); })) ||
          (targets && std::all_of(targets->begin(), targets->end(), ready));
    }
    void substitute(const Selections &before) {
      if (nonnull) {
        std::set<size_t> entries;
        for (const auto id : *nonnull) {
          if (before.nonnull(id)) continue;
          if (const auto target = before.targets.find(id); target != before.targets.end()) {
            nonnull_unproved = nonnull_unproved || target->second.nullable();
            entries.insert(target->second.entries.begin(), target->second.entries.end());
          } else if (before.invalidated.contains(id)) nonnull_unproved = true;
          else entries.insert(id);
        }
        nonnull = std::move(entries);
        return;
      }
      if (initialized_selections) initialized_selections = before.initialization_requirements(*initialized_selections);
      else if (selection) initialized_selections = before.initialization_requirements(*selection);
      entry_initialized = before.substitute_entries(entry_initialized, entry_nonnull, initialized);
      entry_initialized.insert(before.entry_initialized.begin(), before.entry_initialized.end());
      std::erase_if(entry_nonnull, [&](size_t id) { return before.invalidated.contains(id); });
      entry_nonnull.insert(before.entry_nonnull.begin(), before.entry_nonnull.end());
      if (!selection) return;
      if (const auto target = before.targets.find(*selection); target != before.targets.end()) {
        if (target->second.entries.empty()) {
          if (!targets) targets = target->second.places;
          else std::erase_if(*targets, [&](size_t id) { return !target->second.places.contains(id); });
        } else if (const auto entry = target->second.single_entry()) {
          selection = entry;
          return;
        }
      }
      if (before.invalidated.contains(*selection)) selection.reset();
    }
    void enter_initialization(size_t frame) {
      if (initialized_selections) initialized_selections = CleanupEquations::enter_initialization(*initialized_selections, frame);
    }
    void merge(const ReadRequirement &other) {
      for (const auto &facts : other.when) add_condition(when, facts);
      if (nonnull && other.nonnull) {
        nonnull->insert(other.nonnull->begin(), other.nonnull->end());
        nonnull_unproved = nonnull_unproved || other.nonnull_unproved;
        std::erase_if(nonnull_versions, [&](const auto &guard) {
          const auto found = other.nonnull_versions.find(guard.first);
          return found == other.nonnull_versions.end() || found->second != guard.second;
        });
        return;
      }
      combine_initialization(initialized, uninitialized, other.initialized, other.uninitialized, true);
      std::erase_if(entry_initialized, [&](size_t id) { return !other.entry_initialized.contains(id); });
      std::erase_if(entry_nonnull, [&](size_t id) { return !other.entry_nonnull.contains(id); });
      if (initialized_selections && other.initialized_selections)
        initialized_selections->insert(other.initialized_selections->begin(), other.initialized_selections->end());
      else initialized_selections.reset();
      if (selection != other.selection) selection.reset();
      if (targets && other.targets) targets->insert(other.targets->begin(), other.targets->end());
      else targets.reset();
    }
  };
  using Reads = std::map<size_t, ReadRequirement>;
  // Explicit returns retain selected targets or copied value evidence. Cleanup
  // can preserve that pending result, replace it, or join both possibilities.
  struct Returned {
    bool preserves = true;
    std::set<size_t> targets, uninitialized, null_sources;
    ExceptionFacts value{};
    BooleanValue boolean{};
    BooleanValue nullness{};
    // Formal input selections; null_target denotes an independent/unknown origin.
    // The caller substitutes its prepared references, never their mutable slots.
    std::set<size_t> selection_sources{};
    // Sufficient initialization of the saved result, independent of later slot
    // changes. Preserving an incoming result contributes no new requirement.
    std::optional<Initialization> initialization = Initialization{};
    // Slots that still select the saved target on paths needing readiness.
    // These are local completion evidence and never exported to a caller.
    std::set<size_t> selection_bindings{};
    std::optional<size_t> saved_capture{};
    bool operator==(const Returned &) const = default;
    bool needs_initialization() const {
      return !uninitialized.empty() || !initialization || !initialization->empty();
    }
    void initialize_saved_selection() {
      uninitialized.clear();
      initialization = Initialization{};
      selection_bindings.clear();
      saved_capture.reset();
    }
    void initialize(const std::set<size_t> &known, const Selections &selections) {
      std::erase_if(uninitialized, [&](size_t target) { return known.contains(target); });
      if (initialization) std::erase_if(*initialization, [&](const auto &input) {
        return input.physical ? known.contains(input.id) : !input.frame && selections.entry_initialized.contains(input.id);
      });
      if ((saved_capture && selections.initialized_captures.contains(*saved_capture)) ||
          std::ranges::any_of(selection_bindings, [&](size_t binding) { return selections.initialized.contains(binding); }))
        initialize_saved_selection();
      if (!needs_initialization()) { selection_bindings.clear(); saved_capture.reset(); }
    }
    void advance(const Selections &next) {
      if (selection_bindings.empty()) return;
      if (std::ranges::any_of(selection_bindings, [&](size_t binding) { return next.entry_initialized.contains(binding); })) {
        initialize_saved_selection();
        return;
      }
      auto entry = selection_bindings;
      std::erase_if(selection_bindings, [&](size_t binding) { return next.invalidated.contains(binding); });
      for (const auto &[binding, target] : next.targets)
        if (const auto source = target.single_entry(); source && entry.contains(*source)) selection_bindings.insert(binding);
    }
    void substitute(const Selections &before) {
      if (initialization) initialization = before.initialization_requirements(*initialization);
    }
    void enter_initialization(size_t frame) {
      if (initialization) initialization = CleanupEquations::enter_initialization(*initialization, frame);
    }
    void merge(const Returned &other) {
      if (!needs_initialization()) {
        selection_bindings = other.selection_bindings;
        saved_capture = other.saved_capture;
      } else if (other.needs_initialization()) {
        std::erase_if(selection_bindings, [&](size_t binding) { return !other.selection_bindings.contains(binding); });
        if (saved_capture != other.saved_capture) saved_capture.reset();
      }
      preserves = preserves || other.preserves;
      value.merge(other.value);
      targets.insert(other.targets.begin(), other.targets.end());
      uninitialized.insert(other.uninitialized.begin(), other.uninitialized.end());
      null_sources.insert(other.null_sources.begin(), other.null_sources.end());
      merge_boolean(boolean, other.boolean);
      merge_boolean(nullness, other.nullness);
      selection_sources.insert(other.selection_sources.begin(), other.selection_sources.end());
      if (initialization && other.initialization) initialization->insert(other.initialization->begin(), other.initialization->end());
      else initialization.reset();
    }
    void then(const Returned &next) {
      const bool keep = preserves && next.preserves;
      if (!next.preserves) {
        targets.clear(); uninitialized.clear(); null_sources.clear(); value.clear(); boolean.clear(); nullness.clear(); selection_sources.clear();
        initialization = Initialization{};
        selection_bindings.clear();
        saved_capture.reset();
      }
      merge(next);
      preserves = keep;
    }
  };
  struct Effects {
    Reads reads;
    std::set<size_t> initialized;
    Returned returned{};
    Selections selections{};
    // Cleanup may require current bindings; its own installations restore before it returns.
    std::set<size_t> symbols{};
    ExceptionFacts thrown = ExceptionFacts::arrival();
    bool unknown_thrown = false;
    // Omitted targets preserve their incoming payloads.
    std::map<size_t, ExceptionFacts> payloads{};
    // A write can leave its postcondition unchanged and still invalidate a proof.
    std::set<size_t> payload_writes{};
    // Actual presence of immutable incoming targets, independently of payload initialization.
    std::set<size_t> present{};
    BooleanValues booleans{};
    // Source handling requirements are independent of runtime category paths.
    std::set<size_t> declared_failures{};
    MissingInitialization uninitialized{};
    bool needs_checked_cleanup_proof = false;
    bool guarded_pending_failure = false;
    // Necessary immutable input facts for reaching this queue completion.
    ConditionClauses reaching{ConditionFacts{}};
    // Necessary Boolean observations for this completion, relative to action entry.
    ConditionClauses action_reaching{ConditionFacts{}};
    void enter_initialization(size_t frame) {
      selections.enter_initialization(frame);
      returned.enter_initialization(frame);
      for (auto &[read, requirement] : reads) requirement.enter_initialization(frame);
    }
    template <typename Transform> void transform_conditions(const Transform &transform) {
      for (auto &[truth, clauses] : returned.boolean) clauses = transform(clauses);
      for (auto &[truth, clauses] : returned.nullness) clauses = transform(clauses);
      for (auto &[target, value] : booleans) for (auto &[truth, clauses] : value) clauses = transform(clauses);
      for (auto &[read, requirement] : reads) {
        requirement.when = transform(requirement.when);
        for (auto &[place, clauses] : requirement.uninitialized) clauses = transform(clauses);
        CleanupEquations::resolve_initialization(requirement.initialized, requirement.uninitialized);
      }
      for (auto &[place, clauses] : uninitialized) clauses = transform(clauses);
      resolve_initialization();
    }
    void resolve_initialization() {
      CleanupEquations::resolve_initialization(initialized, uninitialized);
      selections.resolve(initialized);
      returned.initialize(initialized, selections);
    }
    template <typename Transform, typename ProofTransform> void transform_values(const Transform &transform, const ProofTransform &proof_transform) {
      returned.value = transform(returned.value);
      thrown = transform(thrown);
      for (auto &[target, value] : payloads) value = transform(value);
      transform_conditions([&](const auto &clauses) {
        ConditionClauses result;
        for (auto facts : clauses) {
          const auto before = facts.exceptions; facts.exceptions.clear();
          for (const auto &value : before) facts.exceptions.insert(proof_transform(value));
          add_condition(result, std::move(facts));
        }
        return result;
      });
    }
    template <typename Transform> void transform_values(const Transform &transform) { transform_values(transform, transform); }
    template <typename Resolve> void resolve_conditions(const Resolve &resolve, std::optional<size_t> action = {}) {
      const auto transform = [&](const auto &conditions) { return conditions.through(resolve, action); };
      transform_values([&](const auto &value) { return value.transform_conditions(transform); });
      transform_conditions([&](const auto &clauses) {
        ConditionClauses result;
        for (auto facts : clauses) {
          const auto observed = std::move(facts.conditions);
          facts.conditions = transform(observed);
          if (!facts.conditions.possible) continue;
          std::optional<ConditionClauses> selected_paths;
          for (const auto &term : observed.terms) {
            if (term.immutable || term.action || term.target != null_target) continue;
            const auto selected = resolve(term.binding);
            if (!selected || !selected->entries.empty() || (selected->places.empty() && !selected->nullable())) continue;
            const auto proof = selected->null_observations();
            const auto path = proof.find(term.equal);
            const auto alternatives = path == proof.end() ? ConditionClauses{} : path->second;
            selected_paths = selected_paths ? conjoin_conditions(*selected_paths, alternatives) : alternatives;
          }
          if (!selected_paths) add_condition(result, std::move(facts));
          else for (auto path : *selected_paths) {
            path.append(facts);
            add_condition(result, std::move(path));
          }
        }
        return result;
      });
    }
  };
  struct Path { std::uint64_t inputs; Completion completion; size_t queue; Effects effects{}; };
  struct Action {
    bool guarded = false;
    std::vector<Path> paths;
    bool enforce_checked_boundary = false;
  };
private:
  struct Queue {
    std::vector<size_t> previous;
    std::optional<size_t> action;
    std::set<size_t> initialized{};
    MissingInitialization uninitialized{};
    ConditionClauses reaching{ConditionFacts{}};
  };
  std::vector<Queue> queues_{{}};
  std::vector<Action> actions_;
  std::set<size_t> observed_inputs_, observed_references_;
  static constexpr size_t states = 2 * (3 + exception_categories.size());
  std::vector<std::optional<size_t>> reads_;
  using Outcomes = std::map<size_t, Effects>;
  using QueueState = std::pair<size_t, size_t>;
  struct SolvedState {
    Outcomes outcomes;
    std::set<QueueState> dependents;
  };
  // New nodes leave existing dependencies intact. Changes to backedges,
  // action definitions or the observation projection can revise old results.
  std::map<QueueState, SolvedState> solved_;
  static ConditionClauses intersect_reaching(const ConditionClauses &left, const ConditionClauses &right) {
    if (unconditional(left)) return right;
    if (unconditional(right)) return left;
    return conjoin_conditions(left, right);
  }
  ConditionClauses queue_conditions(const ConditionClauses &clauses) const {
    if (clauses.empty() || unconditional(clauses)) return clauses;
    if (observed_inputs_.empty() && observed_references_.empty()) return {ConditionFacts{}};
    ConditionClauses result;
    for (auto path : clauses) {
      std::erase_if(path.inputs, [&](const auto &input) { return !observed_inputs_.contains(input.first); });
      std::erase_if(path.present, [&](size_t place) { return !observed_references_.contains(place); });
      std::erase_if(path.absent, [&](size_t place) { return !observed_references_.contains(place); });
      add_reaching_condition(result, std::move(path));
      if (unconditional(result)) break;
    }
    return result;
  }
  Effects compose(const Effects &first, const Effects &second) const {
    auto result = first;
    result.reaching = intersect_reaching(first.reaching, second.reaching);
    if (result.reaching.empty()) return result;
    result.symbols.insert(second.symbols.begin(), second.symbols.end());
    result.payload_writes.insert(second.payload_writes.begin(), second.payload_writes.end());
    result.present.insert(second.present.begin(), second.present.end());
    result.declared_failures.insert(second.declared_failures.begin(), second.declared_failures.end());
    result.needs_checked_cleanup_proof = first.needs_checked_cleanup_proof || second.needs_checked_cleanup_proof;
    result.guarded_pending_failure = first.guarded_pending_failure || second.guarded_pending_failure;
    if (!unconditional(second.action_reaching)) {
      const auto unknown = unknown_boolean();
      const auto lifted = substitute_boolean_payloads(second.action_reaching, [&](size_t place) -> const BooleanValue * {
        if (const auto found = first.booleans.find(place); found != first.booleans.end()) return &found->second;
        return first.payload_writes.contains(place) ? &unknown : nullptr;
      }, false, true);
      result.action_reaching = intersect_reaching(first.action_reaching, lifted);
    }
    auto next = second;
    next.returned.substitute(first.selections);
    next.resolve_conditions([&](size_t binding) { return first.selections.substitute({{}, {}, {binding}}); });
    next.transform_values([&](const auto &value) { return value.through(first.payloads, first.thrown); },
        [&](const auto &value) { return value.through(first.payloads, first.thrown, false, true); });
    next.transform_conditions([&](const auto &clauses) { return through_conditions(clauses, first.booleans); });
    for (const auto &[read, known] : next.reads) {
      const auto place = read_target(read);
      if (known.satisfied(place, first.initialized, first.selections)) continue;
      if (!place) {
        merge_read(result.reads, read, known);
        continue;
      }
      auto before = known;
      combine_initialization(before.initialized, before.uninitialized, first.initialized, first.uninitialized, false);
      before.when = conjoin_conditions(before.when,
          missing_initialization(*place, before.initialized, before.uninitialized));
      before.substitute(first.selections);
      if (before.satisfied(place, first.initialized, {})) continue;
      merge_read(result.reads, read, before);
    }
    combine_initialization(result.initialized, result.uninitialized, next.initialized, next.uninitialized, false);
    result.returned.advance(next.selections);
    result.selections.then(next.selections, result.initialized);
    result.unknown_thrown = first.unknown_thrown || second.unknown_thrown || second.thrown.reads_unknown(first.payloads);
    result.returned.then(next.returned);
    result.returned.initialize(result.initialized, result.selections);
    result.thrown = next.thrown;
    for (auto &[target, value] : next.payloads) result.payloads[target] = std::move(value);
    for (auto &[target, value] : next.booleans) result.booleans[target] = std::move(value);
    if (!unconditional(result.reaching))
      result.transform_conditions([&](const auto &clauses) { return conjoin_conditions(clauses, result.reaching); });
    return result;
  }
  static bool merge(std::map<size_t, Effects> &result, size_t completion, const Effects &effects) {
    if (!effects.selections.possible || effects.reaching.empty()) return false;
    auto [found, inserted] = result.try_emplace(completion, effects);
    if (inserted) return true;
    auto &known = found->second;
    const auto previous_reads = known.reads;
    const auto previous_initialized = known.initialized;
    const auto previous_uninitialized = known.uninitialized;
    const auto previous_returned = known.returned;
    const auto previous_thrown = known.thrown;
    const auto previous_unknown_thrown = known.unknown_thrown;
    const auto previous_payloads = known.payloads;
    const auto previous_payload_writes = known.payload_writes;
    const auto previous_present = known.present;
    const auto previous_booleans = known.booleans;
    const auto previous_declared = known.declared_failures;
    const auto previous_checked_cleanup = known.needs_checked_cleanup_proof;
    const auto previous_guarded_failure = known.guarded_pending_failure;
    const auto previous_selections = known.selections;
    const auto previous_symbols = known.symbols;
    const auto previous_reaching = known.reaching;
    const auto previous_action_reaching = known.action_reaching;
    for (const auto &path : effects.reaching) add_reaching_condition(known.reaching, path);
    for (const auto &path : effects.action_reaching) add_reaching_condition(known.action_reaching, path);
    known.symbols.insert(effects.symbols.begin(), effects.symbols.end());
    known.payload_writes.insert(effects.payload_writes.begin(), effects.payload_writes.end());
    known.declared_failures.insert(effects.declared_failures.begin(), effects.declared_failures.end());
    known.needs_checked_cleanup_proof = previous_checked_cleanup || effects.needs_checked_cleanup_proof;
    known.guarded_pending_failure = previous_guarded_failure || effects.guarded_pending_failure;
    for (const auto &[read, before] : effects.reads) merge_read(known.reads, read, before);
    combine_initialization(known.initialized, known.uninitialized, effects.initialized, effects.uninitialized, true);
    std::erase_if(known.present, [&](size_t id) { return !effects.present.contains(id); });
    known.returned.merge(effects.returned);
    known.thrown.merge(effects.thrown);
    known.unknown_thrown = known.unknown_thrown || effects.unknown_thrown;
    for (auto &[target, value] : known.payloads)
      if (!effects.payloads.contains(target)) value.merge(ExceptionFacts::payload(target));
    for (const auto &[target, value] : effects.payloads) {
      const auto [found, inserted] = known.payloads.try_emplace(target, value);
      if (inserted) found->second.merge(ExceptionFacts::payload(target));
      else found->second.merge(value);
    }
    for (auto &[target, value] : known.booleans)
      if (!effects.booleans.contains(target)) merge_boolean(value, boolean_payload(target));
    for (const auto &[target, value] : effects.booleans) {
      const auto [found, inserted] = known.booleans.try_emplace(target, value);
      if (inserted) merge_boolean(found->second, boolean_payload(target));
      else merge_boolean(found->second, value);
    }
    known.selections.merge(effects.selections);
    known.selections.resolve(known.initialized);
    return previous_reads != known.reads || previous_initialized != known.initialized || previous_uninitialized != known.uninitialized ||
        previous_returned != known.returned || previous_thrown != known.thrown || previous_unknown_thrown != known.unknown_thrown || previous_payloads != known.payloads ||
        previous_selections != known.selections || previous_symbols != known.symbols || previous_payload_writes != known.payload_writes ||
        previous_present != known.present || previous_booleans != known.booleans || previous_declared != known.declared_failures ||
        previous_checked_cleanup != known.needs_checked_cleanup_proof || previous_guarded_failure != known.guarded_pending_failure ||
        previous_reaching != known.reaching || previous_action_reaching != known.action_reaching;
  }
public:
  void observe_reference(const BooleanValue &observations) {
    bool changed = false;
    for (const auto &[null, paths] : observations) for (const auto &path : paths) {
      for (const auto &[input, value] : path.inputs) changed = observed_inputs_.insert(input).second || changed;
      for (const auto place : path.present) changed = observed_references_.insert(place).second || changed;
      for (const auto place : path.absent) changed = observed_references_.insert(place).second || changed;
    }
    if (changed) solved_.clear();
  }
  static void merge_read(Reads &reads, size_t read, const ReadRequirement &before) {
    auto [found, inserted] = reads.emplace(read, before);
    if (!inserted) found->second.merge(before);
  }
  size_t read(std::optional<size_t> place) { reads_.push_back(place); return reads_.size() - 1; }
  std::optional<size_t> read_target(size_t read) const { return reads_.at(read); }
  bool has_actions(size_t queue) const {
    std::set<size_t> seen;
    std::vector<size_t> pending{queue};
    while (!pending.empty()) {
      const auto id = pending.back(); pending.pop_back();
      if (!seen.insert(id).second) continue;
      const auto &node = queues_.at(id);
      if (node.action) return true;
      pending.insert(pending.end(), node.previous.begin(), node.previous.end());
    }
    return false;
  }
  size_t action() { actions_.push_back({}); return actions_.size() - 1; }
  void define(size_t id, Action action) { solved_.clear(); actions_.at(id) = std::move(action); }
  size_t push(size_t previous, size_t action, const ConditionClauses &reaching) {
    queues_.push_back({{previous}, action, {}, {}, reaching}); return queues_.size() - 1;
  }
  size_t join(size_t a, size_t b) {
    if (a == b) return a;
    queues_.push_back({{a, b}, {}}); return queues_.size() - 1;
  }
  size_t known(size_t queue, const std::set<size_t> &initialized, const MissingInitialization &uninitialized,
               const ConditionClauses &reaching) {
    if (initialized.empty() && uninitialized.empty() && unconditional(reaching)) return queue;
    queues_.push_back({{queue}, {}, initialized, uninitialized, reaching}); return queues_.size() - 1;
  }
  size_t append(size_t previous, size_t suffix) {
    if (!previous) return suffix;
    if (!suffix) return previous;
    std::map<size_t, size_t> copied{{0, previous}};
    std::vector<size_t> pending{suffix};
    while (!pending.empty()) {
      const auto id = pending.back(); pending.pop_back();
      if (copied.contains(id)) continue;
      copied[id] = queues_.size();
      queues_.push_back({});
      for (const auto ancestor : queues_.at(id).previous) pending.push_back(ancestor);
    }
    // Copy only the registration graph; each action retains its own nested queue.
    // Allocate every node before wiring edges so loop backedges remain finite.
    for (const auto &[source, target] : copied) if (source) {
      queues_.at(target) = queues_.at(source);
      for (auto &ancestor : queues_.at(target).previous) ancestor = copied.at(ancestor);
    }
    return copied.at(suffix);
  }
  size_t header(size_t entry) {
    queues_.push_back({{entry}, {}}); return queues_.size() - 1;
  }
  void backedge(size_t header, size_t body) { solved_.clear(); queues_.at(header).previous.push_back(body); }
  const Outcomes &apply(size_t queue, Completion input) {
    static const auto identity = [] {
      std::vector<Outcomes> relation(states);
      for (size_t state = 0; state < states; ++state) relation[state].emplace(state, Effects{});
      return relation;
    }();
    if (!queue) return identity.at(input.index());
    const QueueState root{queue, input.index()};
    const auto [entry, inserted] = solved_.try_emplace(root);
    if (!inserted) return entry->second.outcomes;

    std::set<QueueState> pending{root};
    std::map<size_t, ConditionClauses> reaching;
    while (!pending.empty()) {
      const auto key = *pending.begin();
      pending.erase(pending.begin());
      const auto [id, state] = key;
      const auto &node = queues_.at(id);
      auto &result = solved_.at(key);
      const auto [conditions, project] = reaching.try_emplace(id);
      if (project) conditions->second = queue_conditions(node.reaching);
      const auto require = [&](size_t dependency, size_t completion) -> const Outcomes & {
        if (!dependency) return identity.at(completion);
        const QueueState needed{dependency, completion};
        const auto [found, fresh] = solved_.try_emplace(needed);
        found->second.dependents.insert(key);
        if (fresh) pending.insert(needed);
        return found->second.outcomes;
      };
      bool changed = false;
      if (!node.action) {
        Effects known{{}, node.initialized};
        known.uninitialized = node.uninitialized;
        known.reaching = conditions->second;
        for (const auto previous : node.previous) for (const auto &[value, effects] : require(previous, state))
          changed = merge(result.outcomes, value, compose(known, effects)) || changed;
      } else {
        const auto &action = actions_.at(*node.action);
        const auto incoming = Completion::from(state);
        const auto selected = !action.guarded || incoming.reason < 2 ? 0 :
            incoming.reason == 2 ? exception_categories.size() : incoming.reason - 3;
        for (const auto &path : action.paths) {
          if (!(path.inputs & (std::uint64_t{1} << selected))) continue;
          auto next = path.completion;
          if (!next.reason) {
            next = action.guarded && incoming.reason >= 2 ?
                Completion{incoming.returned ? size_t{1} : size_t{0}, incoming.returned} : incoming;
          } else if (next.reason != 1) next.returned = next.returned || incoming.returned;
          auto effects = path.effects;
          effects.reaching = intersect_reaching(effects.reaching, conditions->second);
          // The single-pending-failure model cannot prove coverage of
          // checked failures produced by an action. Retain potential
          // guarded dispatch on the separate ordinary body failure too.
          const auto output = path.completion.reason;
          if (action.enforce_checked_boundary &&
              ((output >= 3 && exception_categories.at(output - 3).checked) || !effects.declared_failures.empty()))
            effects.needs_checked_cleanup_proof = true;
          if (action.guarded && incoming.reason >= 3) effects.guarded_pending_failure = true;
          if (next.reason < 3) effects.thrown.clear();
          for (const auto &[intermediate, nested] : require(path.queue, next.index())) {
            auto invoked = compose(effects, nested);
            invoked.enter_initialization(action_frame(*node.action));
            const auto arrival = incoming.reason >= 3
                ? ExceptionFacts::arrival().restricted({incoming.reason - 3}) : ExceptionFacts{};
            invoked.transform_values([&](const auto &value) { return value.enter(*node.action, arrival); });
            invoked.transform_conditions([&](const auto &clauses) { return enter_conditions(clauses, *node.action); });
            invoked.action_reaching = enter_conditions(invoked.action_reaching, *node.action);
            for (const auto &[value, previous] : require(node.previous.front(), intermediate))
              changed = merge(result.outcomes, value, compose(invoked, previous)) || changed;
          }
        }
      }
      // In a cyclic registration graph a later contribution can change an
      // earlier state. Recheck only the consumers of that state.
      if (changed) pending.insert(result.dependents.begin(), result.dependents.end());
    }
    return entry->second.outcomes;
  }
};

} // namespace cod::language
