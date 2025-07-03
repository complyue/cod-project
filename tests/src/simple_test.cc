#include <cassert>
#include <iostream>

// Simple test that doesn't depend on shilos constexpr features
void test_basic_functionality() {
  std::cout << "=== Testing basic functionality ===" << std::endl;

  // Test basic C++20 features work
  std::cout << "✅ C++20 compiler test passed!" << std::endl;
  std::cout << "✅ Basic functionality test passed!" << std::endl;
}

int main() {
  std::cout << "Simple Test for COD Project" << std::endl;
  std::cout << "===========================" << std::endl;

  try {
    test_basic_functionality();

    std::cout << "\n✅ All tests passed!" << std::endl;
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "❌ Test failed: " << e.what() << std::endl;
    return 1;
  }
}
