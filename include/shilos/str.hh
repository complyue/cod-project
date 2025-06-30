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

  template <typename RT>
  regional_str(memory_region<RT> &mr, const char *str) : regional_str(mr, std::string_view(str)) {}

  bool empty() const { return length_ <= 0; }
  size_t length() const { return length_; }
  size_t size() const { return length_; }
  std::byte *data() { return data_.get(); }
  const std::byte *data() const { return data_.get(); }

  ~regional_str() = default;
  regional_str(const regional_str &) = delete;
  regional_str(regional_str &&) = delete;
  regional_str &operator=(const regional_str &) = delete;
  regional_str &operator=(regional_str &&) = delete;

  operator std::string_view() const { return std::string_view(reinterpret_cast<const char *>(data()), length()); }

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

  // Cross-type equality operators for heterogeneous key support
  bool operator==(std::string_view other) const noexcept {
    if (length_ != other.size()) {
      return false;
    }
    if (length_ == 0) {
      return true;
    }
    return 0 == std::memcmp(data_.get(), other.data(), length_);
  }

  bool operator==(const char *other) const noexcept { return *this == std::string_view(other); }

  bool operator==(const std::string &other) const noexcept { return *this == std::string_view(other); }
};

inline std::ostream &operator<<(std::ostream &os, const regional_str &str) {
  os << static_cast<std::string_view>(str);
  return os;
}

// Cross-type equality operators (symmetric)
inline bool operator==(std::string_view lhs, const regional_str &rhs) noexcept { return rhs == lhs; }

inline bool operator==(const char *lhs, const regional_str &rhs) noexcept { return rhs == lhs; }

inline bool operator==(const std::string &lhs, const regional_str &rhs) noexcept { return rhs == lhs; }

} // namespace shilos

// Specialize std::hash for regional_str
namespace std {
template <> struct hash<shilos::regional_str> {
  size_t operator()(const shilos::regional_str &str) const noexcept {
    // Use std::hash<std::string_view> for consistent hashing
    return std::hash<std::string_view>{}(static_cast<std::string_view>(str));
  }
};
} // namespace std
