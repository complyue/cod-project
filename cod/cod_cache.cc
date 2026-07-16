#include "cod_cache.hh"

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/SHA256.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string_view>
#include <system_error>

#ifdef __APPLE__
#include <sys/sysctl.h>
#include <sys/utsname.h>

// Get the deployment target to use - prioritize environment, then detect system version at runtime
static std::string getMacOSDeploymentTarget() {
  if (const char *env = std::getenv("MACOSX_DEPLOYMENT_TARGET")) {
    if (*env)
      return std::string(env);
  }

  // Use sysctl to get the actual running macOS version
  char version_str[256];
  size_t size = sizeof(version_str);
  if (sysctlbyname("kern.osproductversion", version_str, &size, nullptr, 0) == 0) {
    std::string full_version(version_str);
    // Extract major.minor from version string (e.g., "14.1.2" -> "14.1")
    size_t first_dot = full_version.find('.');
    if (first_dot != std::string::npos) {
      size_t second_dot = full_version.find('.', first_dot + 1);
      if (second_dot != std::string::npos) {
        return full_version.substr(0, second_dot);
      }
      return full_version; // Only major.minor format
    }
  }

  // Fallback: try uname as secondary option
  struct utsname sys_info;
  if (uname(&sys_info) == 0) {
    std::string release(sys_info.release);
    // Darwin kernel version to macOS version mapping (approximate)
    // Darwin 23.x.x -> macOS 14.x, Darwin 22.x.x -> macOS 13.x, etc.
    int darwin_major = std::stoi(release.substr(0, release.find('.')));
    if (darwin_major >= 20) {
      int macos_major = darwin_major - 9; // Darwin 20 -> macOS 11
      return std::to_string(macos_major) + ".0";
    }
  }

  // Fallback to a reasonable default if all methods fail
  return "12.0";
}
#endif

