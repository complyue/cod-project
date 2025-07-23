#include "shilos.hh"
#include "shilos/di.hh"

#include <cassert>
#include <iostream>
#include <string>

// Test function with known location for address capture
__attribute__((noinline)) void testFunction() {
  // Place volatile marker to help identify this function in stack
  volatile int marker = 42;
  std::cout << "Inside testFunction with marker: " << marker << std::endl;
}

int main() {
  std::cout << "Testing getSourceLocation function from shilos/di.hh" << std::endl;

  // Initialize LLVM components required for DWARF debug info handling
  shilos::initialize_llvm_components();

  // Get address of test function
  void *func_addr = (void *)(uintptr_t)&testFunction;
  std::cout << "Obtained address of testFunction: " << func_addr << std::endl;

  // Call getSourceLocation with the function address
  std::string location = shilos::yaml::getSourceLocation(func_addr);

  std::cout << "getSourceLocation result: '" << location << "'" << std::endl;

  // Verify the location contains expected elements
  bool success = !location.empty() && location.find("get_source_location_test.cc") != std::string::npos;

  if (success) {
    std::cout << "\033[0;32m✓ getSourceLocation test passed\033[0m" << std::endl;
    return 0;
  } else {
    std::cout << "\033[0;31m✗ getSourceLocation test failed\033[0m" << std::endl;
    return 1;
  }
}
