#include "cod_cache.hh"

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>

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

std::vector<std::string> &&CompilerArgs(std::vector<std::string> &&compiler_args) {

#ifdef __APPLE__
  // Set deployment target to match the built libraries (eliminates version warnings)
  std::string min_ver = getMacOSDeploymentTarget();
  if (!min_ver.empty()) {
    compiler_args.push_back(std::string("-mmacosx-version-min=") + min_ver);
  }
#endif

  return std::move(compiler_args);
}

std::vector<std::string> &&LinkerArgs(std::vector<std::string> &&linker_args) {

#ifdef __APPLE__
  // Set deployment target to match the built libraries (eliminates version warnings)
  std::string min_ver = getMacOSDeploymentTarget();
  if (!min_ver.empty()) {
    linker_args.push_back(std::string("-mmacosx-version-min=") + min_ver);
  }
#endif

  return std::move(linker_args);
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
  // Create a temporary bitcode file
  auto temp_dir = fs::temp_directory_path() / "cod_bitcode";
  fs::create_directories(temp_dir);

  auto bitcode_path = temp_dir / (source_path.stem().string() + ".bc");

  // Get toolchain compiler path - use the project's build directory
  std::string compiler_path;
  fs::path project_build_dir = project_root_ / "build" / "bin" / "clang++";
  if (fs::exists(project_build_dir)) {
    compiler_path = project_build_dir.string();
  } else {
    // Try to find the project root by going up from current working directory
    fs::path current_dir = fs::current_path();
    fs::path found_clang;

    // Look for build/bin/clang++ in current directory or parent directories
    for (auto dir = current_dir; !dir.empty() && dir != dir.parent_path(); dir = dir.parent_path()) {
      auto candidate = dir / "build" / "bin" / "clang++";
      if (fs::exists(candidate)) {
        found_clang = candidate;
        break;
      }
    }

    if (!found_clang.empty()) {
      compiler_path = found_clang.string();
    } else {
      // Fallback to system clang++ if project clang++ doesn't exist
      compiler_path = "clang++";
    }
  }

  // Build clang command to generate bitcode
  std::string cmd = compiler_path + " -emit-llvm -c ";
  for (const auto &arg : compiler_args) {
    // Skip any clang++ paths in the arguments (they should not be there for bitcode generation)
    if (arg.find("clang++") == std::string::npos) {
      cmd += arg + " ";
    }
  }
  cmd += source_path.string() + " -o " + bitcode_path.string();

  // Execute the command
  int result = std::system(cmd.c_str());
  if (result == 0 && fs::exists(bitcode_path)) {
    return bitcode_path;
  }

  return std::nullopt;
}

BitcodeCompiler::BitcodeCompiler() = default;
BitcodeCompiler::~BitcodeCompiler() = default;

bool BitcodeCompiler::compile_to_bitcode(const fs::path &source_path, const fs::path &output_path,
                                         const std::vector<std::string> &compiler_args) {
  // Get toolchain compiler path - look for project build directory
  std::string compiler_path;
  // Try to find the project root by going up from current working directory
  fs::path current_dir = fs::current_path();
  fs::path project_build_dir;

  // Look for build/bin/clang++ in current directory or parent directories
  for (auto dir = current_dir; !dir.empty() && dir != dir.parent_path(); dir = dir.parent_path()) {
    auto candidate = dir / "build" / "bin" / "clang++";
    if (fs::exists(candidate)) {
      project_build_dir = candidate;
      break;
    }
  }

  if (!project_build_dir.empty()) {
    compiler_path = project_build_dir.string();
  } else {
    // Fallback to system clang++ if project clang++ doesn't exist
    compiler_path = "clang++";
  }

  // Build clang command to generate bitcode
  std::string cmd = compiler_path + " -emit-llvm -c ";
  for (const auto &arg : compiler_args) {
    // Skip any clang++ paths in the arguments (they should not be there for bitcode generation)
    if (arg.find("clang++") == std::string::npos) {
      cmd += arg + " ";
    }
  }
  cmd += source_path.string() + " -o " + output_path.string();

  // Execute the command
  int result = std::system(cmd.c_str());
  return result == 0 && fs::exists(output_path);
}

