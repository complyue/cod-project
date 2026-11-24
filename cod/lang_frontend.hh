//===--- lang_frontend.hh - CoD source compilation pipeline ---------===//
#pragma once

#include "lang_llvm.hh"
#include "lang_checked.hh"
#include "lang_c_provider.hh"

#include <optional>
#include <set>
#include <string>
#include <vector>

namespace cod::language {

struct Compilation {
  CompilationDiagnostics diagnostics;
  std::vector<std::string> scanned_modules;
  std::vector<std::string> execution_order;
  std::string llvm_ir;
  std::string bitcode;
  std::optional<NativeProviderFiles> providers;
  bool native_library = false;
  bool accepted = false;

  explicit Compilation(std::optional<size_t> diagnostic_limit) : diagnostics(diagnostic_limit) {}
};

class Compiler {
  WarningOptions warnings_;
  bool specialize_;
  bool bounds_checks_;
public:
  explicit Compiler(WarningOptions warnings = {}, bool specialize = true, bool bounds_checks = true)
      : warnings_(std::move(warnings)), specialize_(specialize), bounds_checks_(bounds_checks) {}

  Compilation compile(const std::filesystem::path &entry, ModuleSearchInputs inputs,
                      bool emit_text, bool emit_bitcode, bool emit_providers = false,
                      std::optional<size_t> diagnostic_limit = {}) {
    Compilation result(diagnostic_limit);
    auto &diagnostics = result.diagnostics;
    try {
      auto source = ModuleLoader(std::move(inputs), diagnostics).load(entry);
      for (const auto &module : source.modules) if (module.source)
        result.scanned_modules.push_back(module.path.string());
      if (!diagnostics.complete()) return result;
      auto checked = ScalarChecker{}.check(source, diagnostics);
      if (!checked) return result;
      for (auto &warning : checked->warnings) if (!diagnostics.append(std::move(warning))) return result;
      if (std::ranges::any_of(diagnostics.findings(), [&](const auto &finding) {
            const auto *warning = std::get_if<WarningDiagnostic>(&finding.primary);
            return warning && warnings_.promotes(warning->kind);
          })) return result;

      LLVMEmission emission;
      auto [text, bitcode] = emission.emit(*checked, emit_text, emit_bitcode, specialize_, bounds_checks_);
      for (const auto module : checked->execution_order)
        result.execution_order.push_back(source.modules[module].path.string());
      result.llvm_ir = std::move(text);
      result.bitcode = std::move(bitcode);
      if (emit_providers) result.providers = emit_native_providers(*checked, emission.provider_row_layouts(),
                                                                   source.modules.front().path.generic_string());
      result.native_library = !checked->native_entries.empty();
      result.accepted = true;
    } catch (const LocatedFailure &failure) {
      try {
        diagnostics.append(failure.diagnostic);
      } catch (const std::bad_alloc &) {
        diagnostics.exhaust_storage();
      }
    } catch (const std::bad_alloc &) {
      diagnostics.exhaust_storage();
    }
    return result;
  }
};

} // namespace cod::language
