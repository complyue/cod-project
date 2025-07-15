// This file defines YAML utilities for shilos, extracted from prelude.hh.
// It introduces `iopd`, an insertion-order-preserving dictionary used to
// represent YAML mappings while maintaining original key order.
#pragma once

#include <algorithm>
#include <concepts>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace shilos {
namespace yaml {

// ---------------------------------------------------------------------------
// iopd – insertion-order preserving dictionary (template on key type K and value type V)
// ---------------------------------------------------------------------------
template <typename K, typename V> class iopd {
public:
  // Public aliases -----------------------------------------------------------
  using key_type = K;
  using mapped_type = V; // Template-supplied value type
  using size_type = std::size_t;

  struct entry {
    key_type key;
    mapped_type value;

    entry(key_type k, mapped_type v) : key(std::move(k)), value(std::move(v)) {}
  };

  using storage_type = std::vector<entry>;
  using iterator = storage_type::iterator;
  using const_iterator = storage_type::const_iterator;

private:
  // Index table that maps keys to their index inside `storage_`.
  // For string_view keys, we need to be careful about lifetime management
  // For owning key types like std::string, we copy the key to avoid dangling references
  // For non-owning key types like std::string_view, the caller must ensure lifetime
  std::unordered_map<key_type, size_type> index_;
  storage_type storage_;

  // Helper to obtain iterator from index (unchecked)
  iterator nth(size_type idx) { return storage_.begin() + static_cast<std::ptrdiff_t>(idx); }
  const_iterator nth(size_type idx) const { return storage_.cbegin() + static_cast<std::ptrdiff_t>(idx); }

public:
  // Constructors ------------------------------------------------------------
  iopd() = default;

  // Capacity ----------------------------------------------------------------
  [[nodiscard]] bool empty() const noexcept { return storage_.empty(); }
  [[nodiscard]] size_type size() const noexcept { return storage_.size(); }

  // Iteration ---------------------------------------------------------------
  iterator begin() noexcept { return storage_.begin(); }
  iterator end() noexcept { return storage_.end(); }
  const_iterator begin() const noexcept { return storage_.cbegin(); }
  const_iterator end() const noexcept { return storage_.cend(); }
  const_iterator cbegin() const noexcept { return storage_.cbegin(); }
  const_iterator cend() const noexcept { return storage_.cend(); }

  // Lookup ------------------------------------------------------------------
  // For lookup operations, we support both exact key type and compatible types
  template <typename KeyLike>
    requires std::is_convertible_v<KeyLike, key_type> ||
             (std::is_same_v<key_type, std::string_view> && std::is_convertible_v<KeyLike, std::string_view>)
  bool contains(const KeyLike &key) const noexcept {
    if constexpr (std::is_same_v<key_type, std::string_view>) {
      return index_.find(std::string_view(key)) != index_.end();
    } else {
      return index_.find(key_type(key)) != index_.end();
    }
  }

  mapped_type &at(const key_type &key) {
    auto it = index_.find(key);
    if (it == index_.end())
      throw std::out_of_range("iopd::at – key not found");
    return storage_[it->second].value;
  }

  const mapped_type &at(const key_type &key) const {
    auto it = index_.find(key);
    if (it == index_.end())
      throw std::out_of_range("iopd::at – key not found");
    return storage_[it->second].value;
  }

  iterator find(const key_type &key) {
    auto it = index_.find(key);
    return it == index_.end() ? storage_.end() : nth(it->second);
  }

  const_iterator find(const key_type &key) const {
    auto it = index_.find(key);
    return it == index_.end() ? storage_.cend() : nth(it->second);
  }

  // Insertion ----------------------------------------------------------------
  // Inserts a key/value pair.  If the key already exists we *update* its value
  // and return {iterator, false}.  Otherwise the new pair is appended to the
  // storage vector (preserving order) and {iterator, true} is returned.
  std::pair<iterator, bool> insert_or_assign(key_type key, mapped_type value) {
    auto idx_it = index_.find(key);
    if (idx_it != index_.end()) {
      // Overwrite existing value – insertion order stays unchanged
      storage_[idx_it->second].value = std::move(value);
      return {nth(idx_it->second), false};
    }

    const size_type new_index = storage_.size();
    // For string_view keys, we store the view directly - caller must ensure lifetime
    // For owning keys like std::string, this creates a copy as expected
    index_.emplace(key, new_index);
    storage_.emplace_back(std::move(key), std::move(value));
    return {storage_.end() - 1, true};
  }

  // Convenience operator[] that inserts a *default-constructed* value when the
  // key is absent (mirrors semantics of std::map/unordered_map).

  mapped_type &operator[](const key_type &key) {
    auto idx_it = index_.find(key);
    if (idx_it != index_.end()) {
      return storage_[idx_it->second].value;
    }
    const size_type new_index = storage_.size();
    storage_.emplace_back(key, mapped_type());
    index_.emplace(storage_.back().key, new_index);
    return storage_.back().value;
  }

  mapped_type &operator[](key_type &&key) {
    // Look-up before taking ownership of the rvalue
    auto idx_it = index_.find(key);
    if (idx_it != index_.end()) {
      return storage_[idx_it->second].value;
    }

    const size_type new_index = storage_.size();

    // Move key into storage first so we still have an intact copy for the
    // index (obtained via reference to the freshly emplaced entry). This
    // avoids the moved-from/empty key issue.
    storage_.emplace_back(std::move(key), mapped_type());
    index_.emplace(storage_.back().key, new_index);

    return storage_.back().value;
  }

  // ------------------------------------------------------------------------
  // Emplace: construct value in-place with arbitrary constructor arguments.
  // ------------------------------------------------------------------------
  template <typename... Args>
    requires std::constructible_from<mapped_type, Args...>
  std::pair<iterator, bool> emplace(key_type key, Args &&...args) {
    auto idx_it = index_.find(key);
    if (idx_it != index_.end()) {
      return {nth(idx_it->second), false};
    }

    const size_type new_index = storage_.size();
    storage_.emplace_back(key_type(key), mapped_type(std::forward<Args>(args)...));
    index_.emplace(std::move(key), new_index);
    return {storage_.end() - 1, true};
  }

  // Erase --------------------------------------------------------------------
  // Removes the entry with given key, preserving order of remaining items.
  // Returns true if an element was erased.
  bool erase(const key_type &key) {
    auto idx_it = index_.find(key);
    if (idx_it == index_.end())
      return false;

    const size_type idx = idx_it->second;
    storage_.erase(nth(idx));
    index_.erase(idx_it);

    // Update indices of subsequent elements – O(n) but unavoidable if we want
    // to keep contiguous storage while preserving order.
    for (size_type i = idx; i < storage_.size(); ++i) {
      index_[storage_[i].key] = i;
    }
    return true;
  }
};

} // namespace yaml

} // namespace shilos
