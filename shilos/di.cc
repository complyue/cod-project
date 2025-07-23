#include "shilos/di.hh"

#include <cerrno>
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>

// LLVM DWARF debug info for enhanced stack traces
#include "llvm/DebugInfo/DIContext.h"
#include "llvm/DebugInfo/DWARF/DWARFCompileUnit.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/DebugInfo/DWARF/DWARFUnit.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/TargetSelect.h"

namespace shilos {

std::string getSourceLocation(void *address) {
  std::cerr << "getSourceLocation: entered with address " << address << std::endl;

  // Get the base address and path of the module containing the address
  Dl_info info;
  if (!dladdr(address, &info) || !info.dli_fname) {
    std::cerr << "getSourceLocation: dladdr failed or no filename" << std::endl;
    return "";
  }
  std::cerr << "getSourceLocation: found module " << info.dli_fname << std::endl;

  // Create a memory buffer from the binary file
  auto buffer_or = llvm::MemoryBuffer::getFile(info.dli_fname, -1, false);
  if (!buffer_or) {
    std::cerr << "getSourceLocation: failed to create memory buffer for " << info.dli_fname << std::endl;
    return "";
  }
  std::unique_ptr<llvm::MemoryBuffer> buffer = std::move(buffer_or.get());
  std::cerr << "getSourceLocation: created memory buffer" << std::endl;

  // Create an object file from the buffer
  auto object_or = llvm::object::ObjectFile::createObjectFile(buffer->getMemBufferRef());
  if (!object_or) {
    std::cerr << "getSourceLocation: failed to create object file" << std::endl;
    return "";
  }
  std::unique_ptr<llvm::object::ObjectFile> object = std::move(object_or.get());
  std::cerr << "getSourceLocation: created object file" << std::endl;

  // Create DWARF context
  auto context = llvm::DWARFContext::create(*object);
  if (!context) {
    std::cerr << "getSourceLocation: failed to create DWARF context" << std::endl;
    return "";
  }
  std::cerr << "getSourceLocation: created DWARF context" << std::endl;

  // Get the section contribution for the address
  uint64_t section_offset = (uintptr_t)(address) - (uintptr_t)(info.dli_fbase);
  std::cerr << "getSourceLocation: section_offset = 0x" << std::hex << section_offset << std::dec << std::endl;

  // Find the compile unit that contains this address
  auto cu = context->getCompileUnitForCodeAddress(section_offset);
  if (!cu) {
    std::cerr << "getSourceLocation: failed to find compile unit for address" << std::endl;
    return "";
  }
  std::cerr << "getSourceLocation: found compile unit" << std::endl;

  // Get the line table for the compile unit
  auto line_table = context->getLineTableForUnit(static_cast<llvm::DWARFUnit *>(cu));
  if (!line_table) {
    std::cerr << "getSourceLocation: failed to get line table for unit" << std::endl;
    return "";
  }
  std::cerr << "getSourceLocation: got line table" << std::endl;

  // Find the row in the line table for this address
  llvm::DILineInfo row;
  if (!line_table->getFileLineInfoForAddress({section_offset, (uint64_t)-1LL}, info.dli_fname,
                                             llvm::DILineInfoSpecifier::FileLineInfoKind::AbsoluteFilePath, row)) {
    std::cerr << "getSourceLocation: failed to get file line info for address" << std::endl;
    return "";
  }
  std::cerr << "getSourceLocation: got file line info" << std::endl;

  if (!row.FileName.empty()) {
    std::ostringstream oss;
    oss << " at " << row.FileName << ":" << row.Line;
    std::cerr << "getSourceLocation: returning location " << oss.str() << std::endl;
    return oss.str();
  }
  std::cerr << "getSourceLocation: empty filename, returning empty string" << std::endl;
  return "";
}

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

  // Create a memory buffer from the binary file
  auto buffer_or = llvm::MemoryBuffer::getFile(info.dli_fname, -1, false);
  if (!buffer_or) {
    os << "Error: failed to create memory buffer for " << info.dli_fname << std::endl;
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

  // Get the section contribution for the address
  uint64_t section_offset = (uintptr_t)(address) - (uintptr_t)(info.dli_fbase);
  os << "Section offset: 0x" << std::hex << section_offset << std::dec << std::endl;

  // Find the compile unit that contains this address
  auto cu = context->getCompileUnitForCodeAddress(section_offset);
  if (!cu) {
    os << "Error: failed to find compile unit for address" << std::endl;
    return;
  }
  os << "Found compile unit" << std::endl;

  // Get the line table for the compile unit
  auto line_table = context->getLineTableForUnit(static_cast<llvm::DWARFUnit *>(cu));
  if (!line_table) {
    os << "Error: failed to get line table for unit" << std::endl;
    return;
  }
  os << "Got line table successfully" << std::endl;

  // Find the row in the line table for this address
  llvm::DILineInfo row;
  if (!line_table->getFileLineInfoForAddress({section_offset, (uint64_t)-1LL}, info.dli_fname,
                                             llvm::DILineInfoSpecifier::FileLineInfoKind::AbsoluteFilePath, row)) {
    os << "Error: failed to get file line info for address" << std::endl;
    return;
  }
  os << "Got file line info" << std::endl;

  // Extract function information
  // Extract function information
  llvm::DILineInfoSpecifier specifier(llvm::DILineInfoSpecifier::FileLineInfoKind::AbsoluteFilePath);
  auto inline_info = context->getInliningInfoForAddress({section_offset, (uint64_t)-1LL}, specifier);

  // Display function name if available
  if (inline_info.getNumberOfFrames() > 0) {
    auto frame = inline_info.getFrame(0);
    if (!frame.FunctionName.empty()) {
      os << "Function: " << frame.FunctionName << std::endl;
    }
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

  // Display compile unit information
  os << "Compile unit:" << std::endl;
  // Extract compile unit DIE for attribute access
  auto unit_die = cu->getUnitDIE();

  // Display language information
  if (auto lang_attr = unit_die.find(llvm::dwarf::DW_AT_language)) {
    if (auto lang_value = lang_attr->getAsUnsignedConstant()) {
      llvm::StringRef lang_str_ref = llvm::dwarf::LanguageString(*lang_value);
      os << "  Language: " << lang_str_ref.str() << std::endl;
    }
  }

  // Display compilation directory if available
  if (auto comp_dir_attr = unit_die.find(llvm::dwarf::DW_AT_comp_dir)) {
    if (auto comp_dir = comp_dir_attr->getAsCString()) {
      os << "  Compilation directory: " << *comp_dir << std::endl;
    }
  }

  // Display producer information if available
  if (auto producer_attr = unit_die.find(llvm::dwarf::DW_AT_producer)) {
    if (auto producer = producer_attr->getAsCString()) {
      os << "  Producer: " << *producer << std::endl;
    }
  }

  // Display additional debug info
  os << "Additional debug information:" << std::endl;
  uint64_t low_pc = 0, high_pc = 0;
  uint64_t section_index = 0;
  if (unit_die.getLowAndHighPC(low_pc, high_pc, section_index)) {
    os << "  Address range: 0x" << std::hex << low_pc << " - 0x" << high_pc << std::dec << std::endl;
  } else {
    os << "  Address range: <unknown>" << std::endl;
  }

  os << "=== End Debug Info Dump ===" << std::endl;
}

} // namespace shilos
