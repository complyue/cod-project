#pragma once

#include <iostream>
#include <string>

namespace shilos {

// Get source location information from an address using DWARF debug info
std::string getSourceLocation(void *address);
// Dump comprehensive debug information from an address to the specified output stream
// This function extracts and displays DWARF debug information including
// function name, source location, compile unit details, and other relevant debug data
void dumpDebugInfo(void *address, std::ostream &os = std::cerr);

// Initialize LLVM components required for DWARF debug info handling
// This function should be called once before any exception throwing
// to ensure proper stack trace capture with source-level information
void initialize_llvm_components();

} // namespace shilos
