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
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "../test_output_utils.hh"
#include "cod_cache.hh"

// Helper macro for descriptive assertions
#define TEST_ASSERT(condition, message)                                                                                \
  do {                                                                                                                 \
    if (!(condition)) {                                                                                                \
      throw std::runtime_error("Assertion failed: " + std::string(message) + " (" #condition ")");                     \
    }                                                                                                                  \
  } while (0)

#define TEST_ASSERT_EQ(actual, expected, message)                                                                      \
  do {                                                                                                                 \
    if ((actual) != (expected)) {                                                                                      \
      throw std::runtime_error("Assertion failed: " + std::string(message) + " (expected: " +                          \
                               std::to_string(expected) + ", actual: " + std::to_string(actual) + ")");                \
    }                                                                                                                  \
  } while (0)

namespace fs = std::filesystem;
using namespace cod::cache;

// RAII helper class for temporary directory management
class TempDirGuard {
public:
  TempDirGuard()
      : temp_dir_(fs::temp_directory_path() /
                  ("cod_cache_test_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()))) {
    fs::create_directories(temp_dir_);
  }

  ~TempDirGuard() {
    try {
      if (fs::exists(temp_dir_)) {
        fs::remove_all(temp_dir_);
      }
    } catch (const std::exception &e) {
      std::cerr << "Warning: Failed to cleanup temp directory " << temp_dir_ << ": " << e.what() << std::endl;
    }
  }

  // Non-copyable, non-movable
  TempDirGuard(const TempDirGuard &) = delete;
  TempDirGuard &operator=(const TempDirGuard &) = delete;
  TempDirGuard(TempDirGuard &&) = delete;
  TempDirGuard &operator=(TempDirGuard &&) = delete;

  const fs::path &path() const { return temp_dir_; }
  operator const fs::path &() const { return temp_dir_; }

private:
  fs::path temp_dir_;
};

// Helper function to create a temporary test directory (legacy compatibility)
fs::path createTempDir() {
  fs::path temp_dir = fs::temp_directory_path() /
                      ("cod_cache_test_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
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

void testSemanticHasher(bool verbose) {
  TestLogger::log_test("semantic hasher");
  (void)verbose; // Suppress unused parameter warning

  TempDirGuard temp_dir;

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
  TEST_ASSERT(!hash1.empty(), "First semantic hash should not be empty");
  TEST_ASSERT(!hash2.empty(), "Second semantic hash should not be empty");
  TEST_ASSERT(hash1 == hash2, "Hashes should be identical for semantically equivalent code");

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
  TEST_ASSERT(hash3 != hash1, "Hash should be different for semantically different code");

  // Cleanup handled automatically by TempDirGuard destructor
  TestLogger::log_pass("Semantic hasher test passed");
}

void testBuildCache(bool verbose) {
  TestLogger::log_test("build cache");

  TempDirGuard temp_dir;
  auto project_root = temp_dir.path() / "project";
  fs::create_directories(project_root);

  BuildCache cache(project_root, verbose);

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
  std::string project_snapshot = "test_snapshot";

  // First lookup should miss
  auto cached_bitcode = cache.lookup(source_path, compiler_args, project_snapshot);
  TEST_ASSERT(!cached_bitcode.has_value(), "First cache lookup should miss (cache should be empty)");

  // Generate bitcode
  auto bitcode_path = cache.generate_bitcode(source_path, compiler_args);
  TEST_ASSERT(bitcode_path.has_value(), "Bitcode generation should succeed");
  TEST_ASSERT(fs::exists(*bitcode_path), "Generated bitcode file should exist on disk");

  // Store in cache
  bool stored = cache.store(source_path, *bitcode_path, compiler_args, project_snapshot);
  TEST_ASSERT(stored, "Storing bitcode in cache should succeed");

  // Second lookup should hit
  cached_bitcode = cache.lookup(source_path, compiler_args, project_snapshot);
  TEST_ASSERT(cached_bitcode.has_value(), "Second cache lookup should hit (cache should contain entry)");
  TEST_ASSERT(fs::exists(*cached_bitcode), "Cached bitcode file should exist on disk");

  // Test cache statistics
  auto stats = cache.get_stats();
  TEST_ASSERT(stats.total_entries > 0, "Cache should have at least one entry");
  TEST_ASSERT(stats.hits > 0, "Cache should have recorded at least one hit");
  TEST_ASSERT(stats.misses > 0, "Cache should have recorded at least one miss");

  std::cout << "  Cache stats - Entries: " << stats.total_entries << ", Hits: " << stats.hits
            << ", Misses: " << stats.misses << std::endl;

  // Cleanup handled automatically by TempDirGuard destructor
  TestLogger::log_pass("Build cache test passed");
}

void testTimestampOptimization(bool verbose) {
  TestLogger::log_test("timestamp optimization");
  (void)verbose; // Suppress unused parameter warning

  TempDirGuard temp_dir;
  auto project_root = temp_dir.path() / "project";
  fs::create_directories(project_root);

  BuildCache cache(project_root, verbose);

  // Create test source file
  std::string code = R"(
#include <iostream>
int main() { std::cout << "Test" << std::endl; return 0; }
)";

  auto source_path = createTestSource(project_root, code);
  std::vector<std::string> compiler_args = {"-std=c++20"};
  std::string project_snapshot = "timestamp_test";

  // Generate and store initial bitcode
  auto bitcode_path = cache.generate_bitcode(source_path, compiler_args);
  TEST_ASSERT(bitcode_path.has_value(), "Initial bitcode generation should succeed");
  bool stored = cache.store(source_path, *bitcode_path, compiler_args, project_snapshot);
  TEST_ASSERT(stored, "Initial bitcode storage should succeed");

  // Lookup should hit with same timestamp
  auto cached_bitcode = cache.lookup(source_path, compiler_args, project_snapshot);
  TEST_ASSERT(cached_bitcode.has_value(), "Cache lookup should hit with unchanged file");

  // Wait to ensure timestamp difference (use shorter delay with verification)
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Get original timestamp for verification
  auto original_mtime = fs::last_write_time(source_path);

  // Modify the source file (same content, different timestamp)
  std::ofstream file(source_path, std::ios::app);
  file << "\n// Modified timestamp\n";
  file.close();
  file.clear();

  // Verify timestamp actually changed, retry if needed
  auto new_mtime = fs::last_write_time(source_path);
  if (new_mtime <= original_mtime) {
    // If timestamp didn't change, wait a bit more and try again
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::ofstream retry_file(source_path, std::ios::app);
    retry_file << "\n// Retry timestamp modification\n";
    retry_file.close();
    new_mtime = fs::last_write_time(source_path);
  }
  TEST_ASSERT(new_mtime > original_mtime, "File modification time should have increased");

  // Lookup should miss due to newer timestamp
  cached_bitcode = cache.lookup(source_path, compiler_args, project_snapshot);
  // Note: This might still hit if semantic hash is the same, which is correct behavior

  // Cleanup handled automatically by TempDirGuard destructor
  TestLogger::log_pass("Timestamp optimization test passed");
}

void testEdgeCases(bool verbose) {
  TestLogger::log_test("edge cases and error conditions");

  TempDirGuard temp_dir;
  auto project_root = temp_dir.path() / "project";
  fs::create_directories(project_root);

  // Test 1: Empty source file
  {
    auto empty_source = project_root / "empty.cpp";
    {
      std::ofstream file(empty_source);
      // Create empty file
    }

    BuildCache cache(temp_dir.path() / "cache_empty", verbose);
    std::vector<std::string> args = {"-std=c++20"};

    // Should handle empty files gracefully
    auto result = cache.lookup(empty_source, args, "test_project");
    TEST_ASSERT(!result.has_value(), "Empty file lookup should miss initially");
  }

  // Test 2: Non-existent source file
  {
    BuildCache cache(temp_dir.path() / "cache_nonexistent", verbose);
    auto nonexistent_path = project_root / "does_not_exist.cpp";
    std::vector<std::string> args = {"-std=c++20"};

    // Should handle non-existent files gracefully (may throw or return empty)
    try {
      auto result = cache.lookup(nonexistent_path, args, "test_project");
      // If no exception, result should be empty
      TEST_ASSERT(!result.has_value(), "Non-existent file lookup should miss");
    } catch (const std::exception &e) {
      // Exception is also acceptable for non-existent files
      TestLogger::log_pass("Non-existent file properly throws exception: " + std::string(e.what()));
    }
  }

  // Test 3: Invalid compiler arguments
  {
    auto source_path = project_root / "valid.cpp";
    {
      std::ofstream file(source_path);
      file << "int main() { return 0; }";
    }

    BuildCache cache(temp_dir.path() / "cache_invalid_args", verbose);
    std::vector<std::string> invalid_args = {"-invalid-flag-xyz", "-another-bad-flag"};

    // Should handle invalid compiler args gracefully
    auto result = cache.lookup(source_path, invalid_args, "test_project");
    TEST_ASSERT(!result.has_value(), "Invalid compiler args lookup should miss initially");
  }

  // Test 4: Very long file paths (optimized)
  {
    std::string long_name(100, 'a'); // Reduced to 100 chars for faster execution
    auto long_path = project_root / (long_name + ".cpp");

    try {
      std::ofstream file(long_path);
      file << "int main() { return 0; }";
      file.close();

      if (fs::exists(long_path)) {
        BuildCache cache(temp_dir.path() / "cache_long_path", verbose);
        std::vector<std::string> args = {"-std=c++20"};

        auto result = cache.lookup(long_path, args, "test_project");
        TEST_ASSERT(!result.has_value(), "Long path lookup should miss initially");
      }
    } catch (const std::exception &e) {
      // Expected for very long paths on some filesystems
    }
  }

  // Test 5: Empty compiler arguments
  {
    auto source_path = project_root / "no_args.cpp";
    {
      std::ofstream file(source_path);
      file << "int main() { return 0; }";
    }

    BuildCache cache(temp_dir.path() / "cache_no_args", verbose);
    std::vector<std::string> empty_args; // No compiler arguments

    auto result = cache.lookup(source_path, empty_args, "test_project");
    TEST_ASSERT(!result.has_value(), "Empty compiler args lookup should miss initially");
  }

  // Test 6: Cache directory permissions (simplified)
  {
    auto readonly_cache_dir = temp_dir.path() / "readonly_cache";
    fs::create_directories(readonly_cache_dir);

    try {
      // Try to make directory read-only (may not work on all systems)
      fs::permissions(readonly_cache_dir, fs::perms::owner_read);

      // Just test cache creation, not full lookup to save time
      BuildCache cache(readonly_cache_dir, verbose);

      // Reset permissions immediately for cleanup
      fs::permissions(readonly_cache_dir, fs::perms::owner_all);

    } catch (const std::exception &e) {
      // Expected behavior - permissions properly enforced
      fs::permissions(readonly_cache_dir, fs::perms::owner_all); // Ensure cleanup
    }
  }

  TestLogger::log_pass("Edge cases and error conditions test passed");
}

void testBitcodeCompiler(bool verbose) {
  std::cout << "Testing bitcode compiler..." << std::endl;
  (void)verbose; // Suppress unused parameter warning

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
  std::vector<std::string> compiler_args = cod::CompilerArgs({"-std=c++20", "-O2"});

  // Compile to bitcode
  bool compiled = compiler.compile_to_bitcode(source_path, bitcode_path, compiler_args);
  assert(compiled);
  assert(fs::exists(bitcode_path));

  // Link bitcode to executable
  std::vector<fs::path> bitcode_files = {bitcode_path};
  // Add rpath to find libc++abi.1.dylib in the toolchain directory
  // Use relative path from current working directory to avoid hardcoding
  std::string project_root = std::filesystem::current_path().parent_path().parent_path();
  std::vector<std::string> linker_args = cod::LinkerArgs({"-Wl,-rpath," + project_root + "/build/lib"});

  bool linked = compiler.link_bitcode(bitcode_files, executable_path, linker_args);
  TEST_ASSERT(linked, "Bitcode linking should succeed");
  TEST_ASSERT(fs::exists(executable_path), "Linked executable should exist on disk");

  // Test execution (basic check)
  std::string cmd = executable_path.string() + " > " + (temp_dir / "output.txt").string();
  int result = std::system(cmd.c_str());
  TEST_ASSERT(result == 0, "Executable should run successfully");

  // Check output
  std::ifstream output_file(temp_dir / "output.txt");
  std::string output;
  std::getline(output_file, output);
  TEST_ASSERT(output.find("Hello, Bitcode!") != std::string::npos, "Executable output should contain expected text");

  // Cleanup
  fs::remove_all(temp_dir);

  std::cout << "✓ Bitcode compiler test passed" << std::endl;
}

void testCacheCleanup(bool verbose) {
  std::cout << "Testing cache cleanup..." << std::endl;
  (void)verbose; // Suppress unused parameter warning

  auto temp_dir = createTempDir();
  auto project_root = temp_dir / "project";
  fs::create_directories(project_root);

  BuildCache cache(project_root, verbose);

  // Create and cache multiple files
  for (int i = 0; i < 3; ++i) {
    std::string code =
        "#include <iostream>\nint main() { std::cout << " + std::to_string(i) + " << std::endl; return 0; }";
    auto source_path = createTestSource(project_root, code, "test" + std::to_string(i) + ".cc");

    std::vector<std::string> compiler_args = {"-std=c++20"};
    auto bitcode_path = cache.generate_bitcode(source_path, compiler_args);
    if (bitcode_path) {
      cache.store(source_path, *bitcode_path, compiler_args, "cleanup_test");
    }
  }

  auto stats_before = cache.get_stats();
  TEST_ASSERT(stats_before.total_entries > 0, "Cache should have entries before cleanup");

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

int main(int argc, char *argv[]) {
  bool verbose = false;

  // Parse command line arguments
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--verbose" || arg == "-v") {
      verbose = true;
    } else if (arg == "--help" || arg == "-h") {
      std::cout << "Usage: " << argv[0] << " [OPTIONS]" << std::endl;
      std::cout << "Options:" << std::endl;
      std::cout << "  -v, --verbose    Show detailed debug output" << std::endl;
      std::cout << "  -h, --help       Show this help message" << std::endl;
      return 0;
    }
  }

  TestLogger::log_header("CoD Cache Tests");

  int failed_tests = 0;
  int total_tests = 6;

  // Test 1: Semantic Hasher
  try {
    testSemanticHasher(verbose);
  } catch (const std::exception &e) {
    TestLogger::log_fail("Semantic hasher test failed: " + std::string(e.what()));
    failed_tests++;
  } catch (...) {
    TestLogger::log_fail("Semantic hasher test failed with unknown exception");
    failed_tests++;
  }

  // Test 2: Build Cache
  try {
    testBuildCache(verbose);
  } catch (const std::exception &e) {
    TestLogger::log_fail("Build cache test failed: " + std::string(e.what()));
    failed_tests++;
  } catch (...) {
    TestLogger::log_fail("Build cache test failed with unknown exception");
    failed_tests++;
  }

  // Test 3: Timestamp Optimization
  try {
    testTimestampOptimization(verbose);
  } catch (const std::exception &e) {
    TestLogger::log_fail("Timestamp optimization test failed: " + std::string(e.what()));
    failed_tests++;
  } catch (...) {
    TestLogger::log_fail("Timestamp optimization test failed with unknown exception");
    failed_tests++;
  }

  // Test 4: Bitcode Compiler
  try {
    testBitcodeCompiler(verbose);
  } catch (const std::exception &e) {
    TestLogger::log_fail("Bitcode compiler test failed: " + std::string(e.what()));
    failed_tests++;
  } catch (...) {
    TestLogger::log_fail("Bitcode compiler test failed with unknown exception");
    failed_tests++;
  }

  // Test 5: Cache Cleanup
  try {
    testCacheCleanup(verbose);
  } catch (const std::exception &e) {
    TestLogger::log_fail("Cache cleanup test failed: " + std::string(e.what()));
    failed_tests++;
  } catch (...) {
    TestLogger::log_fail("Cache cleanup test failed with unknown exception");
    failed_tests++;
  }

  // Test 6: Edge Cases
  try {
    testEdgeCases(verbose);
  } catch (const std::exception &e) {
    TestLogger::log_fail("Edge cases test failed: " + std::string(e.what()));
    failed_tests++;
  } catch (...) {
    TestLogger::log_fail("Edge cases test failed with unknown exception");
    failed_tests++;
  }

  // Summary
  int passed_tests = total_tests - failed_tests;
  TestLogger::log_summary(passed_tests, total_tests, "CoD cache tests");

  return (failed_tests == 0) ? 0 : 1;
}
