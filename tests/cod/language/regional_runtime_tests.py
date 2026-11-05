#!/usr/bin/env python3
"""Validate emitted regional layouts and relocation without a second allocator."""

import datetime
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import tempfile
import time


HARNESS = r'''
#undef main
#include <cassert>
#include <cstring>
#include <iostream>

int main() {
  if (cod_fixture_main() != 0) return 1;
  static_assert(!std::is_copy_constructible_v<Record0>);
  static_assert(!std::is_move_constructible_v<Record0>);
  static_assert(!std::is_copy_constructible_v<CoDArray<float>>);
  static_assert(sizeof(shilos::regional_ptr<float>) == sizeof(std::intptr_t));
  CoDRegionOwner relocated(4096);
  std::uint64_t head_offset{}, array_offset{}, used{};
  {
    CoDRegionOwner original(4096);
    auto values = original->create<CoDArray<float>>(3, 0.25f);
    array_offset = region_offset(*original, values.get());
    auto tail = original->create<Record0>(2.0f, 0, array_offset);
    auto head = original->create<Record0>(3.0f, region_offset(*original, tail.get()), array_offset);
    head_offset = region_offset(*original, head.get());
    head->f1.get()->f1 = head;
    used = original->occupation();
    assert(&*original != &*relocated);
    // Relocate the complete region image; never copy individual regional objects.
    std::memcpy(static_cast<void *>(&*relocated), static_cast<const void *>(&*original), used);
  }
  const auto head = region_pointer<Record0>(*relocated, head_offset);
  assert(head->f0 == 3.0f && head->f1->f0 == 2.0f);
  assert(head->f1->f1.get() == head.get());
  const auto array = region_pointer<CoDArray<float>>(*relocated, array_offset);
  assert(head->f2.get() == array.get());
  assert(head->f1->f2.get() == array.get());
  assert(array->size == 3 && array->data.get()[2] == 0.25f);
  array_store<float>(*relocated, array_offset, 1, 7.5f);
  assert(head->f1->f2->data.get()[1] == 7.5f);

  CoDRegionOwner pool(32768);
  auto ids = pool->create<CoDArray<std::uint32_t>>(2064, 0);
  auto weights = pool->create<CoDArray<std::uint8_t>>(2064, 1);
  auto frame_a = pool->create<CoDArray<float>>(15, 0.0f);
  auto frame_b = pool->create<CoDArray<float>>(15, 0.0f);
  assert(sizeof(*ids->data.get()) == 4 && sizeof(*weights->data.get()) == 1);
  assert(reinterpret_cast<std::uintptr_t>(ids->data.get()) % alignof(std::uint32_t) == 0);
  assert(reinterpret_cast<std::uintptr_t>(frame_a->data.get()) % alignof(float) == 0);
  assert(frame_a->data.get() != frame_b->data.get());
  const std::uint64_t payload = 2064 * 5 + 15 * 8;
  std::cout << "{\"relocation_passed\":true,\"relocated_bytes\":" << used
            << ",\"slot_payload_bytes\":" << 2064 * 5
            << ",\"two_frames_bytes\":" << 15 * 8
            << ",\"region_header_bytes\":" << sizeof(CoDRegion)
            << ",\"array_header_bytes\":" << sizeof(CoDArray<float>)
            << ",\"pool_occupation_bytes\":" << pool->occupation()
            << ",\"non_element_bytes\":" << pool->occupation() - payload
            << "}\n";
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
    cod = toolchain / "bin/cod"
    compiler = toolchain / "bin/clang++"
    if os.name == "nt":
        raise SystemExit("This native layout measurement currently targets Linux/macOS")
    with tempfile.TemporaryDirectory(prefix="cod-regional-layout-") as temporary:
        scratch = Path(temporary)
        generated = scratch / "generated.cc"
        _, frontend_seconds = checked([cod, "--check", directory / "region-records.cod",
                                        "--emit-cpp", generated], root)
        source = scratch / "relocation.cc"
        source.write_text('#define main cod_fixture_main\n' + generated.read_text() + HARNESS)
        executable = scratch / "relocation"
        command = [compiler, "-std=c++20", "-O2", "-g", "-UNDEBUG", "-stdlib=libc++",
                   "-fuse-ld=lld", "-I" + str(root / "include"), source, "-o", executable,
                   "-L" + str(toolchain / "lib"), "-Wl,-rpath," + str(toolchain / "lib")]
        _, compile_seconds = checked(command, root)
        output, process_seconds = checked([executable], root)
        lines = output.splitlines()
        if lines[:-1] != ["5.25", "10.25", "true"]:
            raise RuntimeError("The source fixture did not retain its expected result: " + output)
        evidence = {"schema_version": 1, "recorded_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                    "platform": platform.platform(), "toolchain": mode, "passed": True,
                    "layout": json.loads(lines[-1]), "frontend_seconds": frontend_seconds,
                    "native_compile_link_seconds": compile_seconds, "native_process_seconds": process_seconds,
                    "generated_cpp_bytes": generated.stat().st_size, "native_harness_bytes": executable.stat().st_size,
                    "notes": ["Native size includes debug information and the relocation harness.",
                              "Timing includes process startup; it is not a simulator throughput result.",
                              "Non-element bytes include region/array headers, root and allocation alignment."],
                    "source_sha256": {str(path.relative_to(root)): hashlib.sha256(path.read_bytes()).hexdigest()
                                      for path in [root / "cod/lang_frontend.hh", root / "cod/lang_region_runtime.hh",
                                                   root / "include/shilos/region.hh", directory / "region-records.cod",
                                                   Path(__file__).resolve()]}}
        if report := os.environ.get("COD_REGIONAL_REPORT"):
            Path(report).write_text(json.dumps(evidence, indent=2) + "\n")
        print(json.dumps(evidence, indent=2))


if __name__ == "__main__":
    main()
