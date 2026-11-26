//===--- cod/main.cc - CoD seed compiler driver -----------------------===//

#include "lang_frontend.hh"

#ifdef _WIN32
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/ConvertUTF.h"
#endif
#include "llvm/Support/FileSystem.h"

#include <charconv>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

#include <utility>

namespace fs = std::filesystem;

enum class LanguageAction { parse, check_declarations, translate };

struct LanguageEmission {
  std::optional<fs::path> ir, bitcode, providers;
  bool bounds_checks = true;
};

// Resolves and uses the toolchain bundled with the running CoD executable.
class CodTool {
private:
  fs::path toolchain_root_;

  void initializeToolchainRoot(const std::string &executable_path) {
#ifdef _WIN32
    llvm::SmallVector<wchar_t, 256> executable_pathbuffer;
    if (const auto error = llvm::sys::windows::UTF8ToUTF16(executable_path, executable_pathbuffer)) {
      throw std::system_error(error, "failed to convert the CoD executable path to UTF-16");
    }
    toolchain_root_ = fs::path(std::wstring(executable_pathbuffer.begin(), executable_pathbuffer.end()))
                          .parent_path()
                          .parent_path();
#else
    toolchain_root_ = fs::path(executable_path).parent_path().parent_path();
#endif
  }

  fs::path getLanguageLibrary() const {
    const auto installed = toolchain_root_ / "lib" / "cod";
    if (fs::is_directory(installed)) return installed;
    const auto development = toolchain_root_.parent_path() / "cod" / "library";
    if (fs::is_regular_file(toolchain_root_ / "CMakeCache.txt") && fs::is_directory(development))
      return development;
    return installed;
  }

public:
  CodTool() {
    const auto executable_path = llvm::sys::fs::getMainExecutable(nullptr, nullptr);
    if (executable_path.empty()) {
      throw std::runtime_error("failed to resolve the CoD executable path");
    }
    initializeToolchainRoot(executable_path);
  }

  int compileLanguage(const fs::path &entry, const fs::path &project, LanguageAction action, bool verbose,
                      const LanguageEmission &emission, const cod::language::WarningOptions &warnings,
                      std::optional<size_t> diagnostic_limit) const;
};

static void printUsage(const char *prog_name) {
  std::cout << "Usage: " << prog_name << " (--parse FILE | --check-declarations FILE | --translate FILE) [OPTIONS]\n"
            << "\n"
            << "Options:\n"
            << "  --check-declarations FILE  Bind module declarations and their signatures\n"
            << "  --translate FILE    Translate a trusted CoD C-entry closure\n"
            << "  --parse FILE        Parse a CoD entry and its reachable source modules\n"
            << "  --project PATH      Select the source project's package root\n"
            << "  --emit-llvm FILE    Save directly emitted textual LLVM IR\n"
            << "  --emit-bc FILE      Save directly emitted LLVM bitcode\n"
            << "  --emit-c-provider DIR  Generate the checked C boundary header and stubs\n"
            << "  --no-bounds-checks Assume source builtin indices are in range\n"
            << "  --max-diagnostics N  Limit the combined source errors and warnings\n"
            << "  --warnings-as-errors  Treat all CoD language warnings as errors\n"
            << "  -Werror=multi-alias-extension Reject overload declarations through multiple aliases\n"
            << "  -v, --verbose       Report source scanning\n"
            << "  -h, --help          Show this help message\n";
}

