#pragma once

#include "shilos.hh"
#include "shilos/dict_yaml.hh"
#include "shilos/str_yaml.hh"
#include "shilos/vector_yaml.hh"

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>
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

// Regional version of CacheKey for zero-copy storage
class regional_cache_key final {
public:
  static inline const UUID TYPE_UUID = UUID("12345678-1234-5678-9abc-123456789001");

private:
  regional_str toolchain_version_;
  regional_vector<regional_str> compiler_flags_;
  regional_str project_snapshot_id_;
  regional_str semantic_hash_;
  std::chrono::system_clock::time_point source_mtime_;

public:
  regional_cache_key() = default;

  template <typename RT>
  regional_cache_key(memory_region<RT> &mr, const std::string &toolchain_version,
                     const std::vector<std::string> &compiler_flags, const std::string &project_snapshot_id,
                     const std::string &semantic_hash, std::chrono::system_clock::time_point source_mtime)
      : toolchain_version_(mr, toolchain_version), compiler_flags_(mr), project_snapshot_id_(mr, project_snapshot_id),
        semantic_hash_(mr, semantic_hash), source_mtime_(source_mtime) {
    for (const auto &flag : compiler_flags) {
      compiler_flags_.emplace_back(mr, flag);
    }
  }

  regional_cache_key(const regional_cache_key &) = delete;
  regional_cache_key(regional_cache_key &&) = delete;
  regional_cache_key &operator=(const regional_cache_key &) = delete;
  regional_cache_key &operator=(regional_cache_key &&) = delete;
  ~regional_cache_key() = default;

  std::string_view toolchain_version() const { return toolchain_version_; }
  const regional_vector<regional_str> &compiler_flags() const { return compiler_flags_; }
  std::string_view project_snapshot_id() const { return project_snapshot_id_; }
  std::string_view semantic_hash() const { return semantic_hash_; }
  std::chrono::system_clock::time_point source_mtime() const { return source_mtime_; }

  std::string to_string() const;
  bool equals(const regional_cache_key &other) const;
};

// Regional version of CacheEntry for zero-copy storage
class regional_cache_entry final {
public:
  static inline const UUID TYPE_UUID = UUID("12345678-1234-5678-9abc-123456789002");

private:
  regional_ptr<regional_cache_key> key_;
  regional_str bitcode_path_;
  std::chrono::system_clock::time_point created_at_;
  size_t file_size_;

public:
  regional_cache_entry() = default;

  template <typename RT>
  regional_cache_entry(memory_region<RT> &mr, regional_cache_key *key, const fs::path &bitcode_path,
                       std::chrono::system_clock::time_point created_at, size_t file_size)
      : key_(key), bitcode_path_(mr, bitcode_path.string()), created_at_(created_at), file_size_(file_size) {}

  regional_cache_entry(const regional_cache_entry &) = delete;
  regional_cache_entry(regional_cache_entry &&) = delete;
  regional_cache_entry &operator=(const regional_cache_entry &) = delete;
  regional_cache_entry &operator=(regional_cache_entry &&) = delete;
  ~regional_cache_entry() = default;

  const regional_cache_key *key() const { return key_.get(); }
  std::string_view bitcode_path() const { return bitcode_path_; }
  std::chrono::system_clock::time_point created_at() const { return created_at_; }
  size_t file_size() const { return file_size_; }

  bool is_valid() const;
};

// Regional cache statistics
struct regional_cache_stats final {
  size_t total_entries;
  size_t total_size_bytes;
  size_t hits;
  size_t misses;

  regional_cache_stats() : total_entries(0), total_size_bytes(0), hits(0), misses(0) {}
};

// Root type for BuildCache DBMR
class BuildCacheRoot final {
public:
  static inline const UUID TYPE_UUID = UUID("12345678-1234-5678-9abc-123456789003");

private:
  regional_str project_root_path_;
  regional_dict<regional_str, regional_cache_entry> cache_index_;
  regional_cache_stats stats_;

public:
  BuildCacheRoot() = default;

  template <typename RT>
  BuildCacheRoot(memory_region<RT> &mr, const fs::path &project_root)
      : project_root_path_(mr, project_root.string()), cache_index_(mr), stats_() {}

  BuildCacheRoot(const BuildCacheRoot &) = delete;
  BuildCacheRoot(BuildCacheRoot &&) = delete;
  BuildCacheRoot &operator=(const BuildCacheRoot &) = delete;
  BuildCacheRoot &operator=(BuildCacheRoot &&) = delete;
  ~BuildCacheRoot() = default;

