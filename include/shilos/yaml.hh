#pragma once

#include "./str.hh"

namespace shilos {

// YAML support for regional_str
inline yaml::Node to_yaml(const regional_str &str) noexcept { return yaml::Node(static_cast<std::string_view>(str)); }

template <typename RT>
  requires ValidMemRegionRootType<RT>
global_ptr<regional_str, RT> from_yaml(memory_region<RT> &mr, const yaml::Node &node) {
  if (auto str = std::get_if<std::string>(&node.value)) {
    return mr.template create<regional_str>(*str);
  }
  throw yaml::TypeError("Invalid YAML node type for regional_str");
}

template <typename RT>
  requires ValidMemRegionRootType<RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_ptr<regional_str> &to_ptr) {
  if (auto str = std::get_if<std::string>(&node.value)) {
    mr.template create_to<regional_ptr>(to_ptr, *str);
  } else {
    throw yaml::TypeError("Invalid YAML node type for regional_str");
  }
}

} // namespace shilos
