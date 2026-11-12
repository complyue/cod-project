#!/usr/bin/env python3
"""Check complete callable copies, failure-safe replacement and allocation-free calls."""
import hashlib
import json
import os
from pathlib import Path
import re
import tempfile

from regional_runtime_tests import run_case

HARNESS = r'''
#undef cod_lang_callable_create
#undef cod_lang_callable_copy
#undef cod_lang_callable_free
#undef cod_lang_print_u64
#undef cod_lang_region_free
#undef cod_lang_exception_finish
#include <cassert>
#include <map>
static std::map<void *, std::uint64_t> images;
static std::uint64_t acquired = 0, released = 0, live_bytes = 0, peak_bytes = 0, call_allocations = 0;
static std::uint64_t before_failure = 0, before_later_argument = 0;
static unsigned failures = 0, construction_failures = 0;
static unsigned recursive_copy_countdown = 0, recursive_copy_failures = 0;
static unsigned recursive_create_countdown = 0, recursive_create_failures = 0;
static bool fail_copy = false, fail_create = false, checked_calls = false;
static void *record_image(void *image) {
  if (!image) return image;
  const auto size = *static_cast<const std::uint64_t *>(image);
  assert(images.emplace(image, size).second);
  ++acquired;
  live_bytes += size;
  if (live_bytes > peak_bytes) peak_bytes = live_bytes;
  return image;
}
extern "C" void *cod_lang_callable_create(std::uint64_t fixed, std::uint64_t count,
    void *const *children, const std::uint64_t *offsets, std::uint64_t *status) {
  if (recursive_create_countdown && !--recursive_create_countdown) {
    ++recursive_create_failures; status[0] = 1; return nullptr;
  }
  if (fail_create) { fail_create = false; ++construction_failures; status[0] = 1; return nullptr; }
  return record_image(cod_image_create(fixed, count, children, offsets, status));
}
extern "C" void *cod_lang_callable_copy(const void *source, std::uint64_t *status) {
  const auto address = reinterpret_cast<std::uintptr_t>(source);
  bool live = false;
  for (const auto &[image, bytes] : images) {
    const auto base = reinterpret_cast<std::uintptr_t>(image);
    live |= address >= base && address < base + bytes;
  }
  assert(live);
  if (recursive_copy_countdown && !--recursive_copy_countdown) {
    ++recursive_copy_failures; status[0] = 1; return nullptr;
  }
  if (fail_copy) { fail_copy = false; ++failures; status[0] = 1; return nullptr; }
  return record_image(cod_image_copy(source, status));
}
extern "C" void cod_lang_callable_free(void *image) {
  if (!image) return;
  const auto found = images.find(image);
  assert(found != images.end());
  live_bytes -= found->second;
  images.erase(found);
  ++released;
  cod_image_free(image);
}
extern "C" void cod_lang_print_u64(std::uint64_t value) {
  if (value == 6001) { fail_copy = true; before_failure = live_bytes; }
  if (value == 6002) assert(live_bytes == before_failure);
  if (value == 6101) before_later_argument = live_bytes;
  if (value == 6102) assert(live_bytes == before_later_argument);
  if (value == 6201) { fail_create = true; before_failure = live_bytes; }
  if (value == 6202) assert(construction_failures == 1 && live_bytes == before_failure);
  if (value == 6301) { recursive_copy_countdown = 3; before_failure = live_bytes; }
  if (value == 6302) assert(recursive_copy_failures == 1 && live_bytes == before_failure);
  if (value == 6401) { recursive_create_countdown = 3; before_failure = live_bytes; }
  if (value == 6402) assert(recursive_create_failures == 1 && live_bytes == before_failure);
  if (value == 7001) call_allocations = acquired;
  if (value == 7002) { assert(acquired == call_allocations); checked_calls = true; }
  cod_trace_u64(value);
}
extern "C" void cod_lang_exception_finish() noexcept {
  assert(failures == 1 && checked_calls);
  assert(acquired == released && images.empty() && live_bytes == 0);
  cod_exception_release();
  std::printf("{\"allocations\":%llu,\"releases\":%llu,\"peak_image_bytes\":%llu,"
              "\"allocation_failure_preserves_source\":true,\"factory_failures\":%u,\"recursive_copy_failures\":%u,"
              "\"recursive_create_failures\":%u,\"call_laps\":10000,\"call_allocations\":0}\n",
      static_cast<unsigned long long>(acquired), static_cast<unsigned long long>(released),
      static_cast<unsigned long long>(peak_bytes), construction_failures, recursive_copy_failures, recursive_create_failures);
}
'''

REGION_HARNESS = 'extern "C" void cod_lang_region_free(void *region) { cod_lang_region_release(region); }\n'

OWNED_REGION_HARNESS = r'''
#undef cod_lang_region_new
static std::map<void *, std::uint64_t> regions;
static unsigned regions_acquired = 0, regions_released = 0, region_failures = 0;
extern "C" void *cod_lang_region_new(std::uint64_t bytes, std::uint64_t *status) {
  if (bytes == 31337) { ++region_failures; status[0] = 1; return nullptr; }
  void *region = cod_lang_region_acquire(bytes, status);
  if (region) { assert(regions.emplace(region, bytes).second); ++regions_acquired; }
  return region;
}
extern "C" void cod_lang_region_free(void *region) {
  if (!region) return;
  assert(regions.erase(region) == 1);
  ++regions_released;
  cod_lang_region_release(region);
}
static void verify_regions() {
  assert(regions.empty() && regions_acquired == regions_released);
  assert(regions_acquired == 10250 && region_failures == 4);
  std::printf("{\"acquired\":%u,\"released\":%u,\"injected_failures\":%u}\n",
              regions_acquired, regions_released, region_failures);
}
'''


