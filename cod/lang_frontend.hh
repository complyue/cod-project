//===--- lang_frontend.hh - CoD source compilation pipeline ---------===//
#pragma once

#include "lang_syntax.hh"
#include "lang_llvm.hh"

#include <set>
#include <string>
#include <vector>

namespace cod::language {

struct Compilation {
  std::vector<std::string> scanned_modules;
  std::vector<std::string> execution_order;
  std::vector<std::string> warnings;
  std::string llvm_ir;
  std::string bitcode;
  bool uses_regions = false;
};

class Compiler {
  WarningOptions warnings_;
  bool specialize_;
public:
  explicit Compiler(WarningOptions warnings = {}, bool specialize = true)
      : warnings_(std::move(warnings)), specialize_(specialize) {}

  Compilation compile(const std::filesystem::path &entry, bool emit_text, bool emit_bitcode) {
    const auto source = ModuleLoader{}.load(entry);
    auto checked = ScalarChecker{}.check(source);
    std::vector<std::string> warnings;
    for (const auto &warning : checked.warnings) {
      if (warnings_.as_errors || warnings_.errors.contains(warning.kind))
        throw std::runtime_error(warning.text() + " (treated as error)");
      warnings.push_back(warning.text());
    }
    auto [text, bitcode] = LLVMEmission{}.emit(checked, emit_text, emit_bitcode, specialize_);
    Compilation result;
    for (const auto &module : source.modules) result.scanned_modules.push_back(module.path.string());
    for (const auto module : source.execution_order) result.execution_order.push_back(source.modules[module].path.string());
    result.warnings = std::move(warnings); result.llvm_ir = std::move(text); result.bitcode = std::move(bitcode);
    return result;
  }
};

} // namespace cod::language
