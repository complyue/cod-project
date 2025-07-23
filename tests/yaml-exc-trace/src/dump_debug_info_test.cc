#include "shilos.hh"
#include "shilos/di.hh"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

// Test function with known location for address capture
__attribute__((noinline)) void testFunction() {
  // Place volatile marker to help identify this function in stack
  volatile int marker = 42;
  std::cout << "Inside testFunction with marker: " << marker << std::endl;
}

int main() {
  std::cout << "Testing dumpDebugInfo function from shilos/di.hh" << std::endl;

  // Get address of test function
  void *func_addr = (void *)(uintptr_t)&testFunction;
  std::cout << "Obtained address of testFunction: " << func_addr << std::endl;

  // Capture output to verify the dump
  std::ostringstream oss;
  shilos::dumpDebugInfo(func_addr, oss);
  std::string output = oss.str();

  std::cout << "dumpDebugInfo output:" << std::endl;
  std::cout << output << std::endl;

  // Verify the output contains expected elements
  bool success = !output.empty() && output.find("=== Debug Info Dump") != std::string::npos &&
                 output.find("Module:") != std::string::npos && output.find("Source location:") != std::string::npos &&
                 output.find("Function:") != std::string::npos && output.find("Compile unit:") != std::string::npos;

  if (success) {
    std::cout << "\033[0;32m✓ dumpDebugInfo test passed\033[0m" << std::endl;
    return 0;
  } else {
    std::cout << "\033[0;31m✗ dumpDebugInfo test failed\033[0m" << std::endl;
    return 1;
  }
}
