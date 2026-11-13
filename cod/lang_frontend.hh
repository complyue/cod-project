//===--- lang_frontend.hh - CoD source compilation pipeline ---------===//
#pragma once

#include "lang_syntax.hh"
#include "lang_llvm.hh"

#include <fstream>
#include <iterator>
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
    std::ifstream input(entry, std::ios::binary);
    if (!input) fail({entry.string()}, "cannot read CoD source");
    const std::string source{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    if (input.bad()) fail({entry.string()}, "failed while reading CoD source");
    const auto forms = Parser(source, entry.string()).parse();
    auto checked = ScalarChecker{}.check(forms);
    std::vector<std::string> warnings;
    for (const auto &warning : checked.warnings) {
      if (warnings_.as_errors || warnings_.errors.contains(warning.kind))
        throw std::runtime_error(warning.text() + " (treated as error)");
      warnings.push_back(warning.text());
    }
    auto [text, bitcode] = LLVMEmission{}.emit(checked, emit_text, emit_bitcode, specialize_);
    return {{entry.string()}, {entry.string()}, std::move(warnings), std::move(text), std::move(bitcode), false};
  }
};

} // namespace cod::language
