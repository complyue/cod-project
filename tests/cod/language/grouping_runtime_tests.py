#!/usr/bin/env python3
"""Check shared groups, selected effects and common finally with native accounting."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import tempfile

from callable_runtime_tests import HARNESS as CALLABLE_HARNESS
from regional_runtime_tests import run_case

HARNESS = CALLABLE_HARNESS.split('extern "C" void cod_lang_exception_finish()')[0] + r'''
#undef cod_lang_region_new
static std::map<void *, std::uint64_t> regions;
static unsigned owners_acquired = 0, owners_released = 0, owners_failed = 0, peak_owners = 0;
extern "C" void *cod_lang_region_new(std::uint64_t bytes, std::uint64_t *status) {
  auto *owner = cod_lang_region_acquire(bytes, status);
  if (!owner) { ++owners_failed; return nullptr; }
  assert(regions.emplace(owner, bytes).second);
  ++owners_acquired;
  if (regions.size() > peak_owners) peak_owners = regions.size();
  return owner;
}
extern "C" void cod_lang_region_free(void *owner) {
  if (!owner) return;
  assert(regions.erase(owner) == 1);
  ++owners_released;
  cod_lang_region_release(owner);
}
extern "C" void cod_lang_exception_finish() noexcept {
  assert(regions.empty() && owners_acquired == owners_released);
  assert(owners_acquired == 6 && owners_failed == 4 && peak_owners == 2);
  assert(images.empty() && acquired == released && live_bytes == 0);
  cod_exception_release();
  std::printf("{\"owners_acquired\":%u,\"owners_released\":%u,\"failed_acquisitions\":%u,\"peak_live_owners\":%u,"
              "\"images_acquired\":%llu,\"images_released\":%llu,\"peak_image_bytes\":%llu}\n",
              owners_acquired, owners_released, owners_failed, peak_owners,
              static_cast<unsigned long long>(acquired), static_cast<unsigned long long>(released),
              static_cast<unsigned long long>(peak_bytes));
}
'''


def main():
    directory = Path(__file__).resolve().parent
    root = directory.parents[2]
    mode = os.environ.get("COD_TEST_TOOLCHAIN", "build")
    if mode not in ("build", "built"):
        raise SystemExit("COD_TEST_TOOLCHAIN must be build or built")
    fixture = directory / "scopes/grouping-flow.cod"
    wrappers = ("#define cod_lang_callable_create cod_image_create\n"
                "#define cod_lang_callable_copy cod_image_copy\n"
                "#define cod_lang_callable_free cod_image_free\n"
                "#define cod_lang_print_u64 cod_trace_u64\n"
                "#define cod_lang_region_new cod_lang_region_acquire\n"
                "#define cod_lang_exception_finish cod_exception_release\n")
    with tempfile.TemporaryDirectory(prefix="cod-grouping-") as temporary:
        trace, evidence = run_case(root, root / mode, fixture, Path(temporary), HARNESS, extra_wrappers=wrappers)
    expected = ["10", "11", "21", "50", "7", "60", "70", "8", "30", "31", "41", "1", "7", "60", "70", "8",
                "80", "81", "90", "80", "0", "81", "0"]
    assert trace == expected, trace
    sources = [root / "cod" / name for name in
               ("lang_exceptions.hh", "lang_syntax.hh", "lang_frontend.hh", "lang_llvm.hh", "lang_native_runtime.hh")]
    sources += [fixture, Path(__file__), directory / "callable_runtime_tests.py", directory / "regional_runtime_tests.py",
                root / "include/shilos/region.hh"]
    evidence.update(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(), toolchain=mode, trace=trace,
                    compiler_sha256=hashlib.sha256((root / mode / "bin/cod").read_bytes()).hexdigest(),
                    source_sha256={str(p.relative_to(root)): hashlib.sha256(p.read_bytes()).hexdigest() for p in sources})
    print("PASS: shared groups, selected region/object/callable effects, caught effects, common finally and exact release")
    if report := os.environ.get("COD_GROUPING_REPORT"):
        Path(report).write_text(json.dumps(evidence, indent=2) + "\n")


if __name__ == "__main__":
    main()
