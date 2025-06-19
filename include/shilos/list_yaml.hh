#pragma once

#include "./list.hh"

namespace shilos {

// YAML support for regional_list
template <typename T>
yaml::Node to_yaml(const regional_list<T> &list)
  requires yaml::YamlConvertible<T, void>
{
  yaml::Sequence seq;
  for (const auto &item : list) {
    seq.emplace_back(item.to_yaml());
  }
  return yaml::Node(seq);
}

template <typename RT, typename T>
  requires yaml::YamlConvertible<T, RT>
global_ptr<regional_list<T>, RT> from_yaml(memory_region<RT> &mr, const yaml::Node &node) {
  if (!std::holds_alternative<yaml::Sequence>(node.value)) {
    throw yaml::TypeError("Expected Sequence for regional_list");
  }
  auto list = mr.template create<regional_list<T>>();
  const auto &seq = std::get<yaml::Sequence>(node.value);

  for (const auto &item : seq) {
    if (!list->head()) {
      // Create first element
      mr.create_to(list->head(), mr, item);
    } else {
      // Append to existing list
      list->head()->append(mr, item);
    }
  }
  return list;
}

template <typename RT, typename T>
  requires yaml::YamlConvertible<T, RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_ptr<regional_list<T>> &to_ptr) {
  to_ptr = from_yaml<RT, T>(mr, node);
}

} // namespace shilos
