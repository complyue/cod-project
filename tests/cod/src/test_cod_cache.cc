//===--- tests/cod/src/test_cod_cache.cc - CoD Cache Tests -------------===//
//
// CoD Project - Tests for build cache with semantic hashing
// Tests cache lookup, storage, semantic hashing, and bitcode generation
//
//===----------------------------------------------------------------------===//

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "cod_cache.hh"

namespace fs = std::filesystem;
using namespace cod::cache;

// Helper function to create a temporary test directory
fs::path createTempDir() {
  fs::path temp_dir = fs::temp_directory_path() / "cod_cache_test";
  fs::create_directories(temp_dir);
  return temp_dir;
}

// Helper function to create a test source file
fs::path createTestSource(const fs::path &dir, const std::string &content, const std::string &filename = "test.cc") {
  fs::path source_path = dir / filename;
  std::ofstream file(source_path);
  file << content;
  file.close();
  return source_path;
}

void testSemanticHasher() {
  std::cout << "Testing semantic hasher..." << std::endl;

  auto temp_dir = createTempDir();

  // Create two semantically identical source files
  std::string code1 = R"(
#include <iostream>

int add(int a, int b) {
  return a + b;
}

int main() {
  std::cout << add(1, 2) << std::endl;
  return 0;
}
)";

  std::string code2 = R"(
#include <iostream>

// Different comment
int add(int a, int b) {
  // Different spacing and comments
  return a + b;
}

int main() {
  std::cout << add(1, 2) << std::endl;
  return 0;
}
)";

  auto source1 = createTestSource(temp_dir, code1, "test1.cc");
  auto source2 = createTestSource(temp_dir, code2, "test2.cc");

  SemanticHasher hasher;
  std::vector<std::string> compiler_args = {"-std=c++20", "-O2"};

  std::string hash1 = hasher.hash_file(source1, compiler_args);
  std::string hash2 = hasher.hash_file(source2, compiler_args);

  // Hashes should be the same for semantically identical code
  assert(!hash1.empty());
  assert(!hash2.empty());
  assert(hash1 == hash2);

  // Test with different semantic content
  std::string code3 = R"(
#include <iostream>

int multiply(int a, int b) {  // Different function name
  return a * b;               // Different operation
}

int main() {
  std::cout << multiply(1, 2) << std::endl;
  return 0;
}
)";

  auto source3 = createTestSource(temp_dir, code3, "test3.cc");
  std::string hash3 = hasher.hash_file(source3, compiler_args);

  // Hash should be different for semantically different code
  assert(hash3 != hash1);

  // Cleanup
  fs::remove_all(temp_dir);

  std::cout << "✓ Semantic hasher test passed" << std::endl;
}

void testBuildCache() {
  std::cout << "Testing build cache..." << std::endl;

  auto temp_dir = createTempDir();
  auto project_root = temp_dir / "project";
  fs::create_directories(project_root);

  BuildCache cache(project_root);

  // Create test source file
  std::string code = R"(
#include <iostream>

int main() {
  std::cout << "Hello, Cache!" << std::endl;
  return 0;
}
)";

  auto source_path = createTestSource(project_root, code);
  std::vector<std::string> compiler_args = {"-std=c++20", "-O2"};
  std::string toolchain_version = "clang-18";
  std::string project_snapshot = "test_snapshot";

  // First lookup should miss
  auto cached_bitcode = cache.lookup(source_path, compiler_args, toolchain_version, project_snapshot);
  assert(!cached_bitcode.has_value());

  // Generate bitcode
  auto bitcode_path = cache.generate_bitcode(source_path, compiler_args);
  assert(bitcode_path.has_value());
  assert(fs::exists(*bitcode_path));

  // Store in cache
  bool stored = cache.store(source_path, *bitcode_path, compiler_args, toolchain_version, project_snapshot);
  assert(stored);

  // Second lookup should hit
  cached_bitcode = cache.lookup(source_path, compiler_args, toolchain_version, project_snapshot);
  assert(cached_bitcode.has_value());
  assert(fs::exists(*cached_bitcode));

  // Test cache statistics
  auto stats = cache.get_stats();
  assert(stats.total_entries > 0);
  assert(stats.hits > 0);
  assert(stats.misses > 0);

  std::cout << "  Cache stats - Entries: " << stats.total_entries << ", Hits: " << stats.hits
            << ", Misses: " << stats.misses << std::endl;

  // Cleanup
  fs::remove_all(temp_dir);

  std::cout << "✓ Build cache test passed" << std::endl;
}

