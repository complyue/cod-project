#pragma once

#include "./prelude.hh"
#include "./vector.hh"
#include <type_traits>

namespace shilos {

// ============================================================================
// YAML SERIALISATION SUPPORT FOR regional_vector<T>
// ----------------------------------------------------------------------------
//  The implementation follows the YamlConvertible specification described in
//  MemoryRegionAndRegionalTypes.md.  A regional_vector<T> can be converted to
//  a yaml::Node as a YAML sequence.  Vice-versa, a YAML sequence can be
//  deserialised directly into an **uninitialised** regional_vector<T> that
//  resides inside a memory_region<RT> by using the free function
//  `from_yaml(mr, node, raw_ptr)` defined below.
//
//  Requirements on the element type T:
//    1.  T must itself satisfy the YamlConvertible concept for the same
//        memory-region root type RT.  That is, both `to_yaml(const T&)` and
//        `from_yaml<T>(mr, node, raw_ptr)` must be available.
//    2.  The standard regional-type restrictions (no copy / move, etc.) apply.
//
//  The implementation purposely bypasses the public emplace_back() interface
//  and populates vector segments directly.  This avoids the need for an
//  intermediate temporary and allows us to construct each element in-place via
//  the lower-level `from_yaml` routine, thereby respecting the lvalue-only
//  constraint of regional types.
// ============================================================================

template <typename T> inline yaml::Node to_yaml(const regional_vector<T> &vec) noexcept {
  yaml::Node seq(yaml::Sequence{});
  for (const auto &elem : vec) {
    // If ADL finds a dedicated to_yaml overload for T, use it.
    if constexpr (requires(const T &e) { to_yaml(e); }) {
      seq.push_back(to_yaml(elem));
    } else if constexpr (std::is_same_v<T, bool>) {
      seq.push_back(yaml::Node(elem));
    } else if constexpr (std::is_integral_v<T>) {
      seq.push_back(yaml::Node(static_cast<int64_t>(elem)));
    } else if constexpr (std::is_floating_point_v<T>) {
      seq.push_back(yaml::Node(static_cast<double>(elem)));
    } else {
      static_assert(sizeof(T) == 0, "Element type of regional_vector does not support YAML serialisation");
    }
  }
  return seq;
}

// ---------------------------------------------------------------------------
//  Deserialisation: raw pointer overload (primary implementation)
// ---------------------------------------------------------------------------

template <typename T, typename RT>
  requires ValidMemRegionRootType<RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_vector<T> *raw_ptr) {
  // Validate input – we expect a YAML sequence.
  if (!node.IsSequence()) {
    throw yaml::TypeError("YAML node for regional_vector must be a sequence");
  }

  // Construct an empty regional_vector in-place.
  new (raw_ptr) regional_vector<T>(mr);
  auto &vec = *raw_ptr;

  const auto &seq = std::get<yaml::Sequence>(node.value);

  // Iterate and append.
  for (const auto &elem_node : seq) {
    if constexpr (std::is_same_v<T, bool> || std::is_integral_v<T> || std::is_floating_point_v<T>) {
      // Parse scalar directly for trivially-copyable "bits" element types.
      if (!elem_node.IsScalar()) {
        throw yaml::TypeError("Expected scalar value for bits element in regional_vector");
      }
      vec.emplace_back(mr, elem_node.as<T>());
    } else {
      // Generic path: delegate to element's own from_yaml.
      vec.emplace_init(mr, [&](T *dst) { from_yaml(mr, elem_node, dst); });
    }
  }
}

// ---------------------------------------------------------------------------
//  Deserialisation: helper that allocates the vector inside the region and
//  returns a global_ptr<regional_vector<T>, RT>
// ---------------------------------------------------------------------------

template <typename T, typename RT>
  requires ValidMemRegionRootType<RT>
global_ptr<regional_vector<T>, RT> vector_from_yaml(memory_region<RT> &mr, const yaml::Node &node) {
  auto raw_ptr = mr.template allocate<regional_vector<T>>();
  from_yaml<T>(mr, node, raw_ptr);
  return mr.cast_ptr(raw_ptr);
}

} // namespace shilos