namespace cod {

namespace {

namespace fs = std::filesystem;

std::chrono::system_clock::time_point to_system_clock_time(std::filesystem::file_time_type file_time) {
#ifdef _WIN32
  const auto system_time = std::chrono::utc_clock::to_sys(std::filesystem::file_time_type::clock::to_utc(file_time));
#else
  const auto system_time = std::filesystem::file_time_type::clock::to_sys(file_time);
#endif
  return std::chrono::time_point_cast<std::chrono::system_clock::duration>(system_time);
}

fs::path clangxx_candidate_for_root(const fs::path &root) {
  const char *test_toolchain = std::getenv("COD_TEST_TOOLCHAIN");
  const char *toolchain_dir = (test_toolchain && std::string_view(test_toolchain) == "built") ? "built" : "build";
#ifdef _WIN32
  auto exe_candidate = root / toolchain_dir / "bin" / "clang++.exe";
  if (fs::exists(exe_candidate)) {
    return exe_candidate;
  }
#endif
  return root / toolchain_dir / "bin" / "clang++";
}

std::string fallback_clangxx() {
#ifdef _WIN32
  return "clang++.exe";
#else
  return "clang++";
#endif
}

std::string find_clangxx(const fs::path &project_root = {}) {
  const fs::path current_executable = llvm::sys::fs::getMainExecutable(nullptr, nullptr);
  if (!current_executable.empty()) {
    auto candidate = current_executable.parent_path() / fallback_clangxx();
    if (fs::exists(candidate)) {
      return candidate.string();
    }
  }

  if (!project_root.empty()) {
    auto candidate = clangxx_candidate_for_root(project_root);
    if (fs::exists(candidate)) {
      return candidate.string();
    }
  }

  fs::path current_dir = fs::current_path();
  for (auto dir = current_dir; !dir.empty() && dir != dir.parent_path(); dir = dir.parent_path()) {
    auto candidate = clangxx_candidate_for_root(dir);
    if (fs::exists(candidate)) {
      return candidate.string();
    }
  }

  return fallback_clangxx();
}

bool execute_and_wait(const std::string &program, std::vector<std::string> arguments) {
  auto executable = llvm::sys::findProgramByName(program);
  if (!executable) {
    std::cerr << "Failed to find compiler '" << program << "': " << executable.getError().message() << '\n';
    return false;
  }

  arguments.insert(arguments.begin(), *executable);
  std::vector<llvm::StringRef> argument_refs;
  argument_refs.reserve(arguments.size());
  for (const auto &argument : arguments) {
    argument_refs.emplace_back(argument);
  }

  std::string error_message;
  const int result = llvm::sys::ExecuteAndWait(*executable, argument_refs, std::nullopt, {}, 0, 0, &error_message);
  if (!error_message.empty()) {
    std::cerr << "Failed to execute compiler '" << *executable << "': " << error_message << '\n';
  }
  return result == 0;
}

std::optional<std::string> bitcode_output_key(const fs::path &project_root, const fs::path &source_path,
                                              const std::string &compiler_path,
                                              const std::vector<std::string> &compiler_args) {
  std::ifstream source(source_path, std::ios::binary);
  if (!source) {
    return std::nullopt;
  }
  const std::string source_contents((std::istreambuf_iterator<char>(source)), std::istreambuf_iterator<char>());

  llvm::SHA256 hash;
  auto add_field = [&](std::string_view value) {
    const std::string size = std::to_string(value.size());
    hash.update(size);
    hash.update(":");
    hash.update(llvm::StringRef(value.data(), value.size()));
  };

  add_field(fs::absolute(project_root).lexically_normal().generic_string());
  add_field(fs::absolute(source_path).lexically_normal().generic_string());
  add_field(compiler_path);
  for (const auto &argument : compiler_args) {
    add_field(argument);
  }
  add_field(source_contents);

  return llvm::toHex(hash.final(), true);
}

} // namespace

std::vector<std::string> CompilerArgs(std::vector<std::string> compiler_args) {

#ifdef __APPLE__
  // Set deployment target to match the built libraries (eliminates version warnings)
  std::string min_ver = getMacOSDeploymentTarget();
  if (!min_ver.empty()) {
    compiler_args.push_back(std::string("-mmacosx-version-min=") + min_ver);
  }
#endif

#ifdef _WIN32
  const bool already_defined = std::any_of(compiler_args.begin(), compiler_args.end(), [](const std::string &arg) {
    return arg == "-D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH" || arg == "/D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH";
  });
  if (!already_defined) {
    compiler_args.push_back("-D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH");
  }
#endif

  return compiler_args;
}

std::vector<std::string> LinkerArgs(std::vector<std::string> linker_args) {

#ifdef __APPLE__
  // Set deployment target to match the built libraries (eliminates version warnings)
  std::string min_ver = getMacOSDeploymentTarget();
  if (!min_ver.empty()) {
    linker_args.push_back(std::string("-mmacosx-version-min=") + min_ver);
  }
#endif

  return linker_args;
}

} // namespace cod

