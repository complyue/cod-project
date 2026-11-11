#!/usr/bin/env python3
"""Check LLVM artifact round-tripping and relocate an LLVM-created region image."""

import datetime
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import tempfile
import time


# The only hook runs when the generated program releases its region. The CoD
# program itself creates and mutates every object inspected here.
HARNESS = r'''
#undef cod_lang_region_free
#include <cassert>
#include <cstring>

struct Values {
  shilos::regional_ptr<float> data;
  std::uint64_t size;
  explicit Values(CoDRegion &) {}
  Values(const Values &) = delete;
  Values(Values &&) = delete;
};
struct Node {
  float value;
  shilos::regional_ptr<Node> next;
  shilos::regional_ptr<Values> data;
  explicit Node(CoDRegion &) {}
  Node(const Node &) = delete;
  Node(Node &&) = delete;
};
extern "C" void cod_lang_region_free(void *owner) {
  if (!owner) return;
  static bool released = false;
  assert(!released);
  released = true;
  auto &original = *static_cast<CoDRegion *>(owner);
  CoDRegionOwner relocated(8192);
  // Reproduce only allocation offsets with the same public allocator/layout.
  auto *values_layout = relocated->allocate(sizeof(Values), alignof(Values));
  relocated->allocate(2 * sizeof(float), alignof(float));
  auto *tail_layout = relocated->allocate(sizeof(Node), alignof(Node));
  auto *head_layout = relocated->allocate(sizeof(Node), alignof(Node));
  const auto base = reinterpret_cast<std::byte *>(&*relocated);
  const auto values_offset = static_cast<std::byte *>(values_layout) - base;
  const auto tail_offset = static_cast<std::byte *>(tail_layout) - base;
  const auto head_offset = static_cast<std::byte *>(head_layout) - base;
  const auto used = original.occupation();
  assert(used == relocated->occupation());
  assert(&original != &*relocated);
  std::memcpy(static_cast<void *>(&*relocated), static_cast<const void *>(&original), used);
  cod_lang_region_release(owner);
  const auto *head = reinterpret_cast<const Node *>(base + head_offset);
  const auto *tail = reinterpret_cast<const Node *>(base + tail_offset);
  const auto *values = reinterpret_cast<const Values *>(base + values_offset);
  assert(head->value == 3.0f && tail->value == 7.0f);
  assert(head->next.get() == tail && tail->next.get() == head);
  assert(head->data.get() == values && tail->data.get() == values);
  assert(values->size == 2 && values->data.get()[0] == 0.5f);
  auto *mutable_values = reinterpret_cast<Values *>(base + values_offset);
  mutable_values->data.get()[1] = 7.5f;
  assert(tail->data->data.get()[1] == 7.5f);
  std::printf("{\"relocation_passed\":true,\"relocated_bytes\":%llu,"
              "\"region_header_bytes\":%zu,\"array_header_bytes\":%zu,\"record_bytes\":%zu}\n",
              static_cast<unsigned long long>(used), sizeof(CoDRegion), sizeof(Values), sizeof(Node));
}
'''


