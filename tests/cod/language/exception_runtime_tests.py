#!/usr/bin/env python3
"""Exercise allocation failure delivery and the exceptional cleanup boundary."""

import hashlib
import datetime
import json
import os
from pathlib import Path
import subprocess
import tempfile

from regional_runtime_tests import checked, run_case


HARNESS = r'''
#undef cod_lang_region_free
#undef cod_lang_print_i64
#include <cassert>
#include <cstdlib>
#include <new>

static void *regions[2]{};
static bool live[2]{};
static unsigned acquired = 0, released = 0, provider_failures = 0;
void *operator new(std::size_t size) {
  if (size == sizeof(CoDRegion) + 513) {
    ++provider_failures;
    throw std::bad_alloc();
  }
  void *p = std::malloc(size ? size : 1);
  if (!p) throw std::bad_alloc();
  if (size == sizeof(CoDRegion) + 4096 || size == sizeof(CoDRegion) + 256) {
    assert(acquired < 2);
    regions[acquired] = p;
    live[acquired++] = true;
  }
  return p;
}
void operator delete(void *p) noexcept {
  for (unsigned i = 0; p && i < acquired; ++i) {
    if (p != regions[i]) continue;
    assert(live[i]);
    live[i] = false;
    ++released;
  }
  std::free(p);
}
void operator delete(void *p, std::size_t) noexcept { ::operator delete(p); }
extern "C" void cod_lang_print_i64(std::int64_t value) {
  assert(provider_failures == 1);
  assert(!cod_causes);
  if (value == 10) assert(released == 0 && live[0] && live[1]);
  else assert((value == 20 || value == 30) && released == 1 && live[0] && !live[1]);
  cod_lang_trace_i64(value);
}
extern "C" void cod_lang_region_free(void *owner) {
  if (!owner) return;
  assert(acquired == 2);
  assert(owner == regions[released == 0 ? 1 : 0]);
  cod_lang_region_release(owner);
  if (released == 2) {
    assert(!live[0] && !live[1] && provider_failures == 1);
    std::puts("{\"provider_failures\":1,\"owners_acquired\":2,\"owners_released\":2,"
              "\"inner_released_before_catch\":true,\"outer_preserved_until_exit\":true}");
  }
}
'''


CHAIN_HARNESS = r'''
#undef cod_lang_region_free
#undef cod_lang_exception_finish
#include <cassert>
#include <cstring>
static unsigned relocated_images = 0;
extern "C" void cod_lang_region_free(void *owner) {
  if (!owner) return;
  auto &original = *static_cast<CoDRegion *>(owner);
  if (original.capacity() != sizeof(CoDRegion) + 256) {
    cod_lang_region_release(owner);
    return;
  }
  const auto used = original.occupation();
  const auto offset = cod_lang_region_root(owner);
  CoDRegionOwner relocated(256);
  std::memcpy(static_cast<void *>(&*relocated), owner, used);
  cod_lang_region_release(owner);
  std::uint64_t fields[9]{};
  std::memcpy(fields, reinterpret_cast<const std::byte *>(&*relocated) + offset, sizeof(fields));
  // Common error: payload + category; exact error: payload only.
  assert(fields[4] == 3 && fields[1] == 0 && fields[8] == 0);
  assert(cod_lang_exception_count(fields[3]) == 2);
  std::uint64_t cause[5]{};
  cod_lang_exception_cause(fields[3], 0, cause);
  assert(cause[1] == 8 && cause[2] == 1 && cause[4] == 2);
  cod_lang_exception_cause(fields[3], 1, cause);
  assert(cause[1] == 0 && cause[3] == 0 && cause[4] == 3);
  assert(cause[0] == fields[5]);
  ++relocated_images;
}
extern "C" void cod_lang_exception_finish() noexcept {
  assert(relocated_images == 1 && cod_causes && cod_causes->size() == 4);
  cod_lang_exception_release();
  assert(!cod_causes);
  std::puts("{\"returned_and_stored_causes\":true,\"relocation_after_original_release\":true,"
            "\"category_preserved\":true,\"immutable_records\":4,\"storage_released_at_exit\":true}");
}
'''


