
#pragma once

#include "./stake.hh"

namespace shilos {

struct stake_header {
  std::uint16_t magic;
  struct {
    std::int16_t major;
    std::int16_t minor;
  } version;
  std::int16_t flags;
  relativ_ptr<std::byte> root; // subject to reinterpretation after sufficient type checks
};

class file_stake : public memory_stake {
public:
};

} // namespace shilos
