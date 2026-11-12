#!/usr/bin/env python3
"""Measure iteration effect lifetimes and hidden-owner retention across exits."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import tempfile

from regional_runtime_tests import run_case

HARNESS = r'''
#undef cod_lang_region_new
#undef cod_lang_region_free
#undef cod_lang_print_i64
#include <cassert>
#include <set>
static std::set<void *> live;
static unsigned acquired = 0, released = 0, failed = 0, peak = 0;
extern "C" void *cod_lang_region_new(std::uint64_t bytes, std::uint64_t *status) {
  void *owner = cod_lang_region_acquire(bytes, status);
  if (!owner) { ++failed; return nullptr; }
  assert(live.insert(owner).second);
  ++acquired;
  if (live.size() > peak) peak = live.size();
  return owner;
}
extern "C" void cod_lang_region_free(void *owner) {
  if (!owner) return;
  assert(live.erase(owner) == 1);
  ++released;
  cod_lang_region_release(owner);
  if (live.empty()) {
    assert(acquired == 13 && released == acquired && failed == 2 && peak == 3);
    std::printf("{\"owners_acquired\":%u,\"owners_released\":%u,"
                "\"failed_acquisitions\":%u,\"peak_live_owners\":%u}\n",
                acquired, released, failed, peak);
  }
}
extern "C" void cod_lang_print_i64(std::int64_t owners) {
  assert(owners >= 1 && owners <= 3 && live.size() == static_cast<unsigned>(owners));
  cod_lang_trace_i64(owners);
}
'''


def main():
    directory = Path(__file__).resolve().parent
    root = directory.parents[2]
    mode = os.environ.get("COD_TEST_TOOLCHAIN", "build")
    if mode not in ("build", "built"):
        raise SystemExit("COD_TEST_TOOLCHAIN must be build or built")
    fixture = directory / "scopes/iteration-effects.cod"
    with tempfile.TemporaryDirectory(prefix="cod-loop-scopes-") as temporary:
        trace, evidence = run_case(root, root / mode, fixture, Path(temporary), HARNESS,
            extra_wrappers="#define cod_lang_region_new cod_lang_region_acquire\n"
                           "#define cod_lang_print_i64 cod_lang_trace_i64\n")
    assert trace == ["3", "3", "2"] * 3 + ["1", "2", "1", "2", "1", "2", "1"], trace
    sources = [root / "cod" / name for name in
               ("lang_exceptions.hh", "lang_syntax.hh", "lang_frontend.hh", "lang_llvm.hh", "lang_native_runtime.hh")]
    sources += [fixture, Path(__file__), directory / "regional_runtime_tests.py", root / "include/shilos/region.hh"]
    evidence.update(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(), toolchain=mode,
                    trace=trace, compiler_sha256=hashlib.sha256((root / mode / "bin/cod").read_bytes()).hexdigest(),
                    source_sha256={str(p.relative_to(root)): hashlib.sha256(p.read_bytes()).hexdigest() for p in sources})
    print("PASS: nested iteration and zero-iteration else effects; normal/throw release; captured/hidden owners")
    if report := os.environ.get("COD_LOOP_SCOPE_REPORT"):
        Path(report).write_text(json.dumps(evidence, indent=2) + "\n")


if __name__ == "__main__":
    main()