CHAIN_FAILURE_SOURCE = '''
using symbol "std:io" @assert;
using symbol "std:io" @print;
using "std:exceptions" OutOfMemory;
using "std:exceptions" NullPointerException;
data Packet {value:u64;}
try {
try {owned region invalid:Packet <- make Packet(capacity=0u64){value<-0u64}; {}}
catch(seed) {
  owned region keep:Packet <- make Packet(capacity=16u64){value<-7u64};
  for(i in 0 .. 2) {{
    try {throw seed from seed} catch(error:const OutOfMemory) {
      behave @assert(error.cause_count == 0u64)
    }
  }};
  val first = try {throw seed from seed} catch(error) {error};
  try {
    owned region inner:Packet <- make Packet(capacity=16u64){value<-0u64};
    throw first from seed
  } catch(error:const OutOfMemory) {
    behave @assert(error.cause_count == 0u64);
    behave @assert(error.requested == 40u64);
    behave @assert(first.cause_count == 1u64 && cause_at(first, 0u64).category == 3u64);
    behave @assert(root(keep).value == 7u64);
    behave @print(10)
  };
  val retry = try {throw first from first} catch(error) {error};
  behave @assert(retry.cause_count == 2u64 && first.cause_count == 1u64)
};
} catch(unexpected:const NullPointerException) {behave @assert(false)};
'''


CHAIN_FAILURE_HARNESS = r'''
#undef cod_lang_region_free
#undef cod_lang_exception_finish
#undef cod_lang_exception_chain
#undef cod_lang_print_i64
#include <cassert>
static bool append_active = false;
static unsigned allocation_in_call = 0;
static unsigned calls = 0, failures = 0, allocations = 0, frees = 0;
static unsigned acquired = 0, released = 0;
static void *backing[4]{};
void *operator new(std::size_t size) {
  if (append_active && (++allocation_in_call == (calls == 2 ? 2u : 1u)) &&
      (calls == 1 || calls == 2 || calls == 4)) { ++failures; throw std::bad_alloc(); }
  void *p = std::malloc(size ? size : 1);
  if (!p) throw std::bad_alloc();
  if (append_active) { assert(allocations < 4); backing[allocations++] = p; }
  else if (size == sizeof(CoDRegion) + 16) ++acquired;
  return p;
}
void operator delete(void *p) noexcept {
  for (auto &address : backing) if (p && p == address) {address = nullptr; ++frees;}
  std::free(p);
}
void operator delete(void *p, std::size_t) noexcept { ::operator delete(p); }
extern "C" std::uint64_t cod_lang_exception_chain(std::uint64_t category, std::uint64_t site,
    std::uint64_t requested, std::uint64_t available, std::uint64_t cause, std::uint64_t *status) {
  append_active = true;
  allocation_in_call = 0;
  ++calls;
  if (calls == 4) assert(cod_causes->size() == cod_causes->capacity());
  const auto result = cod_lang_exception_append(category, site, requested, available, cause, status);
  append_active = false;
  if (calls <= 2) assert(status[0] == 1 && result == 0 && !cod_causes);
  if (calls == 4) assert(status[0] == 1 && result == 0 && cod_causes->size() == 1);
  return result;
}
extern "C" void cod_lang_region_free(void *owner) {
  if (!owner) return;
  assert(++released <= acquired);
  cod_lang_region_release(owner);
}
extern "C" void cod_lang_print_i64(std::int64_t value) {
  assert(value == 10 && failures == 3 && acquired == 2 && released == 1);
  cod_lang_trace_i64(value);
}
extern "C" void cod_lang_exception_finish() noexcept {
  assert(calls == 5 && failures == 3 && acquired == 2 && released == 2 && allocations == 4);
  cod_lang_exception_release();
  assert(frees == 4 && !cod_causes);
  std::puts("{\"real_chain_allocation_failures\":3,\"automatic_cause_added\":false,"
            "\"saved_chain_survives_failure\":true,\"retry_passed\":true,"
            "\"owners_acquired\":2,\"owners_released\":2,"
            "\"chain_allocations\":4,\"chain_deallocations\":4}");
}
'''


