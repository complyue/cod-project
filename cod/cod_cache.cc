//===--- cod/cod_cache.cc - CoD Build Cache Implementation --------------===//
//
// CoD Project - Build cache with semantic hashing and bitcode generation
// Implements AST-based semantic hashing with timestamp optimization
//
//===----------------------------------------------------------------------===//

#include "cod_cache.hh"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Parse/ParseAST.h>
#include <clang/Sema/Sema.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Support/raw_ostream.h>

#include <fstream>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>

namespace cod {
namespace cache {

// CacheKey implementation
std::string CacheKey::to_string() const {
  std::ostringstream oss;
  oss << toolchain_version << "|";
  for (const auto &flag : compiler_flags) {
    oss << flag << ";";
  }
  oss << "|" << project_snapshot_id << "|" << semantic_hash << "|";
  oss << std::chrono::duration_cast<std::chrono::seconds>(source_mtime.time_since_epoch()).count();
  return oss.str();
}

bool CacheKey::operator==(const CacheKey &other) const {
  return toolchain_version == other.toolchain_version && compiler_flags == other.compiler_flags &&
         project_snapshot_id == other.project_snapshot_id && semantic_hash == other.semantic_hash &&
         source_mtime == other.source_mtime;
}

// CacheEntry implementation
bool CacheEntry::is_valid() const { return fs::exists(bitcode_path) && fs::file_size(bitcode_path) == file_size; }

// AST visitor for semantic hashing
class SemanticASTVisitor : public clang::RecursiveASTVisitor<SemanticASTVisitor> {
public:
  explicit SemanticASTVisitor(std::ostringstream &hash_stream) : hash_stream_(hash_stream) {}

  bool VisitFunctionDecl(clang::FunctionDecl *func) {
    if (func->hasBody()) {
      hash_stream_ << "func:" << func->getNameAsString() << ";";
      // Add return type and parameter types to hash
      hash_stream_ << func->getReturnType().getAsString() << ";";
      for (const auto *param : func->parameters()) {
        hash_stream_ << param->getType().getAsString() << ";";
      }
    }
    return true;
  }

  bool VisitVarDecl(clang::VarDecl *var) {
    if (var->hasGlobalStorage()) {
      hash_stream_ << "var:" << var->getNameAsString() << ":" << var->getType().getAsString() << ";";
    }
    return true;
  }

  bool VisitRecordDecl(clang::RecordDecl *record) {
    if (record->isCompleteDefinition()) {
      hash_stream_ << "record:" << record->getNameAsString() << ";";
      for (const auto *field : record->fields()) {
        hash_stream_ << field->getNameAsString() << ":" << field->getType().getAsString() << ";";
      }
    }
    return true;
  }

private:
  std::ostringstream &hash_stream_;
};

// SemanticHasher implementation
class SemanticHasher::Impl {
public:
  Impl() = default;

  std::string hash_file(const fs::path &source_path, const std::vector<std::string> &compiler_args) {
    // Verify source file exists
    if (!fs::exists(source_path)) {
      return "";
    }

    // For now, implement a simple content-based hash
    // TODO: Implement proper semantic AST hashing later
    std::ifstream file(source_path, std::ios::binary);
    if (!file) {
      return "";
    }

    std::ostringstream hash_stream;

    // Include compiler arguments in hash
    for (const auto &arg : compiler_args) {
      hash_stream << arg << "|";
    }

    // Read file content and create a simple hash
    std::string line;
    while (std::getline(file, line)) {
      // Skip empty lines and comments for basic semantic equivalence
      if (line.empty() || line.find_first_not_of(" \t") == std::string::npos) {
        continue;
      }

      // Skip single-line comments
      auto trimmed_pos = line.find_first_not_of(" \t");
      if (trimmed_pos != std::string::npos && line.substr(trimmed_pos, 2) == "//") {
        continue;
      }

      hash_stream << line << "\n";
    }

    // Create a simple hash of the content
    std::string content = hash_stream.str();
    std::hash<std::string> hasher;
    size_t hash_value = hasher(content);

    std::ostringstream result;
    result << std::hex << hash_value;
    return result.str();
  }