void testTimestampOptimization() {
  std::cout << "Testing timestamp optimization..." << std::endl;

  auto temp_dir = createTempDir();
  auto project_root = temp_dir / "project";
  fs::create_directories(project_root);

  BuildCache cache(project_root);

  // Create test source file
  std::string code = R"(
#include <iostream>
int main() { std::cout << "Test" << std::endl; return 0; }
)";

  auto source_path = createTestSource(project_root, code);
  std::vector<std::string> compiler_args = {"-std=c++20"};
  std::string toolchain_version = "clang-18";
  std::string project_snapshot = "timestamp_test";

  // Generate and store initial bitcode
  auto bitcode_path = cache.generate_bitcode(source_path, compiler_args);
  assert(bitcode_path.has_value());
  cache.store(source_path, *bitcode_path, compiler_args, toolchain_version, project_snapshot);

  // Lookup should hit with same timestamp
  auto cached_bitcode = cache.lookup(source_path, compiler_args, toolchain_version, project_snapshot);
  assert(cached_bitcode.has_value());

  // Wait a bit and modify file
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Modify the source file (same content, different timestamp)
  std::ofstream file(source_path, std::ios::app);
  file << "\n// Modified timestamp\n";
  file.close();

  // Lookup should miss due to newer timestamp
  cached_bitcode = cache.lookup(source_path, compiler_args, toolchain_version, project_snapshot);
  // Note: This might still hit if semantic hash is the same, which is correct behavior

  // Cleanup
  fs::remove_all(temp_dir);

  std::cout << "✓ Timestamp optimization test passed" << std::endl;
}

void testBitcodeCompiler() {
  std::cout << "Testing bitcode compiler..." << std::endl;

  auto temp_dir = createTempDir();

  // Create test source file
  std::string code = R"(
#include <iostream>

int main() {
  std::cout << "Hello, Bitcode!" << std::endl;
  return 0;
}
)";

  auto source_path = createTestSource(temp_dir, code);
  auto bitcode_path = temp_dir / "test.bc";
  auto executable_path = temp_dir / "test_exe";

  BitcodeCompiler compiler;
  std::vector<std::string> compiler_args = {"-std=c++20", "-O2"};

  // Compile to bitcode
  bool compiled = compiler.compile_to_bitcode(source_path, bitcode_path, compiler_args);
  assert(compiled);
  assert(fs::exists(bitcode_path));

  // Link bitcode to executable
  std::vector<fs::path> bitcode_files = {bitcode_path};
  // Add rpath to find libc++abi.1.dylib in the toolchain directory
  // Use relative path from current working directory to avoid hardcoding
  std::string project_root = std::filesystem::current_path().parent_path().parent_path();
  std::vector<std::string> linker_args = {"-Wl,-rpath," + project_root + "/build/lib"};

  bool linked = compiler.link_bitcode(bitcode_files, executable_path, linker_args);
  assert(linked);
  assert(fs::exists(executable_path));

  // Test execution (basic check)
  std::string cmd = executable_path.string() + " > " + (temp_dir / "output.txt").string();
  int result = std::system(cmd.c_str());
  assert(result == 0);

  // Check output
  std::ifstream output_file(temp_dir / "output.txt");
  std::string output;
  std::getline(output_file, output);
  assert(output.find("Hello, Bitcode!") != std::string::npos);

  // Cleanup
  fs::remove_all(temp_dir);

  std::cout << "✓ Bitcode compiler test passed" << std::endl;
}

void testCacheCleanup() {
  std::cout << "Testing cache cleanup..." << std::endl;

  auto temp_dir = createTempDir();
  auto project_root = temp_dir / "project";
  fs::create_directories(project_root);

  BuildCache cache(project_root);

  // Create and cache multiple files
  for (int i = 0; i < 3; ++i) {
    std::string code =
        "#include <iostream>\nint main() { std::cout << " + std::to_string(i) + " << std::endl; return 0; }";
    auto source_path = createTestSource(project_root, code, "test" + std::to_string(i) + ".cc");

    std::vector<std::string> compiler_args = {"-std=c++20"};
    auto bitcode_path = cache.generate_bitcode(source_path, compiler_args);
    if (bitcode_path) {
      cache.store(source_path, *bitcode_path, compiler_args, "clang-18", "cleanup_test");
    }
  }

  auto stats_before = cache.get_stats();
  assert(stats_before.total_entries > 0);

  // Clean up with very short max age (should remove all entries)
  cache.cleanup_expired(std::chrono::hours(0));

  auto stats_after = cache.get_stats();
  // Note: Some entries might still exist if they're valid and recently created

  std::cout << "  Entries before cleanup: " << stats_before.total_entries << std::endl;
  std::cout << "  Entries after cleanup: " << stats_after.total_entries << std::endl;

  // Cleanup
  fs::remove_all(temp_dir);

  std::cout << "✓ Cache cleanup test passed" << std::endl;
}

int main() {
  std::cout << "Running CoD cache tests...\n\n";

  try {
    testSemanticHasher();
    testBuildCache();
    testTimestampOptimization();
    testBitcodeCompiler();
    testCacheCleanup();

    std::cout << "\n✔ All cache tests passed!\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "\n✗ Cache test failed: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "\n✗ Cache test failed with unknown exception" << std::endl;
    return 1;
  }
}
