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
enum class Warning { unit_conversion, multi_alias_extension, shadow, unused_capture, missing_return };
struct WarningOptions {
  bool as_errors = false;
  std::set<Warning> errors;
};

class Compiler {
public:
  explicit Compiler(WarningOptions = {}) {}

  Compilation compile(const std::filesystem::path &entry, bool emit_text, bool emit_bitcode) {
    std::ifstream input(entry, std::ios::binary);
    if (!input) fail({entry.string()}, "cannot read CoD source");
    const std::string source{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    if (input.bad()) fail({entry.string()}, "failed while reading CoD source");
    const auto forms = Parser(source, entry.string()).parse();
    if (!forms.empty())
      fail(forms.front()->at,
          "micro-dispatch implementation gap: source execution is not implemented by the current foundation; "
          "this is not a language restriction");

    auto [text, bitcode] = LLVMEmission{}.empty_program(emit_text, emit_bitcode);
    return {{entry.string()}, {entry.string()}, {}, std::move(text), std::move(bitcode), false};
  }
};

} // namespace cod::language
