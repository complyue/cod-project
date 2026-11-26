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

class ParsedCompilation {
  friend class Compiler;

  SourceProgram source_;
  Compilation result_;

  explicit ParsedCompilation(std::optional<size_t> diagnostic_limit) : result_(diagnostic_limit) {}
};

class Compiler {
  WarningOptions warnings_;
  bool specialize_;
  bool bounds_checks_;

  bool promotes_warning(const CompilationDiagnostics &diagnostics) const {
    return std::ranges::any_of(diagnostics.findings(), [&](const auto &finding) {
      const auto *warning = std::get_if<WarningDiagnostic>(&finding.primary);
      return warning && warnings_.promotes(warning->kind);
    });
  }

  static void retain_failure(CompilationDiagnostics &diagnostics, const LocatedFailure &failure) {
    try {
      diagnostics.append(failure.diagnostic);
    } catch (const std::bad_alloc &) {
      diagnostics.exhaust_storage();
    }
  }

public:
  explicit Compiler(WarningOptions warnings = {}, bool specialize = true, bool bounds_checks = true)
      : warnings_(std::move(warnings)), specialize_(specialize), bounds_checks_(bounds_checks) {}

  ParsedCompilation parse(const std::filesystem::path &entry, ModuleSearchInputs inputs,
                          std::optional<size_t> diagnostic_limit = {}) const {
    ParsedCompilation parsed(diagnostic_limit);
    auto &result = parsed.result_;
    auto &diagnostics = result.diagnostics;
    try {
      parsed.source_ = ModuleLoader(std::move(inputs), diagnostics).load(entry);
      for (const auto &module : parsed.source_.modules) if (module.source)
        result.scanned_modules.push_back(module.path.string());
    } catch (const LocatedFailure &failure) {
      retain_failure(diagnostics, failure);
    } catch (const std::bad_alloc &) {
      diagnostics.exhaust_storage();
    }
    return parsed;
  }

  Compilation report_parse(ParsedCompilation parsed) const {
    auto result = std::move(parsed.result_);
    const auto &diagnostics = result.diagnostics;
    result.accepted = diagnostics.complete() && !diagnostics.has_error() &&
        !promotes_warning(diagnostics) && parsed.source_.ready_for_checking();
    return result;
  }

  Compilation compile(ParsedCompilation parsed, bool emit_text, bool emit_bitcode, bool emit_providers = false) {
    auto result = std::move(parsed.result_);
    result.accepted = false;
    auto &diagnostics = result.diagnostics;
    const auto &source = parsed.source_;
    try {
      if (!diagnostics.complete()) return result;
      auto checked = ScalarChecker{}.check(source, diagnostics);
      if (!checked) return result;
      for (auto &warning : checked->warnings) if (!diagnostics.append(std::move(warning))) return result;
      if (promotes_warning(diagnostics)) return result;

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
      retain_failure(diagnostics, failure);
    } catch (const std::bad_alloc &) {
      diagnostics.exhaust_storage();
    }
    return result;
  }
};

} // namespace cod::language
