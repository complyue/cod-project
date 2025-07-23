
#pragma once

#include "shilos/region.hh" // IWYU pragma: keep

#include "shilos/str.hh" // IWYU pragma: keep

#include "shilos/dbmr.hh" // IWYU pragma: keep

namespace shilos {
namespace yaml {
// Initialize LLVM components required for DWARF debug info handling
// This function should be called once before any exception throwing
// to ensure proper stack trace capture with source-level information
void initialize_llvm_components();
} // namespace yaml
} // namespace shilos
