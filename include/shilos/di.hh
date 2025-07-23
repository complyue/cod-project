#pragma once

#include <string>

namespace shilos {

namespace yaml {

// Get source location information from an address using DWARF debug info
std::string getSourceLocation(void *address);

} // namespace yaml

// Initialize LLVM components required for DWARF debug info handling
// This function should be called once before any exception throwing
// to ensure proper stack trace capture with source-level information
void initialize_llvm_components();

} // namespace shilos
