#pragma once

#include "./region.hh"
#include "./str.hh"
#include "./vector.hh"

#include <cassert>
#include <concepts>
#include <functional>
#include <iterator>

namespace shilos {

// Forward declaration
template <typename K, typename V, typename Hash> class regional_dict;

// Simple trait to determine the common rvalue key type
template <typename K> struct common_key_type {
  using type = K; // Default: use the key type itself
};

// Specialization for regional_str: use std::string_view as common key type
template <> struct common_key_type<regional_str> {
  using type = std::string_view;
};

template <typename K> using common_key_type_t = typename common_key_type<K>::type;

// Converter functions to get common key type
template <typename K> const K &to_common_key(const K &key) { return key; }

// Specialization for regional_str -> std::string_view
inline std::string_view to_common_key(const regional_str &key) { return key; }

// Overloads for lookup key types that convert to common key type
inline std::string_view to_common_key(std::string_view key) { return key; }

inline std::string_view to_common_key(const std::string &key) { return std::string_view(key); }

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

  // Default constructor (needed for vector_segment arrays)
  dict_entry() : key_(), value_(), collision_next_index_(INVALID_INDEX) {}

  // Standard constructors for constructing both key and value from arguments
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

  // New constructors for copying an existing key and constructing value from arguments
  template <typename RT, typename... ValueArgs>
    requires std::copy_constructible<K> && std::constructible_from<V, memory_region<RT> &, ValueArgs...>
  dict_entry(memory_region<RT> &mr, const K &existing_key, ValueArgs &&...value_args)
      : key_(existing_key), value_(mr, std::forward<ValueArgs>(value_args)...), collision_next_index_(INVALID_INDEX) {}

  template <typename RT, typename... ValueArgs>
    requires std::copy_constructible<K> && std::constructible_from<V, ValueArgs...>
  dict_entry(memory_region<RT> &mr, const K &existing_key, ValueArgs &&...value_args)
      : key_(existing_key), value_(std::forward<ValueArgs>(value_args)...), collision_next_index_(INVALID_INDEX) {}

  // Constructor for copying existing key with default-constructed value (V needs memory_region)
  template <typename RT>
    requires std::copy_constructible<K> && std::constructible_from<V, memory_region<RT> &>
  dict_entry(memory_region<RT> &mr, const K &existing_key)
      : key_(existing_key), value_(mr), collision_next_index_(INVALID_INDEX) {}

  // Constructor for copying existing key with default-constructed value (V doesn't need memory_region)
  template <typename RT>
    requires std::copy_constructible<K> && std::constructible_from<V> &&
                 (!std::constructible_from<V, memory_region<RT> &>)
  dict_entry(memory_region<RT> &mr, const K &existing_key)
      : key_(existing_key), value_(), collision_next_index_(INVALID_INDEX) {}

  // Constructor for key-only with default-constructed value (V needs memory_region)
  template <typename RT, typename... KeyArgs>
    requires std::constructible_from<K, memory_region<RT> &, KeyArgs...> &&
                 std::constructible_from<V, memory_region<RT> &>
  dict_entry(memory_region<RT> &mr, KeyArgs &&...key_args)
      : key_(mr, std::forward<KeyArgs>(key_args)...), value_(mr), collision_next_index_(INVALID_INDEX) {}

  // Constructor for key-only with default-constructed value (V doesn't need memory_region)
  template <typename RT, typename... KeyArgs>
    requires std::constructible_from<K, memory_region<RT> &, KeyArgs...> && std::constructible_from<V> &&
                 (!std::constructible_from<V, memory_region<RT> &>)
  dict_entry(memory_region<RT> &mr, KeyArgs &&...key_args)
      : key_(mr, std::forward<KeyArgs>(key_args)...), value_(), collision_next_index_(INVALID_INDEX) {}

  // Special constructor for existing regional_str key with string_view value construction
  template <typename RT>
    requires std::same_as<K, regional_str> && std::same_as<V, regional_str>
  dict_entry(memory_region<RT> &mr, const regional_str &existing_key, std::string_view value_str)
      : key_(existing_key), value_(mr, value_str), collision_next_index_(INVALID_INDEX) {}

  // Special constructor for existing regional_str key with string value construction
  template <typename RT>
    requires std::same_as<K, regional_str> && std::same_as<V, regional_str>
  dict_entry(memory_region<RT> &mr, const regional_str &existing_key, const std::string &value_str)
      : key_(existing_key), value_(mr, value_str), collision_next_index_(INVALID_INDEX) {}

