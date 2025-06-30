#pragma once

#include "./region.hh"
#include "./vector.hh"

#include <cassert>
#include <concepts>
#include <functional>

namespace shilos {

// Forward declaration
template <typename K, typename V, typename Hash> class regional_dict;

// Concepts for heterogeneous key operations
template <typename KeyType, typename K, typename Hash, typename RT>
concept CompatibleKey = requires(const KeyType &key_type, const K &k, const Hash &hasher, memory_region<RT> &mr) {
  // Must be able to hash both types consistently
  { hasher(key_type) } -> std::convertible_to<std::size_t>;
  { hasher(k) } -> std::convertible_to<std::size_t>;
  // Must be able to compare KeyType with K
  { key_type == k } -> std::convertible_to<bool>;
  { k == key_type } -> std::convertible_to<bool>;
  // Must be able to construct K from memory_region and KeyType for insertion
  requires std::constructible_from<K, memory_region<RT> &, KeyType>;
};

template <typename K, typename V> class dict_entry {
  // Friend declaration for raw pointer YAML functions
  template <typename K1, typename V1, typename H1, typename RT>
  friend void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_dict<K1, V1, H1> *raw_ptr);

private:
  K key_;
  V value_;
  size_t collision_next_index_; // Index of next entry in collision chain (or INVALID_INDEX)

public:
  static constexpr size_t INVALID_INDEX = SIZE_MAX;

  template <typename RT, typename... KeyArgs, typename... ValueArgs>
    requires std::constructible_from<K, memory_region<RT> &, KeyArgs...> &&
                 std::constructible_from<V, memory_region<RT> &, ValueArgs...>
  dict_entry(memory_region<RT> &mr, KeyArgs &&...key_args, ValueArgs &&...value_args)
      : key_(mr, std::forward<KeyArgs>(key_args)...), value_(mr, std::forward<ValueArgs>(value_args)...),
        collision_next_index_(INVALID_INDEX) {}

  template <typename RT, typename... KeyArgs, typename... ValueArgs>
    requires std::constructible_from<K, KeyArgs...> && std::constructible_from<V, ValueArgs...>
  dict_entry(memory_region<RT> &mr, KeyArgs &&...key_args, ValueArgs &&...value_args)
      : key_(std::forward<KeyArgs>(key_args)...), value_(std::forward<ValueArgs>(value_args)...),
        collision_next_index_(INVALID_INDEX) {}

  template <typename RT, typename... KeyArgs, typename... ValueArgs>
    requires std::constructible_from<K, KeyArgs...> && std::constructible_from<V, memory_region<RT> &, ValueArgs...>
  dict_entry(memory_region<RT> &mr, KeyArgs &&...key_args, ValueArgs &&...value_args)
      : key_(std::forward<KeyArgs>(key_args)...), value_(mr, std::forward<ValueArgs>(value_args)...),
        collision_next_index_(INVALID_INDEX) {}

  template <typename RT, typename... KeyArgs, typename... ValueArgs>
    requires std::constructible_from<K, memory_region<RT> &, KeyArgs...> && std::constructible_from<V, ValueArgs...>
  dict_entry(memory_region<RT> &mr, KeyArgs &&...key_args, ValueArgs &&...value_args)
      : key_(mr, std::forward<KeyArgs>(key_args)...), value_(std::forward<ValueArgs>(value_args)...),
        collision_next_index_(INVALID_INDEX) {}

  // Deleted special members
  dict_entry(const dict_entry &) = delete;
  dict_entry(dict_entry &&) = delete;
  dict_entry &operator=(const dict_entry &) = delete;
  dict_entry &operator=(dict_entry &&) = delete;

  K &key() { return key_; }
  const K &key() const { return key_; }

  V &value() { return value_; }
  const V &value() const { return value_; }

  size_t collision_next_index() const { return collision_next_index_; }
  void set_collision_next_index(size_t index) { collision_next_index_ = index; }
};

