#pragma once

#include "./dict.hh"
#include "./prelude.hh"
#include <type_traits>

namespace shilos {

// ============================================================================
// YAML SERIALISATION SUPPORT FOR regional_dict<K,V,Hash>
// ----------------------------------------------------------------------------
//  Representation: YAML mapping where each key is serialized using the same
//  logic as standalone `to_yaml(key)` (or scalar fallback) and each value via
//  `to_yaml(value)`.
//
//  Deserialisation strategy:
//    • Build an empty dict in-place.
//    • Iterate over YAML map entries.
//    • For each entry
//        – Insert key (using try_emplace) to obtain value pointer
//        – For *bits* / arithmetic / bool value types assign directly
//        – Otherwise destroy default-constructed placeholder and call
//          element’s `from_yaml` for in-place construction.
//
//  Keys:
//    • When `K` is `regional_str` the lookup/insert helpers accept
//      `std::string_view`, so we parse scalar string and pass view.
//    • For arithmetic & bool key types we parse scalar directly.
// ============================================================================

template <typename K, typename V, typename Hash>
inline yaml::Node to_yaml(const regional_dict<K, V, Hash> &d) noexcept {
  yaml::Node m(yaml::Map{});
  for (const auto &[k, v] : d) {
    yaml::Node key_node;
    if constexpr (requires { to_yaml(k); }) {
      key_node = to_yaml(k);
    } else if constexpr (std::is_same_v<K, bool>) {
      key_node = yaml::Node(k);
    } else if constexpr (std::is_integral_v<K>) {
      key_node = yaml::Node(static_cast<int64_t>(k));
    } else if constexpr (std::is_floating_point_v<K>) {
      key_node = yaml::Node(static_cast<double>(k));
    } else if constexpr (std::is_same_v<K, regional_str>) {
      key_node = yaml::Node(std::string_view(k));
    } else {
      static_assert(sizeof(K) == 0, "Key type of regional_dict is not serialisable to YAML");
    }

    // Value node
    yaml::Node value_node;
    if constexpr (requires { to_yaml(v); }) {
      value_node = to_yaml(v);
    } else if constexpr (std::is_same_v<V, bool>) {
      value_node = yaml::Node(v);
    } else if constexpr (std::is_integral_v<V>) {
      value_node = yaml::Node(static_cast<int64_t>(v));
    } else if constexpr (std::is_floating_point_v<V>) {
      value_node = yaml::Node(static_cast<double>(v));
    } else {
      static_assert(sizeof(V) == 0, "Value type of regional_dict is not serialisable to YAML");
    }

    // Insert into map – we must convert key_node to string/int key for YAML library.
    if (auto str = std::get_if<std::string_view>(&key_node.value)) {
      m[*str] = value_node;
    } else if (auto i = std::get_if<int64_t>(&key_node.value)) {
      m[std::to_string(*i)] = value_node; // YAML keys must be strings
    } else {
      // Fallback: stringify YAML node
      m[yaml::format_yaml(key_node)] = value_node;
    }
  }
  return m;
}

// ---------------------------------------------------------------------------
// Deserialisation
// ---------------------------------------------------------------------------

template <typename K, typename V, typename Hash, typename RT>
  requires ValidMemRegionRootType<RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_dict<K, V, Hash> *raw_ptr) {
  if (!node.IsMap())
    throw yaml::TypeError("YAML node for regional_dict must be a mapping");

  new (raw_ptr) regional_dict<K, V, Hash>(mr);
  auto &dict = *raw_ptr;

  const auto &map = std::get<yaml::Map>(node.value);

  for (const auto &entry : map) {
    const auto &k_node_str = entry.key;
    const auto &v_node = entry.value;

    // --- Parse key ---------------------------------------------------------
    std::remove_cv_t<K> key_storage{};
    auto make_key_callable = [&](auto &&key_arg, auto &&init_fn) {
      dict.emplace_init(mr, std::forward<decltype(key_arg)>(key_arg), std::forward<decltype(init_fn)>(init_fn));
    };

    if constexpr (std::is_same_v<K, regional_str>) {
      auto key_view = std::string_view(k_node_str);
      if constexpr (std::is_same_v<V, bool> || std::is_integral_v<V> || std::is_floating_point_v<V>) {
        make_key_callable(key_view, [&](V *dst) { new (dst) V(v_node.as<V>()); });
      } else {
        make_key_callable(key_view, [&](V *dst) { from_yaml(mr, v_node, dst); });
      }
    } else if constexpr (std::is_same_v<K, bool>) {
      bool key_bool = (k_node_str == "true" || k_node_str == "1");
      if constexpr (std::is_same_v<V, bool> || std::is_integral_v<V> || std::is_floating_point_v<V>) {
        make_key_callable(key_bool, [&](V *dst) { new (dst) V(v_node.as<V>()); });
      } else {
        make_key_callable(key_bool, [&](V *dst) { from_yaml(mr, v_node, dst); });
      }
    } else if constexpr (std::is_integral_v<K>) {
      K key_int = static_cast<K>(std::stoll(std::string(k_node_str)));
      if constexpr (std::is_same_v<V, bool> || std::is_integral_v<V> || std::is_floating_point_v<V>) {
        make_key_callable(key_int, [&](V *dst) { new (dst) V(v_node.as<V>()); });
      } else {
        make_key_callable(key_int, [&](V *dst) { from_yaml(mr, v_node, dst); });
      }
    } else if constexpr (std::is_floating_point_v<K>) {
      K key_fp = static_cast<K>(std::stod(std::string(k_node_str)));
      if constexpr (std::is_same_v<V, bool> || std::is_integral_v<V> || std::is_floating_point_v<V>) {
        make_key_callable(key_fp, [&](V *dst) { new (dst) V(v_node.as<V>()); });
      } else {
        make_key_callable(key_fp, [&](V *dst) { from_yaml(mr, v_node, dst); });
      }
    } else {
      yaml::Node key_scalar(k_node_str);
      from_yaml(mr, key_scalar, &key_storage);
      if constexpr (std::is_same_v<V, bool> || std::is_integral_v<V> || std::is_floating_point_v<V>) {
        make_key_callable(key_storage, [&](V *dst) { new (dst) V(v_node.as<V>()); });
      } else {
        make_key_callable(key_storage, [&](V *dst) { from_yaml(mr, v_node, dst); });
      }
    }
  }
}

// Helper allocator -----------------------------------------------------------

template <typename K, typename V, typename Hash, typename RT>
  requires ValidMemRegionRootType<RT>
global_ptr<regional_dict<K, V, Hash>, RT> dict_from_yaml(memory_region<RT> &mr, const yaml::Node &node) {
  auto raw_ptr = mr.template allocate<regional_dict<K, V, Hash>>();
  from_yaml<K, V, Hash>(mr, node, raw_ptr);
  return mr.cast_ptr(raw_ptr);
}

} // namespace shilos
