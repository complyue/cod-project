#pragma once

#include "./region.hh"

#include <cassert>
#include <compare>
#include <fcntl.h>
#include <iostream>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace shilos {

class regional_str final {
private:
  size_t length_;
  regional_ptr<std::byte> data_;

public:
  regional_str() : length_(0), data_() {}

  template <typename RT>
  regional_str(memory_region<RT> &mr, const std::string &str) : regional_str(mr, std::string_view(str)) {}

  template <typename RT> regional_str(memory_region<RT> &mr, std::string_view str) : length_(str.length()), data_() {
    if (length_ <= 0) {
      return;
    }
    std::byte *p_data = mr.template allocate<std::byte>(length_);
    std::memcpy(p_data, str.data(), length_);
    this->data_ = p_data;
  }

  bool empty() const { return length_ <= 0; }
  size_t length() const { return length_; }
  std::byte *data() { return data_.get(); }
  const std::byte *data() const { return data_.get(); }

  ~regional_str() = default;
  regional_str(const regional_str &) = delete;
  regional_str(regional_str &&) = delete;
  regional_str &operator=(const regional_str &) = delete;
  regional_str &operator=(regional_str &&) = delete;

  operator std::string_view() const { return std::string_view(reinterpret_cast<const char *>(data()), length()); }

  yaml::Node to_yaml() const { return yaml::Node(static_cast<std::string_view>(*this)); }

  template <typename RT>
    requires ValidMemRegionRootType<RT>
  static global_ptr<regional_str, RT> from_yaml(memory_region<RT> &mr, const yaml::Node &node) {
    if (auto str = std::get_if<std::string>(&node.value)) {
      return mr.template create<regional_str>(*str);
    }
    throw yaml::TypeError("Invalid YAML node type for regional_str");
  }

  template <typename RT>
    requires ValidMemRegionRootType<RT>
  static void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_ptr<regional_str> &to_ptr) {
    if (auto str = std::get_if<std::string>(&node.value)) {
      mr.template create_to<regional_ptr>(to_ptr, *str);
    } else {
      throw yaml::TypeError("Invalid YAML node type for regional_str");
    }
  }

  std::strong_ordering operator<=>(const regional_str &other) const noexcept {
    if (auto cmp = length_ <=> other.length_; cmp != 0) {
      return cmp;
    }
    if (!data_ || !other.data_) {
      if (data_ == other.data_) {
        return std::strong_ordering::equal;
      }
      return !data_ ? std::strong_ordering::less : std::strong_ordering::greater;
    }
    const int result = std::memcmp(data_.get(), other.data_.get(), length_);
    if (result < 0)
      return std::strong_ordering::less;
    if (result > 0)
      return std::strong_ordering::greater;
    return std::strong_ordering::equal;
  }
  bool operator==(const regional_str &other) const noexcept {
    if (length_ != other.length_) {
      return false;
    }
    if (data_.get() == other.data_.get())
      return true;
    if (!data_ || !other.data_) {
      return false;
    }
    return 0 == std::memcmp(data_.get(), other.data_.get(), length_);
  }
};

inline std::ostream &operator<<(std::ostream &os, const regional_str &str) {
  os << static_cast<std::string_view>(str);
  return os;
}

} // namespace shilos
