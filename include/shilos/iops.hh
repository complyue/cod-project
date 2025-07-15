#pragma once

#include <concepts>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace shilos {
namespace yaml {

// Insertion Order Preserving Set - for safe storage of owned values
// Provides stable references and deduplication
// T: memory owning element type
template <typename T> class iops {
private:
  std::vector<T> storage_;              // Stable storage for values
  std::unordered_map<T, size_t> index_; // Fast lookup by content

public:
  iops() {
    // Reserve some capacity to minimize reallocations
    storage_.reserve(64);
  }

  // Insert a value and return a stable reference to it
  // If the value already exists, returns reference to existing copy
  // U: any type that is hashable and convertible to T
  template <typename U>
    requires std::convertible_to<U, T> && requires(const std::decay_t<U> &u) { std::hash<std::decay_t<U>>{}(u); }
  const T &insert(U &&value) {
    T key{std::forward<U>(value)};
    auto it = index_.find(key);
    if (it != index_.end()) {
      // Value already exists, return reference to existing copy
      return storage_[it->second];
    }

    // New value - add to storage
    size_t index = storage_.size();
    storage_.push_back(std::move(key));

    // Update lookup index with copy of the stored value
    // We copy to avoid potential issues with vector reallocation
    index_.emplace(T{storage_.back()}, index);

    return storage_.back();
  }

  // Move constructor
  iops(iops &&other) noexcept : storage_(std::move(other.storage_)), index_(std::move(other.index_)) {}

  // Move assignment
  iops &operator=(iops &&other) noexcept {
    if (this != &other) {
      storage_ = std::move(other.storage_);
      index_ = std::move(other.index_);
    }
    return *this;
  }

  // Disable copy operations to avoid reference invalidation issues
  iops(const iops &) = delete;
  iops &operator=(const iops &) = delete;
};

// Specialization for std::string to maintain the original string_view optimization
template <> class iops<std::string> {
private:
  std::vector<std::string> storage_;                   // Stable storage for strings
  std::unordered_map<std::string_view, size_t> index_; // Fast lookup by content

public:
  iops() {
    // Reserve some capacity to minimize reallocations
    storage_.reserve(64);
  }

  // Insert a string and return a stable string_view to it
  // If the string already exists, returns view to existing copy
  template <typename U>
    requires std::convertible_to<U, std::string> &&
             requires(const std::decay_t<U> &u) { std::hash<std::decay_t<U>>{}(u); }
  std::string_view insert(U &&str) {
    std::string converted{std::forward<U>(str)};
    std::string_view view(converted);
    auto it = index_.find(view);
    if (it != index_.end()) {
      // String already exists, return view to existing copy
      return std::string_view(storage_[it->second]);
    }

    // New string - add to storage
    size_t index = storage_.size();
    storage_.push_back(std::move(converted));

    // Update lookup index with view to the stored string
    std::string_view stored_view(storage_.back());
    index_.emplace(stored_view, index);

    return stored_view;
  }

  // Move constructor
  iops(iops &&other) noexcept : storage_(std::move(other.storage_)), index_(std::move(other.index_)) {}

  // Move assignment
  iops &operator=(iops &&other) noexcept {
    if (this != &other) {
      storage_ = std::move(other.storage_);
      index_ = std::move(other.index_);
    }
    return *this;
  }

  // Disable copy operations to avoid string_view invalidation issues
  iops(const iops &) = delete;
  iops &operator=(const iops &) = delete;
};

} // namespace yaml
} // namespace shilos
