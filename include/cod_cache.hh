#pragma once

#include "shilos.hh"
#include <chrono>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

// Forward declarations for Clang AST types
namespace clang {
class ASTContext;
class Decl;
class TranslationUnitDecl;
} // namespace clang

namespace cod {
namespace cache {

namespace fs = std::filesystem;
using namespace shilos;

// Cache key components for semantic hashing
struct CacheKey {
  std::string toolchain_version;
  std::vector<std::string> compiler_flags;
  std::string project_snapshot_id;
  std::string semantic_hash; // AST-based hash
  std::chrono::system_clock::time_point source_mtime;

  // Generate combined cache key string
  std::string to_string() const;

  bool operator==(const CacheKey &other) const;
};

// Cache entry metadata
struct CacheEntry {
  CacheKey key;
  fs::path bitcode_path;
  std::chrono::system_clock::time_point created_at;
  size_t file_size;

  bool is_valid() const;
};

// Semantic AST hasher using Clang libraries
class SemanticHasher {
public:
  SemanticHasher();
  ~SemanticHasher();

  // Generate semantic hash from source file
  std::string hash_file(const fs::path &source_path, const std::vector<std::string> &compiler_args);

  // Generate semantic hash from AST
  std::string hash_ast(clang::ASTContext &context, clang::TranslationUnitDecl *tu);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

// Build cache manager with local/global cache separation
class BuildCache {
public:
  explicit BuildCache(const fs::path &project_root);
  ~BuildCache();

  // Cache lookup with timestamp optimization
  std::optional<fs::path> lookup(const fs::path &source_path, const std::vector<std::string> &compiler_args,
                                 const std::string &toolchain_version, const std::string &project_snapshot_id);

  // Store bitcode in cache
  bool store(const fs::path &source_path, const fs::path &bitcode_path, const std::vector<std::string> &compiler_args,
             const std::string &toolchain_version, const std::string &project_snapshot_id);

  // Generate bitcode from source file
  std::optional<fs::path> generate_bitcode(const fs::path &source_path, const std::vector<std::string> &compiler_args);

  // Clean expired cache entries
  void cleanup_expired(std::chrono::hours max_age = std::chrono::hours(24 * 7)); // 1 week default

  // Get cache statistics
  struct Stats {
    size_t total_entries;
    size_t total_size_bytes;
    size_t hits;
    size_t misses;
  };
  Stats get_stats() const;

private:
  fs::path project_root_;
  fs::path local_cache_dir_;  // ./.cod/works/
  fs::path global_cache_dir_; // ~/.cod/cache/ (future)

  SemanticHasher hasher_;
  mutable std::unordered_map<std::string, CacheEntry> cache_index_;
  mutable Stats stats_;

  // Internal helpers
  fs::path get_cache_dir(bool is_local_dep) const;
  CacheKey generate_cache_key(const fs::path &source_path, const std::vector<std::string> &compiler_args,
                              const std::string &toolchain_version, const std::string &project_snapshot_id);
  bool is_timestamp_newer(const fs::path &source_path, const CacheEntry &entry) const;
  void load_cache_index();
  void save_cache_index() const;
};

// Bitcode compiler wrapper
class BitcodeCompiler {
public:
  BitcodeCompiler();
  ~BitcodeCompiler();

  // Compile source to bitcode
  bool compile_to_bitcode(const fs::path &source_path, const fs::path &output_path,
                          const std::vector<std::string> &compiler_args);

  // Link bitcode files into executable
  bool link_bitcode(const std::vector<fs::path> &bitcode_files, const fs::path &output_executable,
                    const std::vector<std::string> &linker_args);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace cache
} // namespace cod

// Hash function for CacheKey
namespace std {
template <> struct hash<cod::cache::CacheKey> {
  size_t operator()(const cod::cache::CacheKey &key) const { return std::hash<std::string>{}(key.to_string()); }
};
} // namespace std