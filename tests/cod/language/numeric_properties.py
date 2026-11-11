#!/usr/bin/env python3
"""Compare typed regional arithmetic with independent rounded/exact references."""

import datetime
import hashlib
import json
import os
from pathlib import Path
import random
import struct
import subprocess
import tempfile
import time


def f32(x):
    return struct.unpack('f', struct.pack('f', x))[0]


def literal(x):
    text = format(x, '.17g')
    if '.' not in text and 'e' not in text:
        text += '.0'
    return text + 'f32'


def main():
    directory = Path(__file__).resolve().parent
    root = directory.parents[2]
    mode = os.environ.get('COD_TEST_TOOLCHAIN', 'build')
    if mode not in ('build', 'built'):
        raise SystemExit('COD_TEST_TOOLCHAIN must be build or built')
    rng = random.Random(0xC0D2026)
    floats = [(f32(rng.uniform(-2 ** 20, 2 ** 20)), f32(rng.uniform(0.25, 2 ** 10)))
              for _ in range(16)]
    floats += [(0.0, 1.0), (-0.0, 1.0), (f32(2 ** -149), 0.5),
               (f32(2 ** -126), 0.25), (16777216.0, 1.0), (16777215.0, 0.5)]
    integers = [(rng.randrange(2 ** 30), rng.randrange(2 ** 30)) for _ in range(16)]
    integers += [(0, 0), (2 ** 32 - 1, 2 ** 32), (2 ** 63 - 1, 1)]
    lines = ['using symbol "std:io" @print;', 'unit Slot;',
             'array Floats[u32(Slot)]::f32;', 'array Integers[u32(Slot)]::u64;',
             'data Sample { a::f32; b::f32; }',
             'function blend(s::Sample in O)::f32 where {owner O} { (s.a * s.b + s.a) / s.b }',
             'function blend(a::u64, b::u64)::u64 { a * b + a }',
             'region symbol @arena;', 'symbol @fa :: Floats;', 'symbol @fb :: Floats;', 'symbol @ia :: Integers;', 'symbol @ib :: Integers;',
             'effect @arena <- region(16384u64);',
             f'effect @fa = alloc @arena Floats({len(floats)}u64, 0.0f32);',
             f'effect @fb = alloc @arena Floats({len(floats)}u64, 0.0f32);',
             f'effect @ia = alloc @arena Integers({len(integers)}u64, 0u64);',
             f'effect @ib = alloc @arena Integers({len(integers)}u64, 0u64);']
    for i, (a, b) in enumerate(floats):
        lines += [f'store(@fa, Slot({i}u32), {literal(a)});',
                  f'store(@fb, Slot({i}u32), {literal(b)});']
    for i, (a, b) in enumerate(integers):
        lines += [f'store(@ia, Slot({i}u32), {a}u64);', f'store(@ib, Slot({i}u32), {b}u64);']
    lines += [f'for (i in Slot(0u32) .. Slot({len(floats)}u32)) {{',
              '  behave @print(blend(deref(alloc @arena Sample(@fa[i], @fb[i]))));', '};',
              f'for (i in Slot(0u32) .. Slot({len(integers)}u32)) {{',
              '  behave @print(blend(@ia[i], @ib[i]));', '};']
    source = '\n'.join(lines) + '\n'
    expected = [format(f32(f32(f32(a * b) + a) / b), '.9g') for a, b in floats]
    expected += [str(a * b + a) for a, b in integers]
    with tempfile.TemporaryDirectory(prefix='cod-numeric-properties-') as temporary:
        entry = Path(temporary) / 'main.cod'
        entry.write_text(source)
        started = time.perf_counter()
        result = subprocess.run([str(root / mode / 'bin/cod'), '--run', str(entry)], cwd=root,
                                capture_output=True, text=True, timeout=120)
        seconds = time.perf_counter() - started
        if result.returncode or result.stderr or result.stdout.splitlines() != expected:
            actual = result.stdout.splitlines()
            mismatches = [{'index': i, 'expected': value, 'actual': actual[i] if i < len(actual) else None}
                          for i, value in enumerate(expected) if i >= len(actual) or actual[i] != value]
            raise RuntimeError(json.dumps({'exit': result.returncode, 'stderr': result.stderr,
                                           'mismatches': mismatches[:16]}, indent=2))
        evidence = {'schema_version': 1, 'recorded_utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
                    'toolchain': mode, 'backend': 'llvm', 'passed': True, 'seed': '0xC0D2026',
                    'f32_samples': len(floats), 'u64_samples': len(integers),
                    'native_compile_link_run_seconds': seconds,
                    'generated_cod_sha256': hashlib.sha256(source.encode()).hexdigest(),
                    'observed_trace_sha256': hashlib.sha256(result.stdout.encode()).hexdigest(),
                    'notes': ['FP32 reference rounds after every arithmetic operation.',
                              'Includes signed zero, subnormals, precision-boundary values and large unsigned integers.',
                              'This validates arithmetic and regional input access, not a neural model.'],
                    'source_sha256': {str(p.relative_to(root)): hashlib.sha256(p.read_bytes()).hexdigest()
                                      for p in [root / 'cod/lang_syntax.hh', root / 'cod/lang_frontend.hh',
                                                root / 'cod/lang_llvm.hh', root / 'cod/lang_native_runtime.hh',
                                                root / 'include/shilos/region.hh',
                                                Path(__file__).resolve()]}}
        if report := os.environ.get('COD_NUMERIC_REPORT'):
            Path(report).write_text(json.dumps(evidence, indent=2) + '\n')
        print(json.dumps(evidence, indent=2))


if __name__ == '__main__':
    main()
