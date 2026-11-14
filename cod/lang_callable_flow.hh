//===--- lang_callable_flow.hh - Finite callable value propagation ---===//
#pragma once

#include <cstddef>
#include <deque>
#include <set>
#include <utility>
#include <vector>

namespace cod::language {

class CallableFlow {
public:
  using Node = size_t;
  using Edges = std::vector<std::pair<Node, Node>>;
private:
  struct Gate { size_t value; Edges edges; bool active = false; };
  struct State {
    std::set<size_t> values;
    std::set<Node> outputs;
    std::vector<Gate> gates;
    bool queued = false;
  };
  std::vector<State> nodes_;
  std::deque<Node> pending_;
  void enqueue(Node node) {
    if (nodes_[node].queued) return;
    nodes_[node].queued = true; pending_.push_back(node);
  }
  void propagate(Node from, Node to) {
    const auto before = nodes_[to].values.size();
    nodes_[to].values.insert(nodes_[from].values.begin(), nodes_[from].values.end());
    if (nodes_[to].values.size() != before) enqueue(to);
  }
public:
  Node node() { nodes_.emplace_back(); return nodes_.size() - 1; }
  void seed(Node node, size_t value) {
    if (nodes_[node].values.insert(value).second) enqueue(node);
  }
  void link(Node from, Node to) {
    if (from != to && nodes_[from].outputs.insert(to).second) propagate(from, to);
  }
  void when(Node node, size_t value, Edges edges) {
    nodes_[node].gates.push_back({value, std::move(edges)}); enqueue(node);
  }
  void solve() {
    while (!pending_.empty()) {
      const auto node = pending_.front(); pending_.pop_front(); nodes_[node].queued = false;
      for (const auto output : nodes_[node].outputs) propagate(node, output);
      for (auto &gate : nodes_[node].gates) if (!gate.active && nodes_[node].values.contains(gate.value)) {
        gate.active = true;
        for (const auto &[from, to] : gate.edges) link(from, to);
      }
    }
  }
  const std::set<size_t> &values(Node node) const { return nodes_[node].values; }
};

} // namespace cod::language
