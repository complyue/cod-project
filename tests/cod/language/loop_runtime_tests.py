#!/usr/bin/env python3
"""Measure scope queue growth and inject native record-allocation failures."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
import time

from foundation_tests import ROOT, MODE, COD
from regional_runtime_tests import checked

WRAPPERS = """
#define cod_lang_defer_allocate cod_record_allocate
#define cod_lang_defer_free cod_record_free
#define cod_lang_print_i64 cod_trace_i64
#define cod_lang_exception_finish cod_exception_release
#define cod_lang_uncaught cod_uncaught
"""
HARNESS = r'''
#undef cod_lang_defer_allocate
#undef cod_lang_defer_free
#undef cod_lang_print_i64
#undef cod_lang_exception_finish
#undef cod_lang_uncaught
#include <cassert>
#include <map>
static std::map<void *, std::uint64_t> records;
static std::uint64_t attempts = 0, registered = 0, drained = 0, observed = 0;
static std::uint64_t live_bytes = 0, peak_bytes = 0, peak_records = 0, record_bytes = 0;
extern "C" void *cod_lang_defer_allocate(std::uint64_t bytes, std::uint64_t *status) {
  ++attempts;
  if (attempts == FAIL_ON) { status[0] = 1; return nullptr; }
  auto *record = cod_record_allocate(bytes, status);
  assert(record && records.emplace(record, bytes).second);
  if (!record_bytes) record_bytes = bytes;
  assert(bytes == record_bytes);
  ++registered; live_bytes += bytes;
  peak_bytes = std::max(peak_bytes, live_bytes);
  peak_records = std::max(peak_records, std::uint64_t(records.size()));
  return record;
}
extern "C" void cod_lang_defer_free(void *record) {
  const auto found = records.find(record);
  assert(found != records.end());
  live_bytes -= found->second; records.erase(found); ++drained;
  cod_record_free(record);
}
extern "C" void cod_lang_print_i64(std::int64_t value) {
  const auto expected = EXPECTED_VALUE;
  assert(value == expected);
  ++observed;
}
extern "C" void cod_lang_exception_finish() noexcept {
  assert(records.empty() && live_bytes == 0 && registered == drained);
  assert(registered == EXPECTED_REGISTRATIONS && observed == registered);
  assert(peak_records == EXPECTED_PEAK && peak_bytes == peak_records * record_bytes);
  assert(attempts == registered + (FAIL_ON ? 1 : 0));
  cod_exception_release();
  std::printf("{\"registrations\":%llu,\"drained\":%llu,\"peak_records\":%llu,"
              "\"peak_bytes\":%llu,\"record_bytes\":%llu,\"allocation_attempts\":%llu}\n",
              (unsigned long long)registered, (unsigned long long)drained,
              (unsigned long long)peak_records, (unsigned long long)peak_bytes,
              (unsigned long long)record_bytes, (unsigned long long)attempts);
}
extern "C" [[noreturn]] void cod_lang_uncaught(std::uint64_t category, std::uint64_t site,
    std::uint64_t requested, std::uint64_t available, std::uint64_t cause) {
  cod_lang_exception_finish();
  cod_uncaught(category, site, requested, available, cause);
}
'''


def main():
    toolchain = ROOT / MODE
    results, batch_bodies = [], {}
    cases = [("batch_small", 10, False, 0, False), ("batch_large", 10000, False, 0, False),
             ("per_iteration", 10000, True, 0, False), ("allocation_failure", 10, False, 4, False),
             ("block_batch", 10000, False, 0, True), ("block_iteration", 10000, True, 0, True),
             ("block_allocation_failure", 10, False, 4, True), ("block_during_cleanup", 10, False, 3, True)]
    with tempfile.TemporaryDirectory(prefix="cod-loop-runtime-") as directory:
        scratch = Path(directory)
        for name, count, local, fail_on, delayed in cases:
            for general in (False, True):
                label = f"{name}/{'general' if general else 'default'}"
                path = scratch / "main.cod"
                loop = 'for(i in 0..n){{defer consume(i);}};' if local else 'defer consume(-1);for(i in 0..n){defer consume(i);};defer consume(-2);'
                if delayed:
                    loop = ('for(i in 0..n){{val copy=i;defer{consume(copy);};}};' if local else
                            'val current=n;defer{consume(-1);};for(i in 0..n){defer{current<-current-1;consume(current);};};defer{consume(-2);};')
                if name == "block_during_cleanup":
                    loop = 'defer{consume(-1);};defer{consume(42);defer{consume(-2);};};'
                path.write_text('using symbol "std:io" @print;\nproc consume(x:T)->void where T {behave @print(x);}\n'
                                'proc batch(n:i64)->void {' + loop + '}\nbatch(' + str(count) + ');\n')
                ir, bc, runtime = [scratch / item for item in ("main.ll", "main.bc", "runtime.cc")]
                command = [COD, "--check", path, "--emit-llvm", ir, "--emit-bc", bc, "--emit-runtime", runtime]
                if general:
                    command.append("--no-specialize")
                output, frontend_seconds = checked(command, ROOT)
                if output:
                    raise RuntimeError("Checking executed runtime effects")
                roundtrip = scratch / "roundtrip.bc"
                checked([toolchain / "bin/llvm-as", ir, "-o", roundtrip], ROOT)
                checked([toolchain / "bin/opt", "-passes=verify", "-disable-output", roundtrip], ROOT)
                if name.startswith("batch_"):
                    body = re.search(r'define internal %cod.outcome @cod.method.1.batch.native\([^\n]*\) \{\n(.*?)\n\}', ir.read_text(), re.S).group(1)
                    if name == "batch_small":
                        batch_bodies[general] = body
                    elif body != batch_bodies[general]:
                        raise RuntimeError("Runtime registration count changed the compiled batch body")
                registrations = fail_on - 1 if fail_on else count if local else count + 2
                peak = 1 if local else registrations
                expected = ('std::int64_t(observed)' if local else
                            f'std::int64_t({registrations}) - 2 - std::int64_t(observed)' if fail_on else
                            f'(observed == 0 ? -2 : observed == {count + 1} ? -1 : std::int64_t({count}) - std::int64_t(observed))')
                if delayed and fail_on:
                    expected = f'(observed == {registrations - 1} ? -1 : std::int64_t({count - 1}) - std::int64_t(observed))'
                if name == "block_during_cleanup":
                    expected = '(observed == 0 ? 42 : -1)'
                defines = f'#define FAIL_ON {fail_on}\n#define EXPECTED_REGISTRATIONS {registrations}\n#define EXPECTED_PEAK {peak}\n#define EXPECTED_VALUE {expected}\n'
                source = scratch / "instrumented.cc"
                source.write_text(defines + WRAPPERS + runtime.read_text() + HARNESS)
                executable = scratch / "native"
                compile_command = [toolchain / "bin/clang++", "-std=c++20", "-O2", "-g", "-UNDEBUG", "-stdlib=libc++",
                                   "-fuse-ld=lld", "-I" + str(ROOT / "include"), bc, source, "-o", executable,
                                   "-L" + str(toolchain / "lib"), "-Wl,-rpath," + str(toolchain / "lib")]
                _, compile_seconds = checked(compile_command, ROOT)
                started = time.perf_counter()
                run = subprocess.run([executable], cwd=ROOT, capture_output=True, text=True, timeout=60)
                seconds = time.perf_counter() - started
                expected_exit = 70 if fail_on else 0
                if run.returncode != expected_exit or ("uncaught OutOfMemory" not in run.stderr if fail_on else run.stderr):
                    raise RuntimeError(f"{label}: exit={run.returncode}\n{run.stdout}\n{run.stderr}")
                metrics = json.loads(run.stdout)
                results.append({"name": label, "passed": True, "metrics": metrics, "frontend_seconds": frontend_seconds,
                                "compile_seconds": compile_seconds, "execution_seconds": seconds,
                                "ir_bytes": ir.stat().st_size, "exit": run.returncode, "stderr": run.stderr,
                                "check_command": list(map(str, command)), "compile_command": list(map(str, compile_command))})
                print(f"PASS {label}: {json.dumps(metrics)}", flush=True)
    if report := os.environ.get("COD_LANGUAGE_REPORT"):
        sources = [ROOT / "cod" / name for name in ("main.cc", "lang_checked.hh", "lang_completion.hh", "lang_diagnostics.hh", "lang_frontend.hh", "lang_llvm.hh", "lang_syntax.hh", "lang_exceptions.hh", "lang_native_runtime.hh")]
        sources += [Path(__file__), Path(__file__).with_name("foundation_tests.py"), Path(__file__).with_name("regional_runtime_tests.py")]
        Path(report).write_text(json.dumps({"recorded_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                                         "toolchain": MODE, "passed": len(results), "total": len(results), "results": results,
                                         "executable_sha256": hashlib.sha256(COD.read_bytes()).hexdigest(),
                                         "sources": {str(path.relative_to(ROOT)): hashlib.sha256(path.read_bytes()).hexdigest() for path in sources}}, indent=2) + "\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
