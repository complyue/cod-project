#pragma once

// Minimal CoD header for testing without cache functionality
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace cod {
// Minimal WorksRoot for testing
struct WorksRoot {
  static constexpr const char *TYPE_UUID = "test-works-root";

  WorksRoot() = default;
};
} // namespace cod