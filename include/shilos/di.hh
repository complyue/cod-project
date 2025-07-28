#pragma once

#include <iostream>
#include <string>

namespace shilos {

// Get source location information from an address using DWARF debug info
void formatBacktraceFrame(int btDepth, void *address, std::string &last_debug_file_path, std::ostringstream &oss);

// Dump comprehensive debug information from an address to the specified output stream
void dumpDebugInfo(void *address, std::ostream &os = std::cerr);

} // namespace shilos