// Temporary stub implementations for SemanticHasher
namespace cod::cache {

SemanticHasher::SemanticHasher() = default;
SemanticHasher::~SemanticHasher() = default;

std::string SemanticHasher::hash_file(const fs::path &source_path, const std::vector<std::string> &compiler_args) {
  // Read and normalize the source file content for semantic hashing
  std::ifstream file(source_path);
  if (!file.is_open()) {
    return "";
  }

  std::string content;
  std::string line;
  while (std::getline(file, line)) {
    // Remove leading/trailing whitespace and normalize
    size_t start = line.find_first_not_of(" \t");
    if (start == std::string::npos) {
      continue; // Skip empty lines
    }
    size_t end = line.find_last_not_of(" \t");
    std::string normalized_line = line.substr(start, end - start + 1);

    // Skip comment-only lines for semantic equivalence
    if (normalized_line.empty() || normalized_line.substr(0, 2) == "//") {
      continue;
    }

    content += normalized_line + "\n";
  }

  // Add compiler args to the hash (they affect semantics)
  for (const auto &arg : compiler_args) {
    content += arg;
  }

  return std::to_string(std::hash<std::string>{}(content));
}

std::string SemanticHasher::hash_ast(clang::ASTContext &context, clang::TranslationUnitDecl *tu) {
  // Stub implementation
  return "stub_ast_hash";
}

std::string CacheKey::to_string() const {
  // Combine all key components into a single string
  std::string result = semantic_hash + "|" + toolchain_version + "|" + project_snapshot_id + "|";
  for (const auto &flag : compiler_flags) {
    result += flag + ";";
  }
  result += std::to_string(std::chrono::duration_cast<std::chrono::seconds>(source_mtime.time_since_epoch()).count());
  return result;
}

bool CacheKey::operator==(const CacheKey &other) const {
  return semantic_hash == other.semantic_hash && toolchain_version == other.toolchain_version &&
         project_snapshot_id == other.project_snapshot_id && compiler_flags == other.compiler_flags &&
         source_mtime == other.source_mtime;
}

// regional_cache_key implementation
std::string regional_cache_key::to_string() const {
  std::ostringstream oss;
  oss << std::string_view(toolchain_version_) << "|";
  for (const auto &flag : compiler_flags_) {
    oss << std::string_view(flag) << ";";
  }
  oss << "|" << std::string_view(project_snapshot_id_) << "|" << std::string_view(semantic_hash_) << "|";
  oss << std::chrono::duration_cast<std::chrono::seconds>(source_mtime_.time_since_epoch()).count();
  return oss.str();
}

bool regional_cache_key::equals(const regional_cache_key &other) const {
  if (toolchain_version_ != other.toolchain_version_ || project_snapshot_id_ != other.project_snapshot_id_ ||
      semantic_hash_ != other.semantic_hash_ || source_mtime_ != other.source_mtime_) {
    return false;
  }

  if (compiler_flags_.size() != other.compiler_flags_.size()) {
    return false;
  }

  for (size_t i = 0; i < compiler_flags_.size(); ++i) {
    if (compiler_flags_[i] != other.compiler_flags_[i]) {
      return false;
    }
  }

  return true;
}

// regional_cache_entry implementation
bool regional_cache_entry::is_valid() const {
  if (!key_ || bitcode_path_.empty()) {
    return false;
  }

  // Check if bitcode file still exists
  fs::path path{std::string{bitcode_path_}};
  if (!fs::exists(path)) {
    return false;
  }

  // Check if file size matches
  std::error_code ec;
  auto actual_size = fs::file_size(path, ec);
  if (ec || actual_size != file_size_) {
    return false;
  }

  return true;
}

// Basic implementation for generate_bitcode
std::optional<fs::path> BuildCache::generate_bitcode(const fs::path &source_path,
                                                     const std::vector<std::string> &compiler_args) {
  auto temp_dir = fs::temp_directory_path() / "cod_bitcode";
  fs::create_directories(temp_dir);

  // Get toolchain compiler path - use the project's build directory
  std::string compiler_path = find_clangxx(project_root_);
  auto effective_compiler_args = CompilerArgs(compiler_args);

  const auto output_key = bitcode_output_key(project_root_, source_path, compiler_path, effective_compiler_args);
  if (!output_key) {
    return std::nullopt;
  }
  const fs::path bitcode_path = temp_dir / (*output_key + ".bc");

  llvm::SmallString<256> temporary_path_text;
  const fs::path temporary_model = temp_dir / (*output_key + "-%%%%%%%%%%%%%%%%.bc");
  if (const std::error_code error = llvm::sys::fs::createUniqueFile(temporary_model.string(), temporary_path_text)) {
    std::cerr << "Failed to reserve temporary bitcode output: " << error.message() << '\n';
    return std::nullopt;
  }
  const fs::path temporary_path(temporary_path_text.str().str());

  // Build clang arguments to generate bitcode
  std::vector<std::string> arguments = {"-emit-llvm", "-c"};
  arguments.insert(arguments.end(), effective_compiler_args.begin(), effective_compiler_args.end());
  arguments.push_back(source_path.string());
  arguments.push_back("-o");
  arguments.push_back(temporary_path.string());

  if (!execute_and_wait(compiler_path, std::move(arguments))) {
    std::error_code remove_error;
    fs::remove(temporary_path, remove_error);
    return std::nullopt;
  }

  std::error_code size_error;
  const auto temporary_size = fs::file_size(temporary_path, size_error);
  const auto current_output_key =
      bitcode_output_key(project_root_, source_path, compiler_path, effective_compiler_args);
  if (size_error || temporary_size == 0 || current_output_key != output_key) {
    std::error_code remove_error;
    fs::remove(temporary_path, remove_error);
    return std::nullopt;
  }

  if (const std::error_code error = llvm::sys::fs::rename(temporary_path.string(), bitcode_path.string())) {
    if (fs::exists(bitcode_path)) {
      std::error_code remove_error;
      fs::remove(temporary_path, remove_error);
      return bitcode_path;
    }
    std::cerr << "Failed to publish bitcode output: " << error.message() << '\n';
    std::error_code remove_error;
    fs::remove(temporary_path, remove_error);
    return std::nullopt;
  }

  return bitcode_path;
}

BitcodeCompiler::BitcodeCompiler() = default;
BitcodeCompiler::~BitcodeCompiler() = default;

bool BitcodeCompiler::compile_to_bitcode(const fs::path &source_path, const fs::path &output_path,
                                         const std::vector<std::string> &compiler_args) {
  // Get toolchain compiler path - look for project build directory
  std::string compiler_path = find_clangxx();

  // Build clang arguments to generate bitcode
  auto effective_compiler_args = CompilerArgs(compiler_args);
  std::vector<std::string> arguments = {"-emit-llvm", "-c"};
  arguments.insert(arguments.end(), effective_compiler_args.begin(), effective_compiler_args.end());
  arguments.push_back(source_path.string());
  arguments.push_back("-o");
  arguments.push_back(output_path.string());

  // Execute the command
  return execute_and_wait(compiler_path, std::move(arguments)) && fs::exists(output_path);
}

bool BitcodeCompiler::link_bitcode(const std::vector<fs::path> &bitcode_files, const fs::path &output_executable,
                                   const std::vector<std::string> &linker_args) {
  // Get toolchain compiler path - look for project build directory
  std::string compiler_path = find_clangxx();

  std::vector<std::string> arguments;

  // Add bitcode files
  for (const auto &bitcode_file : bitcode_files) {
    arguments.push_back(bitcode_file.string());
  }

  // Add linker arguments
  arguments.insert(arguments.end(), linker_args.begin(), linker_args.end());

  // Add output executable
  arguments.push_back("-o");
  arguments.push_back(output_executable.string());

  // Execute the linking command
  return execute_and_wait(compiler_path, std::move(arguments));
}

} // namespace cod::cache

