#pragma once

#include "./vector.hh"

namespace shilos {

// YAML support for regional_vector
template <typename T>
  requires yaml::YamlConvertible<T, void>
inline yaml::Node to_yaml(const regional_vector<T> &vector) noexcept {
  yaml::Node node;
  auto &seq = std::get<yaml::Sequence>(node.value = yaml::Sequence{});

  for (const auto &item : vector) {
    seq.emplace_back(to_yaml(item));
  }

  return node;
}

template <typename T, typename RT>
  requires yaml::YamlConvertible<T, RT>
global_ptr<regional_vector<T>, RT> from_yaml(memory_region<RT> &mr, const yaml::Node &node) {
  if (!std::holds_alternative<yaml::Sequence>(node.value)) {
    throw yaml::TypeError("Expected Sequence for regional_vector");
  }

  auto vector = mr.template create<regional_vector<T>>(mr);
  const auto &seq = std::get<yaml::Sequence>(node.value);

  for (const auto &item_node : seq) {
    auto item = from_yaml<T>(mr, item_node);
    vector->push_back(mr, std::move(*item));
  }

  return vector;
}

template <typename T, typename RT>
  requires yaml::YamlConvertible<T, RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_ptr<regional_vector<T>> &to_ptr) {
  to_ptr = from_yaml<regional_vector<T>>(mr, node);
}

} // namespace shilos
