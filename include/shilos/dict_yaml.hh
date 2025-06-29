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

    // Manually ensure vector space and dict resize
    dict->maybe_resize(mr);

    // Ensure entries vector has space
    if (!dict->entries_.last_segment_ || dict->entries_.last_segment_->is_full()) {
      auto new_segment = mr.template create<vector_segment<dict_entry<K, V>>>(mr);
      if (!dict->entries_.first_segment_) {
        dict->entries_.first_segment_ = dict->entries_.last_segment_ = new_segment.get();
      } else {
        dict->entries_.last_segment_->next() = new_segment.get();
        dict->entries_.last_segment_ = new_segment.get();
      }
      dict->entries_.segment_count_++;
    }

    // Get insertion location for new dict_entry
    size_t insert_index = dict->entries_.last_segment_->size();
    dict_entry<K, V> *entry_location = &dict->entries_.last_segment_->elements_[insert_index];

    // Initialize dict_entry structure
    entry_location->collision_next_index_ = dict_entry<K, V>::INVALID_INDEX;

    // Construct key and value directly in the dict_entry
    from_yaml(mr, key_node, &entry_location->key_);
    from_yaml(mr, value_node, &entry_location->value_);

    // Update vector bookkeeping
    dict->entries_.last_segment_->size_++;
    dict->entries_.total_size_++;

    // Update hash table
    size_t new_entry_idx = dict->entries_.size() - 1;
    size_t bucket_idx = dict->bucket_index(entry_location->key_);
    entry_location->set_collision_next_index(dict->buckets_[bucket_idx]);
    dict->buckets_[bucket_idx] = new_entry_idx;
  }

  return dict;
}

template <typename K, typename V, typename Hash, typename RT>
  requires yaml::YamlConvertible<K, RT> && yaml::YamlConvertible<V, RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_ptr<regional_dict<K, V, Hash>> &to_ptr) {
  // Allocate uninitialized dict
  auto dict_ptr = mr.template allocate<regional_dict<K, V, Hash>>();
  // Use raw pointer version to construct directly
  from_yaml(mr, node, dict_ptr.get());
  // Assign the raw pointer to the regional_ptr (assumes to_ptr is in memory region)
  to_ptr = dict_ptr.get();
}

template <typename K, typename V, typename Hash, typename RT>
  requires yaml::YamlConvertible<K, RT> && yaml::YamlConvertible<V, RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_dict<K, V, Hash> *raw_ptr) {
  if (!std::holds_alternative<yaml::Map>(node.value)) {
    throw yaml::TypeError("Expected Map for regional_dict");
  }

  new (raw_ptr) regional_dict<K, V, Hash>(mr);
  const auto &map = std::get<yaml::Map>(node.value);

  for (const auto &[key_str, value_node] : map) {
    // Parse the key string back to the key type
    yaml::Node key_node(key_str);

    // Manually ensure vector space and construct dict_entry directly
    raw_ptr->maybe_resize(mr);

    // Ensure entries vector has space
    if (!raw_ptr->entries_.last_segment_ || raw_ptr->entries_.last_segment_->is_full()) {
      auto new_segment = mr.template create<vector_segment<dict_entry<K, V>>>(mr);
      if (!raw_ptr->entries_.first_segment_) {
        raw_ptr->entries_.first_segment_ = raw_ptr->entries_.last_segment_ = new_segment.get();
      } else {
        raw_ptr->entries_.last_segment_->next() = new_segment.get();
        raw_ptr->entries_.last_segment_ = new_segment.get();
      }
      raw_ptr->entries_.segment_count_++;
    }

    // Get insertion location for new dict_entry
    size_t insert_index = raw_ptr->entries_.last_segment_->size();
    dict_entry<K, V> *entry_location = &raw_ptr->entries_.last_segment_->elements_[insert_index];

    // Initialize dict_entry structure
    entry_location->collision_next_index_ = dict_entry<K, V>::INVALID_INDEX;

    // Construct key and value directly in the dict_entry
    from_yaml<K>(mr, key_node, &entry_location->key_);
    from_yaml<V>(mr, value_node, &entry_location->value_);

    // Update vector bookkeeping
    raw_ptr->entries_.last_segment_->size_++;
    raw_ptr->entries_.total_size_++;

    // Update hash table
    size_t new_entry_idx = raw_ptr->entries_.size() - 1;
    size_t bucket_idx = raw_ptr->bucket_index(entry_location->key_);
    entry_location->set_collision_next_index(raw_ptr->buckets_[bucket_idx]);
    raw_ptr->buckets_[bucket_idx] = new_entry_idx;
  }
}

} // namespace shilos
