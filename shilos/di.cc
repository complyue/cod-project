#include "shilos/di.hh"

#include <cerrno>
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

// LLVM DWARF debug info for enhanced stack traces
#include "llvm/DebugInfo/DIContext.h"
#include "llvm/DebugInfo/DWARF/DWARFCompileUnit.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/DebugInfo/DWARF/DWARFDebugAranges.h"
#include "llvm/DebugInfo/DWARF/DWARFUnit.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/MemoryBuffer.h"

namespace shilos {

// Structure to hold cached debug information for a module
struct ModuleDebugInfo {
  Dl_info info;
  std::unique_ptr<llvm::MemoryBuffer> buffer;
  std::unique_ptr<llvm::object::ObjectFile> object;
  std::unique_ptr<llvm::DWARFContext> context;

  // Default constructor
  ModuleDebugInfo() = default;

  ModuleDebugInfo(const Dl_info &module_info, std::unique_ptr<llvm::MemoryBuffer> buf,
                  std::unique_ptr<llvm::object::ObjectFile> obj, std::unique_ptr<llvm::DWARFContext> ctx)
      : info(module_info), buffer(std::move(buf)), object(std::move(obj)), context(std::move(ctx)) {}
};

// Cache for module debug information, keyed by the module's file path
static std::unordered_map<std::string, ModuleDebugInfo> debug_info_cache;
static std::mutex cache_mutex;

// Function to clear the debug info cache (useful for testing or when binaries are reloaded)
void clearDebugInfoCache() {
  std::lock_guard<std::mutex> lock(cache_mutex);
  debug_info_cache.clear();
}

llvm::DWARFContext *getModuleDebugInfo(const Dl_info &info) {
  // Check if debug info is already cached
  {
    std::lock_guard<std::mutex> lock(cache_mutex);
    auto it = debug_info_cache.find(info.dli_fname);
    if (it != debug_info_cache.end()) {
      return it->second.context.get();
    }
  }

  // On macOS, debug info is stored in a separate dSYM bundle
  std::string debug_file_path = info.dli_fname;

#ifdef __APPLE__
  // Construct the dSYM path
  // Extract just the filename from the full path
  std::string filename = info.dli_fname;
  auto last_slash = filename.find_last_of("/");
  if (last_slash != std::string::npos) {
    filename = filename.substr(last_slash + 1);
  }

  std::string dsym_path = debug_file_path + ".dSYM/Contents/Resources/DWARF/" + filename;
  std::ifstream dsym_test(dsym_path);
  if (dsym_test.good()) {
    debug_file_path = dsym_path;
  }
#endif

  // Create a memory buffer from the binary file or dSYM
  auto buffer_or = llvm::MemoryBuffer::getFile(debug_file_path, -1, false);
  if (!buffer_or) {
    return nullptr;
  }
  std::unique_ptr<llvm::MemoryBuffer> buffer = std::move(buffer_or.get());

  // Create an object file from the buffer
  auto object_or = llvm::object::ObjectFile::createObjectFile(buffer->getMemBufferRef());
  if (!object_or) {
    return nullptr;
  }
  std::unique_ptr<llvm::object::ObjectFile> object = std::move(object_or.get());

  // Create DWARF context
  auto context = llvm::DWARFContext::create(*object);
  if (!context) {
    return nullptr;
  }

  // Cache the debug info
  {
    std::lock_guard<std::mutex> lock(cache_mutex);
    // Check again for race condition
    auto it = debug_info_cache.find(info.dli_fname);
    if (it == debug_info_cache.end()) {
      debug_info_cache.emplace(info.dli_fname,
                               ModuleDebugInfo(info, std::move(buffer), std::move(object), std::move(context)));
    }
    return debug_info_cache[info.dli_fname].context.get();
  }
}

void formatBacktraceFrame(int btDepth, void *address, std::ostringstream &os) {
  os << "#" << std::setw(2) << std::setfill(' ') << btDepth << " ";

  // Get the base address and path of the module containing the address
  Dl_info info;
  if (!dladdr(address, &info) || !info.dli_fname) {
    os << "📍 <unknown-src-location>";
    return;
  }

  // Check if this is an invalid frame (both function name and source location are <invalid> esque)
  bool is_invalid_function =
      (!info.dli_sname || std::string(info.dli_sname) == "<invalid>" || std::string(info.dli_sname).empty());
  bool is_invalid_module =
      (!info.dli_fname || std::string(info.dli_fname) == "<invalid>" || std::string(info.dli_fname).empty());

  if (is_invalid_function && is_invalid_module) {
    // Output shorter single line when both function name and src location are <invalid> esque
    os << "📍 <unknown-frame>";
    return;
  }

  auto *context = getModuleDebugInfo(info);
  if (!context) {
    // If no debug context, fall back to dladdr symbol name
    std::string function_name;
    if (info.dli_sname && std::string(info.dli_sname) != "<invalid>") {
      // Demangle the symbol name
      int status;
      char *demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
      if (status == 0 && demangled) {
        function_name = demangled;
        free(demangled);
      } else {
        function_name = info.dli_sname;
      }
    }

    // Output lines with emojis when we have partial information
    if (!function_name.empty()) {
      os << "🌀  " << function_name << "\n";
    }

    if (info.dli_fname) {
      os << "📦 " << info.dli_fname;
    }
    return;
  }

  // Get the section contribution for the address
  uint64_t section_offset = (uintptr_t)(address) - (uintptr_t)(info.dli_fbase);

  uint64_t debug_address = section_offset;
#ifdef __APPLE__
  // The dSYM file would contain addresses with an assumed 0x100000000 base
  debug_address += 0x100000000;
#endif

  // Get line info from DWARF debug information
  llvm::DILineInfoSpecifier spec(llvm::DILineInfoSpecifier::FileLineInfoKind::AbsoluteFilePath,
                                 llvm::DILineInfoSpecifier::FunctionNameKind::LinkageName);
  llvm::object::SectionedAddress sectionedAddr = {debug_address, (uint64_t)-1LL};
  llvm::DILineInfo lineInfo = context->getLineInfoForAddress(sectionedAddr, spec);

  // Prefer DWARF function name, demangled if possible
  std::string function_name;
  if (!lineInfo.FunctionName.empty() && lineInfo.FunctionName != "<invalid>") {
    // Demangle the DWARF function name
    int status;
    char *demangled = abi::__cxa_demangle(lineInfo.FunctionName.c_str(), nullptr, nullptr, &status);
    if (status == 0 && demangled) {
      function_name = demangled;
      free(demangled);
    } else {
      function_name = lineInfo.FunctionName;
    }
  } else if (info.dli_sname && std::string(info.dli_sname) != "<invalid>") {
    // Fallback to dladdr symbol name
    int status;
    char *demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
    if (status == 0 && demangled) {
      function_name = demangled;
      free(demangled);
    } else {
      function_name = info.dli_sname;
    }
  }

  // Output lines each starting with a proper emoji and appropriate indentation
  if (!function_name.empty()) {
    os << "🌀  " << function_name << "\n";
  }

  // vscode-clickable source location
  if (!lineInfo.FileName.empty() && lineInfo.FileName != "<invalid>") {
    os << "   👉 " << lineInfo.FileName << ":" << lineInfo.Line;
    if (lineInfo.Column > 0) {
      os << ":" << lineInfo.Column;
    }
    os << "\n";
  }

  if (info.dli_fname) {
    os << "📦 " << info.dli_fname;
  }
}

void dumpDebugInfo(void *address, std::ostream &os) {
  os << "=== Debug Info Dump for address " << address << " ===" << std::endl;

  // Get the base address and path of the module containing the address
  Dl_info info;
  if (!dladdr(address, &info) || !info.dli_fname) {
    os << "  Failed to get module info for address" << std::endl;
    os << "=== End Debug Info Dump ===" << std::endl;
    return;
  }

  // Get debug context for the module
  auto *context = getModuleDebugInfo(info);
  if (!context) {
    os << "  No debug context available for module" << std::endl;
    os << "=== End Debug Info Dump ===" << std::endl;
    return;
  }

  // Calculate section offset
  uint64_t section_offset = (uintptr_t)(address) - (uintptr_t)(info.dli_fbase);
  uint64_t debug_address = section_offset;

#ifdef __APPLE__
  // The dSYM file would contain addresses with an assumed 0x100000000 base
  debug_address += 0x100000000;
#endif

  // Create specifier for comprehensive debug info
  llvm::DILineInfoSpecifier spec(llvm::DILineInfoSpecifier::FileLineInfoKind::AbsoluteFilePath,
                                 llvm::DILineInfoSpecifier::FunctionNameKind::LinkageName);

  // Get line info for the address
  llvm::object::SectionedAddress sectionedAddr = {debug_address, (uint64_t)-1LL};
  llvm::DILineInfo lineInfo = context->getLineInfoForAddress(sectionedAddr, spec);

  // Dump the comprehensive debug information
  os << "  Function: " << (lineInfo.FunctionName.empty() ? "<unknown>" : lineInfo.FunctionName) << std::endl;
  os << "  File: " << (lineInfo.FileName.empty() ? "<unknown>" : lineInfo.FileName) << std::endl;
  os << "  Line: " << lineInfo.Line << std::endl;
  os << "  Column: " << lineInfo.Column << std::endl;
  os << "  Start Line: " << lineInfo.StartLine << std::endl;

  // Also try to get symbol name from dladdr for comparison
  std::string function_name = "<unknown>";
  if (info.dli_sname) {
    // Demangle the symbol name
    int status;
    char *demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
    if (status == 0 && demangled) {
      function_name = demangled;
      free(demangled);
    } else {
      function_name = info.dli_sname;
    }
  }

  os << "  Symbol (dladdr): " << function_name << std::endl;
  os << "  Module: " << info.dli_fname << std::endl;

  os << "=== End Debug Info Dump ===" << std::endl;
}

} // namespace shilos
