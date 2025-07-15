#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace shilos {
namespace yaml {

// Insertion Order Preserving String Set - for safe storage of owned strings
// Provides stable string_view references and deduplication
class iops {
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
  std::string_view insert(std::string str) {
    std::string_view view(str);
    auto it = index_.find(view);
    if (it != index_.end()) {
      // String already exists, return view to existing copy
      return std::string_view(storage_[it->second]);
    }

    // New string - add to storage
    size_t index = storage_.size();
    storage_.push_back(std::move(str));

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
