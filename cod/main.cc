//===--- cod/main.cc - CoD seed compiler driver -----------------------===//

#include "clang_driver.hh"
#include "lang_frontend.hh"

#include "llvm/ADT/SmallString.h"
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/FileSystem.h"

#include <cerrno>
#include <charconv>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#ifdef _WIN32
#include <process.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

enum class LanguageAction { parse, check_declarations, check, run };

struct LanguageEmission {
  std::optional<fs::path> ir, bitcode, providers;
  bool specialize = true;
  bool bounds_checks = true;
};

// Resolves and uses the toolchain bundled with the running CoD executable.
class CodTool {
private:
  std::string executable_path_;
  fs::path toolchain_root_;

  void initializeToolchainRoot() {
#ifdef _WIN32
    llvm::SmallVector<wchar_t, 256> executable_path_buffer;
    if (const auto error = llvm::sys::windows::UTF8ToUTF16(executable_path_, executable_path_buffer)) {
      throw std::system_error(error, "failed to convert the CoD executable path to UTF-16");
    }
    toolchain_root_ = fs::path(std::wstring(executable_path_buffer.begin(), executable_path_buffer.end()))
                          .parent_path()
                          .parent_path();
#else
    toolchain_root_ = fs::path(executable_path_).parent_path().parent_path();
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

  fs::path getLanguageRuntime() const {
    constexpr auto archive_name = "libcod-runtime.a";
    if (const char *override = std::getenv("COD_RUNTIME_ARCHIVE"); override && *override) {
      const auto archive = fs::absolute(override);
      if (!fs::is_regular_file(archive))
        throw std::runtime_error("COD_RUNTIME_ARCHIVE does not name a runtime archive: " + archive.string());
      return archive;
    }
    const auto installed = toolchain_root_ / "lib" / archive_name;
    if (fs::is_regular_file(installed)) return installed;
    if (fs::is_regular_file(toolchain_root_ / "CMakeCache.txt")) {
      const auto development = toolchain_root_.parent_path() / "build-cod-runtime" / archive_name;
      if (fs::is_regular_file(development)) return development;
    }
    throw std::runtime_error("CoD runtime archive is missing; build it with make -C cod/runtime or set COD_RUNTIME_ARCHIVE");
  }

#ifdef _WIN32
  void addToolchainBinToPath() const {
    const std::wstring toolchain_bin = (toolchain_root_ / "bin").wstring();
    if (toolchain_bin.empty()) {
      return;
    }

    const wchar_t *current_path = _wgetenv(L"PATH");
    std::wstring updated_path = toolchain_bin;
    if (current_path != nullptr && *current_path != L'\0') {
      updated_path += L';';
      updated_path += current_path;
    }

    if (_wputenv_s(L"PATH", updated_path.c_str()) != 0) {
      throw std::runtime_error("failed to add the CoD toolchain directory to PATH");
    }
  }
#endif

public:
  CodTool() {
    // this is often used, get it once
    executable_path_ = llvm::sys::fs::getMainExecutable(nullptr, nullptr);
    if (executable_path_.empty()) {
      throw std::runtime_error("failed to resolve the CoD executable path");
    }
    initializeToolchainRoot();
#ifdef _WIN32
    addToolchainBinToPath();
#endif
  }

  int compileLanguage(const fs::path &entry, const fs::path &project, LanguageAction action, bool verbose,
                      const LanguageEmission &emission, const cod::language::WarningOptions &warnings,
                      std::optional<size_t> diagnostic_limit) const;
};

static void printUsage(const char *prog_name) {
  std::cout << "Usage: " << prog_name << " (--parse FILE | --check-declarations FILE | --check FILE | --run FILE) [OPTIONS]\n"
            << "\n"
            << "Options:\n"
            << "  --run FILE          Compile and run a CoD source-language entry\n"
            << "  --check FILE        Check a CoD entry and its demanded call paths\n"
            << "  --check-declarations FILE  Bind module declarations and their signatures\n"
            << "  --parse FILE        Parse a CoD entry and its reachable source modules\n"
            << "  --project PATH      Select the source project's package root\n"
            << "  --emit-llvm FILE    Save directly emitted textual LLVM IR\n"
            << "  --emit-bc FILE      Save directly emitted LLVM bitcode\n"
            << "  --emit-c-provider DIR  Generate the checked C boundary header and stubs\n"
            << "  --no-specialize    Validate CoD calls through general native implementations\n"
            << "  --no-bounds-checks Assume source builtin indices are in range\n"
            << "  --max-diagnostics N  Limit the combined source errors and warnings\n"
            << "  --warnings-as-errors  Treat all CoD language warnings as errors\n"
            << "  -Werror=unit-conversion       Reject ambiguous unit conversion paths\n"
            << "  -Werror=multi-alias-extension Reject overload declarations through multiple aliases\n"
            << "  -Werror=shadow                Reject lexical binding shadowing\n"
            << "  -Werror=unused-capture        Reject unused captures\n"
            << "  -Werror=missing-return        Reject reachable fallthrough when a return is required\n"
            << "  --no-cache          Accepted for compatibility. Source compilation uses no cache.\n"
            << "  -v, --verbose       Report source scanning and execution order\n"
            << "  -h, --help          Show this help message\n"
            << "\n"
            << "COD_RUNTIME_ARCHIVE selects the shared runtime archive for --run.\n";
}

static fs::path getRunnerPath(const fs::path &temp_dir) {
  auto runner = temp_dir / "runner";
#ifdef _WIN32
  runner += ".exe";
#endif
  return runner;
}

static std::optional<int> runExecutableAndWait(const fs::path &binary_path) {
#ifdef _WIN32
  const std::wstring executable = binary_path.wstring();
  const intptr_t exit_code =
      _wspawnl(_P_WAIT, executable.c_str(), executable.c_str(), static_cast<const wchar_t *>(nullptr));
  if (exit_code == -1) {
    std::cerr << "Error: Failed to execute binary: " << std::strerror(errno) << "\n";
    return std::nullopt;
  }
  return static_cast<int>(exit_code);
#else
  pid_t run_pid = fork();
  if (run_pid == 0) {
    // Child process - execute binary
    execl(binary_path.c_str(), binary_path.c_str(), nullptr);
    const int error = errno;
    std::cerr << "Error: Failed to execute binary: " << std::strerror(error) << "\n";
    _exit(127);
  }
  if (run_pid > 0) {
    // Parent process - wait for execution
    int run_status = 0;
    pid_t wait_result;
    do {
      wait_result = waitpid(run_pid, &run_status, 0);
    } while (wait_result == -1 && errno == EINTR);

    if (wait_result == -1) {
      std::cerr << "Error: Failed to wait for binary: " << std::strerror(errno) << "\n";
      return std::nullopt;
    }

    if (WIFEXITED(run_status)) {
      return WEXITSTATUS(run_status);
    }

    std::cerr << "Program terminated abnormally\n";
    return std::nullopt;
  }

  std::cerr << "Error: Failed to fork for execution\n";
  return std::nullopt;
#endif
}

int CodTool::compileLanguage(const fs::path &entry, const fs::path &project, LanguageAction action, bool verbose,
                            const LanguageEmission &emission, const cod::language::WarningOptions &warnings,
                            std::optional<size_t> diagnostic_limit) const {
  try {
    cod::language::Compiler compiler{warnings, emission.specialize, emission.bounds_checks};
    auto parsed = compiler.parse(fs::absolute(entry), {getLanguageLibrary(), project}, diagnostic_limit);
    auto compilation = [&] {
      if (action == LanguageAction::parse) return compiler.report_parse(std::move(parsed));
      if (action == LanguageAction::check_declarations) return compiler.report_declarations(std::move(parsed));
      return compiler.compile(std::move(parsed), emission.ir.has_value(),
          action == LanguageAction::run || emission.bitcode.has_value(), emission.providers.has_value());
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
      for (const auto &module : compilation.execution_order)
        std::cerr << "[CoD execution semantics] " << module << '\n';
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
    if (action != LanguageAction::run) return 0;
    if (compilation.native_library)
      throw std::runtime_error("a C entry library requires a linked native launcher; emit it with --check");
    const auto archive = getLanguageRuntime();

    llvm::SmallString<256> directory;
    const auto model = (fs::temp_directory_path() / "cod-language").string();
    if (const auto error = llvm::sys::fs::createUniqueDirectory(model, directory))
      throw std::system_error(error, "cannot create native compilation directory");
    const fs::path temporary(directory.str().str());
    const auto bitcode = temporary / "program.bc";
    const auto executable = getRunnerPath(temporary);
    struct TemporaryFiles {
      fs::path bitcode, executable, directory;
      ~TemporaryFiles() {
        std::error_code ignored;
        fs::remove(bitcode, ignored);
        fs::remove(executable, ignored);
        fs::remove(directory, ignored);
      }
    } cleanup{bitcode, executable, temporary};
    write(bitcode, compilation.bitcode);
    // Native optimization needs the files, not duplicate in-memory encodings.
    std::string{}.swap(compilation.llvm_ir);
    std::string{}.swap(compilation.bitcode);
    std::vector<std::string> linker_args{"-O2"};
#ifdef _WIN32
    constexpr auto clang = "clang.exe";
#else
    constexpr auto clang = "clang";
    linker_args.push_back("-fuse-ld=lld");
#endif
    const auto bundled_clang = toolchain_root_ / "bin" / clang;
    if (!cod::compiler_driver::link_bitcode({bitcode, archive}, executable, linker_args,
                                          fs::exists(bundled_clang) ? bundled_clang.string() : clang)) return 3;
    return runExecutableAndWait(executable).value_or(3);
  } catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 2;
  }
}

int main(int argc, const char **argv) {
  fs::path project;
  std::optional<fs::path> language_entry;
  LanguageEmission emission;
  LanguageAction action = LanguageAction::check;
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
    } else if (arg == "--parse" || arg == "--check-declarations" || arg == "--run" || arg == "--check") {
      if (i + 1 >= argc || language_entry) {
        std::cerr << "Error: provide exactly one entry with --parse, --check-declarations, --check or --run\n";
        return 1;
      }
      language_entry = fs::absolute(argv[++i]);
      if (arg == "--parse") action = LanguageAction::parse;
      else if (arg == "--check-declarations") action = LanguageAction::check_declarations;
      else if (arg == "--run") action = LanguageAction::run;
      else action = LanguageAction::check;
    } else if (arg == "--no-specialize") {
      emission.specialize = false;
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
    } else if (arg == "-Werror=unit-conversion") {
      warnings.errors.insert(cod::language::Warning::unit_conversion);
    } else if (arg == "-Werror=multi-alias-extension") {
      warnings.errors.insert(cod::language::Warning::multi_alias_extension);
    } else if (arg == "-Werror=shadow") {
      warnings.errors.insert(cod::language::Warning::shadow);
    } else if (arg == "-Werror=unused-capture") {
      warnings.errors.insert(cod::language::Warning::unused_capture);
    } else if (arg == "-Werror=missing-return") {
      warnings.errors.insert(cod::language::Warning::missing_return);
    } else if (arg == "--emit-llvm" || arg == "--emit-bc" || arg == "--emit-c-provider") {
      if (i + 1 >= argc) {
        std::cerr << "Error: " << arg << " requires a path\n";
        return 1;
      }
      auto &output = arg == "--emit-llvm" ? emission.ir : arg == "--emit-bc" ? emission.bitcode : emission.providers;
      output = fs::absolute(argv[++i]);
    } else if (arg == "--no-cache") {
      // CoD source compilation has always accepted this spelling without caching.
    } else if (arg == "-v" || arg == "--verbose") {
      verbose = true;
    } else {
      std::cerr << "Error: Unknown argument " << arg << "\n";
      printUsage(argv[0]);
      return 1;
    }
  }

  if (!language_entry) {
    std::cerr << "Error: provide a CoD entry with --parse, --check-declarations, --check or --run\n";
    printUsage(argv[0]);
    return 1;
  }
  if ((action == LanguageAction::parse || action == LanguageAction::check_declarations) &&
      (emission.ir || emission.bitcode || emission.providers)) {
    std::cerr << "Error: artifact emission requires --check or --run\n";
    return 1;
  }
  return CodTool{}.compileLanguage(*language_entry, project, action, verbose, emission, warnings, diagnostic_limit);
}
