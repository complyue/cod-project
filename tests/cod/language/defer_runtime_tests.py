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
  if (value == 9041) { assert(records.empty()); phase_peak = 0; }
  if (value == 9042) assert(records.empty() && phase_peak == 1);
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


BLOCK_RETURNS = """
using "std:exceptions" MissingReturnException;
proc recovered_without_result() -> const u64 {
  defer catch(e:const InvalidCapacity) { behave @print(95u64); };
  bad();
}
try { behave @print(recovered_without_result()); }
catch(e:const MissingReturnException) { behave @print(96u64); };
proc failed_replacement() -> owned region :Packet {
  defer {bad();};
  defer {return make Packet(capacity=128u64){value<-999u64};};
  return make Packet(capacity=128u64){value<-999u64};
}
try { owned region failed <- failed_replacement(); }
catch(e:const InvalidCapacity) { behave @print(97u64); };
proc failed_callback() -> func()->const u64 {
  defer {bad();};
  defer {return func()->const u64 {return 999u64;};};
  return func()->const u64 {return 999u64;};
}
try { func callback_failure = failed_callback(); }
catch(e:const InvalidCapacity) { behave @print(98u64); };
{{
  defer {behave @print(100u64);};
  defer {defer {behave @print(99u64);};};
}};
proc repeated_owner() -> owned region :Packet {
  for(i in 0u64 .. 1000u64) {defer {return make Packet(capacity=128u64){value<-30u64};};};
  return make Packet(capacity=128u64){value<-10u64};
}
proc repeated_callback() -> func()->const u64 {
  for(i in 0u64 .. 1000u64) {defer {return func()->const u64 {return 60u64;};};};
  return func()->const u64 {return 40u64;};
}
{{ owned region result <- repeated_owner(); behave @print(result.value); }};
{{ func result = repeated_callback(); behave @print(result()); }};
"""


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
        for name in ('defer-order', 'defer-snapshots', 'defer-failures', 'defer-blocks', 'defer-returns', 'defer-errors',
                     'defer-checked', 'defer-value-scope', 'defer-category-flow', 'defer-initialization',
                     'defer-completion-flow', 'defer-handler-initialization', 'defer-queue-initialization',
                     'defer-borrowed-arguments', 'defer-callable-flow', 'defer-pointer-flow', 'loop-pointer-flow', 'defer-first-owner',
                     'defer-owner-queue', 'defer-owned-arguments'):
            fixture = directory / 'scopes' / (name + '.cod')
            source = fixture.read_text()
            if name == 'defer-failures':
                source += EXERCISE
            elif name == 'defer-returns':
                source += BLOCK_RETURNS
            else:
                source += '\nregion symbol @native_fixture; effect @native_fixture <- region(128u64);\n'
            if name == 'defer-value-scope':
                source += '''
using "std:exceptions" OutOfMemory;
behave @print(9001u64);
try {behave @print(value());} catch(e:const OutOfMemory) {behave @print(9002u64);};
'''
            if name == 'defer-completion-flow':
                source += '''
using "std:exceptions" OutOfMemory;
behave @print(9001u64);
try {after_scope();} catch(error:const OutOfMemory) {behave @print(9002u64);};
'''
            if name == 'defer-handler-initialization':
                source += '''
guarded(true);
behave @print(9002u64);
'''
            if name == 'defer-queue-initialization':
                source += '''
using "std:exceptions" OutOfMemory;
behave @print(9001u64);
try {choose(true);} catch(error:const OutOfMemory) {behave @print(9002u64);};
'''
            if name == 'defer-borrowed-arguments':
                source += '''
using "std:exceptions" OutOfMemory;
proc registration_failure()->void {
  val target:u64 <- 50u64;
  defer show(target);
  behave @print(9001u64);
  defer set(target, 999u64);
}
try {registration_failure();} catch(error:const OutOfMemory) {behave @print(9002u64);};
'''
            if name == 'defer-callable-flow':
                source += '''
using "std:exceptions" OutOfMemory;
proc failed_registration()->void {
  func action = one;
  {{
    defer catch(error:const OutOfMemory) {behave @print(action());};
    behave @print(9001u64);
    defer {action <- two;};
  }};
  behave @print(action());
}
failed_registration();
behave @print(9002u64);
proc failed_copy()->void {
  func action = one;
  func replacement = two;
  {{
    defer show(action);
    defer catch(error:const OutOfMemory) {behave @print(action());};
    defer {behave @print(9011u64); action <- replacement;};
  }};
  behave @print(action());
}
failed_copy();
behave @print(9012u64);
'''
            if name == 'defer-pointer-flow':
                source += '''
proc failed_registration()->void {
  val first:u64 <- 50u64;
  val second:u64 <- 51u64;
  ptr selected = first;
  defer {show(selected);};
  behave @print(9001u64);
  defer {selected = second;};
}
try {failed_registration();} catch(error:const OutOfMemory) {behave @print(9002u64);};
swap_repeated(10000u64);
'''
            if name == 'loop-pointer-flow':
                source += '''
proc failed_iteration()->void {
  val first:u64 <- 18u64;
  val second:u64 <- 19u64;
  ptr selected = first;
  try {for(i in 0u64 .. 3u64) {{
    behave @print(9001u64);
    defer {selected = second;};
  }};} catch(error:const OutOfMemory) {show(selected);};
}
failed_iteration();
behave @print(9002u64);
proc stress(count:u64)->void {
  val first:u64 <- 20u64;
  val second:u64 <- 21u64;
  ptr left = first;
  ptr right = second;
  for(i in 0u64 .. count) {{defer {left <=> right;};}};
  show(left);
  show(right);
}
behave @print(9041u64);
stress(10000u64);
behave @print(9042u64);
'''
            if name == 'defer-first-owner':
                source += '''
proc failed_first_owner()->void {
  val value:u64 <-26u64;
  ptr p:u64=null;
  defer catch(e:const OutOfMemory) {
    if(isnull(p)) {behave @print(27u64);} else {behave @print(999u64);};
  };
  behave @print(9001u64);
  defer {p=value;};
}
failed_first_owner();
behave @print(9002u64);
proc first_stress(count:u64)->void {
  val value:u64 <-28u64;
  ptr p:u64=null;
  for(i in 0u64 .. count) {{defer {p=value;};}};
  if(isnull(p)) {behave @print(999u64);} else {behave @print(p);};
}
behave @print(9041u64);
first_stress(10000u64);
behave @print(9042u64);
'''
            if name == 'defer-owner-queue':
                source += r'''
proc conditional_failure(yes:bool)->void {
  defer catch(e:const OutOfMemory) {behave @print(14u64);};
  if(yes) {
    owned region r:Item <-make Item(capacity=256u64){value<-99u64};
    behave @print(9001u64);
    defer show_region(r);
  } else {};
}
conditional_failure(true);
behave @print(9002u64);
proc verify_region(region r:Item)->void {
  if(root(r).value != 13u64) {behave @print(999u64);} else {};
}
proc conditional_stress(count:u64)->void {
  for(i in 0u64 .. count) {{
    if(i % 2u64 == 0u64) {
      owned region r:Item <-make Item(capacity=256u64){value<-13u64};
      defer verify_region(r);
    } else {};
  }};
}
behave @print(9041u64);
conditional_stress(10000u64);
behave @print(9042u64);
'''
            if name == 'defer-owned-arguments':
                source += r'''
proc registration_failure()->void {
  owned region r:Item <-make Item(capacity=256u64){value<-23u64};
  defer catch(e:const OutOfMemory) {consume(r);};
  behave @print(9001u64);
  defer consume(r);
}
registration_failure();
behave @print(9002u64);
proc consume_verify(owned region r:Item)->void {
  if(root(r).value != 22u64) {behave @print(999u64);} else {};
}
proc owned_stress(count:u64)->void {
  for(i in 0u64..count) {{
    owned region r:Item <-make Item(capacity=256u64){value<-22u64};
    defer consume_verify(r);
  }};
}
behave @print(9041u64);
owned_stress(10000u64);
behave @print(9042u64);
'''
            entry = scratch / 'program.cod'
            entry.write_text(source)
            output, evidence = run_case(root, root / mode, entry, scratch, HARNESS, extra_wrappers=wrappers,
                                        frontend_warnings=('[-Wmissing-return]',) if name in ('defer-blocks', 'defer-returns', 'defer-owned-arguments') else
                                        ('[-Wshadow]', ': note: shadowed binding declared here') if name == 'defer-first-owner' else ())
            expected = next(case['stdout'].splitlines() for case in cases if case['file'] == 'scopes/' + name + '.cod')
            if name == 'defer-failures':
                expected += ['9001', '81', '9002', '9011', '82', '9012', '84', '83', '85', '86', '87', '88', '89', '86', '90', '88', '91', '9021', '9022', '9031', '9032']
                assert evidence['layout']['record_failures'] == evidence['layout']['copy_failures'] == 1
            if name == 'defer-returns':
                expected += ['95', '96', '97', '98', '99', '100', '30', '60']
            if name in ('defer-value-scope', 'defer-completion-flow', 'defer-queue-initialization'):
                expected += ['9001', '9002']
                assert evidence['layout']['record_failures'] == 1
            if name == 'defer-handler-initialization':
                expected += ['9001', '70', '71', '9002']
                assert evidence['layout']['record_failures'] == 1
            if name == 'defer-borrowed-arguments':
                expected += ['9001', '50', '9002']
                assert evidence['layout']['record_failures'] == 1
            if name == 'defer-callable-flow':
                expected += ['9001', '1', '1', '9002', '9011', '1', '1', '1', '9012']
                assert evidence['layout']['record_failures'] == evidence['layout']['copy_failures'] == 1
            if name == 'defer-pointer-flow':
                expected += ['9001', '50', '9002', '14', '15']
                assert evidence['layout']['record_failures'] == 1
                assert evidence['layout']['peak_records'] == 10000
            if name == 'loop-pointer-flow':
                expected += ['9001', '18', '9002', '9041', '20', '21', '9042']
                assert evidence['layout']['record_failures'] == 1
                assert evidence['layout']['peak_records'] == 3
            if name == 'defer-first-owner':
                expected += ['9001', '27', '9002', '9041', '28', '9042']
                assert evidence['layout']['record_failures'] == 1
                assert evidence['layout']['peak_records'] == 4
            if name == 'defer-owner-queue':
                expected += ['9001', '14', '9002', '9041', '9042']
                assert evidence['layout']['record_failures'] == 1
                assert evidence['layout']['peak_records'] == 3
            if name == 'defer-owned-arguments':
                expected += ['9001', '23', '9002', '9041', '9042']
                assert evidence['layout']['record_failures'] == 1
            assert output == expected, (name, output, expected)
            evidence['fixture_sha256'] = hashlib.sha256(fixture.read_bytes()).hexdigest()
            evidence['executed_source_sha256'] = hashlib.sha256(source.encode()).hexdigest()
            results[name] = evidence
            print('PASS', name, evidence['layout'], flush=True)
    report = dict(schema_version=1, recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  toolchain=mode, passed=True, cases=results,
                  compiler_sha256=hashlib.sha256((root / mode / 'bin/cod').read_bytes()).hexdigest(),
                  script_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
                  harness_helper_sha256=hashlib.sha256((directory / 'regional_runtime_tests.py').read_bytes()).hexdigest())
    if path := os.environ.get('COD_DEFER_REPORT'):
        Path(path).write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
