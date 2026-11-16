//===--- lang_completion.hh - Finite deferred completion equations ---===//
#pragma once

#include "lang_exceptions.hh"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <map>
#include <optional>
#include <set>
#include <utility>
#include <vector>

namespace cod::language {

// Compiler evidence for a copied exception value. Input ordinals denote the
// categories present at callable entry, independently of subsequent writes.
struct ExceptionFacts {
  std::set<size_t> categories;
  std::map<size_t, std::set<size_t>> inputs;
  // An action identity names its arriving exception; nullopt names the input
  // of a composed cleanup relation. Lexical identities preserve nested captures.
  std::map<std::optional<size_t>, std::set<size_t>> arrivals;
  ExceptionFacts(std::set<size_t> categories = {}) : categories(std::move(categories)) {}
  ExceptionFacts(std::initializer_list<size_t> categories) : categories(categories) {}
  bool operator==(const ExceptionFacts &) const = default;
  bool empty() const { return categories.empty() && inputs.empty() && arrivals.empty(); }
  void clear() { categories.clear(); inputs.clear(); arrivals.clear(); }
  static ExceptionFacts arrival(std::optional<size_t> action = {}) {
    ExceptionFacts result;
    for (size_t category = 1; category < exception_categories.size(); ++category)
      result.arrivals[action].insert(category);
    return result;
  }
  void merge(const ExceptionFacts &other) {
    categories.insert(other.categories.begin(), other.categories.end());
    for (const auto &[input, possible] : other.inputs)
      inputs[input].insert(possible.begin(), possible.end());
    for (const auto &[action, possible] : other.arrivals)
      arrivals[action].insert(possible.begin(), possible.end());
  }
  std::set<size_t> possibilities() const {
    auto result = categories;
    for (const auto &[input, possible] : inputs) result.insert(possible.begin(), possible.end());
    for (const auto &[action, possible] : arrivals) result.insert(possible.begin(), possible.end());
    return result;
  }
  template <typename Keep> ExceptionFacts select(const Keep &keep) const {
    auto result = *this;
    std::erase_if(result.categories, [&](size_t category) { return !keep(category); });
    const auto restrict = [&](auto &sources) {
      for (auto &[origin, possible] : sources)
        std::erase_if(possible, [&](size_t category) { return !keep(category); });
      std::erase_if(sources, [](const auto &source) { return source.second.empty(); });
    };
    restrict(result.inputs); restrict(result.arrivals);
    return result;
  }
  ExceptionFacts restricted(const std::set<size_t> &possible) const {
    return select([&](size_t category) { return possible.contains(category); });
  }
  ExceptionFacts without(const std::set<size_t> &possible) const {
    return select([&](size_t category) { return !possible.contains(category); });
  }
  ExceptionFacts substitute(const std::vector<ExceptionFacts> &arguments) const {
    ExceptionFacts result{categories}; result.arrivals = arrivals;
    for (const auto &[input, possible] : inputs) result.merge(arguments.at(input).restricted(possible));
    return result;
  }
  ExceptionFacts replace_arrival(std::optional<size_t> action, const ExceptionFacts &value) const {
    auto result = *this;
    if (const auto found = arrivals.find(action); found != arrivals.end()) {
      result.arrivals.erase(action);
      result.merge(value.restricted(found->second));
    }
    return result;
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
  struct Targets {
    // Physical cells only; absence cannot justify initializing a cell.
    std::set<size_t> places;
    bool nullable = false;
    // Targets selected by these bindings on entry to the summarized sequence.
    std::set<size_t> entries;
    bool operator==(const Targets &) const = default;
    void merge(const Targets &other) {
      places.insert(other.places.begin(), other.places.end());
      nullable = nullable || other.nullable;
      entries.insert(other.entries.begin(), other.entries.end());
    }
  };
  // A selection fact describes the current target, so rebinding can invalidate it.
  struct Selections {
    std::set<size_t> initialized, invalidated;
    // Initialization of entry targets survives changes to the current selection.
    std::set<size_t> entry_initialized, entry_nonnull;
    // Actual branch conditions are separate from conditional callee guarantees.
    std::set<size_t> null_inputs, nonnull_inputs;
    bool possible = true;
    std::map<size_t, Targets> targets;
    bool operator==(const Selections &) const = default;
    bool nonnull(size_t id) const {
      if (const auto target = targets.find(id); target != targets.end())
        return !target->second.nullable && (!target->second.places.empty() || !target->second.entries.empty()) &&
            std::all_of(target->second.entries.begin(), target->second.entries.end(), [&](size_t entry) { return entry_nonnull.contains(entry); });
      return !invalidated.contains(id) && entry_nonnull.contains(id);
    }
    void initialize(size_t id) {
      initialized.insert(id);
      if (!invalidated.contains(id)) entry_initialized.insert(id);
    }
    void refine(size_t id, bool null) {
      // Null never reads a payload.
      if (null) initialize(id);
      if (!null && !invalidated.contains(id)) entry_nonnull.insert(id);
      if (auto target = targets.find(id); target != targets.end()) {
        if (!null && target->second.places.empty() && target->second.entries.size() == 1)
          entry_nonnull.insert(*target->second.entries.begin());
        if (null || !target->second.places.empty() || !target->second.entries.empty()) target->second.nullable = null;
        if (null) { target->second.places.clear(); target->second.entries.clear(); }
      }
    }
    void test(size_t id, bool null) {
      if (!invalidated.contains(id)) {
        (null ? null_inputs : nonnull_inputs).insert(id);
        if ((null ? nonnull_inputs : null_inputs).contains(id)) possible = false;
      }
      if (const auto target = targets.find(id); target != targets.end()) {
        if (target->second.entries.empty() && (null ? !target->second.nullable : target->second.places.empty())) possible = false;
        if (target->second.places.empty() && target->second.entries.size() == 1 && (!null || !target->second.nullable)) {
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
    void invalidate(size_t id) { initialized.erase(id); invalidated.insert(id); targets.erase(id); }
    std::optional<Targets> substitute(const Targets &next) const {
      Targets result{next.places, next.nullable, {}};
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
          if (std::all_of(target.entries.begin(), target.entries.end(), [&](size_t entry) { return entry_initialized.contains(entry); }) &&
              std::all_of(target.places.begin(), target.places.end(), [&](size_t place) { return known.contains(place); }))
            changed = initialized.insert(id).second || changed;
          if (target.entries.empty() && !target.nullable && target.places.size() == 1 && initialized.contains(id))
            changed = known.insert(*target.places.begin()).second || changed;
        }
      } while (changed);
    }
    std::set<size_t> substitute_entries(const std::set<size_t> &entries, const std::set<size_t> &nonnull,
                                        std::set<size_t> &known) const {
      std::set<size_t> preserved;
      for (const auto id : entries) {
        if (const auto target = targets.find(id); target != targets.end()) {
          if (!target->second.nullable || nonnull.contains(id)) {
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
      initialized.insert(next.initialized.begin(), next.initialized.end());
      resolve(known);
    }
    void merge(const Selections &other) {
      if (!other.possible) return;
      if (!possible) { *this = other; return; }
      const auto before = *this;
      std::erase_if(initialized, [&](size_t id) { return !other.initialized.contains(id); });
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
    bool operator==(const ReadRequirement &) const = default;
    bool satisfied(size_t place, const std::set<size_t> &known, const Selections &selections) const {
      if (nonnull) return !nonnull_unproved && std::all_of(nonnull->begin(), nonnull->end(),
          [&](size_t id) { return selections.nonnull(id); });
      const auto ready = [&](size_t id) { return initialized.contains(id) || known.contains(id); };
      return ready(place) || (selection && selections.initialized.contains(*selection)) ||
          (targets && std::all_of(targets->begin(), targets->end(), ready));
    }
    void substitute(const Selections &before) {
      if (nonnull) {
        std::set<size_t> entries;
        for (const auto id : *nonnull) {
          if (before.nonnull(id)) continue;
          if (const auto target = before.targets.find(id); target != before.targets.end()) {
            nonnull_unproved = nonnull_unproved || target->second.nullable;
            entries.insert(target->second.entries.begin(), target->second.entries.end());
          } else if (before.invalidated.contains(id)) nonnull_unproved = true;
          else entries.insert(id);
        }
        nonnull = std::move(entries);
        return;
      }
      entry_initialized = before.substitute_entries(entry_initialized, entry_nonnull, initialized);
      entry_initialized.insert(before.entry_initialized.begin(), before.entry_initialized.end());
      std::erase_if(entry_nonnull, [&](size_t id) { return before.invalidated.contains(id); });
      entry_nonnull.insert(before.entry_nonnull.begin(), before.entry_nonnull.end());
      if (!selection) return;
      if (const auto target = before.targets.find(*selection); target != before.targets.end()) {
        if (target->second.entries.empty()) {
          if (!targets) targets = target->second.places;
          else std::erase_if(*targets, [&](size_t id) { return !target->second.places.contains(id); });
        } else if (target->second.entries.size() == 1 && target->second.places.empty() && !target->second.nullable) {
          selection = *target->second.entries.begin();
          return;
        }
      }
      if (before.invalidated.contains(*selection)) selection.reset();
    }
    void merge(const ReadRequirement &other) {
      if (nonnull && other.nonnull) {
        nonnull->insert(other.nonnull->begin(), other.nonnull->end());
        nonnull_unproved = nonnull_unproved || other.nonnull_unproved;
        std::erase_if(nonnull_versions, [&](const auto &guard) {
          const auto found = other.nonnull_versions.find(guard.first);
          return found == other.nonnull_versions.end() || found->second != guard.second;
        });
        return;
      }
      std::erase_if(initialized, [&](size_t id) { return !other.initialized.contains(id); });
      std::erase_if(entry_initialized, [&](size_t id) { return !other.entry_initialized.contains(id); });
      std::erase_if(entry_nonnull, [&](size_t id) { return !other.entry_nonnull.contains(id); });
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
    bool operator==(const Returned &) const = default;
    void initialize(const std::set<size_t> &known) {
      std::erase_if(uninitialized, [&](size_t target) { return known.contains(target); });
    }
    void merge(const Returned &other) {
      preserves = preserves || other.preserves;
      value.merge(other.value);
      targets.insert(other.targets.begin(), other.targets.end());
      uninitialized.insert(other.uninitialized.begin(), other.uninitialized.end());
      null_sources.insert(other.null_sources.begin(), other.null_sources.end());
    }
    void then(const Returned &next) {
      const bool keep = preserves && next.preserves;
      if (!next.preserves) { targets.clear(); uninitialized.clear(); null_sources.clear(); value.clear(); }
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
  };
  struct Path { std::uint64_t inputs; Completion completion; size_t queue; Effects effects{}; };
  struct Action { bool guarded = false; std::vector<Path> paths; };
private:
  struct Queue { std::vector<size_t> previous; std::optional<size_t> action; std::set<size_t> initialized{}; };
  std::vector<Queue> queues_{{}};
  std::vector<Action> actions_;
  static constexpr size_t states = 2 * (3 + exception_categories.size());
  std::vector<size_t> reads_;
  using Relation = std::vector<std::map<size_t, Effects>>;
  std::vector<Relation> solved_;
  Effects compose(const Effects &first, const Effects &second) const {
    auto result = first;
    result.symbols.insert(second.symbols.begin(), second.symbols.end());
    for (const auto &[read, known] : second.reads) if (!known.satisfied(reads_.at(read), first.initialized, first.selections)) {
      auto before = known;
      before.initialized.insert(first.initialized.begin(), first.initialized.end());
      before.substitute(first.selections);
      if (before.satisfied(reads_.at(read), first.initialized, {})) continue;
      merge_read(result.reads, read, before);
    }
    result.initialized.insert(second.initialized.begin(), second.initialized.end());
    result.selections.then(second.selections, result.initialized);
    auto returned = second.returned;
    returned.value = returned.value.replace_arrival({}, first.thrown);
    result.returned.then(returned);
    result.returned.initialize(result.initialized);
    result.thrown = second.thrown.replace_arrival({}, first.thrown);
    return result;
  }
  static bool merge(std::map<size_t, Effects> &result, size_t completion, const Effects &effects) {
    if (!effects.selections.possible) return false;
    auto [found, inserted] = result.try_emplace(completion, effects);
    if (inserted) return true;
    auto &known = found->second;
    const auto previous_reads = known.reads;
    const auto previous_initialized = known.initialized;
    const auto previous_returned = known.returned;
    const auto previous_thrown = known.thrown;
    const auto previous_selections = known.selections;
    const auto previous_symbols = known.symbols;
    known.symbols.insert(effects.symbols.begin(), effects.symbols.end());
    for (const auto &[read, before] : effects.reads) merge_read(known.reads, read, before);
    std::erase_if(known.initialized, [&](size_t id) { return !effects.initialized.contains(id); });
    known.returned.merge(effects.returned);
    known.thrown.merge(effects.thrown);
    known.selections.merge(effects.selections);
    known.selections.resolve(known.initialized);
    return previous_reads != known.reads || previous_initialized != known.initialized ||
        previous_returned != known.returned || previous_thrown != known.thrown || previous_selections != known.selections || previous_symbols != known.symbols;
  }
public:
  static void merge_read(Reads &reads, size_t read, const ReadRequirement &before) {
    auto [found, inserted] = reads.emplace(read, before);
    if (!inserted) found->second.merge(before);
  }
  size_t read(size_t binding) { solved_.clear(); reads_.push_back(binding); return reads_.size() - 1; }
  size_t binding(size_t read) const { return reads_.at(read); }
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
  size_t action() { solved_.clear(); actions_.push_back({}); return actions_.size() - 1; }
  void define(size_t id, Action action) { solved_.clear(); actions_.at(id) = std::move(action); }
  size_t push(size_t previous, size_t action) {
    solved_.clear(); queues_.push_back({{previous}, action}); return queues_.size() - 1;
  }
  size_t join(size_t a, size_t b) {
    if (a == b) return a;
    solved_.clear(); queues_.push_back({{a, b}, {}}); return queues_.size() - 1;
  }
  size_t known(size_t queue, const std::set<size_t> &initialized) {
    if (initialized.empty()) return queue;
    solved_.clear(); queues_.push_back({{queue}, {}, initialized}); return queues_.size() - 1;
  }
  size_t header(size_t entry) {
    solved_.clear(); queues_.push_back({{entry}, {}}); return queues_.size() - 1;
  }
  void backedge(size_t header, size_t body) { solved_.clear(); queues_.at(header).previous.push_back(body); }
  const std::map<size_t, Effects> &apply(size_t queue, Completion input) {
    if (solved_.empty()) {
      solved_.assign(queues_.size(), Relation(states));
      for (size_t state = 0; state < states; ++state) solved_[0][state].emplace(state, Effects{});
      bool changed;
      do {
        changed = false;
        for (size_t id = 1; id < queues_.size(); ++id) {
          const auto &node = queues_[id];
          for (size_t state = 0; state < states; ++state) {
            auto &result = solved_[id][state];
            if (!node.action) {
              for (const auto previous : node.previous) for (const auto &[value, effects] : solved_[previous][state])
                changed = merge(result, value, compose({{}, node.initialized}, effects)) || changed;
              continue;
            }
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
              if (next.reason < 3) effects.thrown.clear();
              for (const auto &[intermediate, nested] : solved_[path.queue][next.index()]) {
                auto invoked = compose(effects, nested);
                const auto arrival = incoming.reason >= 3
                    ? ExceptionFacts::arrival().restricted({incoming.reason - 3}) : ExceptionFacts{};
                invoked.returned.value = invoked.returned.value.replace_arrival(*node.action, arrival);
                invoked.thrown = invoked.thrown.replace_arrival(*node.action, arrival);
                for (const auto &[value, previous] : solved_[node.previous.front()][intermediate])
                  changed = merge(result, value, compose(invoked, previous)) || changed;
              }
            }
          }
        }
      } while (changed);
    }
    return solved_.at(queue).at(input.index());
  }
};

} // namespace cod::language
