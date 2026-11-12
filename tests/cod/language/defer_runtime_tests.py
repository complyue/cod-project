#!/usr/bin/env python3
"""Measure deferred registrations, complete callable snapshots and failure cleanup."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import tempfile

from regional_runtime_tests import run_case

HARNESS = r'''
#undef cod_lang_defer_allocate
#undef cod_lang_defer_free
#undef cod_lang_callable_create
#undef cod_lang_callable_copy
#undef cod_lang_callable_free
#undef cod_lang_region_new
#undef cod_lang_region_free
#undef cod_lang_print_u64
#undef cod_lang_exception_finish
#include <cassert>
#include <map>
static std::map<void *, std::uint64_t> records, images, regions;
static std::uint64_t owners = 0, owners_released = 0;
static std::uint64_t registrations = 0, drained = 0, created = 0, released = 0;
static std::uint64_t live_bytes = 0, peak_bytes = 0, peak_records = 0;
static std::uint64_t phase_peak = 0, observed = 0, expected = 0;
static unsigned phase = 0, allocation_failures = 0, copy_failures = 0;
static bool fail_record = false, fail_copy = false;
static void *image(void *value) {
  if (value) { assert(images.emplace(value, *static_cast<std::uint64_t *>(value)).second); ++created; }
  return value;
}
extern "C" void *cod_lang_callable_create(std::uint64_t bytes, std::uint64_t count,
    void *const *children, const std::uint64_t *offsets, std::uint64_t *status) {
  return image(cod_image_create(bytes, count, children, offsets, status));
}
extern "C" void *cod_lang_callable_copy(const void *value, std::uint64_t *status) {
  if (fail_copy) { fail_copy = false; ++copy_failures; status[0] = 1; return nullptr; }
  return image(cod_image_copy(value, status));
}
extern "C" void cod_lang_callable_free(void *value) {
  if (!value) return;
  assert(images.erase(value) == 1); ++released;
  cod_image_free(value);
}
extern "C" void *cod_lang_defer_allocate(std::uint64_t bytes, std::uint64_t *status) {
  if (fail_record) { fail_record = false; ++allocation_failures; status[0] = 1; return nullptr; }
  auto *value = cod_record_allocate(bytes, status);
  if (!value) return value;
  assert(records.emplace(value, bytes).second);
  ++registrations; live_bytes += bytes;
  peak_bytes = std::max(peak_bytes, live_bytes);
  peak_records = std::max(peak_records, std::uint64_t(records.size()));
  phase_peak = std::max(phase_peak, std::uint64_t(records.size()));
  return value;
}
extern "C" void cod_lang_defer_free(void *value) {
  const auto found = records.find(value);
  assert(found != records.end());
  live_bytes -= found->second; records.erase(found); ++drained;
  cod_record_free(value);
}
extern "C" void *cod_lang_region_new(std::uint64_t capacity, std::uint64_t *status) {
  auto *owner = cod_region_allocate(capacity, status);
  if (owner) { assert(regions.emplace(owner, capacity).second); ++owners; }
  return owner;
}
extern "C" void cod_lang_region_free(void *value) {
  if (value) { assert(regions.erase(value) == 1); ++owners_released; }
  cod_lang_region_release(value);
}
extern "C" void cod_lang_print_u64(std::uint64_t value) {
  if (phase && value >= 10000) {
    assert(value == expected); ++observed;
    if (phase == 1) --expected; else ++expected;
    return;
  }
  if (value == 9001) fail_record = true;
  if (value == 9002) assert(allocation_failures == 1 && records.empty());
  if (value == 9011) fail_copy = true;
  if (value == 9012) assert(copy_failures == 1 && records.empty());
  if (value == 9021 || value == 9031) {
    assert(records.empty()); phase = value == 9021 ? 1 : 2;
    phase_peak = observed = 0; expected = phase == 1 ? 19999 : 10000;
  }
  if (value == 9022 || value == 9032) {
    assert(records.empty() && observed == 10000);
    assert(phase_peak == (phase == 1 ? 10000 : 1)); phase = 0;
  }
  cod_trace_u64(value);
}
extern "C" void cod_lang_exception_finish() {
  assert(records.empty() && images.empty() && regions.empty() && live_bytes == 0);
  assert(owners == owners_released);
  assert(registrations == drained && created == released);
  cod_exception_release();
  std::printf("{\"registrations\":%llu,\"drained\":%llu,\"peak_records\":%llu,"
              "\"peak_record_bytes\":%llu,\"images\":%llu,\"images_released\":%llu,"
              "\"record_failures\":%u,\"copy_failures\":%u,\"owners\":%llu,\"owners_released\":%llu}\n",
              (unsigned long long)registrations, (unsigned long long)drained,
              (unsigned long long)peak_records, (unsigned long long)peak_bytes,
              (unsigned long long)created, (unsigned long long)released, allocation_failures, copy_failures,
              (unsigned long long)owners, (unsigned long long)owners_released);
}
'''

EXERCISE = '''
using "std:exceptions" OutOfMemory;
proc record_failure() -> void {
  proc selected = proc(x:u64)->void { behave @print(x); };
  defer good(81u64);
  behave @print(9001u64);
  defer selected(999u64);
}
proc later() -> const u64 { behave @print(999u64); return 0u64; }
proc snapshot_failure() -> void {
  proc selected = proc(x:u64)->void { behave @print(x); };
  defer good(82u64);
  behave @print(9011u64);
  defer selected(later());
}
try { record_failure(); } catch(e:const OutOfMemory) { behave @print(9002u64); };
try { snapshot_failure(); } catch(e:const OutOfMemory) { behave @print(9012u64); };
try {{
  defer good(83u64);
  defer use(proc(x:u64)->void { bad(x, 0u64); }, 84u64);
}} catch(e:const InvalidCapacity) { behave @print(85u64); };
proc owner_result(mode:u64) -> owned region :Packet {
  defer bad(86u64, if(mode == 1u64) {0u64} else {128u64});
  return make Packet(capacity=128u64){value<-87u64};
}
proc callable_result(mode:u64) -> func()->const u64 {
  defer bad(88u64, if(mode == 1u64) {0u64} else {128u64});
  return func()->const u64 { return 89u64; };
}
for(i in 0u64 .. 2u64) {{
  try { owned region result <- owner_result(i); behave @print(result.value); }
  catch(e:const InvalidCapacity) { behave @print(90u64); };
  try { func callback = callable_result(i); behave @print(callback()); }
  catch(e:const InvalidCapacity) { behave @print(91u64); };
}};
behave @print(9021u64);
{{ for(i in 0u64 .. 10000u64) { defer good(10000u64 + i); }; }};
behave @print(9022u64);
behave @print(9031u64);
for(i in 0u64 .. 10000u64) {{ defer good(10000u64 + i); }};
behave @print(9032u64);
'''


def main():
    directory = Path(__file__).resolve().parent
    root = directory.parents[2]
    mode = os.environ.get('COD_TEST_TOOLCHAIN', 'build')
    if mode not in ('build', 'built'):
        raise SystemExit('COD_TEST_TOOLCHAIN must be build or built')
    cases = json.loads((directory / 'cases.json').read_text())
    wrappers = ''.join(f'#define {name} {replacement}\n' for name, replacement in (
        ('cod_lang_defer_allocate', 'cod_record_allocate'), ('cod_lang_defer_free', 'cod_record_free'),
        ('cod_lang_callable_create', 'cod_image_create'), ('cod_lang_callable_copy', 'cod_image_copy'),
        ('cod_lang_callable_free', 'cod_image_free'), ('cod_lang_print_u64', 'cod_trace_u64'),
        ('cod_lang_exception_finish', 'cod_exception_release'), ('cod_lang_region_new', 'cod_region_allocate')))
    results = {}
    with tempfile.TemporaryDirectory(prefix='cod-defer-native-') as temporary:
        scratch = Path(temporary)
        for name in ('defer-order', 'defer-snapshots', 'defer-failures'):
            fixture = directory / 'scopes' / (name + '.cod')
            source = fixture.read_text()
            if name == 'defer-failures':
                source += EXERCISE
            else:
                source += '\nregion symbol @native_fixture; effect @native_fixture <- region(128u64);\n'
            entry = scratch / 'program.cod'
            entry.write_text(source)
            output, evidence = run_case(root, root / mode, entry, scratch, HARNESS, extra_wrappers=wrappers)
            expected = next(case['stdout'].splitlines() for case in cases if case['file'] == 'scopes/' + name + '.cod')
            if name == 'defer-failures':
                expected += ['9001', '81', '9002', '9011', '82', '9012', '84', '83', '85', '86', '87', '88', '89', '86', '90', '88', '91', '9021', '9022', '9031', '9032']
                assert evidence['layout']['record_failures'] == evidence['layout']['copy_failures'] == 1
            assert output == expected, (name, output, expected)
            evidence['fixture_sha256'] = hashlib.sha256(fixture.read_bytes()).hexdigest()
            evidence['executed_source_sha256'] = hashlib.sha256(source.encode()).hexdigest()
            results[name] = evidence
            print('PASS', name, evidence['layout'], flush=True)
    report = dict(schema_version=1, recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  toolchain=mode, passed=True, cases=results,
                  compiler_sha256=hashlib.sha256((root / mode / 'bin/cod').read_bytes()).hexdigest(),
                  script_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest())
    if path := os.environ.get('COD_DEFER_REPORT'):
        Path(path).write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