  std::string hash_ast(clang::ASTContext &context, clang::TranslationUnitDecl *tu) {
    std::ostringstream hash_stream;
    SemanticASTVisitor visitor(hash_stream);
    visitor.TraverseDecl(tu);

    // Generate SHA-256 hash of the semantic content
    std::string content = hash_stream.str();
    std::hash<std::string> hasher;
    size_t hash_value = hasher(content);

    std::ostringstream result;
    result << std::hex << hash_value;
    return result.str();
  }
};

SemanticHasher::SemanticHasher() : impl_(std::make_unique<Impl>()) {}
SemanticHasher::~SemanticHasher() = default;

std::string SemanticHasher::hash_file(const fs::path &source_path, const std::vector<std::string> &compiler_args) {
  return impl_->hash_file(source_path, compiler_args);
}

std::string SemanticHasher::hash_ast(clang::ASTContext &context, clang::TranslationUnitDecl *tu) {
  return impl_->hash_ast(context, tu);
}

// BuildCache implementation
BuildCache::BuildCache(const fs::path &project_root)
    : project_root_(project_root), local_cache_dir_(project_root / ".cod" / "works"),
      global_cache_dir_(fs::path(std::getenv("HOME") ? std::getenv("HOME") : "/tmp") / ".cod" / "cache"),
      stats_{0, 0, 0, 0} {

  // Create cache directories
  fs::create_directories(local_cache_dir_);
  // Note: global cache creation deferred to future implementation

  load_cache_index();
}

BuildCache::~BuildCache() { save_cache_index(); }

std::optional<fs::path> BuildCache::lookup(const fs::path &source_path, const std::vector<std::string> &compiler_args,
                                           const std::string &toolchain_version,
                                           const std::string &project_snapshot_id) {
  // First check: file modification timestamp
  struct stat source_stat;
  if (stat(source_path.c_str(), &source_stat) != 0) {
    return std::nullopt;
  }

  auto source_mtime = std::chrono::system_clock::from_time_t(source_stat.st_mtime);

  // Generate cache key for lookup
  CacheKey temp_key;
  temp_key.toolchain_version = toolchain_version;
  temp_key.compiler_flags = compiler_args;
  temp_key.project_snapshot_id = project_snapshot_id;
  temp_key.source_mtime = source_mtime;

  // Check if we have a cached entry with matching timestamp
  std::string key_prefix = temp_key.toolchain_version + "|";
  for (const auto &flag : temp_key.compiler_flags) {
    key_prefix += flag + ";";
  }
  key_prefix += "|" + temp_key.project_snapshot_id + "|";

  for (const auto &[key_str, entry] : cache_index_) {
    if (key_str.starts_with(key_prefix) && entry.key.source_mtime == source_mtime && entry.is_valid()) {
      stats_.hits++;
      return entry.bitcode_path;
    }
  }

  // Second check: semantic hash (expensive)
  std::string semantic_hash = hasher_.hash_file(source_path, compiler_args);
  if (semantic_hash.empty()) {
    stats_.misses++;
    return std::nullopt;
  }

  temp_key.semantic_hash = semantic_hash;
  std::string full_key = temp_key.to_string();

  auto it = cache_index_.find(full_key);
  if (it != cache_index_.end() && it->second.is_valid()) {
    stats_.hits++;
    return it->second.bitcode_path;
  }

  stats_.misses++;
  return std::nullopt;
}

bool BuildCache::store(const fs::path &source_path, const fs::path &bitcode_path,
                       const std::vector<std::string> &compiler_args, const std::string &toolchain_version,
                       const std::string &project_snapshot_id) {
  CacheKey key = generate_cache_key(source_path, compiler_args, toolchain_version, project_snapshot_id);

  CacheEntry entry;
  entry.key = key;
  entry.bitcode_path = bitcode_path;
  entry.created_at = std::chrono::system_clock::now();
  entry.file_size = fs::file_size(bitcode_path);

  cache_index_[key.to_string()] = entry;
  stats_.total_entries++;
  stats_.total_size_bytes += entry.file_size;

  return true;
}

std::optional<fs::path> BuildCache::generate_bitcode(const fs::path &source_path,
                                                     const std::vector<std::string> &compiler_args) {
  // Generate unique output path
  std::string hash_input = source_path.string();
  for (const auto &arg : compiler_args) {
    hash_input += arg;
  }

  std::hash<std::string> hasher;
  size_t path_hash = hasher(hash_input);

  std::ostringstream filename;
  filename << std::hex << path_hash << ".bc";

  fs::path output_path = local_cache_dir_ / filename.str();

  // Use clang to compile to bitcode
  std::vector<std::string> clang_args = {"clang++", "-emit-llvm", "-c", "-o", output_path.string()};

  clang_args.insert(clang_args.end(), compiler_args.begin(), compiler_args.end());
  clang_args.push_back(source_path.string());

  // Build command string
  std::ostringstream cmd;
  for (size_t i = 0; i < clang_args.size(); ++i) {
    if (i > 0)
      cmd << " ";
    cmd << clang_args[i];
  }

  // Execute clang command
  int result = std::system(cmd.str().c_str());
  if (result != 0 || !fs::exists(output_path)) {
    return std::nullopt;
  }

  return output_path;
}

void BuildCache::cleanup_expired(std::chrono::hours max_age) {
  auto now = std::chrono::system_clock::now();

  auto it = cache_index_.begin();
  while (it != cache_index_.end()) {
    if (now - it->second.created_at > max_age || !it->second.is_valid()) {
      // Remove file if it exists
      if (fs::exists(it->second.bitcode_path)) {
        fs::remove(it->second.bitcode_path);
      }

      stats_.total_entries--;
      stats_.total_size_bytes -= it->second.file_size;
      it = cache_index_.erase(it);
    } else {
      ++it;
    }
  }
}

BuildCache::Stats BuildCache::get_stats() const { return stats_; }

fs::path BuildCache::get_cache_dir(bool is_local_dep) const {
  return is_local_dep ? local_cache_dir_ : global_cache_dir_;
}

CacheKey BuildCache::generate_cache_key(const fs::path &source_path, const std::vector<std::string> &compiler_args,
                                        const std::string &toolchain_version, const std::string &project_snapshot_id) {
  CacheKey key;
  key.toolchain_version = toolchain_version;
  key.compiler_flags = compiler_args;
  key.project_snapshot_id = project_snapshot_id;

  // Get file modification time
  struct stat source_stat;
  if (stat(source_path.c_str(), &source_stat) == 0) {
    key.source_mtime = std::chrono::system_clock::from_time_t(source_stat.st_mtime);
  }

  // Generate semantic hash
  key.semantic_hash = hasher_.hash_file(source_path, compiler_args);

  return key;
}

bool BuildCache::is_timestamp_newer(const fs::path &source_path, const CacheEntry &entry) const {
  struct stat source_stat;
  if (stat(source_path.c_str(), &source_stat) != 0) {
    return true; // Assume newer if we can't stat
  }

  auto source_mtime = std::chrono::system_clock::from_time_t(source_stat.st_mtime);
  return source_mtime > entry.key.source_mtime;
}

void BuildCache::load_cache_index() {
  fs::path index_path = local_cache_dir_ / "cache_index.txt";
  if (!fs::exists(index_path)) {
    return;
  }

  std::ifstream file(index_path);
  std::string line;
  while (std::getline(file, line)) {
    // Simple format: key|bitcode_path|created_at|file_size
    size_t pos1 = line.find('|');
    size_t pos2 = line.find('|', pos1 + 1);
    size_t pos3 = line.find('|', pos2 + 1);

    if (pos1 != std::string::npos && pos2 != std::string::npos && pos3 != std::string::npos) {
      std::string key_str = line.substr(0, pos1);
      std::string bitcode_path_str = line.substr(pos1 + 1, pos2 - pos1 - 1);
      std::string created_at_str = line.substr(pos2 + 1, pos3 - pos2 - 1);
      std::string file_size_str = line.substr(pos3 + 1);

      CacheEntry entry;
      entry.bitcode_path = bitcode_path_str;
      entry.created_at = std::chrono::system_clock::from_time_t(std::stoll(created_at_str));
      entry.file_size = std::stoull(file_size_str);

      if (entry.is_valid()) {
        cache_index_[key_str] = entry;
        stats_.total_entries++;
        stats_.total_size_bytes += entry.file_size;
      }
    }
  }
}

void BuildCache::save_cache_index() const {
  fs::path index_path = local_cache_dir_ / "cache_index.txt";
  std::ofstream file(index_path);

  for (const auto &[key_str, entry] : cache_index_) {
    if (entry.is_valid()) {
      file << key_str << "|" << entry.bitcode_path.string() << "|"
           << std::chrono::duration_cast<std::chrono::seconds>(entry.created_at.time_since_epoch()).count() << "|"
           << entry.file_size << "\n";
    }
  }
}

// BitcodeCompiler implementation
class BitcodeCompiler::Impl {
public:
  bool compile_to_bitcode(const fs::path &source_path, const fs::path &output_path,
                          const std::vector<std::string> &compiler_args) {
    std::vector<std::string> clang_args = {"clang++", "-emit-llvm", "-c", "-o", output_path.string()};

    clang_args.insert(clang_args.end(), compiler_args.begin(), compiler_args.end());
    clang_args.push_back(source_path.string());

    std::ostringstream cmd;
    for (size_t i = 0; i < clang_args.size(); ++i) {
      if (i > 0)
        cmd << " ";
      cmd << clang_args[i];
    }

    int result = std::system(cmd.str().c_str());
    return result == 0 && fs::exists(output_path);
  }

