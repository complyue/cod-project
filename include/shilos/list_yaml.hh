#pragma once

#include "./list.hh"
#include "./prelude.hh"
#include <type_traits>

namespace shilos {

// ============================================================================
// YAML SERIALISATION SUPPORT FOR regional_fifo<T> / regional_lifo<T>
// ----------------------------------------------------------------------------
//  Representation: YAML sequence in the natural iteration order.
//
//  Deserialisation strategy mirrors vector_yaml.hh:
//    • Fast-path for bits / arithmetic / bool element types (direct scalar).
//    • Generic path creates default element (via container API) then delegates
//      to element's own from_yaml for regional types or nested containers.
//
//  NOTE: Both containers share identical semantics; implementation is factored
//  through an internal helper to avoid duplication.
// ============================================================================

template <template <typename> class ListC, typename T>
inline yaml::Node list_to_yaml(const ListC<T> &lst, yaml::YamlAuthor &author) {
  auto seq = author.createSequence();
  for (const auto &elem : lst) {
    if constexpr (requires { to_yaml(elem, author); }) {
      author.pushToSequence(seq, to_yaml(elem, author));
    } else if constexpr (std::is_same_v<T, bool>) {
      author.pushToSequence(seq, yaml::Node(elem));
    } else if constexpr (std::is_integral_v<T>) {
      author.pushToSequence(seq, yaml::Node(static_cast<int64_t>(elem)));
    } else if constexpr (std::is_floating_point_v<T>) {
      author.pushToSequence(seq, yaml::Node(static_cast<double>(elem)));
    } else {
      static_assert(sizeof(T) == 0, "Element type of regional list does not support YAML serialisation");
    }
  }
  return seq;
}

template <typename T> inline yaml::Node to_yaml(const regional_fifo<T> &lst, yaml::YamlAuthor &author) {
  return list_to_yaml<regional_fifo>(lst, author);
}

template <typename T> inline yaml::Node to_yaml(const regional_lifo<T> &lst, yaml::YamlAuthor &author) {
  return list_to_yaml<regional_lifo>(lst, author);
}

// ---------------------------------------------------------------------------
// Deserialisation helpers
// ---------------------------------------------------------------------------

template <template <typename> class ListC, typename T, typename RT>
static void list_from_yaml_impl(memory_region<RT> &mr, const yaml::Node &node, ListC<T> *raw_ptr) {
  if (!node.IsSequence())
    throw yaml::TypeError("YAML node for regional list must be a sequence");

  new (raw_ptr) ListC<T>(mr);
  auto &lst = *raw_ptr;

  const auto &seq = std::get<yaml::Sequence>(node.value);
  for (const auto &elem_node : seq) {
    if constexpr (std::is_same_v<T, bool> || std::is_integral_v<T> || std::is_floating_point_v<T>) {
      if (!elem_node.IsScalar())
        throw yaml::TypeError("Expected scalar for bits element in regional list");

      if constexpr (std::is_same_v<T, bool>) {
        lst.emplace_init(mr, [&](T *dst) { new (dst) T(elem_node.asBool()); });
      } else if constexpr (std::is_same_v<T, int>) {
        lst.emplace_init(mr, [&](T *dst) { new (dst) T(elem_node.asInt()); });
      } else if constexpr (std::is_same_v<T, int64_t>) {
        lst.emplace_init(mr, [&](T *dst) { new (dst) T(elem_node.asInt64()); });
      } else if constexpr (std::is_same_v<T, float>) {
        lst.emplace_init(mr, [&](T *dst) { new (dst) T(elem_node.asFloat()); });
      } else if constexpr (std::is_same_v<T, double>) {
        lst.emplace_init(mr, [&](T *dst) { new (dst) T(elem_node.asDouble()); });
      } else {
        static_assert(sizeof(T) == 0, "Unsupported scalar type for regional list");
      }
    } else {
      lst.emplace_init(mr, [&](T *dst) { from_yaml(mr, elem_node, dst); });
    }
  }
}

// Primary deserialisers ------------------------------------------------------

template <typename T, typename RT>
  requires ValidMemRegionRootType<RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_fifo<T> *raw_ptr) {
  list_from_yaml_impl<regional_fifo>(mr, node, raw_ptr);
}

template <typename T, typename RT>
  requires ValidMemRegionRootType<RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_lifo<T> *raw_ptr) {
  list_from_yaml_impl<regional_lifo>(mr, node, raw_ptr);
}

// Helper allocators ----------------------------------------------------------

template <typename T, typename RT>
  requires ValidMemRegionRootType<RT>
global_ptr<regional_fifo<T>, RT> fifo_from_yaml(memory_region<RT> &mr, const yaml::Node &node) {
  auto raw_ptr = mr.template allocate<regional_fifo<T>>();
  from_yaml<T>(mr, node, raw_ptr);
  return mr.cast_ptr(raw_ptr);
}

template <typename T, typename RT>
  requires ValidMemRegionRootType<RT>
global_ptr<regional_lifo<T>, RT> lifo_from_yaml(memory_region<RT> &mr, const yaml::Node &node) {
  auto raw_ptr = mr.template allocate<regional_lifo<T>>();
  from_yaml<T>(mr, node, raw_ptr);
  return mr.cast_ptr(raw_ptr);
}

} // namespace shilos
