#pragma once

#include "./region.hh"
#include "./str.hh"
#include "./vector.hh"
#include <type_traits>

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
  typename std::aligned_storage<sizeof(V), alignof(V)>::type value_storage_;
  size_t collision_next_index_; // Index of next entry in collision chain (or INVALID_INDEX)
  bool is_deleted_;             // Tombstone flag for deleted entries

public:
  static constexpr size_t INVALID_INDEX = SIZE_MAX;

  // Default constructor (needed for vector_segment arrays)
  dict_entry() : key_(), collision_next_index_(INVALID_INDEX), is_deleted_(false) {}

  // Standard constructors for constructing both key and value from arguments
  template <typename RT, typename... KeyArgs, typename... ValueArgs>
    requires std::constructible_from<K, memory_region<RT> &, KeyArgs...> &&
                 std::constructible_from<V, memory_region<RT> &, ValueArgs...>
  dict_entry(memory_region<RT> &mr, KeyArgs &&...key_args, ValueArgs &&...value_args)
      : key_(mr, std::forward<KeyArgs>(key_args)...), collision_next_index_(INVALID_INDEX), is_deleted_(false) {
    new (&value_storage_) V(mr, std::forward<ValueArgs>(value_args)...);
  }

  template <typename RT, typename... KeyArgs, typename... ValueArgs>
    requires std::constructible_from<K, KeyArgs...> && std::constructible_from<V, ValueArgs...>
  dict_entry(memory_region<RT> &mr, KeyArgs &&...key_args, ValueArgs &&...value_args)
      : key_(std::forward<KeyArgs>(key_args)...), collision_next_index_(INVALID_INDEX), is_deleted_(false) {
    new (&value_storage_) V(mr, std::forward<ValueArgs>(value_args)...);
  }

  template <typename RT, typename... KeyArgs, typename... ValueArgs>
    requires std::constructible_from<K, KeyArgs...> && std::constructible_from<V, memory_region<RT> &, ValueArgs...>
  dict_entry(memory_region<RT> &mr, KeyArgs &&...key_args, ValueArgs &&...value_args)
      : key_(std::forward<KeyArgs>(key_args)...), collision_next_index_(INVALID_INDEX), is_deleted_(false) {
    new (&value_storage_) V(mr, std::forward<ValueArgs>(value_args)...);
  }

  template <typename RT, typename... KeyArgs, typename... ValueArgs>
    requires std::constructible_from<K, memory_region<RT> &, KeyArgs...> && std::constructible_from<V, ValueArgs...>
  dict_entry(memory_region<RT> &mr, KeyArgs &&...key_args, ValueArgs &&...value_args)
      : key_(mr, std::forward<KeyArgs>(key_args)...), collision_next_index_(INVALID_INDEX), is_deleted_(false) {
    new (&value_storage_) V(std::forward<ValueArgs>(value_args)...);
  }

  // Constructor for single key argument and value arguments
  template <typename RT, typename KeyArg, typename... ValueArgs>
    requires std::constructible_from<K, memory_region<RT> &, const KeyArg &> &&
                 std::constructible_from<V, const ValueArgs &...> &&
                 (!std::same_as<std::remove_cvref_t<KeyArg>, memory_region<RT>>)
  dict_entry(memory_region<RT> &mr, const KeyArg &key_arg, const ValueArgs &...value_args)
      : key_(mr, key_arg), collision_next_index_(INVALID_INDEX), is_deleted_(false) {
    new (&value_storage_) V(value_args...);
  }

  template <typename RT, typename KeyArg, typename... ValueArgs>
    requires std::constructible_from<K, const KeyArg &> && std::constructible_from<V, const ValueArgs &...> &&
                 (!std::same_as<std::remove_cvref_t<KeyArg>, memory_region<RT>>)
  dict_entry(memory_region<RT> &mr, const KeyArg &key_arg, const ValueArgs &...value_args)
      : key_(key_arg), collision_next_index_(INVALID_INDEX), is_deleted_(false) {
    new (&value_storage_) V(value_args...);
  }

  // Constructor for when both key and value need memory region
  template <typename RT, typename KeyArg, typename... ValueArgs>
    requires std::constructible_from<K, memory_region<RT> &, const KeyArg &> &&
                 std::constructible_from<V, memory_region<RT> &, const ValueArgs &...> &&
                 (!std::same_as<std::remove_cvref_t<KeyArg>, memory_region<RT>>)
  dict_entry(memory_region<RT> &mr, const KeyArg &key_arg, const ValueArgs &...value_args)
      : key_(mr, key_arg), collision_next_index_(INVALID_INDEX), is_deleted_(false) {
    new (&value_storage_) V(mr, value_args...);
  }

  // Constructor for key-only with default-constructed value (V needs memory_region)
  template <typename RT, typename... KeyArgs>
    requires std::constructible_from<K, memory_region<RT> &, const KeyArgs &...> &&
                 std::constructible_from<V, memory_region<RT> &>
  dict_entry(memory_region<RT> &mr, const KeyArgs &...key_args)
      : key_(mr, key_args...), collision_next_index_(INVALID_INDEX), is_deleted_(false) {
    new (&value_storage_) V(mr);
  }

  // Constructor for key-only with default-constructed value (V doesn't need memory_region)
  template <typename RT, typename... KeyArgs>
    requires std::constructible_from<K, memory_region<RT> &, const KeyArgs &...> && std::constructible_from<V> &&
                 (!std::constructible_from<V, memory_region<RT> &>)
  dict_entry(memory_region<RT> &mr, const KeyArgs &...key_args)
      : key_(mr, key_args...), collision_next_index_(INVALID_INDEX), is_deleted_(false) {}

  // Deleted special members
  dict_entry(const dict_entry &) = delete;
  dict_entry(dict_entry &&) = delete;
  dict_entry &operator=(const dict_entry &) = delete;
  dict_entry &operator=(dict_entry &&) = delete;

  K &key() { return key_; }
  const K &key() const { return key_; }

  V &value() { return *reinterpret_cast<V *>(&value_storage_); }
  const V &value() const { return *reinterpret_cast<const V *>(&value_storage_); }

  V *raw_value_ptr() { return reinterpret_cast<V *>(&value_storage_); }
  // removed mark

  // Internal ctor for emplace_init: constructs key, leaves value uninitialised
  template <typename RT, typename KeyArg>
  dict_entry(std::in_place_t, memory_region<RT> &mr, const KeyArg &key_arg)
      : key_(mr, key_arg), collision_next_index_(INVALID_INDEX), is_deleted_(false) {}

  size_t collision_next_index() const { return collision_next_index_; }
  void set_collision_next_index(size_t index) { collision_next_index_ = index; }

  bool is_deleted() const { return is_deleted_; }
  void mark_deleted() { is_deleted_ = true; }
  void mark_active() { is_deleted_ = false; }
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
    if (buckets_.empty() || static_cast<double>(size()) / buckets_.size() > MAX_LOAD_FACTOR) {
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

  // Find entry using common key comparison (skips deleted entries)
  template <typename KeyType> size_t find_entry_index(const KeyType &lookup_key) const {
    if (buckets_.empty())
      return INVALID_INDEX;

    size_t bucket_idx = bucket_index(lookup_key);
    size_t entry_idx = buckets_[bucket_idx];

    auto common_lookup_key = to_common_key(lookup_key);

    while (entry_idx != INVALID_INDEX) {
      const dict_entry<K, V> &entry = entries_[entry_idx];
      if (!entry.is_deleted()) {
        auto common_stored_key = to_common_key(entry.key());
        if (common_stored_key == common_lookup_key) {
          return entry_idx;
        }
      }
      entry_idx = entry.collision_next_index();
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
    requires std::constructible_from<V, memory_region<RT> &, const ValueArgs &...>
  std::pair<V *, bool> insert(memory_region<RT> &mr, const KeyType &key, const ValueArgs &...value_args) {
    maybe_resize(mr);

    // Check if key already exists
    size_t existing_idx = find_entry_index(key);
    if (existing_idx != INVALID_INDEX) {
      // Key exists - no insertion, return existing value pointer and false
      return {&entries_[existing_idx].value(), false};
    }

    // Insert new entry
    size_t new_entry_idx = entries_.size();
    entries_.emplace_back(mr, key, value_args...);

    // Add to hash table
    size_t bucket_idx = bucket_index(key);
    entries_[new_entry_idx].set_collision_next_index(buckets_[bucket_idx]);
    buckets_[bucket_idx] = new_entry_idx;

    return {&entries_[new_entry_idx].value(), true};
  }

  template <typename RT, typename KeyType, typename... ValueArgs>
    requires std::constructible_from<V, const ValueArgs &...>
  std::pair<V *, bool> insert(memory_region<RT> &mr, const KeyType &key, const ValueArgs &...value_args) {
    maybe_resize(mr);

    // Check if key already exists
    size_t existing_idx = find_entry_index(key);
    if (existing_idx != INVALID_INDEX) {
      // Key exists - no insertion, return existing value pointer and false
      return {&entries_[existing_idx].value(), false};
    }

    // Insert new entry
    size_t new_entry_idx = entries_.size();
    entries_.emplace_back(mr, key, value_args...);

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
  std::pair<V *, bool> emplace(memory_region<RT> &mr, const KeyType &key, const ValueArgs &...value_args) {
    return insert<RT, KeyType, ValueArgs...>(mr, key, value_args...);
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
    requires std::constructible_from<V, memory_region<RT> &, const ValueArgs &...>
  std::pair<V *, bool> insert_or_assign(memory_region<RT> &mr, const KeyType &key, const ValueArgs &...value_args) {
    maybe_resize(mr);

    // Check if key already exists
    size_t existing_idx = find_entry_index(key);
    if (existing_idx != INVALID_INDEX) {
      // Update existing value
      dict_entry<K, V> &existing = entries_[existing_idx];
      existing.value().~V();
      new (&existing.value()) V(mr, value_args...);

      return {&existing.value(), false};
    }

    // Insert new entry
    size_t new_entry_idx = entries_.size();
    entries_.emplace_back(mr, key, value_args...);

    // Add to hash table
    size_t bucket_idx = bucket_index(key);
    entries_[new_entry_idx].set_collision_next_index(buckets_[bucket_idx]);
    buckets_[bucket_idx] = new_entry_idx;

    return {&entries_[new_entry_idx].value(), true};
  }

  template <typename RT, typename KeyType, typename... ValueArgs>
    requires std::constructible_from<V, const ValueArgs &...>
  std::pair<V *, bool> insert_or_assign(memory_region<RT> &mr, const KeyType &key, const ValueArgs &...value_args) {
    maybe_resize(mr);

    // Check if key already exists
    size_t existing_idx = find_entry_index(key);
    if (existing_idx != INVALID_INDEX) {
      // Update existing value
      dict_entry<K, V> &existing = entries_[existing_idx];
      existing.value().~V();
      new (&existing.value()) V(value_args...);

      return {&existing.value(), false};
    }

    // Insert new entry
    size_t new_entry_idx = entries_.size();
    entries_.emplace_back(mr, key, value_args...);

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
    requires std::constructible_from<V, memory_region<RT> &, const ValueArgs &...>
  std::pair<V *, bool> try_emplace(memory_region<RT> &mr, const KeyType &key, const ValueArgs &...value_args) {
    return insert<RT, KeyType, ValueArgs...>(mr, key, value_args...);
  }

  template <typename RT, typename KeyType, typename... ValueArgs>
    requires std::constructible_from<V, const ValueArgs &...>
  std::pair<V *, bool> try_emplace(memory_region<RT> &mr, const KeyType &key, const ValueArgs &...value_args) {
    return insert<RT, KeyType, ValueArgs...>(mr, key, value_args...);
  }

  // === In-place initialisation insertion (no default construction needed) ===
  template <typename RT, typename KeyType, typename InitFn>
    requires std::invocable<InitFn, V *>
  V &emplace_init(memory_region<RT> &mr, const KeyType &key, InitFn &&init_fn) {
    maybe_resize(mr);

    // Check existing
    size_t existing_idx = find_entry_index(key);
    if (existing_idx != INVALID_INDEX) {
      // Reinitialize existing value
      dict_entry<K, V> &entry = entries_[existing_idx];
      std::forward<InitFn>(init_fn)(entry.raw_value_ptr());
      return entry.value();
    }

    size_t new_idx_placeholder = entries_.size();
    entries_.emplace_init(
        mr, [&](dict_entry<K, V> *dst_entry) { new (dst_entry) dict_entry<K, V>(std::in_place, mr, key); });

    // Hash table link - new entry is at previous size position
    size_t bucket_idx = bucket_index(key);
    entries_[new_idx_placeholder].set_collision_next_index(buckets_[bucket_idx]);
    buckets_[bucket_idx] = new_idx_placeholder;

    // Construct value
    std::forward<InitFn>(init_fn)(entries_[new_idx_placeholder].raw_value_ptr());

    return entries_[new_idx_placeholder].value();
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

  // === CAPACITY AND ITERATION ===

  bool empty() const {
    // Check if all entries are deleted
    for (const auto &entry : entries_) {
      if (!entry.is_deleted()) {
        return false;
      }
    }
    return true;
  }

  size_t size() const {
    // Count only non-deleted entries
    size_t count = 0;
    for (const auto &entry : entries_) {
      if (!entry.is_deleted()) {
        count++;
      }
    }
    return count;
  }

  size_t bucket_count() const { return buckets_.size(); }
  double load_factor() const { return buckets_.empty() ? 0.0 : static_cast<double>(size()) / buckets_.size(); }

  // Insertion order iteration - just iterate over entries vector!
  class iterator {
    // Friend declaration to allow regional_dict to access private members
    friend class regional_dict;

    typename regional_vector<dict_entry<K, V>>::iterator it_;
    typename regional_vector<dict_entry<K, V>>::iterator end_it_;

  public:
    iterator(typename regional_vector<dict_entry<K, V>>::iterator it,
             typename regional_vector<dict_entry<K, V>>::iterator end_it)
        : it_(it), end_it_(end_it) {
      // Skip initial deleted entries
      while (it_ != end_it_ && it_->is_deleted()) {
        ++it_;
      }
    }

    std::pair<const K &, V &> operator*() { return {it_->key(), it_->value()}; }

    iterator &operator++() {
      do {
        ++it_;
      } while (it_ != end_it_ && it_->is_deleted());
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
    // Friend declaration to allow regional_dict to access private members
    friend class regional_dict;

    typename regional_vector<dict_entry<K, V>>::const_iterator it_;
    typename regional_vector<dict_entry<K, V>>::const_iterator end_it_;

  public:
    const_iterator(typename regional_vector<dict_entry<K, V>>::const_iterator it,
                   typename regional_vector<dict_entry<K, V>>::const_iterator end_it)
        : it_(it), end_it_(end_it) {
      // Skip initial deleted entries
      while (it_ != end_it_ && it_->is_deleted()) {
        ++it_;
      }
    }

    std::pair<const K &, const V &> operator*() const { return {it_->key(), it_->value()}; }

    const_iterator &operator++() {
      do {
        ++it_;
      } while (it_ != end_it_ && it_->is_deleted());
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

  iterator begin() { return iterator(entries_.begin(), entries_.end()); }
  iterator end() { return iterator(entries_.end(), entries_.end()); }

  const_iterator begin() const { return const_iterator(entries_.begin(), entries_.end()); }
  const_iterator end() const { return const_iterator(entries_.end(), entries_.end()); }

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
    return iterator(it, entries_.end());
  }

  template <typename KeyType> const_iterator find(const KeyType &key) const {
    size_t entry_idx = find_entry_index(key);
    if (entry_idx == INVALID_INDEX) {
      return end();
    }
    auto it = entries_.begin();
    std::advance(it, entry_idx);
    return const_iterator(it, entries_.end());
  }

  // === DELETION METHODS ===

  /**
   * @brief Remove element by key
   * @param key Key to remove (can be any type convertible to common key type)
   * @return Number of elements removed (0 or 1)
   */
  template <typename KeyType> size_t erase(const KeyType &key) {
    size_t entry_idx = find_entry_index(key);
    if (entry_idx == INVALID_INDEX) {
      return 0; // Key not found
    }

    erase_entry_at_index(entry_idx);
    return 1;
  }

  /**
   * @brief Remove element by iterator
   * @param it Iterator pointing to element to erase
   * @return Iterator to the element that followed the erased element
   */
  iterator erase(iterator it) {
    if (it == end()) {
      throw std::out_of_range("Cannot erase end iterator");
    }

    // Calculate the entry index from iterator position
    auto vector_it = it.it_;
    size_t entry_idx = std::distance(entries_.begin(), vector_it);

    erase_entry_at_index(entry_idx);

    // Return iterator starting from current position, which will skip deleted entries
    return iterator(vector_it, entries_.end());
  }

  /**
   * @brief Remove range of elements [first, last)
   * @param first Iterator to first element to erase
   * @param last Iterator to one past last element to erase
   * @return Iterator to the element that followed the last erased element
   */
  iterator erase(iterator first, iterator last) {
    if (first == last) {
      return last; // Empty range
    }

    // Mark all elements in the range as deleted
    auto current = first;
    while (current != last) {
      auto vector_it = current.it_;
      size_t entry_idx = std::distance(entries_.begin(), vector_it);
      erase_entry_at_index(entry_idx);
      ++current;
    }

    // Return iterator starting from the last position, which will skip deleted entries
    return iterator(last.it_, entries_.end());
  }

  /**
   * @brief Remove all elements using tombstone approach
   */
  void clear() {
    // Mark all entries as deleted
    for (auto &entry : entries_) {
      entry.mark_deleted();
    }

    // Clear all bucket entries
    for (size_t i = 0; i < buckets_.size(); ++i) {
      buckets_[i] = INVALID_INDEX;
    }
  }

private:
  /**
   * @brief Internal method to erase entry at specific index using tombstone approach
   * @param entry_idx Index of entry to mark as deleted
   *
   * This method handles:
   * 1. Removing entry from hash table collision chain
   * 2. Marking entry as deleted (tombstone approach)
   * No need to move elements, which avoids issues with regional types
   */
  void erase_entry_at_index(size_t entry_idx) {
    if (entry_idx >= entries_.size()) {
      throw std::out_of_range("Invalid entry index");
    }

    dict_entry<K, V> &entry_to_remove = entries_[entry_idx];

    if (entry_to_remove.is_deleted()) {
      return; // Already deleted
    }

    // Step 1: Remove from hash table collision chain
    if (!buckets_.empty()) {
      size_t bucket_idx = hash_key(entry_to_remove.key()) % buckets_.size();

      if (buckets_[bucket_idx] == entry_idx) {
        // Entry is first in collision chain - find next non-deleted entry
        size_t next_idx = entry_to_remove.collision_next_index();
        while (next_idx != INVALID_INDEX && entries_[next_idx].is_deleted()) {
          next_idx = entries_[next_idx].collision_next_index();
        }
        buckets_[bucket_idx] = next_idx;
      } else {
        // Find and unlink from collision chain
        size_t current_idx = buckets_[bucket_idx];
        while (current_idx != INVALID_INDEX) {
          dict_entry<K, V> &current_entry = entries_[current_idx];
          if (!current_entry.is_deleted() && current_entry.collision_next_index() == entry_idx) {
            // Skip over the deleted entry in the chain
            size_t next_idx = entry_to_remove.collision_next_index();
            while (next_idx != INVALID_INDEX && entries_[next_idx].is_deleted()) {
              next_idx = entries_[next_idx].collision_next_index();
            }
            current_entry.set_collision_next_index(next_idx);
            break;
          }
          current_idx = current_entry.collision_next_index();
        }
      }
    }

    // Step 2: Mark entry as deleted (tombstone approach)
    entry_to_remove.mark_deleted();
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
