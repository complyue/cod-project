
#pragma once

#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>

namespace shilos {

class UUID {
private:
  uint8_t data_[16];

  constexpr static void byte_to_hex(std::uint8_t byte, char *output) {
    constexpr std::string_view hex_chars = "0123456789ABCDEF";
    output[0] = hex_chars[byte >> 4];
    output[1] = hex_chars[byte & 0x0F];
  }

  constexpr static std::uint8_t hex_to_byte(char c) {
    if (c >= '0' && c <= '9')
      return c - '0';
    if (c >= 'a' && c <= 'f')
      return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
      return c - 'A' + 10;
    throw std::invalid_argument(std::string("Invalid hex character: [") + c + "]");
  }

public:
  // Generate a random UUID
  UUID() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint8_t> dist(0, 255);
    for (auto &byte : data_) {
      byte = dist(gen);
    }
    // Adjust UUID fields according to RFC 4122:
    // - Set version to 4 (random UUID)
    data_[6] = (data_[6] & 0x0F) | 0x40;
    // - Set variant to 2 (RFC 4122 variant)
    data_[8] = (data_[8] & 0x3F) | 0x80;
  }

  // Construct from string_view
  explicit constexpr UUID(std::string_view str) {
    if (str.size() != 36)
      throw std::invalid_argument("Invalid UUID string size");
    size_t ui = 0;
    for (size_t i = 0; i < 36;) {
      if (i == 8 || i == 13 || i == 18 || i == 23) {
        if (str[i] != '-')
          throw std::invalid_argument("Invalid UUID format");
        i++;
      } else {
        data_[ui++] = (hex_to_byte(str[i]) << 4) | hex_to_byte(str[i + 1]);
        i += 2;
      }
    }
  }

  constexpr std::string to_string() const {
    char result[36];
    size_t idx = 0;
    for (size_t i = 0; i < 16; ++i) {
      byte_to_hex(data_[i], &result[idx]);
      idx += 2;
      // Add dashes at the appropriate positions
      if (i == 3 || i == 5 || i == 7 || i == 9) {
        result[idx++] = '-';
      }
    }
    return std::string(result, 36);
  }

  auto operator<=>(const UUID &other) const = default;
  bool operator==(const UUID &other) const = default;

  friend std::ostream &operator<<(std::ostream &os, const UUID &uuid) { return os << uuid.to_string(); }

  static UUID parse(const std::string &str) { return UUID(std::string_view(str)); }
  static UUID parse(std::string_view str) { return UUID(str); }
};

} // namespace shilos

inline std::ostream &operator<<(std::ostream &os, const shilos::UUID &uuid) { return os << uuid.to_string(); }
