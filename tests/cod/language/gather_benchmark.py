#!/usr/bin/env python3
"""Measure a CoD SoA kernel and compare every reported step with FP32 arithmetic."""

import datetime
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import struct
import subprocess
import tempfile
import time


def run(command, root):
    started = time.perf_counter()
    result = subprocess.run([str(x) for x in command], cwd=root, capture_output=True, text=True, timeout=120)
    elapsed = time.perf_counter() - started
    if result.returncode or result.stderr:
        raise RuntimeError(f"{command}\nexit={result.returncode}\n{result.stdout}\n{result.stderr}")
    return result.stdout, elapsed


def reference(classes, frames):
    pack, unpack = struct.Struct('f').pack, struct.Struct('f').unpack
    def f32(x):
        return unpack(pack(x))[0]
    widths = [(100 if j % 2 == 0 else 144) * 2 ** (j // 2) for j in range(classes)]
    capacities = [width for j, width in enumerate(widths) for _ in range(2 ** (classes - 1 - j))]
    previous = [float(i + 1) for i in range(len(capacities))]
    checksums = []
    for step in range(frames + 1):
        total = 0.0
        for value in previous:
            total = f32(total + value)
        checksums.append(format(total, '.9g'))
        if step == frames:
            break
        following = []
        for target, capacity in enumerate(capacities):
            value = 0.0
            for local in range(capacity):
                value = f32(value + f32(previous[(target + local) % len(previous)] / 256.0))
            following.append(value)
        previous = following
    return checksums, len(capacities), sum(capacities)


def main():
    directory = Path(__file__).resolve().parent
    root = directory.parents[2]
    mode = os.environ.get('COD_TEST_TOOLCHAIN', 'build')
    if mode not in ('build', 'built'):
        raise SystemExit('COD_TEST_TOOLCHAIN must be build or built')
    if os.name == 'nt':
        raise SystemExit('This native timing harness currently targets Linux/macOS')
    classes, frames = 12, 8
    expected, neurons, slots = reference(classes, frames)
    toolchain = root / mode
    fixture = directory / 'region-gather.cod'
    source = fixture.read_text()
    source = re.sub(r'"([^"\n]+\.cod)"', lambda m: json.dumps(str(directory / m[1])), source)
    source = source.replace('Layout(4u32, 1u64)', f'Layout({classes}u32, 1u64)')
    source = source.replace('ClassId(4u32)', f'ClassId({classes}u32)')
    source = source.replace('region(32768u64)', 'region(4194304u64)')
    source = source.replace('frame in 0u32 .. 4u32', f'frame in 0u32 .. {frames}u32')
    source = source.replace('behave @assert(used_bytes(@arena) < 12000u64);',
                            'behave @print(used_bytes(@arena));')
    with tempfile.TemporaryDirectory(prefix='cod-gather-benchmark-') as temporary:
        scratch = Path(temporary)
        entry, generated, executable = scratch / 'main.cod', scratch / 'main.cc', scratch / 'kernel'
        entry.write_text(source)
        _, frontend_seconds = run([toolchain / 'bin/cod', '--check', entry, '--emit-cpp', generated], root)
        command = [toolchain / 'bin/clang++', '-std=c++20', '-O2', '-g', '-stdlib=libc++', '-fuse-ld=lld',
                   '-I' + str(root / 'include'), generated, '-o', executable,
                   '-L' + str(toolchain / 'lib'), '-Wl,-rpath,' + str(toolchain / 'lib')]
        _, compile_seconds = run(command, root)
        output, process_seconds = run([executable], root)
        lines = output.splitlines()
        if lines[:-1] != expected:
            raise RuntimeError(f'FP32 result mismatch: expected={expected}, actual={lines}')
        occupied = int(lines[-1])
        payload = slots * 5 + neurons * 8
        evidence = {'schema_version': 1, 'recorded_utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
                    'platform': platform.platform(), 'toolchain': mode, 'passed': True,
                    'classes': classes, 'rows': neurons, 'slots': slots, 'updates': frames,
                    'edge_visits': slots * frames, 'checksums': expected,
                    'element_payload_bytes': payload, 'region_occupation_bytes': occupied,
                    'non_element_bytes': occupied - payload,
                    'frontend_seconds': frontend_seconds, 'native_compile_link_seconds': compile_seconds,
                    'native_process_seconds': process_seconds, 'generated_cpp_bytes': generated.stat().st_size,
                    'native_executable_bytes': executable.stat().st_size,
                    'notes': ['Timing includes initialization, process startup and printed checksums.',
                              'Native size includes debug information; no persistent code-sharing layer is used.',
                              'FP32 reference uses independent row capacities and rounded additions.',
                              'This numerical kernel defines no neuron transition or training rule.'],
                    'source_sha256': {str(p.relative_to(root)): hashlib.sha256(p.read_bytes()).hexdigest()
                                      for p in [*sorted(directory.glob('region-gather*.cod')),
                                                *sorted(directory.glob('pool-*.cod')),
                                                root / 'cod/lang_frontend.hh', root / 'cod/lang_region_runtime.hh',
                                                Path(__file__).resolve()]}}
        if report := os.environ.get('COD_GATHER_REPORT'):
            Path(report).write_text(json.dumps(evidence, indent=2) + '\n')
        print(json.dumps(evidence, indent=2))


if __name__ == '__main__':
    main()
