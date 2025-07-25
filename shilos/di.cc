#include "shilos/di.hh"

#include <cerrno>
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

// LLVM DWARF debug info for enhanced stack traces
#include "llvm/ADT/DenseSet.h"
#include "llvm/DebugInfo/DIContext.h"
#include "llvm/DebugInfo/DWARF/DWARFCompileUnit.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/DebugInfo/DWARF/DWARFDebugAranges.h"
#include "llvm/DebugInfo/DWARF/DWARFDie.h"
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

std::string getSourceLocation(void *address) {
  // Get the base address and path of the module containing the address
  Dl_info info;
  if (!dladdr(address, &info) || !info.dli_fname) {
    return "<unknown-src-location>";
  }

  std::ostringstream oss;

  // If we have a symbol name, demangle it for better readability
  std::string function_name = "<unknown-function>";
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

  oss << "in " << function_name;

  auto *context = getModuleDebugInfo(info);
  if (!context) {
    oss << " (" << info.dli_fname << ")";
    return oss.str();
  }

  // Get the section contribution for the address
  uint64_t section_offset = (uintptr_t)(address) - (uintptr_t)(info.dli_fbase);

  uint64_t debug_address = section_offset;
#ifdef __APPLE__
  // The dSYM file would contain addresses with an assumed 0x100000000 base
  debug_address += 0x100000000;
#endif

  // Find the compile unit that contains this address
  auto cu = context->getCompileUnitForCodeAddress(debug_address);
  if (!cu) {
    oss << " (" << info.dli_fname << ")";
    return oss.str();
  }

  // Get the line table for the compile unit
  auto line_table = context->getLineTableForUnit(static_cast<llvm::DWARFUnit *>(cu));
  if (!line_table) {
    oss << " (" << info.dli_fname << ")";
    return oss.str();
  }

  // Find the row in the line table for this address
  llvm::DILineInfo row;
  if (!line_table->getFileLineInfoForAddress({debug_address, (uint64_t)-1LL}, info.dli_fname,
                                             llvm::DILineInfoSpecifier::FileLineInfoKind::AbsoluteFilePath, row)) {
    oss << " (" << info.dli_fname << ")";
    return oss.str();
  }

  // vscode-clickable source location
  if (row.FileName.empty()) {
    oss << " at " << "<unknown-src-file>";
  } else {
    oss << " at " << row.FileName << ":" << row.Line;
    if (row.Column > 0) {
      oss << ":" << row.Column;
    }
  }

  oss << " (" << info.dli_fname << ")";
  return oss.str();
}

// Shadow class to access private members of DWARFDebugAranges
// This mirrors the structure in llvm-project/llvm/include/llvm/DebugInfo/DWARF/DWARFDebugAranges.h
class DWARFDebugArangesShadow {
public:
  struct Range {
    uint64_t LowPC;    /// Start of address range.
    uint64_t Length;   /// End of address range (not including this address).
    uint64_t CUOffset; /// Offset of the compile unit or die.
  };

  struct RangeEndpoint {
    uint64_t Address;
    uint64_t CUOffset;
    bool IsRangeStart;
  };

  using RangeColl = std::vector<Range>;
  using RangeCollIterator = RangeColl::const_iterator;

  // This mirrors the private members of DWARFDebugAranges
  std::vector<RangeEndpoint> Endpoints;
  RangeColl Aranges;
  llvm::DenseSet<uint64_t> ParsedCUOffsets;
};

void dumpDebugInfo(void *address, std::ostream &os) {
  os << "=== Debug Info Dump for address " << address << " ===" << std::endl;

  // Get the base address and path of the module containing the address
  Dl_info info;
  if (!dladdr(address, &info) || !info.dli_fname) {
    os << "Error: dladdr failed or no filename found" << std::endl;
    return;
  }
  os << "Module: " << info.dli_fname << std::endl;
  os << "Base address: " << info.dli_fbase << std::endl;

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
    os << "Using dSYM debug info from: " << debug_file_path << std::endl;
  } else {
    os << "dSYM not found at: " << dsym_path << std::endl;
  }