int CodTool::compileLanguage(const fs::path &entry, const fs::path &project, LanguageAction action, bool verbose,
                            const LanguageEmission &emission, const cod::language::WarningOptions &warnings,
                            std::optional<size_t> diagnostic_limit) const {
  try {
    cod::language::Compiler compiler{warnings, emission.bounds_checks};
    auto parsed = compiler.parse(fs::absolute(entry), {getLanguageLibrary(), project}, diagnostic_limit);
    auto compilation = [&] {
      if (action == LanguageAction::parse) return compiler.report_parse(std::move(parsed));
      if (action == LanguageAction::check_declarations) return compiler.report_declarations(std::move(parsed));
      return compiler.compile(std::move(parsed), emission.ir.has_value(),
          emission.bitcode.has_value(), emission.providers.has_value());
    }();
    for (const auto &finding : compilation.diagnostics.findings()) {
      if (const auto *warning = std::get_if<cod::language::WarningDiagnostic>(&finding.primary)) {
        std::cerr << warning->text();
        if (warnings.promotes(warning->kind)) std::cerr << " (treated as error)";
      } else {
        const auto &error = std::get<cod::language::SyntaxDiagnostic>(finding.primary);
        std::cerr << cod::language::location_text(error.at) << ": " << error.message;
      }
      std::cerr << '\n';
      for (const auto &site : finding.import_sites)
        std::cerr << cod::language::location_text(site) << ": note: imported here\n";
    }
    using Coverage = cod::language::CompilationDiagnostics::Coverage;
    if (compilation.diagnostics.coverage() == Coverage::capped)
      std::cerr << "CoD diagnostic limit reached. Coverage is incomplete.\n";
    if (compilation.diagnostics.coverage() == Coverage::storage_exhausted)
      std::cerr << "CoD diagnostic coverage is incomplete because storage was exhausted.\n";
    if (verbose) {
      for (const auto &module : compilation.scanned_modules)
        std::cerr << "[CoD scan] " << module << '\n';
    }
    if (!compilation.accepted) return 2;
    const auto write = [](const fs::path &path, const std::string &contents) {
      std::ofstream output(path, std::ios::binary);
      output.write(contents.data(), static_cast<std::streamsize>(contents.size()));
      output.close();
      if (!output) throw std::runtime_error("cannot write generated artifact: " + path.string());
    };
    if (emission.ir) write(*emission.ir, compilation.llvm_ir);
    if (emission.bitcode) write(*emission.bitcode, compilation.bitcode);
    if (emission.providers) {
      using Files = cod::language::NativeProviderFiles;
      fs::create_directories(*emission.providers);
      write(*emission.providers / Files::header_name, compilation.providers->header);
      write(*emission.providers / Files::stubs_name, compilation.providers->stubs);
    }
    return 0;
  } catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 2;
  }
}

int main(int argc, const char **argv) {
  fs::path project;
  std::optional<fs::path> language_entry;
  LanguageEmission emission;
  LanguageAction action = LanguageAction::translate;
  bool verbose = false;
  cod::language::WarningOptions warnings;
  std::optional<size_t> diagnostic_limit;

  for (int i = 1; i < argc; ++i) {
    const std::string_view arg = argv[i];
    if (arg == "-h" || arg == "--help") {
      printUsage(argv[0]);
      return 0;
    } else if (arg == "--project") {
      if (i + 1 >= argc) {
        std::cerr << "Error: " << arg << " requires a path argument\n";
        return 1;
      }
      project = fs::absolute(argv[++i]);
    } else if (arg == "--parse" || arg == "--check-declarations" || arg == "--translate") {
      if (i + 1 >= argc || language_entry) {
        std::cerr << "Error: provide exactly one entry with --parse, --check-declarations or --translate\n";
        return 1;
      }
      language_entry = fs::absolute(argv[++i]);
      if (arg == "--parse") action = LanguageAction::parse;
      else if (arg == "--check-declarations") action = LanguageAction::check_declarations;
      else action = LanguageAction::translate;
    } else if (arg == "--no-bounds-checks") {
      emission.bounds_checks = false;
    } else if (arg == "--max-diagnostics") {
      if (i + 1 >= argc || diagnostic_limit) {
        std::cerr << "Error: provide one positive count for --max-diagnostics\n";
        return 1;
      }
      const std::string_view count = argv[++i];
      size_t limit = 0;
      const auto end = count.data() + count.size();
      const auto parsed = std::from_chars(count.data(), end, limit);
      if (parsed.ec != std::errc{} || parsed.ptr != end || !limit) {
        std::cerr << "Error: --max-diagnostics requires a positive decimal count\n";
        return 1;
      }
      diagnostic_limit = limit;
    } else if (arg == "--warnings-as-errors") {
      warnings.as_errors = true;
    } else if (arg == "-Werror=multi-alias-extension") {
      warnings.errors.insert(cod::language::Warning::multi_alias_extension);
    } else if (arg == "--emit-llvm" || arg == "--emit-bc" || arg == "--emit-c-provider") {
      if (i + 1 >= argc) {
        std::cerr << "Error: " << arg << " requires a path\n";
        return 1;
      }
      auto &output = arg == "--emit-llvm" ? emission.ir : arg == "--emit-bc" ? emission.bitcode : emission.providers;
      output = fs::absolute(argv[++i]);
    } else if (arg == "-v" || arg == "--verbose") {
      verbose = true;
    } else {
      std::cerr << "Error: Unknown argument " << arg << "\n";
      printUsage(argv[0]);
      return 1;
    }
  }

  if (!language_entry) {
    std::cerr << "Error: provide a CoD entry with --parse, --check-declarations or --translate\n";
    printUsage(argv[0]);
    return 1;
  }
  if ((action == LanguageAction::parse || action == LanguageAction::check_declarations) &&
      (emission.ir || emission.bitcode || emission.providers)) {
    std::cerr << "Error: artifact emission requires --translate\n";
    return 1;
  }
  return CodTool{}.compileLanguage(*language_entry, project, action, verbose, emission, warnings, diagnostic_limit);
}