  // Special constructor for existing regional_str key with int value (for mixed dict)
  template <typename RT>
    requires std::same_as<K, regional_str> && std::same_as<V, int>
  dict_entry(memory_region<RT> &mr, const regional_str &existing_key, int value)
      : key_(existing_key), value_(value), collision_next_index_(INVALID_INDEX) {}

  // Special constructor for constructing both regional_str key and value from string_view
  template <typename RT>
    requires std::same_as<K, regional_str> && std::same_as<V, regional_str>
  dict_entry(memory_region<RT> &mr, std::string_view key_str, std::string_view value_str)
      : key_(mr, key_str), value_(mr, value_str), collision_next_index_(INVALID_INDEX) {}

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

/**
 * @brief Regional dictionary container with C++ standard container semantics
 *
 * This dictionary implementation follows C++ standard container conventions:
 * - insert(): Inserts only if key doesn't exist, returns iterator and bool indicating insertion
 * - emplace(): Constructs in-place only if key doesn't exist
 * - insert_or_assign(): Inserts new entry or updates existing value
 * - try_emplace(): Constructs only if key doesn't exist (no update if key exists)
 * - operator[]: Inserts with default value if key doesn't exist, always returns reference
 * - at(): Throws if key doesn't exist, never inserts
 *
 * All insertion methods maintain consistent semantics:
 * - Return std::pair<iterator, bool> where bool indicates whether insertion occurred
 * - Support heterogeneous key types for lookup and insertion via common key conversion
 * - Preserve insertion order during iteration
 * - Handle memory region allocation requirements automatically
 */
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

  // Hash using common key type
  template <typename KeyType> size_t hash_key(const KeyType &key) const {
    auto common_key = to_common_key(key);
    if constexpr (std::same_as<K, regional_str> && std::same_as<common_key_type_t<K>, std::string_view>) {
      // Use std::hash<std::string_view> for regional_str keys
      return std::hash<std::string_view>{}(common_key);
    } else {
      return hasher_(common_key);
    }
  }