  std::string_view project_root_path() const { return project_root_path_; }
  regional_dict<regional_str, regional_cache_entry> &cache_index() { return cache_index_; }
  const regional_dict<regional_str, regional_cache_entry> &cache_index() const { return cache_index_; }
  regional_cache_stats &stats() { return stats_; }
  const regional_cache_stats &stats() const { return stats_; }
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
};

// Build cache manager with DBMR-based storage (no load/store operations)
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
  fs::path cache_dbmr_path_; // ./.cod/cache.dbmr

  SemanticHasher hasher_;
  std::unique_ptr<DBMR<BuildCacheRoot>> cache_dbmr_;

  // Internal helpers
  CacheKey generate_cache_key(const fs::path &source_path, const std::vector<std::string> &compiler_args,
                              const std::string &toolchain_version, const std::string &project_snapshot_id);
  bool is_timestamp_newer(const fs::path &source_path, const regional_cache_entry &entry) const;
  void ensure_cache_dbmr();
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
};

// YAML support for regional_cache_key
inline yaml::Node to_yaml(const regional_cache_key &key, yaml::YamlAuthor &author) {
  yaml::Node map(yaml::Map{});

  author.setMapValue(map, "toolchain_version", author.createString(key.toolchain_version()));
  author.setMapValue(map, "compiler_flags", to_yaml(key.compiler_flags(), author));
  author.setMapValue(map, "project_snapshot_id", author.createString(key.project_snapshot_id()));
  author.setMapValue(map, "semantic_hash", author.createString(key.semantic_hash()));

  auto time_t = std::chrono::system_clock::to_time_t(key.source_mtime());
  std::ostringstream oss;
  oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
  author.setMapValue(map, "source_mtime", author.createString(oss.str()));

  return map;
}

template <typename RT>
  requires ValidMemRegionRootType<RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_cache_key *raw_ptr) {
  if (!node.IsMap()) {
    throw yaml::TypeError("YAML node for regional_cache_key must be a mapping");
  }

  auto mapping = std::get<yaml::Map>(node.value);

  std::string toolchain_version;
  std::vector<std::string> compiler_flags;
  std::string project_snapshot_id;
  std::string semantic_hash;
  std::chrono::system_clock::time_point source_mtime;

  for (const auto &entry : mapping) {
    const auto &key_node = entry.key;
    const auto &value_node = entry.value;
    auto key_str = std::string(key_node);

    if (key_str == "toolchain_version") {
      toolchain_version = value_node.asString();
    } else if (key_str == "compiler_flags") {
      if (!value_node.IsSequence()) {
        throw yaml::TypeError("compiler_flags must be a sequence");
      }

      if (auto dash_seq = std::get_if<yaml::DashSequence>(&value_node.value)) {
        for (const auto &item : *dash_seq) {
          compiler_flags.push_back(item.value.asString());
        }
      } else if (auto simple_seq = std::get_if<yaml::SimpleSequence>(&value_node.value)) {
        for (const auto &item : *simple_seq) {
          compiler_flags.push_back(item.asString());
        }
      }
    } else if (key_str == "project_snapshot_id") {
      project_snapshot_id = value_node.asString();
    } else if (key_str == "semantic_hash") {
      semantic_hash = value_node.asString();
    } else if (key_str == "source_mtime") {
      std::string time_str = value_node.asString();
      std::tm tm = {};
      std::istringstream ss(time_str);
      ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
      if (ss.fail()) {
        throw yaml::TypeError("Invalid time format for source_mtime");
      }
      source_mtime = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    }
  }

  new (raw_ptr)
      regional_cache_key(mr, toolchain_version, compiler_flags, project_snapshot_id, semantic_hash, source_mtime);
}

// YAML support for regional_cache_entry
inline yaml::Node to_yaml(const regional_cache_entry &entry, yaml::YamlAuthor &author) {
  yaml::Node map(yaml::Map{});

  if (entry.key()) {
    author.setMapValue(map, "key", to_yaml(*entry.key(), author));
  }
  author.setMapValue(map, "bitcode_path", author.createString(std::string(entry.bitcode_path())));

  auto time_t = std::chrono::system_clock::to_time_t(entry.created_at());
  std::ostringstream oss;
  oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
  author.setMapValue(map, "created_at", author.createString(oss.str()));

  author.setMapValue(map, "file_size", yaml::Node(static_cast<int64_t>(entry.file_size())));

  return map;
}