bool BitcodeCompiler::link_bitcode(const std::vector<fs::path> &bitcode_files, const fs::path &output_executable,
                                   const std::vector<std::string> &linker_args) {
  // Get toolchain compiler path - look for project build directory
  std::string compiler_path;
  // Try to find the project root by going up from current working directory
  fs::path current_dir = fs::current_path();
  fs::path project_build_dir;

  // Look for build/bin/clang++ in current directory or parent directories
  for (auto dir = current_dir; !dir.empty() && dir != dir.parent_path(); dir = dir.parent_path()) {
    auto candidate = dir / "build" / "bin" / "clang++";
    if (fs::exists(candidate)) {
      project_build_dir = candidate;
      break;
    }
  }

  if (!project_build_dir.empty()) {
    compiler_path = project_build_dir.string();
  } else {
    // Fallback to system clang++ if project clang++ doesn't exist
    compiler_path = "clang++";
  }

  // Build the linking command
  std::string command = compiler_path;

  // Add bitcode files
  for (const auto &bitcode_file : bitcode_files) {
    command += " " + bitcode_file.string();
  }

  // Add linker arguments
  for (const auto &arg : linker_args) {
    command += " " + arg;
  }

  // Add output executable
  command += " -o " + output_executable.string();

  // Execute the linking command
  int result = std::system(command.c_str());
  return result == 0;
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
    std::cerr << "[DEBUG] store: Starting cache store operation" << std::endl;
    std::cerr << "[DEBUG] store: cache_dbmr_ ptr = " << static_cast<void *>(cache_dbmr_.get()) << std::endl;

    ensure_cache_dbmr();
    if (!cache_dbmr_) {
      std::cerr << "[DEBUG] store: cache_dbmr_ is null" << std::endl;
      return false;
    }

    std::cerr << "[DEBUG] store: Getting region from cache_dbmr_" << std::endl;
    auto &region = cache_dbmr_->region();
    std::cerr << "[DEBUG] store: region ptr = " << static_cast<void *>(&region) << std::endl;

    std::cerr << "[DEBUG] store: Getting root from region" << std::endl;
    auto root = region.root();
    std::cerr << "[DEBUG] store: root ptr = " << static_cast<void *>(root.get()) << std::endl;

    if (!root) {
      std::cerr << "[DEBUG] store: root is null" << std::endl;
      return false;
    }

    std::cerr << "[DEBUG] store: Generating cache key" << std::endl;
    auto key = generate_cache_key(source_path, compiler_args, project_snapshot_id);
    std::string key_str = key.to_string();
    std::cerr << "[DEBUG] store: cache_key = " << key_str << std::endl;

    std::cerr << "[DEBUG] store: Getting cache index" << std::endl;
    auto &cache_index = root->cache_index();
    std::cerr << "[DEBUG] store: cache_index ptr = " << static_cast<void *>(&cache_index) << std::endl;

    // Get file size
    std::error_code size_ec;
    auto file_size = fs::file_size(bitcode_path, size_ec);
    if (size_ec) {
      file_size = 0;
    }
    std::cerr << "[DEBUG] store: file_size = " << file_size << std::endl;

    std::cerr << "[DEBUG] store: Allocating regional_cache_key in region" << std::endl;
    // Create the cache key in the region
    auto *cache_key_ptr = region.allocate<regional_cache_key>();
    std::cerr << "[DEBUG] store: cache_key_ptr = " << static_cast<void *>(cache_key_ptr) << std::endl;

    if (!cache_key_ptr) {
      std::cerr << "[DEBUG] store: Failed to allocate regional_cache_key" << std::endl;
      return false;
    }

    std::cerr << "[DEBUG] store: Constructing regional_cache_key" << std::endl;
    new (cache_key_ptr) regional_cache_key(region, key.toolchain_version, key.compiler_flags, key.project_snapshot_id,
                                           key.semantic_hash, key.source_mtime);

    std::cerr << "[DEBUG] store: Inserting cache entry into index" << std::endl;
    // Insert the cache entry into the index
    auto [entry_ptr, inserted] =
        cache_index.insert(region, key_str, cache_key_ptr, bitcode_path, std::chrono::system_clock::now(), file_size);
    std::cerr << "[DEBUG] store: inserted = " << inserted << std::endl;

    if (inserted) {
      std::cerr << "[DEBUG] store: Updating stats" << std::endl;
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

    std::cerr << "[DEBUG] store: Cache store completed successfully" << std::endl;
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
  struct stat file_stat;
  if (stat(source_path.c_str(), &file_stat) == 0) {
    key.source_mtime = std::chrono::system_clock::from_time_t(file_stat.st_mtime);
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

  // Convert file_time_type to system_clock time_point for comparison
  auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
      source_mtime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());

  return sctp > entry.created_at();
}

} // namespace cod::cache