template <typename K, typename V, typename Hash = std::hash<K>> class regional_dict {
  // Friend declarations for YAML functions
  template <typename K1, typename V1, typename H1, typename RT>
  friend void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_dict<K1, V1, H1> *raw_ptr);
  template <typename K1, typename V1, typename H1, typename RT>
  friend global_ptr<regional_dict<K1, V1, H1>, RT> from_yaml(memory_region<RT> &mr, const yaml::Node &node);

private:
  regional_vector<dict_entry<K, V>> entries_; // All entries in insertion order
  regional_vector<size_t> buckets_;           // Hash table: indices into entries_
  Hash hasher_;

  static constexpr double MAX_LOAD_FACTOR = 0.75;
  static constexpr size_t INITIAL_BUCKET_COUNT = 16;
  static constexpr size_t INVALID_INDEX = SIZE_MAX;

  template <typename KeyType, typename RT>
    requires CompatibleKey<KeyType, K, Hash, RT>
  size_t hash_key(const KeyType &key) const {
    return hasher_(key);
  }

  template <typename KeyType, typename RT>
    requires CompatibleKey<KeyType, K, Hash, RT>
  size_t bucket_index(const KeyType &key) const {
    return hash_key<KeyType, RT>(key) % buckets_.size();
  }

  template <typename RT> void maybe_resize(memory_region<RT> &mr) {
    if (buckets_.empty() || static_cast<double>(entries_.size()) / buckets_.size() > MAX_LOAD_FACTOR) {
      resize_hash_table(mr);
    }
  }

  template <typename RT> void resize_hash_table(memory_region<RT> &mr) {
    size_t new_bucket_count = buckets_.empty() ? INITIAL_BUCKET_COUNT : buckets_.size() * 2;

    // Ensure we have enough buckets
    while (buckets_.size() < new_bucket_count) {
      buckets_.emplace_back(mr, INVALID_INDEX);
    }

    // Clear all buckets
    for (size_t i = 0; i < new_bucket_count; ++i) {
      buckets_[i] = INVALID_INDEX;
    }

    // Rehash all entries
    for (size_t entry_idx = 0; entry_idx < entries_.size(); ++entry_idx) {
      dict_entry<K, V> &entry = entries_[entry_idx];

      // Clear collision chain for this entry
      entry.set_collision_next_index(INVALID_INDEX);

      // Insert into appropriate bucket
      size_t bucket_idx = (hasher_(entry.key())) % buckets_.size();
      entry.set_collision_next_index(buckets_[bucket_idx]);
      buckets_[bucket_idx] = entry_idx;
    }
  }

  template <typename KeyType, typename RT>
    requires CompatibleKey<KeyType, K, Hash, RT>
  size_t find_entry_index(const KeyType &key) const {
    if (buckets_.empty())
      return INVALID_INDEX;

    size_t bucket_idx = bucket_index<KeyType, RT>(key);
    size_t entry_idx = buckets_[bucket_idx];

    while (entry_idx != INVALID_INDEX) {
      if (entries_[entry_idx].key() == key) {
        return entry_idx;
      }
      entry_idx = entries_[entry_idx].collision_next_index();
    }

    return INVALID_INDEX;
  }

  // Non-templated version for exact key type K (backward compatibility)
  size_t find_entry_index_exact(const K &key) const {
    if (buckets_.empty())
      return INVALID_INDEX;

    size_t bucket_idx = (hasher_(key)) % buckets_.size();
    size_t entry_idx = buckets_[bucket_idx];

    while (entry_idx != INVALID_INDEX) {
      if (entries_[entry_idx].key() == key) {
        return entry_idx;
      }
      entry_idx = entries_[entry_idx].collision_next_index();
    }

    return INVALID_INDEX;
  }

