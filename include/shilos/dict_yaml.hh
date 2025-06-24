#pragma once

#include "./dict.hh"

namespace shilos {

// YAML support for regional_dict
template <typename K, typename V, typename Hash>
  requires yaml::YamlConvertible<K, void> && yaml::YamlConvertible<V, void>
inline yaml::Node to_yaml(const regional_dict<K, V, Hash> &dict) noexcept {
  yaml::Node node;
  auto &map = std::get<yaml::Map>(node.value = yaml::Map{});

  for (const auto &[key, value] : dict) {
    // Convert key to YAML and then to string for the map key
    auto key_node = to_yaml(key);
    std::string key_str;

    if (auto str_ptr = std::get_if<std::string>(&key_node.value)) {
      key_str = *str_ptr;
    } else {
      // For non-string keys, we'll format them as YAML and use that as the key
      key_str = yaml::format_yaml(key_node);
    }

    map[key_str] = to_yaml(value);
  }

  return node;
}

template <typename K, typename V, typename Hash, typename RT>
  requires yaml::YamlConvertible<K, RT> && yaml::YamlConvertible<V, RT>
global_ptr<regional_dict<K, V, Hash>, RT> from_yaml(memory_region<RT> &mr, const yaml::Node &node) {
  if (!std::holds_alternative<yaml::Map>(node.value)) {
    throw yaml::TypeError("Expected Map for regional_dict");
  }

  auto dict = mr.template create<regional_dict<K, V, Hash>>(mr);
  const auto &map = std::get<yaml::Map>(node.value);

  for (const auto &[key_str, value_node] : map) {
    // Parse the key string back to the key type
    yaml::Node key_node(key_str);
    auto key = from_yaml<K>(mr, key_node);
    auto value = from_yaml<V>(mr, value_node);

    dict->put(mr, *key, std::move(*value));
  }

  return dict;
}

template <typename K, typename V, typename Hash, typename RT>
  requires yaml::YamlConvertible<K, RT> && yaml::YamlConvertible<V, RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_ptr<regional_dict<K, V, Hash>> &to_ptr) {
  to_ptr = from_yaml<regional_dict<K, V, Hash>>(mr, node);
}

} // namespace shilos