#endif

  // Create a memory buffer from the binary file or dSYM
  auto buffer_or = llvm::MemoryBuffer::getFile(debug_file_path, -1, false);
  if (!buffer_or) {
    os << "Error: failed to create memory buffer for " << debug_file_path << std::endl;
    return;
  }
  std::unique_ptr<llvm::MemoryBuffer> buffer = std::move(buffer_or.get());
  os << "Memory buffer created successfully" << std::endl;

  // Create an object file from the buffer
  auto object_or = llvm::object::ObjectFile::createObjectFile(buffer->getMemBufferRef());
  if (!object_or) {
    os << "Error: failed to create object file" << std::endl;
    return;
  }
  std::unique_ptr<llvm::object::ObjectFile> object = std::move(object_or.get());
  os << "Object file created successfully" << std::endl;

  // Create DWARF context
  auto context = llvm::DWARFContext::create(*object);
  if (!context) {
    os << "Error: failed to create DWARF context" << std::endl;
    return;
  }
  os << "DWARF context created successfully" << std::endl;

  // Dump comprehensive information about the DWARF context
  os << "--- DWARF Context Information ---" << std::endl;
  // Dump debug ranges information using shadow class to access private members
  os << "Debug Aranges:" << std::endl;
  auto debugAranges = context->getDebugAranges();
  if (debugAranges) {
    // Cast to our shadow class to access private members
    const DWARFDebugArangesShadow *shadow = reinterpret_cast<const DWARFDebugArangesShadow *>(debugAranges);

    if (!shadow->Aranges.empty()) {
      for (size_t i = 0; i < shadow->Aranges.size(); ++i) {
        const auto &range = shadow->Aranges[i];
        os << "  Range[" << i << "]: 0x" << std::hex << range.LowPC << " - 0x" << (range.LowPC + range.Length)
           << std::dec << " (CU offset: 0x" << std::hex << range.CUOffset << std::dec << ")" << std::endl;
      }
    } else {
      os << "  No address ranges found" << std::endl;
    }
  } else {
    os << "  No debug aranges available" << std::endl;
  }

  os << "Number of compile units: " << context->getNumCompileUnits() << std::endl;

  // Dump information about all sections in the object file
  os << "--- Object File Sections ---" << std::endl;
  auto obj_sections = object->sections();
  for (auto &section : obj_sections) {
    auto sec_name = section.getName();
    if (sec_name) {
      os << "Section: " << sec_name->str() << ", Address: 0x" << std::hex << section.getAddress() << ", Size: 0x"
         << section.getSize() << ", Index: " << section.getIndex() << std::dec << std::endl;
    }
  }

  // List all compile units and their address ranges
  os << "Compile Units:" << std::endl;
  for (size_t i = 0; i < context->getNumCompileUnits(); ++i) {
    auto cu = context->getCompileUnitForOffset(i);
    if (cu) {
      os << "  CU[" << i << "]: " << std::hex << "0x" << cu->getOffset() << " - 0x"
         << (cu->getOffset() + cu->getLength()) << std::dec << ", Address Range: ";

      // Get the first DIE of the compile unit to get address ranges
      auto unit_die = cu->getUnitDIE();
      if (unit_die.isValid()) {
        // Dump information about the compile unit DIE
        os << "  DIE Tag: " << llvm::dwarf::TagString(unit_die.getTag()).str() << std::endl;

        // Dump all attributes of the compile unit DIE
        os << "  Attributes:" << std::endl;
        for (auto &attr : unit_die.attributes()) {
          auto attr_name = llvm::dwarf::AttributeString(attr.Attr);
          os << "    " << attr_name.str() << " (0x" << std::hex << attr.Attr << std::dec << "): ";

          switch (attr.Value.getForm()) {
          case llvm::dwarf::DW_FORM_string:
          case llvm::dwarf::DW_FORM_strp:
            if (auto str = attr.Value.getAsCString()) {
              os << "\"" << *str << "\"";
            }
            break;
          case llvm::dwarf::DW_FORM_udata:
            if (auto val = attr.Value.getAsUnsignedConstant()) {
              // Handle DW_AT_language attribute specifically
              if (attr.Attr == llvm::dwarf::DW_AT_language) {
                llvm::StringRef lang_str_ref = llvm::dwarf::LanguageString(*val);
                os << lang_str_ref.str() << " (" << *val << ")";
              } else {
                os << *val;
              }
            } else {
              os << "(invalid)";
            }
            break;
          case llvm::dwarf::DW_FORM_sdata:
            if (auto val = attr.Value.getAsSignedConstant()) {
              os << *val;
            } else {
              os << "(invalid)";
            }
            break;
          case llvm::dwarf::DW_FORM_data1:
          case llvm::dwarf::DW_FORM_data2:
          case llvm::dwarf::DW_FORM_data4:
          case llvm::dwarf::DW_FORM_data8:
            if (auto val = attr.Value.getAsUnsignedConstant()) {
              os << "0x" << std::hex << *val << std::dec;
            } else {
              os << "(invalid)";
            }
            break;
            if (auto val = attr.Value.getAsSignedConstant()) {
              os << *val;
            } else {
              os << "(invalid)";
            }
            break;
          case llvm::dwarf::DW_FORM_addr:
            if (auto val = attr.Value.getAsAddress()) {
              os << "0x" << std::hex << *val << std::dec;
            } else {
              os << "(invalid)";
            }
            break;
          default:
            os << "(value type not handled)";
            break;
          }
          os << std::endl;
        }

        // Get address ranges
        uint64_t low_pc = 0, high_pc = 0;
        uint64_t section_index = 0;
        if (unit_die.getLowAndHighPC(low_pc, high_pc, section_index)) {
          os << std::hex << "0x" << low_pc << " - 0x" << high_pc << std::dec;
        } else {
          // Check for ranges attribute
          auto ranges_attr = unit_die.find(llvm::dwarf::DW_AT_ranges);
          if (ranges_attr) {
            os << "(ranges table)";
          }
        }

        // Extract compile unit name
        auto name_attr = unit_die.find(llvm::dwarf::DW_AT_name);
        if (name_attr) {
          if (auto name = name_attr->getAsCString()) {
            os << ", File: " << *name;
          }
        }
      }
      os << std::endl;
    }
  }

  // Get the section contribution for the address
  uint64_t section_offset = (uintptr_t)(address) - (uintptr_t)(info.dli_fbase);
  os << "Section offset: 0x" << std::hex << section_offset << std::dec << std::endl;

  // The dSYM file would contain addresses with an assumed 0x100000000 base
  uint64_t debug_address = section_offset;
  if (debug_file_path != info.dli_fname) {
    debug_address += 0x100000000;
  }

  os << "Using effective debug_address: 0x" << std::hex << debug_address << std::dec << std::endl;

  // Find the compile unit that contains this address
  auto cu = context->getCompileUnitForCodeAddress(debug_address);
  if (!cu) {
    os << "Error: failed to find compile unit for address" << std::endl;

    // Additional diagnostic: check if address falls within any known section
    bool in_text_section = false;
    auto sections = object->sections();
    for (auto &section : sections) {
      auto sec_name = section.getName();
      if (sec_name && *sec_name == "__text") {
        uint64_t sec_addr = section.getAddress();
        uint64_t sec_size = section.getSize();
        if (section_offset >= sec_addr && section_offset < sec_addr + sec_size) {
          in_text_section = true;
          break;
        }
      }
    }

    if (in_text_section) {
      os << "Note: Address is within __text section but no debug info found" << std::endl;
    } else {
      os << "Note: Address is not within __text section" << std::endl;
    }

    return;
  }
  os << "Found compile unit" << std::endl;

  // Dump detailed compile unit information
  os << "--- Compile Unit Details ---" << std::endl;
  auto cu_die = cu->getUnitDIE();
  if (cu_die.isValid()) {
    // Extract and display all relevant attributes from the compile unit DIE
    os << "Compile Unit DIE:" << std::endl;

    // Language
    if (auto lang_attr = cu_die.find(llvm::dwarf::DW_AT_language)) {
      if (auto lang_value = lang_attr->getAsUnsignedConstant()) {
        llvm::StringRef lang_str_ref = llvm::dwarf::LanguageString(*lang_value);
        os << "  Language: " << lang_str_ref.str() << " (" << *lang_value << ")" << std::endl;
      }
    }

    // Producer (compiler)
    if (auto producer_attr = cu_die.find(llvm::dwarf::DW_AT_producer)) {
      if (auto producer = producer_attr->getAsCString()) {
        os << "  Producer: " << *producer << std::endl;
      }
    }

    // Compilation directory
    if (auto comp_dir_attr = cu_die.find(llvm::dwarf::DW_AT_comp_dir)) {
      if (auto comp_dir = comp_dir_attr->getAsCString()) {
        os << "  Compilation Directory: " << *comp_dir << std::endl;
      }
    }

    // Source file name
    if (auto name_attr = cu_die.find(llvm::dwarf::DW_AT_name)) {
      if (auto name = name_attr->getAsCString()) {
        os << "  Source File: " << *name << std::endl;
      }
    }

    // Include directory
    if (auto include_dir_attr = cu_die.find(llvm::dwarf::DW_AT_comp_dir)) {
      if (auto include_dir = include_dir_attr->getAsCString()) {
        os << "  Include Directory: " << *include_dir << std::endl;
      }
    }

    // DWARF version
    os << "  DWARF Version: " << cu->getVersion() << std::endl;

    // Address size
    // Cannot access address size directly, omit this information

    // Offset size
    os << "  Offset Size: " << cu->getLength() << " bytes" << std::endl;
  }

  // Dump section information
  os << "--- Section Information ---" << std::endl;
  auto sections = object->sections();
  for (auto &section : sections) {
    auto sec_name = section.getName();
    if (sec_name) {
      os << "Section: " << sec_name->str() << ", Address: 0x" << std::hex << section.getAddress() << ", Size: 0x"
         << section.getSize() << std::dec << std::endl;
    }
  }

  // Get the line table for the compile unit
  auto line_table = context->getLineTableForUnit(cu);
  if (!line_table) {
    os << "Error: failed to get line table for unit" << std::endl;
    return;
  }
  os << "Got line table successfully" << std::endl;

  // Find the row in the line table for this address
  llvm::DILineInfo row;
  if (!line_table->getFileLineInfoForAddress({debug_address, (uint64_t)-1LL}, info.dli_fname,
                                             llvm::DILineInfoSpecifier::FileLineInfoKind::AbsoluteFilePath, row)) {
    os << "Error: failed to get file line info for address" << std::endl;
    return;
  }
  os << "Got file line info" << std::endl;

  // Extract function information using the compile unit's DIE
  bool found_function = false;
  auto unit_die = cu->getUnitDIE();

  // First try: direct subprogram lookup
  for (auto &die : unit_die) {
    if (die.getTag() == llvm::dwarf::DW_TAG_subprogram) {
      uint64_t low_pc = 0, high_pc = 0;
      uint64_t section_index = 0;
      if (die.getLowAndHighPC(low_pc, high_pc, section_index)) {
        if (section_offset >= low_pc && section_offset < high_pc) {
          // First try: linkage name (mangled) which is more reliable
          if (auto linkage_attr = die.find(llvm::dwarf::DW_AT_linkage_name)) {
            if (auto linkage_name = linkage_attr->getAsCString()) {
              os << "Function: " << *linkage_name << std::endl;
              found_function = true;
              break;
            }
          }
          // Fall back to DW_AT_name if linkage name not available
          else if (auto name_attr = die.find(llvm::dwarf::DW_AT_name)) {
            if (auto name = name_attr->getAsCString()) {
              os << "Function: " << *name << std::endl;
              found_function = true;
              break;
            }
          }
        }
      }
    }
  }

  // Second try: inlined function lookup if not found
  if (!found_function) {
    llvm::DILineInfoSpecifier specifier(llvm::DILineInfoSpecifier::FileLineInfoKind::AbsoluteFilePath);
    auto inline_info = context->getInliningInfoForAddress({section_offset, (uint64_t)-1LL}, specifier);

    if (inline_info.getNumberOfFrames() > 0) {
      auto frame = inline_info.getFrame(0);
      if (!frame.FunctionName.empty()) {
        os << "Function: " << frame.FunctionName << std::endl;
        found_function = true;
      }
    }
  }

  // If still no function found, indicate this
  if (!found_function) {
    os << "Function: <unknown>" << std::endl;
  }

  // Display source location
  if (!row.FileName.empty()) {
    os << "Source location: " << row.FileName << ":" << row.Line;
    if (row.Column > 0) {
      os << ":" << row.Column;
    }
    os << std::endl;
  } else {
    os << "Source location: <unknown>" << std::endl;
  }

  os << "=== End Debug Info Dump ===" << std::endl;
}

// Add the function declaration to the header
// This will need to be added to shilos/di.hh:
// void clearDebugInfoCache();

} // namespace shilos