public:
  regional_dict() : entries_(), buckets_(), hasher_() {}

  template <typename RT> regional_dict(memory_region<RT> &mr) : entries_(mr), buckets_(mr), hasher_() {}

  template <typename RT>
  regional_dict(memory_region<RT> &mr, const Hash &hash) : entries_(mr), buckets_(mr), hasher_(hash) {}

  // Deleted special members
  regional_dict(const regional_dict &) = delete;
  regional_dict(regional_dict &&) = delete;
  regional_dict &operator=(const regional_dict &) = delete;
  regional_dict &operator=(regional_dict &&) = delete;

  // Insert or update entry - supports heterogeneous keys for insertion
  template <typename RT, typename KeyType, typename... ValueArgs>
    requires CompatibleKey<KeyType, K, Hash, RT> && std::constructible_from<V, memory_region<RT> &, ValueArgs...>
  std::pair<V *, bool> put(memory_region<RT> &mr, const KeyType &key, ValueArgs &&...value_args) {
    maybe_resize(mr);

    // Check if key already exists
    size_t existing_idx = find_entry_index<KeyType, RT>(key);
    if (existing_idx != INVALID_INDEX) {
      // Update existing value
      dict_entry<K, V> &existing = entries_[existing_idx];
      existing.value().~V();
      new (&existing.value()) V(mr, std::forward<ValueArgs>(value_args)...);
      return {&existing.value(), false};
    }

    // Create new entry at end of entries vector (insertion order preserved)
    size_t new_entry_idx = entries_.size();
    entries_.emplace_back(mr, key, std::forward<ValueArgs>(value_args)...);

    // Add to hash table
    size_t bucket_idx = bucket_index<KeyType, RT>(key);
    entries_[new_entry_idx].set_collision_next_index(buckets_[bucket_idx]);
    buckets_[bucket_idx] = new_entry_idx;

    return {&entries_[new_entry_idx].value(), true};
  }

  template <typename RT, typename KeyType, typename... ValueArgs>
    requires CompatibleKey<KeyType, K, Hash, RT> && std::constructible_from<V, ValueArgs...>
  std::pair<V *, bool> put(memory_region<RT> &mr, const KeyType &key, ValueArgs &&...value_args) {
    maybe_resize(mr);

    // Check if key already exists
    size_t existing_idx = find_entry_index<KeyType, RT>(key);
    if (existing_idx != INVALID_INDEX) {
      // Update existing value
      dict_entry<K, V> &existing = entries_[existing_idx];
      existing.value().~V();
      new (&existing.value()) V(std::forward<ValueArgs>(value_args)...);
      return {&existing.value(), false};
    }

    // Create new entry at end of entries vector (insertion order preserved)
    size_t new_entry_idx = entries_.size();
    entries_.emplace_back(mr, key, std::forward<ValueArgs>(value_args)...);

    // Add to hash table
    size_t bucket_idx = bucket_index<KeyType, RT>(key);
    entries_[new_entry_idx].set_collision_next_index(buckets_[bucket_idx]);
    buckets_[bucket_idx] = new_entry_idx;

    return {&entries_[new_entry_idx].value(), true};
  }

  // Overloads for exact key type K (backward compatibility)
  template <typename RT, typename... ValueArgs>
    requires std::constructible_from<V, memory_region<RT> &, ValueArgs...>
  std::pair<V *, bool> put(memory_region<RT> &mr, const K &key, ValueArgs &&...value_args) {
    return put<RT, K, ValueArgs...>(mr, key, std::forward<ValueArgs>(value_args)...);
  }

  template <typename RT, typename... ValueArgs>
    requires std::constructible_from<V, ValueArgs...>
  std::pair<V *, bool> put(memory_region<RT> &mr, const K &key, ValueArgs &&...value_args) {
    return put<RT, K, ValueArgs...>(mr, key, std::forward<ValueArgs>(value_args)...);
  }

  // Lookup operations - support heterogeneous keys
  template <typename KeyType, typename RT>
    requires CompatibleKey<KeyType, K, Hash, RT>
  V *get(const KeyType &key) {
    size_t entry_idx = find_entry_index<KeyType, RT>(key);
    return entry_idx != INVALID_INDEX ? &entries_[entry_idx].value() : nullptr;
  }

  template <typename KeyType, typename RT>
    requires CompatibleKey<KeyType, K, Hash, RT>
  const V *get(const KeyType &key) const {
    size_t entry_idx = find_entry_index<KeyType, RT>(key);
    return entry_idx != INVALID_INDEX ? &entries_[entry_idx].value() : nullptr;
  }

  template <typename KeyType, typename RT>
    requires CompatibleKey<KeyType, K, Hash, RT>
  V &operator[](const KeyType &key) {
    size_t entry_idx = find_entry_index<KeyType, RT>(key);
    assert(entry_idx != INVALID_INDEX && "Key not found in dictionary");
    return entries_[entry_idx].value();
  }

  template <typename KeyType, typename RT>
    requires CompatibleKey<KeyType, K, Hash, RT>
  const V &operator[](const KeyType &key) const {
    size_t entry_idx = find_entry_index<KeyType, RT>(key);
    assert(entry_idx != INVALID_INDEX && "Key not found in dictionary");
    return entries_[entry_idx].value();
  }

  template <typename KeyType, typename RT>
    requires CompatibleKey<KeyType, K, Hash, RT>
  bool contains(const KeyType &key) const {
    return find_entry_index<KeyType, RT>(key) != INVALID_INDEX;
  }

  // Backward compatibility overloads for exact key type K (no RT needed)
  V *get(const K &key) {
    size_t entry_idx = find_entry_index_exact(key);
    return entry_idx != INVALID_INDEX ? &entries_[entry_idx].value() : nullptr;
  }

  const V *get(const K &key) const {
    size_t entry_idx = find_entry_index_exact(key);
    return entry_idx != INVALID_INDEX ? &entries_[entry_idx].value() : nullptr;
  }

  V &operator[](const K &key) {
    size_t entry_idx = find_entry_index_exact(key);
    assert(entry_idx != INVALID_INDEX && "Key not found in dictionary");
    return entries_[entry_idx].value();
  }

  const V &operator[](const K &key) const {
    size_t entry_idx = find_entry_index_exact(key);
    assert(entry_idx != INVALID_INDEX && "Key not found in dictionary");
    return entries_[entry_idx].value();
  }

  bool contains(const K &key) const { return find_entry_index_exact(key) != INVALID_INDEX; }

  // Capacity
  bool empty() const { return entries_.empty(); }
  size_t size() const { return entries_.size(); }
  size_t bucket_count() const { return buckets_.size(); }
  double load_factor() const { return buckets_.empty() ? 0.0 : static_cast<double>(entries_.size()) / buckets_.size(); }

  // Insertion order iteration - just iterate over entries vector!
  class iterator {
    typename regional_vector<dict_entry<K, V>>::iterator it_;

  public:
    iterator(typename regional_vector<dict_entry<K, V>>::iterator it) : it_(it) {}

    std::pair<const K &, V &> operator*() { return {it_->key(), it_->value()}; }

    iterator &operator++() {
      ++it_;
      return *this;
    }

    iterator operator++(int) {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const iterator &other) const { return it_ == other.it_; }
    bool operator!=(const iterator &other) const { return it_ != other.it_; }
  };

  class const_iterator {
    typename regional_vector<dict_entry<K, V>>::const_iterator it_;

  public:
    const_iterator(typename regional_vector<dict_entry<K, V>>::const_iterator it) : it_(it) {}

    std::pair<const K &, const V &> operator*() const { return {it_->key(), it_->value()}; }

    const_iterator &operator++() {
      ++it_;
      return *this;
    }

    const_iterator operator++(int) {
      const_iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const const_iterator &other) const { return it_ == other.it_; }
    bool operator!=(const const_iterator &other) const { return it_ != other.it_; }
  };

  iterator begin() { return iterator(entries_.begin()); }
  iterator end() { return iterator(entries_.end()); }

  const_iterator begin() const { return const_iterator(entries_.begin()); }
  const_iterator end() const { return const_iterator(entries_.end()); }

  const_iterator cbegin() const { return begin(); }
  const_iterator cend() const { return end(); }
};

// Comparison operators
template <typename K, typename V, typename Hash>
bool operator==(const regional_dict<K, V, Hash> &lhs, const regional_dict<K, V, Hash> &rhs)
  requires std::equality_comparable<K> && std::equality_comparable<V>
{
  if (lhs.size() != rhs.size())
    return false;

  for (const auto &[key, value] : lhs) {
    const V *rhs_value = rhs.get(key);
    if (!rhs_value || !(*rhs_value == value)) {
      return false;
    }
  }

  return true;
}

} // namespace shilos