POOL_HARNESS = r'''
#undef cod_lang_region_free
#include <cassert>
#include <cstring>

struct Payloads {
  shilos::regional_ptr<std::int64_t> data;
  std::uint64_t size;
  explicit Payloads(CoDRegion &) {}
  Payloads(const Payloads &) = delete;
  Payloads(Payloads &&) = delete;
};
struct Links {
  shilos::regional_ptr<std::uint32_t> data;
  std::uint64_t size;
  explicit Links(CoDRegion &) {}
  Links(const Links &) = delete;
  Links(Links &&) = delete;
};
struct Pool {
  std::uint32_t head;
  shilos::regional_ptr<Payloads> payloads;
  shilos::regional_ptr<Links> links;
  explicit Pool(CoDRegion &) {}
  Pool(const Pool &) = delete;
  Pool(Pool &&) = delete;
};
extern "C" void cod_lang_region_free(void *owner) {
  if (!owner) return;
  static unsigned failed_constructions = 0;
  auto &original = *static_cast<CoDRegion *>(owner);
  if (original.capacity() != sizeof(CoDRegion) + 8192) {
    constexpr std::uint64_t capacities[] = {23, 24, 72, 88};
    assert(failed_constructions < 4);
    assert(original.capacity() == sizeof(CoDRegion) + capacities[failed_constructions++]);
    cod_lang_region_release(owner);
    return;
  }
  assert(failed_constructions == 4);
  ++failed_constructions;
  CoDRegionOwner relocated(8192);
  auto *pool_layout = static_cast<void *>(relocated->root().get());
  relocated->allocate(sizeof(Pool) - sizeof(CoDRegionRoot), 1);
  auto *payloads_layout = relocated->allocate(sizeof(Payloads), alignof(Payloads));
  relocated->allocate(4 * sizeof(std::int64_t), alignof(std::int64_t));
  auto *links_layout = relocated->allocate(sizeof(Links), alignof(Links));
  relocated->allocate(4 * sizeof(std::uint32_t), alignof(std::uint32_t));
  auto *base = reinterpret_cast<std::byte *>(&*relocated);
  const auto payloads_offset = static_cast<std::byte *>(payloads_layout) - base;
  const auto links_offset = static_cast<std::byte *>(links_layout) - base;
  const auto pool_offset = static_cast<std::byte *>(pool_layout) - base;
  const auto used = original.occupation();
  assert(used == relocated->occupation());
  assert(&original != &*relocated);
  std::memcpy(static_cast<void *>(&*relocated), static_cast<const void *>(&original), used);
  cod_lang_region_release(owner);
  auto *pool = reinterpret_cast<Pool *>(base + pool_offset);
  assert(pool->payloads.get() == reinterpret_cast<Payloads *>(base + payloads_offset));
  assert(pool->links.get() == reinterpret_cast<Links *>(base + links_offset));
  assert(pool->payloads->size == 4 && pool->links->size == 4);
  assert(pool->payloads->data.get()[0] == 10 && pool->payloads->data.get()[2] == 12);
  assert(pool->payloads->data.get()[1] == -9999 && pool->payloads->data.get()[3] == 9999);
  // Independently consume the relocated free chain, without changing any pointer.
  for (const std::uint32_t expected : {3u, 1u}) {
    const auto slot = pool->head;
    assert(slot == expected);
    pool->head = pool->links->data.get()[slot];
    pool->payloads->data.get()[slot] = 100 + slot;
  }
  assert(pool->head == 4);
  assert(pool->payloads->data.get()[1] == 101 && pool->payloads->data.get()[3] == 103);
  assert(relocated->occupation() == used);
  std::printf("{\"relocation_passed\":true,\"relocated_bytes\":%llu,"
              "\"slots\":4,\"cod_reuse_laps\":10000,\"bounded_occupation\":true,"
              "\"failed_constructions_released\":4,\"typed_root_factory\":true,"
              "\"relocated_free_chain\":[3,1,4],\"independent_native_reuse_passed\":true}\n",
              static_cast<unsigned long long>(used));
}
'''