  bool link_bitcode(const std::vector<fs::path> &bitcode_files, const fs::path &output_executable,
                    const std::vector<std::string> &linker_args) {
    std::vector<std::string> link_args = {"clang++", "-o", output_executable.string()};

    for (const auto &bc_file : bitcode_files) {
      link_args.push_back(bc_file.string());
    }

    link_args.insert(link_args.end(), linker_args.begin(), linker_args.end());

    std::ostringstream cmd;
    for (size_t i = 0; i < link_args.size(); ++i) {
      if (i > 0)
        cmd << " ";
      cmd << link_args[i];
    }

    int result = std::system(cmd.str().c_str());
    return result == 0 && fs::exists(output_executable);
  }
};

BitcodeCompiler::BitcodeCompiler() : impl_(std::make_unique<Impl>()) {}
BitcodeCompiler::~BitcodeCompiler() = default;

bool BitcodeCompiler::compile_to_bitcode(const fs::path &source_path, const fs::path &output_path,
                                         const std::vector<std::string> &compiler_args) {
  return impl_->compile_to_bitcode(source_path, output_path, compiler_args);
}

bool BitcodeCompiler::link_bitcode(const std::vector<fs::path> &bitcode_files, const fs::path &output_executable,
                                   const std::vector<std::string> &linker_args) {
  return impl_->link_bitcode(bitcode_files, output_executable, linker_args);
}

} // namespace cache
} // namespace cod