def main():
    directory = Path(__file__).resolve().parent
    root = directory.parents[2]
    toolchain = root / os.environ.get("COD_TEST_TOOLCHAIN", "build")
    fixture = directory / "closures/closure-selection-implementation.cod"
    with tempfile.TemporaryDirectory(prefix="cod-callable-runtime-") as temporary:
        scratch = Path(temporary)
        entry = scratch / "entry.cod"
        entry.write_text(fixture.read_text() + '''
using "std:exceptions" OutOfMemory;
behave @print(6001u64);
try { func transient = reader(state); current <- copied; {} }
catch (e:const OutOfMemory) { behave @print(6002u64); };
behave @print(current());
current <- copied;
behave @print(current());
behave @print(6201u64);
try { current <- wrap(current); {} }
catch (e:const OutOfMemory) { behave @print(6202u64); };
behave @print(current());
behave @print(7001u64);
behave @print(fold (i in 0u64 .. 10000u64, total = 0u64) {{
  state.value <- i % 97u64;
  total + current() + copied() + nested() + retained()
}});
behave @print(7002u64);
''')
        wrappers = ''.join(f"#define {original} {replacement}\n" for original, replacement in (
            ("cod_lang_callable_create", "cod_image_create"),
            ("cod_lang_callable_copy", "cod_image_copy"),
            ("cod_lang_callable_free", "cod_image_free"),
            ("cod_lang_print_u64", "cod_trace_u64"),
            ("cod_lang_exception_finish", "cod_exception_release")))
        output, evidence = run_case(root, toolchain, entry, scratch, HARNESS + REGION_HARNESS, extra_wrappers=wrappers)
        factories = [line for line in (scratch / "main.ll").read_text().splitlines()
                     if re.match(r"define internal .*@cod\.call\.(reader|alternate|choose|wrap)(?:\.|\()", line)]
        assert factories and all(re.match(r"define internal \{ i64, \{ i64, i64, i64, i64 \} \} .*\(ptr [^,]+, ptr ", line)
                                 for line in factories), factories
        evidence['native_factory_definitions'] = len(factories)
        evidence['caller_destination_abi'] = True
        assert output == ['99', '107', '7', '17', '107', '109', '9', '19', '109', '98', '97', '24', '26',
                          '26', '24', '31', '31', '23', '19', '327', '2569', '19', '19',
                          '6001', '6002', '109', '9', '6201', '6202', '9', '7001',
                          str(sum(4 * (i % 97) + 2570 for i in range(10000))), '7002'], output
        evidence.update(compiler_sha256=hashlib.sha256((toolchain / "bin/cod").read_bytes()).hexdigest(),
                        fixture_sha256=hashlib.sha256(fixture.read_bytes()).hexdigest(),
                        script_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest())
        parameters = directory / "closures/callable-parameters.cod"
        entry.write_text(parameters.read_text() + '''
using "std:exceptions" OutOfMemory;
using "std:exceptions" InvalidCapacity;
data Packet { value:u64; }
function later_argument(capacity:u64) -> const u64 {
  owned region packet:Packet <- make Packet(capacity = capacity) { value <- 0u64 };
  root(packet).value
}
function read_borrowed(action:func()->const u64 in O) -> const u64 where {owner O} { action() }
behave @print(6001u64);
try { read_mode(original, { behave @print(9999u64); 0u64 }); {} }
catch (e:const OutOfMemory) { behave @print(6002u64); };
behave @print(original());
behave @print(6101u64);
try { read_mode(original, later_argument(0u64)); {} }
catch (e:const InvalidCapacity) {
  behave @print(6102u64);
  behave @print(6101u64);
  try { read_after(original, throw e); {} }
  catch (again:const InvalidCapacity) { behave @print(6102u64); };
};
behave @print(original());
behave @print(7001u64);
behave @print(fold (i in 0u64 .. 10000u64, total = 0u64) {{ total + read_borrowed(original) }});
behave @print(7002u64);
''')
        parameter_output, parameter_evidence = run_case(root, toolchain, entry, scratch, HARNESS + REGION_HARNESS, extra_wrappers=wrappers)
        assert parameter_output == ['1', '1', '13', '13', '7', '9', '7', '9', 'false', '9', '7',
                                    '6001', '6002', '7', '6101', '6102', '6101', '6102', '7',
                                    '7001', '70000', '7002'], parameter_output
        parameter_evidence['fixture_sha256'] = hashlib.sha256(parameters.read_bytes()).hexdigest()
        evidence['parameters'] = parameter_evidence
        arguments = directory / "closures/callable-arguments.cod"
        entry.write_text(arguments.read_text() + '''
using "std:exceptions" OutOfMemory;
using "std:exceptions" InvalidCapacity;
function later_argument(capacity:u64) -> const u64 {
  owned region temporary:State <- make State(capacity = capacity) { value <- 0u64 };
  root(temporary).value
}
func nested = func (x:u64) -> const u64 { saved(x) + 1u64 };
behave @print(6001u64);
try { current({ current <- reader(state); 99u64 }); {} }
catch (e:const OutOfMemory) { behave @print(6002u64); };
behave @print(current(0u64));
# Keep the persistent replacement's size equal across the cleanup interval.
current <- reader(state);
behave @print(6101u64);
try { current({ current <- reader(state); later_argument(0u64) }); {} }
catch (e:const InvalidCapacity) { behave @print(6102u64); };
behave @print(current(0u64));
behave @print(7001u64);
behave @print(fold (i in 0u64 .. 10000u64, total = 0u64) {{
  state.value <- i % 97u64;
  total + current(i) + saved(i) + nested(i)
}});
behave @print(7002u64);
''')
        argument_output, argument_evidence = run_case(root, toolchain, entry, scratch, HARNESS + REGION_HARNESS, extra_wrappers=wrappers)
        assert argument_output == ['13', '14', '24', '25', '15', '25', '43', '55', '209', '3.5', '99', '7', '0', '98',
                                   '6001', '6002', '20', '6101', '6102', '10', '7001',
                                   str(sum(3 * i + 5 * (i % 97) + 1 for i in range(10000))), '7002'], argument_output
        argument_evidence['fixture_sha256'] = hashlib.sha256(arguments.read_bytes()).hexdigest()
        evidence['arguments'] = argument_evidence
        procedures = directory / "closures/callable-procedures.cod"
        entry.write_text(procedures.read_text() + '''
using "std:exceptions" OutOfMemory;
using "std:exceptions" InvalidCapacity;
symbol @meter:proc(u64)->const u64;
symbol @copying:proc(u64)->const u64;
proc recursive_meter = proc(depth:u64)->const u64 {
  if (depth == 0u64) { @bias }
  else { effect @bias <- @bias + 1u64; behave @meter(depth - 1u64) + 1u64 }
};
proc copying_meter = proc(depth:u64)->const u64 {
  proc local_copy = @copying;
  effect @copying = local_copy;
  if (depth == 0u64) { 1u64 } else { behave @copying(depth - 1u64) + 1u64 }
};
effect @meter = recursive_meter;
effect @copying = copying_meter;
data Packet { value:u64; }
function later_argument(capacity:u64) -> const u64 {
  owned region packet:Packet <- make Packet(capacity = capacity) { value <- 0u64 };
  root(packet).value
}
behave @print(6001u64);
try { selected({ selected <- first; 99u64 }); {} }
catch (e:const OutOfMemory) { behave @print(6002u64); };
behave @print(selected(2u64));
behave @print(6101u64);
try { selected({ selected <- first; later_argument(0u64) }); {} }
catch (e:const InvalidCapacity) { behave @print(6102u64); };
behave @print(selected(2u64));
behave @print(6301u64);
try { copying_meter(8u64); {} }
catch (e:const OutOfMemory) { behave @print(6302u64); };
behave @print(copying_meter(8u64));
behave @print(7001u64);
behave @print(fold (i in 0u64 .. 10000u64, total = 0u64) {{
  effect @bias <- i % 97u64;
  effect @action = selected;
  total + selected(i) + copied(i) + perform @action(i) + recursive_meter(i % 16u64)
}});
behave @print(7002u64);
''')
        procedure_output, procedure_evidence = run_case(root, toolchain, entry, scratch, HARNESS + REGION_HARNESS, extra_wrappers=wrappers)
        assert procedure_output == ['13', '13', '5', '13', '13', '9', '22', '22', '14', '3', '5', '5', '3', '99', '6', '12', '4', '5', '19', '11', '3', '27', '3', '3', '9', '1', '5', '6',
                                    '6001', '6002', '6', '6101', '6102', '5', '6301', '6302', '9', '7001',
                                    str(sum(3 * i + 3 * (i % 97) + 3 + 2 * (i % 16) for i in range(10000))), '7002'], procedure_output
        bodies = dict(re.findall(r'^define[^\n]*@([^\s(]+)[^\n]*\{(.*?)^}',
                                 (scratch / "main.ll").read_text(), re.MULTILINE | re.DOTALL))
        calls = {name: set(re.findall(r'call[^\n]*@([^\s(]+)', body)) & bodies.keys()
                 for name, body in bodies.items() if name.startswith('cod.call.procedure.context')}
        self_cycles = sum(name in targets for name, targets in calls.items())
        mutual_cycles = sum(a < b and a in calls.get(b, set()) for a, targets in calls.items() for b in targets)
        assert self_cycles and mutual_cycles, calls
        procedure_evidence['native_self_cycles'] = self_cycles
        procedure_evidence['native_mutual_cycles'] = mutual_cycles
        procedure_evidence['fixture_sha256'] = hashlib.sha256(procedures.read_bytes()).hexdigest()
        evidence['procedures'] = procedure_evidence
        captures = directory / "closures/callable-captures.cod"
        entry.write_text(captures.read_text() + '''
using "std:exceptions" OutOfMemory;
behave @print(6001u64);
try { proc failed = snapshot; {} }
catch (e:const OutOfMemory) { behave @print(6002u64); };
behave @print(6201u64);
try {
  proc fresh = proc(n:u64)->const u64 { relay(n) };
  effect @saved = fresh;
  perform @saved(2u64);
  {}
} catch (e:const OutOfMemory) { behave @print(6202u64); };
effect @saved = snapshot;
behave @print(perform @saved(2u64));
behave @print(7001u64);
behave @print(fold (i in 0u64 .. 10000u64, total = 0u64) {{ total + perform @saved(i) }});
behave @print(7002u64);
''')
        capture_output, capture_evidence = run_case(root, toolchain, entry, scratch, HARNESS, extra_wrappers=wrappers)
        assert capture_output == ['3', '12', '3', '24', '3', '40', '34', '42', '6001', '6002', '6201', '6202',
                                  '3', '7001', '50005000', '7002'], capture_output
        # Repeatedly defining an idle procedure must not copy its unused @saved
        # predecessor into a growing environment graph.
        assert capture_evidence['layout']['peak_image_bytes'] < 16384, capture_evidence
        capture_evidence['fixture_sha256'] = hashlib.sha256(captures.read_bytes()).hexdigest()
        evidence['captures'] = capture_evidence
        invocation = directory / "closures/callable-invocation.cod"
        entry.write_text(invocation.read_text() + '''
using "std:exceptions" OutOfMemory;
current <- active;
behave @print(6001u64);
try { current({ behave @print(9999u64); 2u64 }); {} }
catch (e:const OutOfMemory) { behave @print(6002u64); };
behave @print(current(2u64));
proc stable = proc(n:u64)->const u64 { n + 1u64 };
behave @print(7001u64);
behave @print(fold (i in 0u64 .. 10000u64, total = 0u64) {{ total + stable(i) }});
behave @print(7002u64);
''')
        invocation_output, invocation_evidence = run_case(root, toolchain, entry, scratch, HARNESS,
                                                         extra_wrappers=wrappers)
        assert invocation_output == ['112', '212', '312', '13', '42', '9056', '6001', '6002', '112',
                                     '7001', '50005000', '7002'], invocation_output
        assert invocation_evidence['layout']['peak_image_bytes'] < 1024, invocation_evidence
        invocation_evidence['fixture_sha256'] = hashlib.sha256(invocation.read_bytes()).hexdigest()
        evidence['invocation'] = invocation_evidence
        expression_targets = directory / "closures/callable-expression-targets.cod"
        entry.write_text(expression_targets.read_text() + '''
using "std:exceptions" OutOfMemory;
current <- wrapped;
behave @print(6001u64);
try { borrow(current)({ behave @print(999u64); current <- second; 2u64 }); {} }
catch(e:const OutOfMemory) { behave @print(6002u64); };
behave @print(borrow(current)(2u64));
behave @print(fold(i in 0u64 .. 128u64, total = 0u64) {{ total + factory()(i) }});
behave @print(7001u64);
behave @print(fold(i in 0u64 .. 10000u64, total = 0u64) {{ total + (base)(i) }});
behave @print(7002u64);
''')
        expression_output, expression_evidence = run_case(root, toolchain, entry, scratch,
            HARNESS + REGION_HARNESS, extra_wrappers=wrappers)
        assert expression_output == ['5', '6', '8', '42', '123', '103', '12', '104', '13', '21', '14',
                                     '1231', '9', '13', '104', '6001', '6002', '103', '8256',
                                     '7001', '50005000', '7002'], expression_output
        assert expression_evidence['layout']['peak_image_bytes'] < 2048, expression_evidence
        expression_evidence['fixture_sha256'] = hashlib.sha256(expression_targets.read_bytes()).hexdigest()
        evidence['expression_targets'] = expression_evidence
        short_circuit = directory / "closures/callable-short-circuit.cod"
        entry.write_text(short_circuit.read_text() + '''
using "std:exceptions" OutOfMemory;
behave @print(6001u64);
try {
  false && evaluate(stable);
  true && evaluate(stable);
  {}
} catch(e:const OutOfMemory) { behave @print(6002u64); };
behave @print(6201u64);
try {
  true || build(state)();
  false || build(state)();
  {}
} catch(e:const OutOfMemory) { behave @print(6202u64); };
behave @print(state.created);
behave @print(state.called);
behave @print(7001u64);
behave @print(fold(i in 0u64 .. 10000u64,total=0u64) {{
  total + if (i % 2u64 == 0u64 && stable()) { 1u64 } else { 0u64 }
}});
behave @print(7002u64);
''')
        short_output, short_evidence = run_case(root, toolchain, entry, scratch,
                                               HARNESS + REGION_HARNESS, extra_wrappers=wrappers)
        assert short_output == ['16', '6001', '6002', '6201', '6202', '17', '16',
                                '7001', '5000', '7002'], short_output
        assert short_evidence['layout']['factory_failures'] == 1, short_evidence
        short_evidence['fixture_sha256'] = hashlib.sha256(short_circuit.read_bytes()).hexdigest()
        evidence['short_circuit'] = short_evidence
        recursive_arguments = directory / "closures/callable-recursive-factory-argument.cod"
        entry.write_text(recursive_arguments.read_text() + '''
using "std:exceptions" OutOfMemory;
owned region retained <- make State(capacity=256u64){value<-17u64};
func saved=maker(retained);
func copied=saved;
behave @print(6001u64);
try { copied <- saved; {} }
catch(e:const OutOfMemory) { behave @print(6002u64); };
behave @print(6301u64);
try {
  owned region failed <- make State(capacity=256u64){value<-1u64};
  step(failed,maker(failed),16u64); {}
} catch(e:const OutOfMemory) { behave @print(6302u64); };
behave @print(6401u64);
try {
  owned region failed <- make State(capacity=256u64){value<-1u64};
  step(failed,maker(failed),16u64); {}
} catch(e:const OutOfMemory) { behave @print(6402u64); };
func stable=copied();
behave @print(7001u64);
behave @print(fold(i in 0u64..10000u64,total=0u64) {{ total+stable() }});
behave @print(7002u64);
''')
        recursive_harness = 'static void verify_regions();\n' + HARNESS.replace(
            '  cod_exception_release();', '  verify_regions();\n  cod_exception_release();') + OWNED_REGION_HARNESS.replace(
            'regions_acquired == 10250 && region_failures == 4', 'regions_acquired >= 158 && region_failures == 0')
        recursive_output, recursive_evidence = run_case(root, toolchain, entry, scratch, recursive_harness,
            extra_wrappers=wrappers + '#define cod_lang_region_new cod_lang_region_acquire\n')
        recursive_evidence['regions'] = json.loads(recursive_output.pop())
        assert recursive_output == ['39', '67', '105', '72', '15', '39', '6001', '6002', '6301', '6302',
                                    '6401', '6402', '7001', '170000', '7002'], recursive_output
        recursive_evidence['fixture_sha256'] = hashlib.sha256(recursive_arguments.read_bytes()).hexdigest()
        evidence['recursive_arguments'] = recursive_evidence
        effects = directory / "closures/callable-effects.cod"
        entry.write_text(effects.read_text() + '''
using "std:exceptions" OutOfMemory;
current <- first;
behave @print(6001u64);
try { relay(false); }
catch (e:const NullPointerException) {}
catch (e:const OutOfMemory) { behave @print(6002u64); };
behave @print(current(2u64));
behave @print(7001u64);
behave @print(fold (i in 0u64 .. 10000u64, total = 0u64) {{ total + current(i) }});
behave @print(7002u64);
''')
        effects_output, effects_evidence = run_case(root, toolchain, entry, scratch, HARNESS,
                                                   extra_wrappers=wrappers)
        assert effects_output == ['3', '12', '12', '10', '11', '12', '12', '12', '1', '11', '12', '22',
                                  '6001', '6002', '3', '7001', '50005000', '7002'], effects_output
        effects_evidence['fixture_sha256'] = hashlib.sha256(effects.read_bytes()).hexdigest()
        evidence['effects'] = effects_evidence
        regional = directory / "closures/callable-regions.cod"
        entry.write_text(regional.read_text() + '''
using "std:exceptions" OutOfMemory;
behave @print(6001u64);
try { proc failed = snapshot; {} }
catch (e:const OutOfMemory) { behave @print(6002u64); };
behave @print(perform @saved(2u64));
behave @print(7001u64);
behave @print(fold (i in 0u64 .. 10000u64, total = 0u64) {{ total + perform @saved(i) }});
behave @print(7002u64);
''')
        regional_output, regional_evidence = run_case(root, toolchain, entry, scratch, HARNESS + REGION_HARNESS,
                                                     extra_wrappers=wrappers)
        assert regional_output == ['13', '52', '13', '152', '14', '4', '33', '44', '6001', '6002', '14',
                                   '7001', '50115000', '7002'], regional_output
        regional_evidence['fixture_sha256'] = hashlib.sha256(regional.read_bytes()).hexdigest()
        evidence['regional_captures'] = regional_evidence
        factories = directory / "closures/callable-factories.cod"
        entry.write_text(factories.read_text() + '''
using "std:exceptions" OutOfMemory;
behave @print(6001u64);
try { chained <- saved; {} }
catch (e:const OutOfMemory) { behave @print(6002u64); };
behave @print(chained());
behave @print(6201u64);
try { chained <- compose(8u64); {} }
catch (e:const OutOfMemory) { behave @print(6202u64); };
behave @print(chained());
behave @print(6401u64);
try { chained <- compose(8u64); {} }
catch (e:const OutOfMemory) { behave @print(6402u64); };
behave @print(chained());
behave @print(7001u64);
behave @print(fold (i in 0u64 .. 10000u64, total = 0u64) {{ total + chained() }});
behave @print(7002u64);
''')
        factory_output, factory_evidence = run_case(root, toolchain, entry, scratch, HARNESS + REGION_HARNESS,
                                                    extra_wrappers=wrappers)
        assert factory_output == ['1', '2', '33', '17', '19', '20', '1', '2', '100', '13', '7', '7', '7',
                                  '6001', '6002', '33', '6201', '6202', '33', '6401', '6402', '33',
                                  '7001', '330000', '7002'], factory_output
        assert factory_evidence['layout']['recursive_create_failures'] == 1, factory_evidence
        factory_evidence['fixture_sha256'] = hashlib.sha256(factories.read_bytes()).hexdigest()
        evidence['recursive_factories'] = factory_evidence
        lexical = directory / "closures/callable-lexical-factory.cod"
        entry.write_text(lexical.read_text().replace('effect @selected = first;', '''using "std:exceptions" OutOfMemory;
behave @print(6001u64);
try { proc transient = first; {} }
catch (e:const OutOfMemory) { behave @print(6002u64); };
behave @print(6201u64);
try { proc transient = factory(8u64); {} }
catch (e:const OutOfMemory) { behave @print(6202u64); behave @print(@bias); };
effect @selected = first;''') + '''
behave @print(7001u64);
behave @print(fold (i in 0u64 .. 10000u64, total = 0u64) {{ total + first() }});
behave @print(7002u64);
''')
        lexical_output, lexical_evidence = run_case(root, toolchain, entry, scratch, HARNESS,
                                                    extra_wrappers=wrappers)
        assert lexical_output == ['10', '6001', '6002', '6201', '6202', '10', '9', '13', '10',
                                  '7001', '100000', '7002'], lexical_output
        assert lexical_evidence['layout']['factory_failures'] == 1, lexical_evidence
        lexical_evidence['fixture_sha256'] = hashlib.sha256(lexical.read_bytes()).hexdigest()
        evidence['lexical_factory'] = lexical_evidence
        higher = directory / "closures/callable-higher-order.cod"
        entry.write_text(higher.read_text() + '''
using "std:exceptions" OutOfMemory;
using "std:exceptions" InvalidCapacity;
data Packet { value:u64; }
function later_argument(capacity:u64) -> const u64 {
  owned region packet:Packet <- make Packet(capacity = capacity) { value <- 0u64 };
  root(packet).value
}
behave @print(6001u64);
try { plain(selected, { behave @print(9999u64); 1u64 }); {} }
catch (e:const OutOfMemory) { behave @print(6002u64); };
behave @print(selected(3u64));
behave @print(6101u64);
try { plain(selected, later_argument(0u64)); {} }
catch (e:const InvalidCapacity) { behave @print(6102u64); };
behave @print(6301u64);
try { walk(original,8u64); {} }
catch (e:const OutOfMemory) { behave @print(6302u64); };
behave @print(original(2u64));
behave @print(7001u64);
behave @print(fold (i in 0u64 .. 10000u64, total = 0u64) {{ total + saved(2u64) }});
behave @print(7002u64);
''')
        higher_output, higher_evidence = run_case(root, toolchain, entry, scratch, HARNESS + REGION_HARNESS,
                                                  extra_wrappers=wrappers)
        assert higher_output == ['1', '3', '4', '13', '103', '112', '3', '12', '17', '2', '10', '46', '10',
                                 '23', '10', '99', '6001', '6002', '13', '6101', '6102', '6301', '6302', '3',
                                 '7001', '1030000', '7002'], higher_output
        assert higher_evidence['layout']['recursive_copy_failures'] == 1, higher_evidence
        higher_evidence['fixture_sha256'] = hashlib.sha256(higher.read_bytes()).hexdigest()
        evidence['higher_order'] = higher_evidence
        generic = directory / "closures/callable-generic.cod"
        entry.write_text(generic.read_text() + '''
using "std:exceptions" OutOfMemory;
behave @print(6001u64);
try { func failed = selected; {} }
catch(e:const OutOfMemory) { behave @print(6002u64); };
behave @print(selected(3u64));
behave @print(6301u64);
try { iterate(originals,8u64,1u64); {} }
catch(e:const OutOfMemory) { behave @print(6302u64); };
behave @print(originals(3u64));
behave @print(7001u64);
behave @print(fold (i in 0u64 .. 10000u64, total = 0u64) {{ total + selected(i) }});
behave @print(7002u64);
''')
        generic_output, generic_evidence = run_case(root, toolchain, entry, scratch, HARNESS,
                                                    extra_wrappers=wrappers)
        assert generic_output == ['6', '3', '9', '4.5', '10', '60', '4', '16', '16', '6', '6', '10', '0',
                                  '12', '6', '10', '4', '10', '4', '3', '4', '6', '3', '7', '9', '4.5',
                                  '6001', '6002', '9', '6301', '6302', '6', '7001', '149985000', '7002'], generic_output
        assert generic_evidence['layout']['recursive_copy_failures'] == 1, generic_evidence
        generic_evidence['fixture_sha256'] = hashlib.sha256(generic.read_bytes()).hexdigest()
        evidence['generic'] = generic_evidence
        regional_parameters = directory / "closures/callable-region-parameters.cod"
        entry.write_text(regional_parameters.read_text() + '''
using "std:exceptions" OutOfMemory;
behave @print(6001u64);
try { higher(selected, first, { behave @print(999u64); 0u64 }); {} }
catch(e:const OutOfMemory) { behave @print(6002u64); };
behave @print(selected(first,0u64));
behave @print(7001u64);
behave @print(fold(i in 0u64 .. 10000u64, total=0u64) {{ total + readonly(second) }});
behave @print(7002u64);
''')
        regional_output, regional_evidence = run_case(root, toolchain, entry, scratch, HARNESS + REGION_HARNESS,
                                                     extra_wrappers=wrappers)
        assert regional_output == ['12', '23', '16', '29', '16', '99', '24', '38', '30', '46', '34', '45',
                                   '46', '46', '97', '48', '49', '51', '6001', '6002', '51',
                                   '7001', '490000', '7002'], regional_output
        regional_evidence['fixture_sha256'] = hashlib.sha256(regional_parameters.read_bytes()).hexdigest()
        evidence['regional_parameters'] = regional_evidence
        owner_parameters = directory / "closures/callable-owner-parameters.cod"
        entry.write_text(owner_parameters.read_text() + '''
using "std:exceptions" OutOfMemory;
behave @print(6001u64);
try { func failed = integrate_view; {} }
catch(e:const OutOfMemory) { behave @print(6002u64); };
behave @print(integrate_view(values,0.0f32));
behave @print(7001u64);
behave @print(fold(i in 0u64 .. 10000u64,integrated=0.0f32) {{ integrated+integrate_view(values,0.0f32) }});
behave @print(7002u64);
''')
        owner_output, owner_evidence = run_case(root, toolchain, entry, scratch, HARNESS + REGION_HARNESS,
                                               extra_wrappers=wrappers)
        assert owner_output == ['3', '7', '14', '14', '17', '3.5', '22', '22', '8', '8', '4', '7', '7',
                                '25', '35', '5', '5', '41', '6', '7', '43', '41', '6001', '6002', '7',
                                '7001', '70000', '7002'], owner_output
        owner_evidence['fixture_sha256'] = hashlib.sha256(owner_parameters.read_bytes()).hexdigest()
        evidence['owner_parameters'] = owner_evidence
        borrowed_results = directory / "closures/callable-borrowed-results.cod"
        entry.write_text(borrowed_results.read_text() + '''
using "std:exceptions" OutOfMemory;
behave @print(6001u64);
try { func failed = selected; {} }
catch(e:const OutOfMemory) { behave @print(6002u64); };
ref after_failure = selected(a,b,true);
after_failure <- 43u64;
behave @print(b);
behave @print(7001u64);
behave @print(fold(i in 0u64 .. 10000u64,sum_returns=0u64) {{ sum_returns+selected(a,b,true) }});
behave @print(7002u64);
''')
        borrowed_output, borrowed_evidence = run_case(root, toolchain, entry, scratch, HARNESS + REGION_HARNESS,
                                                      extra_wrappers=wrappers)
        assert borrowed_output == ['4', '7', '4', '8', '13', '1.25', '19', '2.5', '1', '23', '31', '37',
                                   '41', '53', '31', '6001', '6002', '43', '7001', '430000', '7002'], borrowed_output
        assert re.search(r'define internal \{ i64, \{ i64, i64, i64, i64 \}, ptr \} @cod\.call\.',
                         (scratch / "main.ll").read_text())
        borrowed_evidence['borrowed_result_abi'] = True
        borrowed_evidence['fixture_sha256'] = hashlib.sha256(borrowed_results.read_bytes()).hexdigest()
        evidence['borrowed_results'] = borrowed_evidence
        nullable_views = directory / "closures/callable-nullable-views.cod"
        entry.write_text(nullable_views.read_text() + '''
using "std:exceptions" OutOfMemory;
behave @print(6001u64);
try { func failed = identity; {} }
catch(e:const OutOfMemory) { behave @print(6002u64); };
ptr recovered = identity(maybe);
try { recovered <- 31u64; behave @print(number); }
catch(e:const NullPointerException) { behave @print(999u64); };
behave @print(7001u64);
behave @print(fold(i in 0u64 .. 10000u64,null_returns=0u64) {{
  null_returns+if(isnull(identity(empty))) {1u64} else {0u64}
}});
behave @print(7002u64);
''')
        nullable_output, nullable_evidence = run_case(root, toolchain, entry, scratch, HARNESS + REGION_HARNESS,
                                                      extra_wrappers=wrappers)
        assert nullable_output == ['true', 'false', 'true', '9', '9', 'true', '9', '17', '2', '0', 'false',
                                   '23', 'true', 'false', '2', '0', 'false', 'true', '29',
                                   '6001', '6002', '31', '7001', '10000', '7002'], nullable_output
        nullable_evidence['fixture_sha256'] = hashlib.sha256(nullable_views.read_bytes()).hexdigest()
        evidence['nullable_views'] = nullable_evidence
        borrowed_holders = directory / "closures/callable-borrowed-holders.cod"
        entry.write_text(borrowed_holders.read_text() + '''
using "std:exceptions" OutOfMemory;
behave @print(6001u64);
try { generic(plain,decorated); } catch(e:const OutOfMemory) { behave @print(6002u64); };
behave @print(plain());
generic(plain,decorated);
behave @print(plain());
behave @print(7001u64);
behave @print(fold(i in 0u64 .. 10000u64,total=0u64) {{ total+observe(original,0u64) }});
behave @print(7002u64);
''')
        holder_output, holder_evidence = run_case(root, toolchain, entry, scratch, HARNESS + REGION_HARNESS,
                                                 extra_wrappers=wrappers)
        assert holder_output == ['3', '2', 'false', 'true', '1', '2', '1', '3', '2', '3', '2', '0',
                                 '51', '53', '43', '21', '16', '6001', '6002', '53', '43',
                                 '7001', '30000', '7002'], holder_output
        holder_evidence['fixture_sha256'] = hashlib.sha256(borrowed_holders.read_bytes()).hexdigest()
        evidence['borrowed_holders'] = holder_evidence
        holder_results = directory / "closures/callable-holder-results.cod"
        entry.write_text(holder_results.read_text() + '''
using "std:exceptions" OutOfMemory;
behave @print(6001u64);
try { func failed = captured; {} } catch(e:const OutOfMemory) { behave @print(6002u64); };
behave @print(captured());
behave @print(7001u64);
behave @print(fold(i in 0u64 .. 10000u64,total=0u64) {{
 ptr lap_view = choose(a,b,i%2u64==0u64);
 total+lap_view()
}});
behave @print(7002u64);
''')
        result_output, result_evidence = run_case(root, toolchain, entry, scratch, HARNESS + REGION_HARNESS,
                                                 extra_wrappers=wrappers)
        assert result_output == ['2', '1', '3', '1', '2', '1', 'true', '1', '3', '17', 'true', '2',
                                 '2', '1', '2', '3', '43', '2', '3', '2', '3', '43',
                                 '6001', '6002', '3', '7001', '30000', '7002'], result_output
        assert re.search(r'define internal \{ i64, \{ i64, i64, i64, i64 \}, ptr \} @cod\.call\.',
                         (scratch / "main.ll").read_text())
        result_evidence['borrowed_result_abi'] = True
        result_evidence['fixture_sha256'] = hashlib.sha256(holder_results.read_bytes()).hexdigest()
        evidence['holder_results'] = result_evidence
        owner_flows = directory / "closures/callable-owner-flows.cod"
        entry.write_text(owner_flows.read_text() + '''
function access(region state:State)->const u64 {state.ticks}
function access(const region state:State)->const u64 {state.ticks+100u64}
function inspect(const region state:State)->const u64 {state.ticks}
using "std:exceptions" OutOfMemory;
behave @print(6001u64);
try { func failed = current; {} } catch(e:const OutOfMemory) { behave @print(6002u64); };
behave @print(current());
behave @print(7001u64);
behave @print(fold(i in 0u64 .. 10000u64,total=0u64) {{
 region view:State = current -> state;
 view.ticks <- view.ticks;
 total+access(view)+inspect(view)
}});
behave @print(7002u64);
''')
        owner_output, owner_evidence = run_case(root, toolchain, entry, scratch, HARNESS + REGION_HARNESS,
                                               extra_wrappers=wrappers)
        assert owner_output == ['18', '26', '29', '32', '6001', '6002', '32',
                                '7001', '640000', '7002'], owner_output
        owner_evidence['fixture_sha256'] = hashlib.sha256(owner_flows.read_bytes()).hexdigest()
        evidence['owner_flows'] = owner_evidence
        owner_factories = directory / "closures/callable-owner-factories.cod"
        entry.write_text(owner_factories.read_text() + '''
using "std:exceptions" OutOfMemory;
func primary_factory=reader;
behave @print(6001u64);
try { stored_factory <- primary_factory; {} } catch(e:const OutOfMemory) { behave @print(6002u64); };
func after_failed_copy=stored_factory(item,0u64);
behave @print(after_failed_copy());
behave @print(6201u64);
try { func failed = stored_factory(item,8u64); {} } catch(e:const OutOfMemory) { behave @print(6202u64); };
behave @print(result());
behave @print(7001u64);
behave @print(fold(i in 0u64 .. 10000u64,total=0u64) {{
 item.value <- i;
 total+result()
}});
behave @print(7002u64);
''')
        factory_output, factory_evidence = run_case(root, toolchain, entry, scratch, HARNESS + REGION_HARNESS,
                                                   extra_wrappers=wrappers)
        assert factory_output == ['7', '3', '5', '11', '11', '13', '16', '18', '18', '106', '107', '7', '17',
                                  '6001', '6002', '107', '6201', '6202', '18',
                                  '7001', '49995000', '7002'], factory_output
        definitions = re.findall(r'^define internal [^\n]*@([^ (]+)\([^\n]*\) \{\n(.*?)^\}',
                                 (scratch / 'main.ll').read_text(), re.M | re.S)
        recursive = [name for name, body in definitions if '@' + name + '(' in body]
        assert any(name.startswith('cod.call.function.context') for name in recursive), recursive
        assert any(name.startswith('cod.call.procedure.context') for name in recursive), recursive
        factory_evidence['recursive_native_definitions'] = recursive
        factory_evidence['fixture_sha256'] = hashlib.sha256(owner_factories.read_bytes()).hexdigest()
        evidence['owner_factories'] = factory_evidence
        array_results = directory / "closures/callable-array-results.cod"
        entry.write_text(array_results.read_text() + '''
using "std:exceptions" OutOfMemory;
behave @print(6001u64);
try { func failed=selected; {} } catch(e:const OutOfMemory){behave @print(6002u64);};
ref preserved=selected(a,b);
behave @print(preserved[0u64]);
behave @print(7001u64);
behave @print(fold(i in 0u64..10000u64,total=0u64){{
 ref samples=selected(a,b);
 samples[0u64] <- f32(i%97u64);
 total+u64(samples[0u64])
}});
behave @print(7002u64);
''')
        array_output, array_evidence = run_case(root, toolchain, entry, scratch, HARNESS + REGION_HARNESS,
                                               extra_wrappers=wrappers)
        assert array_output == ['2', '6', '20', '7', '4', '2', '1.5', 'true', '99', '9', '2', '6',
                                '6001', '6002', '6', '7001', str(sum(i % 97 for i in range(10000))), '7002'], array_output
        array_evidence['fixture_sha256'] = hashlib.sha256(array_results.read_bytes()).hexdigest()
        evidence['array_results'] = array_evidence
        conversions = directory / "closures/callable-conversions.cod"
        entry.write_text(conversions.read_text() + '''
using "std:exceptions" OutOfMemory;
func original=first;
behave @print(6001u64);
try { selected <- original; {} } catch(e:const OutOfMemory){behave @print(6002u64);};
behave @print(selected(start(1.0)));
behave @print(7001u64);
behave @print(fold(i in 0u64..10000u64,total=0.0){{total+selected(start(f64(i%97u64)))}});
behave @print(7002u64);
''')
        conversion_output, conversion_evidence = run_case(root, toolchain, entry, scratch, HARNESS,
                                                         extra_wrappers=wrappers)
        assert conversion_output == ['11', '21', '22', '23', '23', '0', '12', '13', '14', '24', '7', '5',
                                     '6001', '6002', '21', '7001',
                                     str(sum(i % 97 + 20 for i in range(10000))), '7002'], conversion_output
        conversion_evidence['fixture_sha256'] = hashlib.sha256(conversions.read_bytes()).hexdigest()
        evidence['conversions'] = conversion_evidence
        record_results = directory / "closures/callable-record-results.cod"
        entry.write_text(record_results.read_text() + '''
using "std:exceptions" OutOfMemory;
behave @print(6001u64);
try { func failed=selected; {} } catch(e:const OutOfMemory){behave @print(6002u64);};
ref preserved=selected(head,tail);
behave @print(preserved.value);
behave @print(7001u64);
behave @print(fold(i in 0u64..10000u64,total=0u64){{
 ref node=selected(head,tail);
 node.value <- i%97u64;
 total+node.value
}});
behave @print(7002u64);
''')
        record_output, record_evidence = run_case(root, toolchain, entry, scratch, HARNESS + REGION_HARNESS,
                                                 extra_wrappers=wrappers)
        assert record_output == ['4', '8', '11', '4', 'true', '99', '4', '37', '1', '2',
                                 '6001', '6002', '11', '7001',
                                 str(sum(i % 97 for i in range(10000))), '7002'], record_output
        record_evidence['fixture_sha256'] = hashlib.sha256(record_results.read_bytes()).hexdigest()
        evidence['record_results'] = record_evidence
        initialization = directory / "closures/callable-initialization.cod"
        entry.write_text(initialization.read_text() + '''
using "std:exceptions" OutOfMemory;
behave @print(6001u64);
try { func failed=selected; {} } catch(e:const OutOfMemory){behave @print(6002u64);};
ref initialized=make local u64;
selected(initialized);
behave @print(initialized);
behave @print(7001u64);
behave @print(fold(i in 0u64..10000u64,total=0u64){{
 ref slot=make local u64;
 selected(slot);
 total+slot
}});
behave @print(7002u64);
''')
        initialization_output, initialization_evidence = run_case(
            root, toolchain, entry, scratch, HARNESS + REGION_HARNESS, extra_wrappers=wrappers)
        assert initialization_output == ['7', '9', '23', '31', '19', '9', '13', '17', '29', '7', '5',
                                         '6001', '6002', '9', '7001', '90000', '7002'], initialization_output
        initialization_evidence['fixture_sha256'] = hashlib.sha256(initialization.read_bytes()).hexdigest()
        evidence['initialization'] = initialization_evidence
        region_results = directory / "closures/callable-region-results.cod"
        for name in ('record-vocabulary.cod', 'region-result-adapter.cod'):
            (scratch / name).write_text((directory / 'closures' / name).read_text())
        entry.write_text(region_results.read_text() + '''
using "std:exceptions" OutOfMemory;
behave @print(6001u64);
try {func failed=selected;{}} catch(e:const OutOfMemory){behave @print(6002u64);};
region preserved=selected(state);
behave @print(preserved.value);
behave @print(7001u64);
behave @print(fold(i in 0u64..10000u64,total=0u64){{
 state.value<-i%97u64;
 region result=selected(state);
 total+result.value
}});
behave @print(7002u64);
''')
        region_output, region_evidence = run_case(
            root, toolchain, entry, scratch, HARNESS + REGION_HARNESS, extra_wrappers=wrappers)
        assert region_output == ['5', '7', '7', '7', '7', '39', '12', '41', '41', '1', '2', '33', '17',
                                 '6001', '6002', '35', '7001',
                                 str(sum(i % 97 + 2 for i in range(10000))), '7002'], region_output
        region_evidence['fixture_sha256'] = hashlib.sha256(region_results.read_bytes()).hexdigest()
        evidence['region_results'] = region_evidence
        owned_results = directory / "closures/callable-owned-results.cod"
        entry.write_text(owned_results.read_text() + '''
using "std:exceptions" OutOfMemory;
behave @print(6001u64);
try {func failed=selected;{}} catch(e:const OutOfMemory){behave @print(6002u64);};
try {owned region rejected<-failing(31337u64,0u64);}
catch(e:const OutOfMemory){behave @print(41u64);};
try {
  owned region seed<-make State(capacity=256u64){value<-7u64};
  ref fixed=root(seed);
  owned region unused<-rotate(fixed,fixed,seed,current_reader(seed),3u64,31337u64);
  behave @assert(false);
} catch(e:const OutOfMemory) {};
try {
  owned region seed<-make State(capacity=256u64){value<-7u64};
  staged_step(seed,staged_reader(seed),3u64,31337u64,true);
  behave @assert(false);
} catch(e:const OutOfMemory) {};
try {
  owned region seed<-make State(capacity=256u64){value<-7u64};
  proc unused=staged_factory(seed,stable,staged_reader(seed),3u64,31337u64);
  behave @assert(false);
} catch(e:const OutOfMemory) {};
behave @print(7001u64);
behave @print(fold(i in 0u64..10000u64,total=0u64){{
  owned region result<-selected(256u64,i%97u64);
  total+result.value
}});
behave @print(7002u64);
''')
        owned_harness = 'static void verify_regions();\n' + HARNESS.replace(
            '  cod_exception_release();', '  verify_regions();\n  cod_exception_release();') + OWNED_REGION_HARNESS
        owned_output, owned_evidence = run_case(root, toolchain, entry, scratch, owned_harness,
            extra_wrappers=wrappers + '#define cod_lang_region_new cod_lang_region_acquire\n')
        owned_evidence['regions'] = json.loads(owned_output.pop())
        assert owned_output == ['8', '3', '9', '3', '12', '19', '1', '2', '29', '31',
                                '6001', '6002', '41', '7001',
                                str(sum(i % 97 + 2 for i in range(10000))), '7002'], owned_output
        owned_evidence['fixture_sha256'] = hashlib.sha256(owned_results.read_bytes()).hexdigest()
        evidence['owned_results'] = owned_evidence
        print(json.dumps(evidence, indent=2))
        if report := os.environ.get("COD_CALLABLE_REPORT"):
            Path(report).write_text(json.dumps(evidence, indent=2) + "\n")


if __name__ == "__main__":
    main()
