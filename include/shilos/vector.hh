#pragma once

#include "./region.hh"

#include <cassert>
#include <compare>
#include <iterator>
#include <stdexcept>

namespace shilos {

// Forward declaration
template <typename T> class regional_vector;

template <typename T> class vector_segment {
  // Friend declaration for raw pointer YAML functions
  template <typename U, typename RT>
  friend void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_vector<U> *raw_ptr);

public:
  static constexpr size_t SEGMENT_SIZE = 64;

private:
  T elements_[SEGMENT_SIZE];
  regional_ptr<vector_segment<T>> next_;
  size_t size_; // Number of elements used in this segment

public:
  template <typename RT> vector_segment(memory_region<RT> &mr) : elements_(), next_(), size_(0) {}

  // Deleted special members
  vector_segment(const vector_segment &) = delete;
  vector_segment(vector_segment &&) = delete;
  vector_segment &operator=(const vector_segment &) = delete;
  vector_segment &operator=(vector_segment &&) = delete;

  T &operator[](size_t index) {
    assert(index < size_);
    return elements_[index];
  }

  const T &operator[](size_t index) const {
    assert(index < size_);
    return elements_[index];
  }

  bool is_full() const { return size_ >= SEGMENT_SIZE; }
  size_t size() const { return size_; }
  size_t capacity() const { return SEGMENT_SIZE; }

  regional_ptr<vector_segment<T>> &next() { return next_; }
  const regional_ptr<vector_segment<T>> &next() const { return next_; }

  // Add element to this segment (assumes not full)
  template <typename RT, typename... Args>
    requires std::constructible_from<T, memory_region<RT> &, Args...>
  void emplace_back(memory_region<RT> &mr, Args &&...args) {
    assert(!is_full());
    new (&elements_[size_]) T(mr, std::forward<Args>(args)...);
    size_++;
  }

  template <typename RT, typename... Args>
    requires std::constructible_from<T, Args...>
  void emplace_back(memory_region<RT> &mr, Args &&...args) {
    assert(!is_full());
    new (&elements_[size_]) T(std::forward<Args>(args)...);
    size_++;
  }

  // Iterator support for segment
  T *begin() { return elements_; }
  T *end() { return elements_ + size_; }
  const T *begin() const { return elements_; }
  const T *end() const { return elements_ + size_; }
};

template <typename T> class regional_vector {
  // Friend declarations for YAML functions
  template <typename U, typename RT>
  friend void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_vector<U> *raw_ptr);
  template <typename U, typename RT>
  friend global_ptr<regional_vector<U>, RT> from_yaml(memory_region<RT> &mr, const yaml::Node &node);

private:
  regional_ptr<vector_segment<T>> first_segment_;
  regional_ptr<vector_segment<T>> last_segment_;
  size_t segment_count_;
  size_t total_size_;

  // Helper to find segment and local index for global index
  std::pair<vector_segment<T> *, size_t> locate_element(size_t global_index) {
    assert(global_index < total_size_);

    size_t current_offset = 0;
    vector_segment<T> *current = first_segment_.get();

    while (current) {
      if (global_index < current_offset + current->size()) {
        return {current, global_index - current_offset};
      }
      current_offset += current->size();
      current = current->next().get();
    }

    throw std::out_of_range("Vector index out of range");
  }

  std::pair<const vector_segment<T> *, size_t> locate_element(size_t global_index) const {
    assert(global_index < total_size_);

    size_t current_offset = 0;
    const vector_segment<T> *current = first_segment_.get();

    while (current) {
      if (global_index < current_offset + current->size()) {
        return {current, global_index - current_offset};
      }
      current_offset += current->size();
      current = current->next().get();
    }

    throw std::out_of_range("Vector index out of range");
  }

