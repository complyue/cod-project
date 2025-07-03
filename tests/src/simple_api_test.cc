#include "shilos.hh"
#include <iostream>

using namespace shilos;

struct SimpleRoot {
  static const UUID TYPE_UUID;

  SimpleRoot(memory_region<SimpleRoot> &mr) {}
};

const UUID SimpleRoot::TYPE_UUID = UUID("12345678-1234-5678-9abc-123456789012");

int main() {
  std::cout << "Testing basic API..." << std::endl;

  try {
    // Test basic memory region allocation
    auto mr_ptr = memory_region<SimpleRoot>::alloc_region(1024 * 1024);
    memory_region<SimpleRoot> &mr = *mr_ptr;
    auto root = mr.root();

    std::cout << "✓ Memory region allocation successful" << std::endl;

    // Cleanup
    delete mr_ptr;

    std::cout << "✓ Simple API test passed!" << std::endl;
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "❌ Test failed: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "❌ Test failed with unknown exception" << std::endl;
    return 1;
  }
}