template <typename RT>
  requires ValidMemRegionRootType<RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_cache_entry *raw_ptr) {
  if (!node.IsMap()) {
    throw yaml::TypeError("YAML node for regional_cache_entry must be a mapping");
  }

  auto mapping = std::get<yaml::Map>(node.value);

  regional_cache_key *key_ptr = nullptr;
  std::filesystem::path bitcode_path;
  std::chrono::system_clock::time_point created_at;
  size_t file_size = 0;

  for (const auto &entry : mapping) {
    const auto &key_node = entry.key;
    const auto &value_node = entry.value;
    auto key_str = std::string(key_node);

    if (key_str == "key") {
      key_ptr = mr.template allocate<regional_cache_key>();
      from_yaml(mr, value_node, key_ptr);
    } else if (key_str == "bitcode_path") {
      bitcode_path = value_node.asString();
    } else if (key_str == "created_at") {
      std::string time_str = value_node.asString();
      std::tm tm = {};
      std::istringstream ss(time_str);
      ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
      if (ss.fail()) {
        throw yaml::TypeError("Invalid time format for created_at");
      }
      created_at = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    } else if (key_str == "file_size") {
      file_size = static_cast<size_t>(value_node.asInt64());
    }
  }

  new (raw_ptr) regional_cache_entry(mr, key_ptr, bitcode_path, created_at, file_size);
}

// YAML support for regional_cache_stats
inline yaml::Node to_yaml(const regional_cache_stats &stats, yaml::YamlAuthor &author) {
  yaml::Node map(yaml::Map{});

  author.setMapValue(map, "total_entries", yaml::Node(static_cast<int64_t>(stats.total_entries)));
  author.setMapValue(map, "total_size_bytes", yaml::Node(static_cast<int64_t>(stats.total_size_bytes)));
  author.setMapValue(map, "hits", yaml::Node(static_cast<int64_t>(stats.hits)));
  author.setMapValue(map, "misses", yaml::Node(static_cast<int64_t>(stats.misses)));

  return map;
}

template <typename RT>
  requires ValidMemRegionRootType<RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_cache_stats *raw_ptr) {
  if (!node.IsMap()) {
    throw yaml::TypeError("YAML node for regional_cache_stats must be a mapping");
  }

  auto mapping = std::get<yaml::Map>(node.value);

  regional_cache_stats stats;

  for (const auto &entry : mapping) {
    const auto &key_node = entry.key;
    const auto &value_node = entry.value;
    auto key_str = std::string(key_node);

    if (key_str == "total_entries") {
      stats.total_entries = static_cast<size_t>(value_node.asInt64());
    } else if (key_str == "total_size_bytes") {
      stats.total_size_bytes = static_cast<size_t>(value_node.asInt64());
    } else if (key_str == "hits") {
      stats.hits = static_cast<size_t>(value_node.asInt64());
    } else if (key_str == "misses") {
      stats.misses = static_cast<size_t>(value_node.asInt64());
    }
  }

  new (raw_ptr) regional_cache_stats(stats);
}

// YAML support for BuildCacheRoot
inline yaml::Node to_yaml(const BuildCacheRoot &root, yaml::YamlAuthor &author) {
  yaml::Node map(yaml::Map{});

  author.setMapValue(map, "project_root_path", author.createString(std::string(root.project_root_path())));
  author.setMapValue(map, "cache_index", to_yaml(root.cache_index(), author));
  author.setMapValue(map, "stats", to_yaml(root.stats(), author));

  return map;
}

template <typename RT>
  requires ValidMemRegionRootType<RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, BuildCacheRoot *raw_ptr) {
  if (!node.IsMap()) {
    throw yaml::TypeError("YAML node for BuildCacheRoot must be a mapping");
  }

  auto mapping = std::get<yaml::Map>(node.value);

  std::filesystem::path project_root;

  for (const auto &entry : mapping) {
    const auto &key_node = entry.key;
    const auto &value_node = entry.value;
    auto key_str = std::string(key_node);

    if (key_str == "project_root_path") {
      project_root = value_node.asString();
    }
  }

  new (raw_ptr) BuildCacheRoot(mr, project_root);

  auto &root = *raw_ptr;

  for (const auto &entry : mapping) {
    const auto &key_node = entry.key;
    const auto &value_node = entry.value;
    auto key_str = std::string(key_node);

    if (key_str == "cache_index") {
      from_yaml(mr, value_node, &root.cache_index());
    } else if (key_str == "stats") {
      from_yaml(mr, value_node, &root.stats());
    }
  }
}

} // namespace cache
} // namespace cod

// Hash function for CacheKey
namespace std {
template <> struct hash<cod::cache::CacheKey> {
  size_t operator()(const cod::cache::CacheKey &key) const { return std::hash<std::string>{}(key.to_string()); }
};
} // namespace std
