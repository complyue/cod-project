#pragma once

#include "./list.hh"

namespace shilos {

// YAML support for regional_fifo
template <typename T>
  requires yaml::YamlConvertible<T, void>
inline yaml::Node to_yaml(const regional_fifo<T> &fifo) noexcept {
  yaml::Node node;
  auto &seq = std::get<yaml::Sequence>(node.value = yaml::Sequence{});

  for (const auto &item : fifo) {
    seq.emplace_back(to_yaml(item));
  }

  return node;
}

template <typename T, typename RT>
  requires yaml::YamlConvertible<T, RT>
global_ptr<regional_fifo<T>, RT> from_yaml(memory_region<RT> &mr, const yaml::Node &node) {
  if (!std::holds_alternative<yaml::Sequence>(node.value)) {
    throw yaml::TypeError("Expected Sequence for regional_fifo");
  }

  auto fifo = mr.template create<regional_fifo<T>>(mr);
  const auto &seq = std::get<yaml::Sequence>(node.value);

  for (const auto &item_node : seq) {
    auto item = from_yaml<T>(mr, item_node);
    fifo->enque(mr, std::move(*item));
  }

  return fifo;
}

template <typename T, typename RT>
  requires yaml::YamlConvertible<T, RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_ptr<regional_fifo<T>> &to_ptr) {
  to_ptr = from_yaml<regional_fifo<T>>(mr, node);
}

// YAML support for regional_lifo
template <typename T>
  requires yaml::YamlConvertible<T, void>
inline yaml::Node to_yaml(const regional_lifo<T> &lifo) noexcept {
  yaml::Node node;
  auto &seq = std::get<yaml::Sequence>(node.value = yaml::Sequence{});

  for (const auto &item : lifo) {
    seq.emplace_back(to_yaml(item));
  }

  return node;
}

template <typename T, typename RT>
  requires yaml::YamlConvertible<T, RT>
global_ptr<regional_lifo<T>, RT> from_yaml(memory_region<RT> &mr, const yaml::Node &node) {
  if (!std::holds_alternative<yaml::Sequence>(node.value)) {
    throw yaml::TypeError("Expected Sequence for regional_lifo");
  }

  auto lifo = mr.template create<regional_lifo<T>>(mr);
  const auto &seq = std::get<yaml::Sequence>(node.value);

  // For LIFO, we want to preserve order when deserializing
  // Push items in reverse order so first item in YAML becomes top of stack
  for (auto it = seq.rbegin(); it != seq.rend(); ++it) {
    auto item = from_yaml<T>(mr, *it);
    lifo->push(mr, std::move(*item));
  }

  return lifo;
}

template <typename T, typename RT>
  requires yaml::YamlConvertible<T, RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_ptr<regional_lifo<T>> &to_ptr) {
  to_ptr = from_yaml<regional_lifo<T>>(mr, node);
}

} // namespace shilos