RECURSION_HARNESS = r'''
#undef cod_lang_region_new
#undef cod_lang_region_free
#undef cod_lang_exception_finish
#include <cassert>
static void *frames[5]{};
static unsigned live_frames = 0, frame_allocations = 0, frame_releases = 0;
extern "C" void *cod_lang_region_new(std::uint64_t capacity, std::uint64_t *status) {
  auto *owner = cod_lang_region_acquire(capacity, status);
  if (owner) {
    assert(capacity == 128 && live_frames < 5);
    frames[live_frames++] = owner;
    ++frame_allocations;
  }
  return owner;
}
extern "C" void cod_lang_region_free(void *owner) {
  if (!owner) return;
  assert(live_frames && frames[--live_frames] == owner);
  ++frame_releases;
  cod_lang_region_release(owner);
}
extern "C" void cod_lang_exception_finish() noexcept {
  assert(!live_frames && frame_allocations == 5 && frame_releases == 5);
  cod_lang_exception_release();
  std::puts("{\"recursive_frames\":5,\"released_once_in_reverse_order\":true}");
}
'''


def main():
    directory = Path(__file__).resolve().parent
    root = directory.parents[2]
    mode = os.environ.get("COD_TEST_TOOLCHAIN", "build")
    if mode not in ("build", "built"):
        raise SystemExit("COD_TEST_TOOLCHAIN must be build or built")
    toolchain = root / mode
    with tempfile.TemporaryDirectory(prefix="cod-exception-") as temporary:
        scratch = Path(temporary)
        output, evidence = run_case(root, toolchain, directory / "exception-allocation.cod", scratch, HARNESS,
                                   extra_wrappers="#define cod_lang_print_i64 cod_lang_trace_i64\n")
        assert output == ["10", "20", "30"], output
        print("PASS: injected provider OOM, finally/catch ordering and exact owner cleanup")
        output, chain = run_case(root, toolchain, directory / "exception-chain.cod", scratch, CHAIN_HARNESS,
                                 extra_wrappers="#define cod_lang_exception_finish cod_lang_exception_release\n")
        assert output == ["1"], output
        evidence["retained_chain"] = chain
        print("PASS: retained immutable causes, stored categories, relocated image and exit cleanup")
        fault = scratch / "chain-failure.cod"
        fault.write_text(CHAIN_FAILURE_SOURCE)
        output, chain_failure = run_case(root, toolchain, fault, scratch, CHAIN_FAILURE_HARNESS,
            extra_wrappers=("#define cod_lang_exception_finish cod_lang_exception_release\n"
                            "#define cod_lang_exception_chain cod_lang_exception_append\n"
                            "#define cod_lang_print_i64 cod_lang_trace_i64\n"))
        assert output == ["10"], output
        evidence["chain_allocation_failure"] = chain_failure
        print("PASS: real chain-allocation OOM, preserved prior chain, retry and exact cleanup")
        ir, runtime = scratch / "uncaught.ll", scratch / "uncaught-runtime.cc"
        uncaught_source = scratch / "uncaught-callables.cod"
        uncaught_source.write_text('function identity(n:u64)->const u64 { n }\n'
            'func module_value = identity;\n'
            'false && (func()->const bool { true })();\n' +
            (directory / "exception-reraise.cod").read_text().replace(
                'try {', 'try {\n  func local_value = module_value;', 1))
        checked([toolchain / "bin/cod", "--check", uncaught_source,
                 "--emit-llvm", ir, "--emit-runtime", runtime], root)
        source, executable = scratch / "uncaught.cc", scratch / "uncaught"
        source.write_text("#define cod_lang_region_free cod_lang_region_release\n"
            "#define cod_lang_callable_create cod_image_create\n"
            "#define cod_lang_callable_copy cod_image_copy\n"
            "#define cod_lang_callable_free cod_image_free\n" + runtime.read_text() + r'''
#undef cod_lang_region_free
#undef cod_lang_callable_create
#undef cod_lang_callable_copy
#undef cod_lang_callable_free
#include <cassert>
#include <set>
static std::set<void *> images;
static unsigned acquired = 0, released = 0;
static void *record_image(void *image) {
  if (image) { assert(images.insert(image).second); ++acquired; }
  return image;
}
extern "C" void *cod_lang_callable_create(std::uint64_t fixed, std::uint64_t count,
    void *const *children, const std::uint64_t *offsets, std::uint64_t *status) {
  return record_image(cod_image_create(fixed, count, children, offsets, status));
}
extern "C" void *cod_lang_callable_copy(const void *image, std::uint64_t *status) {
  return record_image(cod_image_copy(image, status));
}
extern "C" void cod_lang_callable_free(void *image) {
  if (!image) return;
  assert(images.erase(image) == 1);
  ++released;
  cod_image_free(image);
}
extern "C" void cod_lang_region_free(void *owner) {
  if (!owner) return;
  if (cod_lang_region_capacity(owner) == sizeof(CoDRegion) + 4096)
    assert(images.empty() && acquired >= 2 && released == acquired);
  std::printf("release %llu\n", static_cast<unsigned long long>(
      cod_lang_region_capacity(owner) - sizeof(CoDRegion)));
  cod_lang_region_release(owner);
}
''')
        checked([toolchain / "bin/clang++", "-std=c++20", "-I" + str(root / "include"),
                 ir, source, "-o", executable], root)
        uncaught = subprocess.run([str(executable)], text=True, capture_output=True, timeout=30)
        assert uncaught.returncode == 70 and "uncaught InvalidCapacity" in uncaught.stderr, uncaught
        assert uncaught.stdout.splitlines() == ["release 64", "10", "20", "release 4096"], uncaught.stdout
        evidence["uncaught"] = {"exit": uncaught.returncode, "trace": uncaught.stdout.splitlines(),
                                "stderr": uncaught.stderr, "callable_cleanup_before_module_region": True}
        print("PASS: uncaught rethrow runs local cleanup, finally blocks, callable release and module cleanup")
        loop = scratch / "uncaught-chain.cod"
        loop.write_text('''using "std:exceptions" SystemException;
using "std:exceptions" NullPointerException;
using symbol "std:io" @assert;
data Packet {value:u64;}
data Saved {error:SystemException;}
try {owned region invalid:Packet <- make Packet(capacity=0u64){value<-0u64}; {}}
catch(seed) {
  owned region saved:Saved <- make Saved(capacity=64u64){error<-seed};
  for(i in 0 .. 2) {{
    if(i == 1) {
      try {throw root(saved).error}
      catch(unexpected:const NullPointerException) {behave @assert(false);}
    } else {};
    val cause = try {
      owned region small:Packet <- make Packet(capacity=1u64){value<-0u64}; seed
    } catch(error) {error};
    val chained = try {throw seed from cause} catch(error) {error};
    setfield(root(saved), error, chained)
  }}
};
''')
        checked([toolchain / "bin/cod", "--check", loop, "--emit-llvm", ir, "--emit-runtime", runtime], root)
        source.write_text("#define cod_lang_region_free cod_lang_region_release\n" + runtime.read_text() + r'''
#undef cod_lang_region_free
#include <cassert>
static unsigned released = 0;
extern "C" void cod_lang_region_free(void *owner) {
  if (!owner) return;
  ++released;
  cod_lang_region_release(owner);
}
static const int audit = [] {
  return std::atexit([] {
    assert(released == 2 && !cod_causes);
    std::puts("uncaught chain cleanup passed");
  });
}();
''')
        checked([toolchain / "bin/clang++", "-std=c++20", "-I" + str(root / "include"),
                 ir, source, "-o", executable], root)
        uncaught_chain = subprocess.run([str(executable)], text=True, capture_output=True, timeout=30)
        diagnostic = uncaught_chain.stderr.splitlines()
        assert uncaught_chain.returncode == 70 and len(diagnostic) == 2, uncaught_chain
        assert "uncaught InvalidCapacity" in diagnostic[0] and "uncaught-chain.cod:6:" in diagnostic[0], diagnostic
        assert "caused by RegionCapacityExceeded" in diagnostic[1] and "uncaught-chain.cod:15:" in diagnostic[1], diagnostic
        assert uncaught_chain.stdout == "uncaught chain cleanup passed\n", uncaught_chain
        evidence["uncaught_chain"] = {"exit": 70, "diagnostics": diagnostic, "released_owners": 2,
                                      "later_emitted_cause_site_resolved": True, "chain_storage_released": True}
        print("PASS: uncaught chain sites from a preceding iteration and complete execution cleanup")
        output, recursive = run_case(root, toolchain, directory / "recursive-calls.cod", scratch,
            RECURSION_HARNESS, extra_wrappers=("#define cod_lang_region_new cod_lang_region_acquire\n"
                                              "#define cod_lang_exception_finish cod_lang_exception_release\n"))
        assert output == ["720", "true", "false", "16", "10", "1", "99", "100"], output
        evidence["recursive_calls"] = recursive
        print("PASS: recursive native frames, effect captures, checked NPE and exact LIFO region cleanup")
        rejects = {
            "non-exception-cause": ('region symbol arena; try {effect @arena <- region(0u64); {}} '
                                    'catch(error) {throw error from 42;};',
                                    "an explicit cause requires an intrinsic exception value"),
            "duplicate-category": ('using "std:exceptions" InvalidCapacity; using "std:exceptions" InvalidCapacity as Bad; '
                                   'try {} catch(e:const InvalidCapacity){} catch(e:const Bad){};',
                                   "duplicate exception category"),
            "catch-order": ('try {} catch(e){} catch(f){};', "catch-all must be the final catch clause"),
            "finally-value": ('try {1} finally {2};', "expected void, got i64"),
            "rethrow-context": ('throw;', "bare throw requires an active catch"),
            "function-effects": ('symbol x:i64; effect @x <- 1; '
                                 'function f() -> i64 {try {@x} finally {}}; f();',
                                 "a function cannot read symbolic effects"),
        }
        for name, (source, diagnostic) in rejects.items():
            path = scratch / (name + ".cod")
            path.write_text(source)
            result = subprocess.run([str(toolchain / "bin/cod"), "--check", str(path)],
                                    text=True, capture_output=True, timeout=30)
            assert result.returncode == 2 and diagnostic in result.stderr, (name, result.returncode, result.stderr)
            print("PASS: " + name)
        if report := os.environ.get("COD_EXCEPTION_REPORT"):
            evidence["recorded_utc"] = datetime.datetime.now(datetime.timezone.utc).isoformat()
            evidence["compile_rejections"] = list(rejects)
            evidence["compiler_sha256"] = hashlib.sha256((toolchain / "bin/cod").read_bytes()).hexdigest()
            evidence["source_sha256"] = {str(path.relative_to(root)): hashlib.sha256(path.read_bytes()).hexdigest()
                for path in [root / "cod/lang_exceptions.hh", root / "cod/lang_syntax.hh", root / "cod/lang_frontend.hh", root / "cod/lang_llvm.hh",
                             root / "cod/lang_native_runtime.hh", root / "include/shilos/region.hh",
                             directory / "exception-allocation.cod", directory / "exception-chain.cod",
                             directory / "exception-reraise.cod", directory / "recursive-calls.cod", directory / "regional_runtime_tests.py",
                             Path(__file__).resolve()]}
            Path(report).write_text(json.dumps(evidence, indent=2) + "\n")


if __name__ == "__main__":
    main()
