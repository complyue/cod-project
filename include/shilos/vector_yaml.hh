#pragma once

#include "./vector.hh"

namespace shilos {

// YAML support for regional_vector
template <typename T>
  requires yaml::YamlConvertible<T, void>
inline yaml::Node to_yaml(const regional_vector<T> &vector) noexcept {
  yaml::Node node;
  auto &seq = std::get<yaml::Sequence>(node.value = yaml::Sequence{});

  for (const auto &item : vector) {
    seq.emplace_back(to_yaml(item));
  }

  return node;
}

template <typename T, typename RT>
  requires yaml::YamlConvertible<T, RT>
global_ptr<regional_vector<T>, RT> from_yaml(memory_region<RT> &mr, const yaml::Node &node) {
  if (!std::holds_alternative<yaml::Sequence>(node.value)) {
    throw yaml::TypeError("Expected Sequence for regional_vector");
  }

  auto vector = mr.template create<regional_vector<T>>(mr);
  const auto &seq = std::get<yaml::Sequence>(node.value);

  for (const auto &item_node : seq) {
    // Ensure we have a segment with space
    if (!vector->last_segment_ || vector->last_segment_->is_full()) {
      auto new_segment = mr.template create<vector_segment<T>>(mr);
      if (!vector->first_segment_) {
        vector->first_segment_ = vector->last_segment_ = new_segment.get();
      } else {
        vector->last_segment_->next() = new_segment.get();
        vector->last_segment_ = new_segment.get();
      }
      vector->segment_count_++;
    }

    // Get the current insertion point in the last segment
    size_t insert_index = vector->last_segment_->size();
    T *element_location = &vector->last_segment_->elements_[insert_index];

    // Construct the element directly using raw pointer version of from_yaml
    from_yaml(mr, item_node, element_location);

    // Update segment and vector size
    vector->last_segment_->size_++;
    vector->total_size_++;
  }

  return vector;
}

template <typename T, typename RT>
  requires yaml::YamlConvertible<T, RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_ptr<regional_vector<T>> &to_ptr) {
  // Allocate uninitialized vector
  auto vector_ptr = mr.template allocate<regional_vector<T>>();
  // Use raw pointer version to construct directly
  from_yaml(mr, node, vector_ptr.get());
  // Assign the raw pointer to the regional_ptr (assumes to_ptr is in memory region)
  to_ptr = vector_ptr.get();
}

template <typename T, typename RT>
  requires yaml::YamlConvertible<T, RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_vector<T> *raw_ptr) {
  if (!std::holds_alternative<yaml::Sequence>(node.value)) {
    throw yaml::TypeError("Expected Sequence for regional_vector");
  }

  new (raw_ptr) regional_vector<T>(mr);
  const auto &seq = std::get<yaml::Sequence>(node.value);

  for (const auto &item_node : seq) {
    // Ensure we have a segment with space
    if (!raw_ptr->last_segment_ || raw_ptr->last_segment_->is_full()) {
      auto new_segment = mr.template create<vector_segment<T>>(mr);
      if (!raw_ptr->first_segment_) {
        raw_ptr->first_segment_ = raw_ptr->last_segment_ = new_segment.get();
      } else {
        raw_ptr->last_segment_->next() = new_segment.get();
        raw_ptr->last_segment_ = new_segment.get();
      }
      raw_ptr->segment_count_++;
    }

    // Get the current insertion point in the last segment
    size_t insert_index = raw_ptr->last_segment_->size();
    T *element_location = &raw_ptr->last_segment_->elements_[insert_index];

    // Construct the element directly using raw pointer version of from_yaml
    from_yaml<T>(mr, item_node, element_location);

    // Update segment and vector size
    raw_ptr->last_segment_->size_++;
    raw_ptr->total_size_++;
  }
}

} // namespace shilos
