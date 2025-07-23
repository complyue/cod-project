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
namespace yaml {

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

} // namespace yaml

// Initialize LLVM components required for DWARF debug info handling
// This function should be called once before any exception throwing
// to ensure proper stack trace capture with source-level information
void initialize_llvm_components() {
  static bool initialized = false;
  if (!initialized) {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllDisassemblers();
    llvm::InitializeAllTargets();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    initialized = true;
  }
}

} // namespace shilos