RESOURCE_HARNESS = r'''
#undef cod_lang_region_new
#undef cod_lang_region_free
#undef cod_lang_region_capacity
#include <cassert>

struct Resource {
  void *address = nullptr;
  bool live = false;
};
static Resource resources[4];
static unsigned acquired = 0, released = 0, inspected = 0;

static unsigned live_resource(void *owner) {
  // An allocator may reuse an address after an earlier loop iteration releases it.
  for (unsigned remaining = acquired; remaining; --remaining) {
    const auto i = remaining - 1;
    if (resources[i].address != owner) continue;
    assert(resources[i].live);
    return i;
  }
  assert(false && "operation on an unacquired region");
  return 0;
}
extern "C" void *cod_lang_region_new(std::uint64_t capacity, std::uint64_t *status) {
  assert(acquired < 4);
  if (acquired) assert(released == acquired - 1);
  void *owner = cod_lang_region_acquire(capacity, status);
  resources[acquired++] = {owner, true};
  return owner;
}
extern "C" std::uint64_t cod_lang_region_capacity(void *owner) {
  live_resource(owner);
  ++inspected;
  return cod_lang_region_inspect(owner);
}
extern "C" void cod_lang_region_free(void *owner) {
  // A transferred holder is empty; the service's null case releases nothing.
  if (!owner) return;
  const auto index = live_resource(owner);
  constexpr unsigned order[] = {1, 2, 3, 0};
  assert(released < 4 && index == order[released]);
  resources[index].live = false;
  ++released;
  cod_lang_region_release(owner);
  if (released == 4) {
    assert(acquired == 4 && inspected == 18);
    std::puts("{\"owners_acquired\":4,\"owners_released\":4,"
              "\"release_order\":[1,2,3,0],\"live_resource_reads\":18,"
              "\"loop_iterations\":3,\"untaken_branch_acquires_nothing\":true,"
              "\"captured_and_hidden_owners_survive_replay\":true}");
  }
}
'''


def checked(command, root):
    started = time.perf_counter()
    result = subprocess.run([str(x) for x in command], cwd=root, capture_output=True, text=True, timeout=120)
    seconds = time.perf_counter() - started
    if result.returncode or result.stderr:
        raise RuntimeError(f"{command}\nexit={result.returncode}\n{result.stdout}\n{result.stderr}")
    return result.stdout, seconds


def run_case(root, toolchain, entry, scratch, harness, capture_lifetimes=False, extra_wrappers=""):
    ir, bitcode, runtime = [scratch / name for name in ("main.ll", "main.bc", "runtime.cc")]
    _, frontend_seconds = checked([toolchain / "bin/cod", "--check", entry,
                                   "--emit-llvm", ir, "--emit-bc", bitcode, "--emit-runtime", runtime], root)
    roundtrip = scratch / "roundtrip.bc"
    disassembled = scratch / "roundtrip.ll"
    checked([toolchain / "bin/llvm-as", ir, "-o", roundtrip], root)
    checked([toolchain / "bin/llvm-dis", bitcode, "-o", disassembled], root)
    checked([toolchain / "bin/opt", "-passes=verify", "-disable-output", roundtrip], root)
    if "define i32 @main()" not in disassembled.read_text():
        raise RuntimeError("Bitcode did not retain the generated entry function")
    source = scratch / "relocation.cc"
    wrappers = '#define cod_lang_region_free cod_lang_region_release\n'
    if capture_lifetimes:
        wrappers += ('#define cod_lang_region_new cod_lang_region_acquire\n'
                     '#define cod_lang_region_capacity cod_lang_region_inspect\n')
    source.write_text(wrappers + extra_wrappers + runtime.read_text() + harness)
    executable = scratch / "relocation"
    command = [toolchain / "bin/clang++", "-std=c++20", "-O2", "-g", "-UNDEBUG", "-stdlib=libc++",
               "-fuse-ld=lld", "-I" + str(root / "include"), roundtrip, source, "-o", executable,
               "-L" + str(toolchain / "lib"), "-Wl,-rpath," + str(toolchain / "lib")]
    _, compile_seconds = checked(command, root)
    output, process_seconds = checked([executable], root)
    lines = output.splitlines()
    return lines[:-1], {"layout": json.loads(lines[-1]), "frontend_seconds": frontend_seconds,
                       "native_compile_link_seconds": compile_seconds, "native_process_seconds": process_seconds,
                       "llvm_ir_bytes": ir.stat().st_size, "bitcode_bytes": bitcode.stat().st_size}