namespace cod::cache {

namespace fs = std::filesystem;

BuildCache::BuildCache(const fs::path &project_root, bool verbose)
    : project_root_(project_root), cache_dbmr_path_(project_root / ".cod" / "cache.dbmr"), verbose_(verbose) {
  ensure_cache_dbmr();
}

BuildCache::~BuildCache() {
  // DBMR destructor handles cleanup automatically
}

void BuildCache::ensure_cache_dbmr() {
  if (cache_dbmr_) {
    return;
  }

  try {
    // Try to open existing DBMR file
    if (fs::exists(cache_dbmr_path_)) {
      if (verbose_) {
        std::cout << "[DEBUG] Opening existing cache DBMR: " << cache_dbmr_path_ << std::endl;
      }
      cache_dbmr_.reset(new DBMR<BuildCacheRoot>(cache_dbmr_path_.string(), 0));
    } else {
      // Create new DBMR file
      if (verbose_) {
        std::cout << "[DEBUG] Creating new cache DBMR: " << cache_dbmr_path_ << std::endl;
      }
      fs::create_directories(cache_dbmr_path_.parent_path());
      cache_dbmr_.reset(new DBMR<BuildCacheRoot>(
          DBMR<BuildCacheRoot>::create(cache_dbmr_path_.string(), 1024 * 1024, project_root_.string())));
    }

    // Verify the DBMR was created successfully
    if (!cache_dbmr_) {
      if (verbose_) {
        std::cout << "[ERROR] Failed to create valid cache DBMR" << std::endl;
      }
      cache_dbmr_.reset();
      return;
    }

    auto &region = cache_dbmr_->region();
    auto root = region.root();

    if (!root.get()) {
      if (verbose_) {
        std::cout << "[ERROR] Failed to create valid cache DBMR" << std::endl;
      }
      cache_dbmr_.reset();
      return;
    }

    if (verbose_) {
      std::cout << "[DEBUG] Cache DBMR initialized successfully" << std::endl;
    }
  } catch (const std::exception &e) {
    if (verbose_) {
      std::cout << "[ERROR] Exception opening cache DBMR: " << e.what() << std::endl;
      std::cout << "[DEBUG] Attempting to recreate cache DBMR" << std::endl;
    }
    // If opening fails, try to create a new one
    try {
      if (fs::exists(cache_dbmr_path_)) {
        fs::remove(cache_dbmr_path_);
      }
      fs::create_directories(cache_dbmr_path_.parent_path());
      cache_dbmr_.reset(new DBMR<BuildCacheRoot>(
          DBMR<BuildCacheRoot>::create(cache_dbmr_path_.string(), 1024 * 1024, project_root_.string())));

      if (verbose_) {
        std::cout << "[DEBUG] Successfully recreated cache DBMR" << std::endl;
      }
    } catch (const std::exception &e2) {
      if (verbose_) {
        std::cout << "[ERROR] Failed to recreate cache DBMR: " << e2.what() << std::endl;
      }
      cache_dbmr_.reset();
    }
  }
}

std::optional<fs::path> BuildCache::lookup(const fs::path &source_path, const std::vector<std::string> &compiler_args,
                                           const std::string &project_snapshot_id) {
  if (verbose_) {
    std::cout << "[DEBUG] Cache lookup starting for: " << source_path << std::endl;
  }

  ensure_cache_dbmr();
  if (!cache_dbmr_) {
    if (verbose_) {
      std::cout << "[DEBUG] cache_dbmr_ is null" << std::endl;
    }
    return std::nullopt;
  }

  // Generate cache key
  auto key = generate_cache_key(source_path, compiler_args, project_snapshot_id);
  std::string key_str = key.to_string();

  if (verbose_) {
    std::cout << "[DEBUG] Generated key: " << key_str << std::endl;
  }

  try {
    auto &region = cache_dbmr_->region();
    auto root = region.root();
    if (!root) {
      if (verbose_) {
        std::cout << "[DEBUG] Cache root is null" << std::endl;
      }
      return std::nullopt;
    }

    auto &cache_index = root->cache_index();

    if (verbose_) {
      std::cout << "[DEBUG] Cache index size: " << cache_index.size() << std::endl;
    }

    auto it = cache_index.find(key_str);

    if (it != cache_index.end()) {
      const auto &[cache_key, entry] = *it;

      if (verbose_) {
        std::cout << "[DEBUG] Entry found, checking validity" << std::endl;
      }

      // Check if entry is still valid
      if (entry.is_valid() && !is_timestamp_newer(source_path, entry)) {
        // Cache hit - update stats
        auto &stats = region.root()->stats();
        stats.hits++;
        if (verbose_) {
          std::cout << "[DEBUG] Cache hit! Returning: " << entry.bitcode_path() << std::endl;
        }
        return fs::path{std::string{entry.bitcode_path()}};
      } else {
        // Entry is invalid or outdated, remove it
        if (verbose_) {
          std::cout << "[DEBUG] Entry invalid or outdated, removing" << std::endl;
        }
        cache_index.erase(it);
      }
    }

    // Cache miss - update stats
    auto &stats = region.root()->stats();
    stats.misses++;
    if (verbose_) {
      std::cout << "[DEBUG] Cache miss" << std::endl;
    }
    return std::nullopt;

  } catch (const std::exception &e) {
    if (verbose_) {
      std::cout << "[DEBUG] Exception in cache lookup: " << e.what() << std::endl;
    }
    return std::nullopt;
  } catch (...) {
    if (verbose_) {
      std::cout << "[DEBUG] Unknown exception in cache lookup" << std::endl;
    }
    return std::nullopt;
  }
}

bool BuildCache::store(const fs::path &source_path, const fs::path &bitcode_path,
                       const std::vector<std::string> &compiler_args, const std::string &project_snapshot_id) {
  try {

    ensure_cache_dbmr();
    if (!cache_dbmr_) {
      return false;
    }

    auto &region = cache_dbmr_->region();

    auto root = region.root();

    if (!root) {
      return false;
    }

    auto key = generate_cache_key(source_path, compiler_args, project_snapshot_id);
    std::string key_str = key.to_string();

    auto &cache_index = root->cache_index();

    // Get file size
    std::error_code size_ec;
    auto file_size = fs::file_size(bitcode_path, size_ec);
    if (size_ec) {
      file_size = 0;
    }

    // Create the cache key in the region
    auto *cache_key_ptr = region.allocate<regional_cache_key>();

    if (!cache_key_ptr) {
      return false;
    }

    new (cache_key_ptr) regional_cache_key(region, key.toolchain_version, key.compiler_flags, key.project_snapshot_id,
                                           key.semantic_hash, key.source_mtime);

    // Insert the cache entry into the index
    auto [entry_ptr, inserted] =
        cache_index.insert(region, key_str, cache_key_ptr, bitcode_path, std::chrono::system_clock::now(), file_size);

    if (inserted) {
      // Update stats only if insertion was successful
      auto &stats = region.root()->stats();
      stats.total_entries++;
      stats.total_size_bytes += file_size;

      if (verbose_) {
        std::cout << "[DEBUG] Stored cache entry for: " << source_path << std::endl;
      }
    } else {
      if (verbose_) {
        std::cout << "[DEBUG] Cache entry already exists for: " << source_path << std::endl;
      }
    }

    return true;
  } catch (const std::exception &e) {
    std::cerr << "[ERROR] store exception: " << e.what() << std::endl;
    return false;
  } catch (...) {
    std::cerr << "[ERROR] store: Unknown exception occurred" << std::endl;
    return false;
  }
}

BuildCache::Stats BuildCache::get_stats() const {
  if (!cache_dbmr_) {
    return {0, 0, 0, 0};
  }
  const auto &regional_stats = cache_dbmr_->region().root()->stats();
  return {regional_stats.total_entries, regional_stats.total_size_bytes, regional_stats.hits, regional_stats.misses};
}

CacheKey BuildCache::generate_cache_key(const fs::path &source_path, const std::vector<std::string> &compiler_args,
                                        const std::string &project_snapshot_id) {
  CacheKey key;
  key.semantic_hash = hasher_.hash_file(source_path, compiler_args);
  key.toolchain_version = "llvm-builtin"; // Fixed toolchain identifier since cod ships with LLVM
  key.project_snapshot_id = project_snapshot_id;
  key.compiler_flags = compiler_args;

  // Get file modification time
  std::error_code ec;
  auto source_mtime = fs::last_write_time(source_path, ec);
  if (!ec) {
    key.source_mtime = to_system_clock_time(source_mtime);
  }

  return key;
}

void BuildCache::cleanup_expired(std::chrono::hours max_age) {
  // Implementation would iterate through cache entries and remove expired ones
  // For now, this is a placeholder
}

// Helper function to check if source file is newer than cached entry
bool BuildCache::is_timestamp_newer(const fs::path &source_path, const regional_cache_entry &entry) const {
  std::error_code ec;
  auto source_mtime = fs::last_write_time(source_path, ec);
  if (ec) {
    return true; // If we can't get the timestamp, assume it's newer
  }

  return to_system_clock_time(source_mtime) > entry.created_at();
}

} // namespace cod::cache
