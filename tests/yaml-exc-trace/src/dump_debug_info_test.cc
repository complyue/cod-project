#include "shilos.hh"
#include "shilos/di.hh"

#include <cassert>
#include <iostream>

#ifdef _WIN32
#define COD_TEST_NOINLINE __declspec(noinline)
#else
#define COD_TEST_NOINLINE __attribute__((noinline))
#endif

// Test function with known location for address capture
COD_TEST_NOINLINE void testFunction() {
  // Place volatile marker to help identify this function in stack
  volatile int marker = 42;
  std::cerr << "Inside testFunction with marker: " << marker << std::endl;
}

int main() {
  std::cerr << "Testing dumpDebugInfo function from shilos/di.hh" << std::endl;

  // Get address of test function
  void *func_addr = (void *)(uintptr_t)&testFunction;
  std::cerr << "Obtained address of testFunction: " << func_addr << std::endl;

  shilos::dumpDebugInfo(func_addr, std::cerr);

  std::cerr << std::endl;

  std::cerr << "\033[0;32m✓ dumpDebugInfo test passed\033[0m" << std::endl;
  return 0;
}