public:
  regional_vector() : first_segment_(), last_segment_(), segment_count_(0), total_size_(0) {}

  template <typename RT>
  regional_vector(memory_region<RT> &mr) : first_segment_(), last_segment_(), segment_count_(0), total_size_(0) {}

  // Deleted special members
  regional_vector(const regional_vector &) = delete;
  regional_vector(regional_vector &&) = delete;
  regional_vector &operator=(const regional_vector &) = delete;
  regional_vector &operator=(regional_vector &&) = delete;

  // Add element, growing storage as needed
  template <typename RT, typename... Args>
    requires std::constructible_from<T, memory_region<RT> &, Args...>
  void emplace_back(memory_region<RT> &mr, Args &&...args) {
    // If no segments or last segment is full, create new segment
    if (!last_segment_ || last_segment_->is_full()) {
      auto new_segment = mr.template create_bits<vector_segment<T>>(mr);

      if (!first_segment_) {
        first_segment_ = last_segment_ = new_segment.get();
      } else {
        last_segment_->next() = new_segment.get();
        last_segment_ = new_segment.get();
      }
      segment_count_++;
    }

    last_segment_->emplace_back(mr, std::forward<Args>(args)...);
    total_size_++;
  }

  template <typename RT, typename... Args>
    requires std::constructible_from<T, Args...>
  void emplace_back(memory_region<RT> &mr, Args &&...args) {
    // If no segments or last segment is full, create new segment
    if (!last_segment_ || last_segment_->is_full()) {
      auto new_segment = mr.template create_bits<vector_segment<T>>(mr);

      if (!first_segment_) {
        first_segment_ = last_segment_ = new_segment.get();
      } else {
        last_segment_->next() = new_segment.get();
        last_segment_ = new_segment.get();
      }
      segment_count_++;
    }

    last_segment_->emplace_back(mr, std::forward<Args>(args)...);
    total_size_++;
  }

  // Element access
  T &operator[](size_t index) {
    auto [segment, local_index] = locate_element(index);
    return (*segment)[local_index];
  }

  const T &operator[](size_t index) const {
    auto [segment, local_index] = locate_element(index);
    return (*segment)[local_index];
  }

  T &at(size_t index) {
    if (index >= total_size_) {
      throw std::out_of_range("Vector index out of range");
    }
    return (*this)[index];
  }

  const T &at(size_t index) const {
    if (index >= total_size_) {
      throw std::out_of_range("Vector index out of range");
    }
    return (*this)[index];
  }

  T &front() {
    assert(!empty());
    return first_segment_->operator[](0);
  }

  const T &front() const {
    assert(!empty());
    return first_segment_->operator[](0);
  }

  T &back() {
    assert(!empty());
    return last_segment_->operator[](last_segment_->size() - 1);
  }

  const T &back() const {
    assert(!empty());
    return last_segment_->operator[](last_segment_->size() - 1);
  }

  // Capacity
  bool empty() const { return total_size_ == 0; }
  size_t size() const { return total_size_; }
  size_t segment_count() const { return segment_count_; }

  // Reserve capacity by ensuring we have enough segments
  template <typename RT> void reserve(memory_region<RT> &mr, size_t min_capacity) {
    size_t needed_segments = (min_capacity + vector_segment<T>::SEGMENT_SIZE - 1) / vector_segment<T>::SEGMENT_SIZE;

    while (segment_count_ < needed_segments) {
      auto new_segment = mr.template create_bits<vector_segment<T>>(mr);

      if (!first_segment_) {
        first_segment_ = last_segment_ = new_segment.get();
      } else {
        last_segment_->next() = new_segment.get();
        last_segment_ = new_segment.get();
      }
      segment_count_++;
    }
  }

  // Iterator implementation
  class iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T *;
    using reference = T &;

  private:
    vector_segment<T> *current_segment_;
    size_t local_index_;
    size_t global_index_;

  public:
    iterator(vector_segment<T> *segment, size_t local_idx, size_t global_idx)
        : current_segment_(segment), local_index_(local_idx), global_index_(global_idx) {}

    T &operator*() { return (*current_segment_)[local_index_]; }
    T *operator->() { return &(*current_segment_)[local_index_]; }

    iterator &operator++() {
      global_index_++;
      local_index_++;

      // Move to next segment if needed
      if (current_segment_ && local_index_ >= current_segment_->size()) {
        current_segment_ = current_segment_->next().get();
        local_index_ = 0;
      }
      return *this;
    }

    iterator operator++(int) {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const iterator &other) const { return global_index_ == other.global_index_; }
    bool operator!=(const iterator &other) const { return global_index_ != other.global_index_; }

    size_t index() const { return global_index_; }
  };

  class const_iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = const T *;
    using reference = const T &;

  private:
    const vector_segment<T> *current_segment_;
    size_t local_index_;
    size_t global_index_;

  public:
    const_iterator(const vector_segment<T> *segment, size_t local_idx, size_t global_idx)
        : current_segment_(segment), local_index_(local_idx), global_index_(global_idx) {}

    const T &operator*() const { return (*current_segment_)[local_index_]; }
    const T *operator->() const { return &(*current_segment_)[local_index_]; }

    const_iterator &operator++() {
      global_index_++;
      local_index_++;

      // Move to next segment if needed
      if (current_segment_ && local_index_ >= current_segment_->size()) {
        current_segment_ = current_segment_->next().get();
        local_index_ = 0;
      }
      return *this;
    }

    const_iterator operator++(int) {
      const_iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const const_iterator &other) const { return global_index_ == other.global_index_; }
    bool operator!=(const const_iterator &other) const { return global_index_ != other.global_index_; }

    size_t index() const { return global_index_; }
  };

  iterator begin() { return iterator(first_segment_.get(), 0, 0); }

  iterator end() { return iterator(nullptr, 0, total_size_); }

  const_iterator begin() const { return const_iterator(first_segment_.get(), 0, 0); }

  const_iterator end() const { return const_iterator(nullptr, 0, total_size_); }

  const_iterator cbegin() const { return begin(); }
  const_iterator cend() const { return end(); }
};

// Comparison operators
template <typename T>
auto operator<=>(const regional_vector<T> &lhs, const regional_vector<T> &rhs)
  requires std::three_way_comparable<T>
{
  auto lhs_it = lhs.begin();
  auto rhs_it = rhs.begin();

  while (lhs_it != lhs.end() && rhs_it != rhs.end()) {
    if (auto cmp = *lhs_it <=> *rhs_it; cmp != std::strong_ordering::equal) {
      return cmp;
    }
    ++lhs_it;
    ++rhs_it;
  }

  // Compare sizes if all compared elements are equal
  return lhs.size() <=> rhs.size();
}

template <typename T>
bool operator==(const regional_vector<T> &lhs, const regional_vector<T> &rhs)
  requires std::equality_comparable<T>
{
  if (lhs.size() != rhs.size())
    return false;

  auto lhs_it = lhs.begin();
  auto rhs_it = rhs.begin();

  while (lhs_it != lhs.end()) {
    if (!(*lhs_it == *rhs_it))
      return false;
    ++lhs_it;
    ++rhs_it;
  }

  return true;
}

} // namespace shilos
