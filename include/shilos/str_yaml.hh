#pragma once

#include "./str.hh"

namespace shilos {

// YAML support for regional_str

inline yaml::Node to_yaml(const regional_str &str, yaml::YamlAuthor &author) {
  return author.createString(std::string_view(str));
}

template <typename RT>
  requires ValidMemRegionRootType<RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_str *raw_ptr) {
  if (auto str = std::get_if<std::string_view>(&node.value)) {
    new (raw_ptr) regional_str(mr, *str);
  } else {
    throw yaml::TypeError("Invalid YAML node type for regional_str");
  }
}

} // namespace shilos