  template <typename KeyType> size_t bucket_index(const KeyType &key) const { return hash_key(key) % buckets_.size(); }

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
      size_t bucket_idx = hash_key(entry.key()) % buckets_.size();
      entry.set_collision_next_index(buckets_[bucket_idx]);
      buckets_[bucket_idx] = entry_idx;
    }
  }

  // Find entry using common key comparison
  template <typename KeyType> size_t find_entry_index(const KeyType &lookup_key) const {
    if (buckets_.empty())
      return INVALID_INDEX;

    size_t bucket_idx = bucket_index(lookup_key);
    size_t entry_idx = buckets_[bucket_idx];

    auto common_lookup_key = to_common_key(lookup_key);

    while (entry_idx != INVALID_INDEX) {
      auto common_stored_key = to_common_key(entries_[entry_idx].key());
      if (common_stored_key == common_lookup_key) {
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

  // === STANDARD C++ CONTAINER INSERTION METHODS ===

  /**
   * @brief Inserts new key-value pair only if key doesn't exist
   * @param mr Memory region for allocation
   * @param key Key to insert (can be any type convertible to common key type)
   * @param value_args Arguments for constructing the value
   * @return std::pair<V*, bool> where bool indicates if insertion occurred, V* points to the value
   *
   * Standard semantics: Does NOT update existing values. If key exists, no insertion occurs.
   */
  template <typename RT, typename KeyType, typename... ValueArgs>
    requires std::constructible_from<V, memory_region<RT> &, ValueArgs...>
  std::pair<V *, bool> insert(memory_region<RT> &mr, const KeyType &key, ValueArgs &&...value_args) {
    maybe_resize(mr);

    // Check if key already exists
    size_t existing_idx = find_entry_index(key);
    if (existing_idx != INVALID_INDEX) {
      // Key exists - no insertion, return existing value pointer and false
      return {&entries_[existing_idx].value(), false};
    }

    // Insert new entry
    size_t new_entry_idx = entries_.size();
    entries_.emplace_back(mr, key, std::forward<ValueArgs>(value_args)...);

    // Add to hash table
    size_t bucket_idx = bucket_index(key);
    entries_[new_entry_idx].set_collision_next_index(buckets_[bucket_idx]);
    buckets_[bucket_idx] = new_entry_idx;

    return {&entries_[new_entry_idx].value(), true};
  }

  template <typename RT, typename KeyType, typename... ValueArgs>
    requires std::constructible_from<V, ValueArgs...>
  std::pair<V *, bool> insert(memory_region<RT> &mr, const KeyType &key, ValueArgs &&...value_args) {
    maybe_resize(mr);

    // Check if key already exists
    size_t existing_idx = find_entry_index(key);
    if (existing_idx != INVALID_INDEX) {
      // Key exists - no insertion, return existing value pointer and false
      return {&entries_[existing_idx].value(), false};
    }

    // Insert new entry
    size_t new_entry_idx = entries_.size();
    entries_.emplace_back(mr, key, std::forward<ValueArgs>(value_args)...);

    // Add to hash table
    size_t bucket_idx = bucket_index(key);
    entries_[new_entry_idx].set_collision_next_index(buckets_[bucket_idx]);
    buckets_[bucket_idx] = new_entry_idx;

    return {&entries_[new_entry_idx].value(), true};
  }

  /**
   * @brief Constructs value in-place only if key doesn't exist
   * @param mr Memory region for allocation
   * @param key Key to emplace
   * @param value_args Arguments for constructing the value
   * @return std::pair<V*, bool> where bool indicates if construction occurred, V* points to the value
   *
   * Standard semantics: Like insert(), does NOT update existing values.
   */
  template <typename RT, typename KeyType, typename... ValueArgs>
  std::pair<V *, bool> emplace(memory_region<RT> &mr, const KeyType &key, ValueArgs &&...value_args) {
    return insert<RT, KeyType, ValueArgs...>(mr, key, std::forward<ValueArgs>(value_args)...);
  }

  /**
   * @brief Inserts new entry or assigns to existing entry
   * @param mr Memory region for allocation
   * @param key Key to insert or assign
   * @param value_args Arguments for constructing/assigning the value
   * @return std::pair<V*, bool> where bool indicates if insertion (not assignment) occurred, V* points to the value
   *
   * Standard semantics: Always succeeds. Updates existing values.
   */
  template <typename RT, typename KeyType, typename... ValueArgs>
    requires std::constructible_from<V, memory_region<RT> &, ValueArgs...>
  std::pair<V *, bool> insert_or_assign(memory_region<RT> &mr, const KeyType &key, ValueArgs &&...value_args) {
    maybe_resize(mr);

    // Check if key already exists
    size_t existing_idx = find_entry_index(key);
    if (existing_idx != INVALID_INDEX) {
      // Update existing value
      dict_entry<K, V> &existing = entries_[existing_idx];
      existing.value().~V();
      new (&existing.value()) V(mr, std::forward<ValueArgs>(value_args)...);

      return {&existing.value(), false};
    }

    // Insert new entry
    size_t new_entry_idx = entries_.size();
    entries_.emplace_back(mr, key, std::forward<ValueArgs>(value_args)...);

    // Add to hash table
    size_t bucket_idx = bucket_index(key);
    entries_[new_entry_idx].set_collision_next_index(buckets_[bucket_idx]);
    buckets_[bucket_idx] = new_entry_idx;

    return {&entries_[new_entry_idx].value(), true};
  }

  template <typename RT, typename KeyType, typename... ValueArgs>
    requires std::constructible_from<V, ValueArgs...>
  std::pair<V *, bool> insert_or_assign(memory_region<RT> &mr, const KeyType &key, ValueArgs &&...value_args) {
    maybe_resize(mr);

    // Check if key already exists
    size_t existing_idx = find_entry_index(key);
    if (existing_idx != INVALID_INDEX) {
      // Update existing value
      dict_entry<K, V> &existing = entries_[existing_idx];
      existing.value().~V();
      new (&existing.value()) V(std::forward<ValueArgs>(value_args)...);

      return {&existing.value(), false};
    }

    // Insert new entry
    size_t new_entry_idx = entries_.size();
    entries_.emplace_back(mr, key, std::forward<ValueArgs>(value_args)...);

    // Add to hash table
    size_t bucket_idx = bucket_index(key);
    entries_[new_entry_idx].set_collision_next_index(buckets_[bucket_idx]);
    buckets_[bucket_idx] = new_entry_idx;

    return {&entries_[new_entry_idx].value(), true};
  }

  /**
   * @brief Constructs value in-place only if key doesn't exist, never updates
   * @param mr Memory region for allocation
   * @param key Key to try emplacing
   * @param value_args Arguments for constructing the value
   * @return std::pair<V*, bool> where bool indicates if construction occurred, V* points to the value
   *
   * Standard semantics: Like emplace(), but more explicit about not updating existing values.
   */
  template <typename RT, typename KeyType, typename... ValueArgs>
    requires std::constructible_from<V, memory_region<RT> &, ValueArgs...>
  std::pair<V *, bool> try_emplace(memory_region<RT> &mr, const KeyType &key, ValueArgs &&...value_args) {
    return insert<RT, KeyType, ValueArgs...>(mr, key, std::forward<ValueArgs>(value_args)...);
  }

  template <typename RT, typename KeyType, typename... ValueArgs>
    requires std::constructible_from<V, ValueArgs...>
  std::pair<V *, bool> try_emplace(memory_region<RT> &mr, const KeyType &key, ValueArgs &&...value_args) {
    return insert<RT, KeyType, ValueArgs...>(mr, key, std::forward<ValueArgs>(value_args)...);
  }

  // === LOOKUP AND ACCESS METHODS ===

  /**
   * @brief Access element with bounds checking
   * @param key Key to look up (can be any type convertible to common key type)
   * @return Reference to the value
   * @throws std::out_of_range if key doesn't exist
   */
  template <typename KeyType> V &at(const KeyType &key) {
    size_t entry_idx = find_entry_index(key);
    if (entry_idx == INVALID_INDEX) {
      throw std::out_of_range("Key not found in regional_dict");
    }
    return entries_[entry_idx].value();
  }

  template <typename KeyType> const V &at(const KeyType &key) const {
    size_t entry_idx = find_entry_index(key);
    if (entry_idx == INVALID_INDEX) {
      throw std::out_of_range("Key not found in regional_dict");
    }
    return entries_[entry_idx].value();
  }

  /**
   * @brief Access element, inserting with default value if key doesn't exist
   * @param mr Memory region for potential allocation
   * @param key Key to access
   * @return Reference to the value (existing or newly created)
   */
  template <typename KeyType, typename RT>
    requires std::constructible_from<V, memory_region<RT> &>
  V &at_or_create(memory_region<RT> &mr, const KeyType &key) {
    auto [value_ptr, inserted] = try_emplace<RT, KeyType>(mr, key);
    return *value_ptr;
  }

  template <typename KeyType, typename RT>
    requires std::constructible_from<V> && (!std::constructible_from<V, memory_region<RT> &>)
  V &at_or_create(memory_region<RT> &mr, const KeyType &key) {
    auto [value_ptr, inserted] = try_emplace<RT, KeyType>(mr, key);
    return *value_ptr;
  }

  /**
   * @brief Find element by key
   * @param key Key to search for (can be any type convertible to common key type)
   * @return Pointer to value if found, nullptr otherwise
   */
  template <typename KeyType> V *find_value(const KeyType &key) {
    size_t entry_idx = find_entry_index(key);
    return entry_idx != INVALID_INDEX ? &entries_[entry_idx].value() : nullptr;
  }

  template <typename KeyType> const V *find_value(const KeyType &key) const {
    size_t entry_idx = find_entry_index(key);
    return entry_idx != INVALID_INDEX ? &entries_[entry_idx].value() : nullptr;
  }

  template <typename KeyType> bool contains(const KeyType &key) const { return find_entry_index(key) != INVALID_INDEX; }

  // Legacy method names for backward compatibility (deprecated)
  template <typename KeyType> V *get(const KeyType &key) { return find_value(key); }

  template <typename KeyType> const V *get(const KeyType &key) const { return find_value(key); }

  // === CAPACITY AND ITERATION ===

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

  // Find method that returns iterator (standard container interface)
  template <typename KeyType> iterator find(const KeyType &key) {
    size_t entry_idx = find_entry_index(key);
    if (entry_idx == INVALID_INDEX) {
      return end();
    }
    auto it = entries_.begin();
    std::advance(it, entry_idx);
    return iterator(it);
  }

  template <typename KeyType> const_iterator find(const KeyType &key) const {
    size_t entry_idx = find_entry_index(key);
    if (entry_idx == INVALID_INDEX) {
      return end();
    }
    auto it = entries_.begin();
    std::advance(it, entry_idx);
    return const_iterator(it);
  }
};

// Comparison operators
template <typename K, typename V, typename Hash>
bool operator==(const regional_dict<K, V, Hash> &lhs, const regional_dict<K, V, Hash> &rhs)
  requires std::equality_comparable<K> && std::equality_comparable<V>
{
  if (lhs.size() != rhs.size())
    return false;

  for (const auto &[key, value] : lhs) {
    const V *rhs_value = rhs.find_value(key);
    if (!rhs_value || !(*rhs_value == value)) {
      return false;
    }
  }

  return true;
}

} // namespace shilos
