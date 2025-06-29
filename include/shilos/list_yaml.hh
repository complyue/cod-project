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
    // Allocate uninitialized space for the cons node
    auto node_ptr = mr.template allocate<regional_cons<T>>();

    // Initialize the next pointer to null
    new (&node_ptr->next()) regional_ptr<regional_cons<T>>();

    // Construct the value directly using raw pointer version of from_yaml
    from_yaml(mr, item_node, &node_ptr->value());

    // Link into the fifo
    if (!fifo->head_) {
      fifo->head_ = fifo->tail_ = node_ptr;
    } else {
      fifo->tail_->next() = node_ptr;
      fifo->tail_ = node_ptr;
    }
  }

  return fifo;
}

template <typename T, typename RT>
  requires yaml::YamlConvertible<T, RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_ptr<regional_fifo<T>> &to_ptr) {
  // Allocate uninitialized fifo
  auto fifo_ptr = mr.template allocate<regional_fifo<T>>();
  // Use raw pointer version to construct directly
  from_yaml(mr, node, fifo_ptr.get());
  // Assign the raw pointer to the regional_ptr (assumes to_ptr is in memory region)
  to_ptr = fifo_ptr.get();
}

template <typename T, typename RT>
  requires yaml::YamlConvertible<T, RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_fifo<T> *raw_ptr) {
  if (!std::holds_alternative<yaml::Sequence>(node.value)) {
    throw yaml::TypeError("Expected Sequence for regional_fifo");
  }

  new (raw_ptr) regional_fifo<T>(mr);
  const auto &seq = std::get<yaml::Sequence>(node.value);

  for (const auto &item_node : seq) {
    // Allocate uninitialized space for the cons node
    auto node_ptr = mr.template allocate<regional_cons<T>>();

    // Initialize the next pointer to null
    new (&node_ptr->next()) regional_ptr<regional_cons<T>>();

    // Construct the value directly using raw pointer version of from_yaml
    from_yaml<T>(mr, item_node, &node_ptr->value());

    // Link into the fifo
    if (!raw_ptr->head_) {
      raw_ptr->head_ = raw_ptr->tail_ = node_ptr;
    } else {
      raw_ptr->tail_->next() = node_ptr;
      raw_ptr->tail_ = node_ptr;
    }
  }
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
    // Allocate uninitialized space for the cons node
    auto node_ptr = mr.template allocate<regional_cons<T>>();

    // Initialize the next pointer to null
    new (&node_ptr->next()) regional_ptr<regional_cons<T>>();

    // Construct the value directly using raw pointer version of from_yaml
    from_yaml(mr, *it, &node_ptr->value());

    // Link into the lifo (add to front/top)
    node_ptr->next() = lifo->head_.get();
    if (!lifo->head_) {
      lifo->tail_ = node_ptr;
    }
    lifo->head_ = node_ptr;
  }

  return lifo;
}

template <typename T, typename RT>
  requires yaml::YamlConvertible<T, RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_ptr<regional_lifo<T>> &to_ptr) {
  // Allocate uninitialized lifo
  auto lifo_ptr = mr.template allocate<regional_lifo<T>>();
  // Use raw pointer version to construct directly
  from_yaml(mr, node, lifo_ptr.get());
  // Assign the raw pointer to the regional_ptr (assumes to_ptr is in memory region)
  to_ptr = lifo_ptr.get();
}

template <typename T, typename RT>
  requires yaml::YamlConvertible<T, RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_lifo<T> *raw_ptr) {
  if (!std::holds_alternative<yaml::Sequence>(node.value)) {
    throw yaml::TypeError("Expected Sequence for regional_lifo");
  }

  new (raw_ptr) regional_lifo<T>(mr);
  const auto &seq = std::get<yaml::Sequence>(node.value);

  // For LIFO, we want to preserve order when deserializing
  // Push items in reverse order so first item in YAML becomes top of stack
  for (auto it = seq.rbegin(); it != seq.rend(); ++it) {
    // Allocate uninitialized space for the cons node
    auto node_ptr = mr.template allocate<regional_cons<T>>();

    // Initialize the next pointer to null
    new (&node_ptr->next()) regional_ptr<regional_cons<T>>();

    // Construct the value directly using raw pointer version of from_yaml
    from_yaml<T>(mr, *it, &node_ptr->value());

    // Link into the lifo (add to front/top)
    node_ptr->next() = raw_ptr->head_.get();
    if (!raw_ptr->head_) {
      raw_ptr->tail_ = node_ptr;
    }
    raw_ptr->head_ = node_ptr;
  }
}

} // namespace shilos
