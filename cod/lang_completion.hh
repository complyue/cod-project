//===--- lang_completion.hh - Finite deferred completion equations ---===//
#pragma once

#include "lang_exceptions.hh"

#include <cstddef>
#include <cstdint>
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
  struct Path { std::uint64_t inputs; Completion completion; size_t queue; };
  struct Action { bool guarded = false; std::vector<Path> paths; };
private:
  struct Queue { std::vector<size_t> previous; std::optional<size_t> action; };
  std::vector<Queue> queues_{{}};
  std::vector<Action> actions_;
  static constexpr size_t states = 2 * (3 + exception_categories.size());
  using Relation = std::vector<std::set<size_t>>;
  std::vector<Relation> solved_;
public:
  size_t action() { solved_.clear(); actions_.push_back({}); return actions_.size() - 1; }
  void define(size_t id, Action action) { solved_.clear(); actions_.at(id) = std::move(action); }
  size_t push(size_t previous, size_t action) {
    solved_.clear(); queues_.push_back({{previous}, action}); return queues_.size() - 1;
  }
  size_t join(size_t a, size_t b) {
    if (a == b) return a;
    solved_.clear(); queues_.push_back({{a, b}, {}}); return queues_.size() - 1;
  }
  size_t header(size_t entry) {
    solved_.clear(); queues_.push_back({{entry}, {}}); return queues_.size() - 1;
  }
  void backedge(size_t header, size_t body) { solved_.clear(); queues_.at(header).previous.push_back(body); }
  const std::set<size_t> &apply(size_t queue, Completion input) {
    if (solved_.empty()) {
      solved_.assign(queues_.size(), Relation(states));
      for (size_t state = 0; state < states; ++state) solved_[0][state].insert(state);
      bool changed;
      do {
        changed = false;
        for (size_t id = 1; id < queues_.size(); ++id) {
          const auto &node = queues_[id];
          for (size_t state = 0; state < states; ++state) {
            auto &result = solved_[id][state];
            const auto add = [&](const std::set<size_t> &values) {
              for (const auto value : values) changed = result.insert(value).second || changed;
            };
            if (!node.action) {
              for (const auto previous : node.previous) add(solved_[previous][state]);
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
              for (const auto intermediate : solved_[path.queue][next.index()])
                add(solved_[node.previous.front()][intermediate]);
            }
          }
        }
      } while (changed);
    }
    return solved_.at(queue).at(input.index());
  }
};

} // namespace cod::language
