//===--- lang_completion.hh - Finite deferred completion equations ---===//
#pragma once

#include "lang_exceptions.hh"

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <utility>
#include <vector>

namespace cod::language {

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
  using Reads = std::map<size_t, std::set<size_t>>;
  struct Effects { Reads reads; std::set<size_t> initialized; };
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
    for (const auto &[read, known] : second.reads) if (!first.initialized.contains(reads_.at(read))) {
      auto before = known;
      before.insert(first.initialized.begin(), first.initialized.end());
      merge_read(result.reads, read, before);
    }
    result.initialized.insert(second.initialized.begin(), second.initialized.end());
    return result;
  }
  static bool merge(std::map<size_t, Effects> &result, size_t completion, const Effects &effects) {
    auto [found, inserted] = result.try_emplace(completion, effects);
    if (inserted) return true;
    auto &known = found->second;
    const auto previous_reads = known.reads;
    const auto previous_initialized = known.initialized.size();
    for (const auto &[read, before] : effects.reads) merge_read(known.reads, read, before);
    std::erase_if(known.initialized, [&](size_t id) { return !effects.initialized.contains(id); });
    return previous_reads != known.reads || previous_initialized != known.initialized.size();
  }
public:
  static void merge_read(Reads &reads, size_t read, const std::set<size_t> &before) {
    auto [found, inserted] = reads.emplace(read, before);
    if (!inserted) std::erase_if(found->second, [&](size_t id) { return !before.contains(id); });
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
              for (const auto &[intermediate, nested] : solved_[path.queue][next.index()]) {
                const auto invoked = compose(path.effects, nested);
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
