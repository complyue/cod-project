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

// Temporary stub implementations for SemanticHasher
namespace cod::cache {

SemanticHasher::SemanticHasher() = default;
SemanticHasher::~SemanticHasher() = default;

std::string SemanticHasher::hash_file(const fs::path &source_path, const std::vector<std::string> &compiler_args) {
  // Stub implementation - just hash the file path and args
  std::string combined = source_path.string();
  for (const auto &arg : compiler_args) {
    combined += arg;
  }
  return std::to_string(std::hash<std::string>{}(combined));
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

  // Get toolchain compiler path
  std::string compiler_path;
  std::string exe_path = llvm::sys::fs::getMainExecutable("cod", nullptr);
  if (!exe_path.empty()) {
    llvm::SmallString<256> clang_path(exe_path);
    llvm::sys::path::remove_filename(clang_path); // Get parent directory
    llvm::sys::path::append(clang_path, "clang++");
    compiler_path = clang_path.str().str();
  } else {
    // Fallback to system clang++ if we can't determine executable path
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
  // Get toolchain compiler path
  std::string compiler_path;
  std::string exe_path = llvm::sys::fs::getMainExecutable("cod", nullptr);
  if (!exe_path.empty()) {
    llvm::SmallString<256> clang_path(exe_path);
    llvm::sys::path::remove_filename(clang_path); // Get parent directory
    llvm::sys::path::append(clang_path, "clang++");
    compiler_path = clang_path.str().str();
  } else {
    // Fallback to system clang++ if we can't determine executable path
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
  // Get toolchain compiler path
  std::string compiler_path;
  // Use nullptr - getMainExecutable can work without a specific address on many systems
  std::string exe_path = llvm::sys::fs::getMainExecutable("cod", nullptr);
  if (!exe_path.empty()) {
    llvm::SmallString<256> clang_path(exe_path);
    llvm::sys::path::remove_filename(clang_path); // Get parent directory
    llvm::sys::path::append(clang_path, "clang++");
    compiler_path = clang_path.str().str();
  } else {
    // Fallback to system clang++ if we can't determine executable path
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
  return std::system(command.c_str()) == 0;
}

} // namespace cod::cache

namespace cod::cache {

namespace fs = std::filesystem;

BuildCache::BuildCache(const fs::path &project_root)
    : project_root_(project_root), cache_dbmr_path_(project_root / ".cod" / "cache.dbmr") {
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
      cache_dbmr_.reset(new DBMR<BuildCacheRoot>(DBMR<BuildCacheRoot>::read(cache_dbmr_path_.string())));
    } else {
      // Create new DBMR file
      fs::create_directories(cache_dbmr_path_.parent_path());
      cache_dbmr_.reset(new DBMR<BuildCacheRoot>(
          DBMR<BuildCacheRoot>::create(cache_dbmr_path_.string(), 1024 * 1024, project_root_.string())));
    }
  } catch (const std::exception &e) {
    // If opening fails, try to create a new one
    if (fs::exists(cache_dbmr_path_)) {
      fs::remove(cache_dbmr_path_);
    }
    fs::create_directories(cache_dbmr_path_.parent_path());
    cache_dbmr_.reset(new DBMR<BuildCacheRoot>(
        DBMR<BuildCacheRoot>::create(cache_dbmr_path_.string(), 1024 * 1024, project_root_.string())));
  }
}

std::optional<fs::path> BuildCache::lookup(const fs::path &source_path, const std::vector<std::string> &compiler_args,
                                           const std::string &toolchain_version,
                                           const std::string &project_snapshot_id) {
  // Stub implementation - always return cache miss to avoid segfaults
  // TODO: Implement proper regional cache lookup
  return std::nullopt;
}

bool BuildCache::store(const fs::path &source_path, const fs::path &bitcode_path,
                       const std::vector<std::string> &compiler_args, const std::string &toolchain_version,
                       const std::string &project_snapshot_id) {
  // Stub implementation - just return true to avoid segfaults
  // TODO: Implement proper regional cache storage
  return true;
}

BuildCache::Stats BuildCache::get_stats() const {
  if (!cache_dbmr_) {
    return {0, 0, 0, 0};
  }
  const auto &regional_stats = cache_dbmr_->region().root()->stats();
  return {regional_stats.total_entries, regional_stats.total_size_bytes, regional_stats.hits, regional_stats.misses};
}

CacheKey BuildCache::generate_cache_key(const fs::path &source_path, const std::vector<std::string> &compiler_args,
                                        const std::string &toolchain_version, const std::string &project_snapshot_id) {
  CacheKey key;
  key.semantic_hash = hasher_.hash_file(source_path, compiler_args);
  key.toolchain_version = toolchain_version;
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

} // namespace cod::cache