def main():
    directory = Path(__file__).resolve().parent
    root = directory.parents[2]
    mode = os.environ.get("COD_TEST_TOOLCHAIN", "build")
    if mode not in ("build", "built"):
        raise SystemExit("COD_TEST_TOOLCHAIN must be build or built")
    toolchain = root / mode
    if os.name == "nt":
        raise SystemExit("This native layout measurement currently targets Linux/macOS")
    with tempfile.TemporaryDirectory(prefix="cod-regional-layout-") as temporary:
        scratch = Path(temporary)
        entry = scratch / "main.cod"
        entry.write_text((directory / "region-records.cod").read_text() + "\ntry {setfield(@tail, next, @head)} catch(e:const NullPointerException){behave @assert(false)};\n")
        output, records = run_case(root, toolchain, entry, scratch, HARNESS)
        if output != ["5.5", "10.5", "true"]:
            raise RuntimeError("The source fixture did not retain its expected result: " + repr(output))
        output, pool = run_case(root, toolchain, directory / "region-pool.cod", scratch, POOL_HARNESS)
        if output != [str(pool["layout"]["relocated_bytes"])]:
            raise RuntimeError("Pool occupation differs from independent native layout: " + repr(output))
        output, resources = run_case(root, toolchain, directory / "region-owner-capture.cod", scratch,
                                     RESOURCE_HARNESS, capture_lifetimes=True)
        if output:
            raise RuntimeError("Resource capture fixture produced unexpected output: " + repr(output))
        evidence = {"schema_version": 4, "recorded_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                    "platform": platform.platform(), "toolchain": mode, "passed": True,
                    "executable": {"path": str((toolchain / "bin/cod").relative_to(root)),
                                   "sha256": hashlib.sha256((toolchain / "bin/cod").read_bytes()).hexdigest()},
                    "llvm_artifact_roundtrip": True, **records, "pool": pool, "resources": resources,
                    "notes": ["The CoD program creates the inspected records, array, sharing and cycle through LLVM IR.",
                              "The graph retains a module-lifetime dynamic region; the pool owns a typed root returned by its factory and releases it at lexical scope exit.",
                              "The hooks copy each whole image and free its original before inspecting relocated references.",
                              "The CoD pool factory is selected with its result type through an adapter. Four partial-construction failures release once; successful typed-root construction supports 10000 reuse laps without allocation.",
                              "Abstract calls preserve the owner of returned payload references; alias writes are observed by later pool reads.",
                              "An independent C++ check traverses and reuses the relocated free list; no CoD code resumes after relocation.",
                              "Slot release is a user-library membership change. Returning a live slot exactly once is its caller precondition.",
                              "The existing resource-capture fixture is instrumented at allocation/read/release boundaries, with no compiler changes.",
                              "Three loop iterations preserve captured/hidden owners, restore the outer binding and release each inner owner before the next acquisition.",
                              "An untaken branch acquires no owner; the module owner releases last. Allocation-address reuse is allowed after release."],
                    "source_sha256": {str(path.relative_to(root)): hashlib.sha256(path.read_bytes()).hexdigest()
                                      for path in [root / "cod/lang_syntax.hh", root / "cod/lang_frontend.hh", root / "cod/lang_llvm.hh",
                                                   root / "cod/lang_native_runtime.hh", root / "include/shilos/region.hh",
                                                   directory / "region-records.cod", directory / "region-pool.cod",
                                                   directory / "reusable-pool.cod", directory / "pool-interface.cod",
                                                   directory / "pool-adapter.cod", directory / "pool-algorithms.cod",
                                                   directory / "region-owner-capture.cod",
                                                   Path(__file__).resolve()]}}
        if report := os.environ.get("COD_REGIONAL_REPORT"):
            Path(report).write_text(json.dumps(evidence, indent=2) + "\n")
        print(json.dumps(evidence, indent=2))


if __name__ == "__main__":
    main()
