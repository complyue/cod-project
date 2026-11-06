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


def checked(command, root):
    started = time.perf_counter()
    result = subprocess.run([str(x) for x in command], cwd=root, capture_output=True, text=True, timeout=120)
    seconds = time.perf_counter() - started
    if result.returncode or result.stderr:
        raise RuntimeError(f"{command}\nexit={result.returncode}\n{result.stdout}\n{result.stderr}")
    return result.stdout, seconds


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
        entry, ir, bitcode, runtime = [scratch / name for name in ("main.cod", "main.ll", "main.bc", "runtime.cc")]
        entry.write_text((directory / "region-records.cod").read_text() + "\nsetfield(@tail, next, @head);\n")
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
        source.write_text('#define cod_lang_region_free cod_lang_region_release\n' + runtime.read_text() + HARNESS)
        executable = scratch / "relocation"
        command = [toolchain / "bin/clang++", "-std=c++20", "-O2", "-g", "-UNDEBUG", "-stdlib=libc++",
                   "-fuse-ld=lld", "-I" + str(root / "include"), roundtrip, source, "-o", executable,
                   "-L" + str(toolchain / "lib"), "-Wl,-rpath," + str(toolchain / "lib")]
        _, compile_seconds = checked(command, root)
        output, process_seconds = checked([executable], root)
        lines = output.splitlines()
        if lines[:-1] != ["5.5", "10.5", "true"]:
            raise RuntimeError("The source fixture did not retain its expected result: " + output)
        evidence = {"schema_version": 2, "recorded_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                    "platform": platform.platform(), "toolchain": mode, "passed": True,
                    "llvm_artifact_roundtrip": True, "layout": json.loads(lines[-1]),
                    "frontend_seconds": frontend_seconds, "native_compile_link_seconds": compile_seconds,
                    "native_process_seconds": process_seconds, "llvm_ir_bytes": ir.stat().st_size,
                    "bitcode_bytes": bitcode.stat().st_size,
                    "notes": ["The CoD program creates the inspected records, array, sharing and cycle through LLVM IR.",
                              "The release hook copies the whole image, frees the original, then checks references and mutation.",
                              "Only system services and the independent layout/relocation check use C++."],
                    "source_sha256": {str(path.relative_to(root)): hashlib.sha256(path.read_bytes()).hexdigest()
                                      for path in [root / "cod/lang_syntax.hh", root / "cod/lang_frontend.hh", root / "cod/lang_llvm.hh",
                                                   root / "cod/lang_native_runtime.hh", root / "include/shilos/region.hh",
                                                   directory / "region-records.cod", Path(__file__).resolve()]}}
        if report := os.environ.get("COD_REGIONAL_REPORT"):
            Path(report).write_text(json.dumps(evidence, indent=2) + "\n")
        print(json.dumps(evidence, indent=2))


if __name__ == "__main__":
    main()
