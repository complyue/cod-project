#!/usr/bin/env python3
"""Run isolated architecture experiments; never configure or mutate build/."""
from pathlib import Path
import argparse
import datetime
import hashlib
import json
import platform
import random
import re
import shutil
import statistics
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def invoke(command, cwd=ROOT):
    start = time.monotonic()
    result = subprocess.run(command, cwd=cwd, capture_output=True, text=True, timeout=60)
    return dict(command=[str(x) for x in command], exit=result.returncode,
                seconds=time.monotonic() - start, stdout=result.stdout, stderr=result.stderr)


def native_probe(cold_count):
    """IR is a small independently generated boundary probe, not CoD output."""
    code = '''@binding = thread_local global i64 7
@operands = global i64 0
@fmt = private constant [17 x i8] c"%ld %ld %ld %ld\\0A\\00"
declare i32 @printf(ptr, ...)
define i64 @shared(i64 %a, i64 %b, i64 %context) {
  %sum = add i64 %a, %b
  %answer = add i64 %sum, %context
  ret i64 %answer
}
define i64 @current() {
  %context = load i64, ptr @binding
  %answer = call i64 @shared(i64 11, i64 13, i64 %context)
  ret i64 %answer
}
define i64 @dependency() {
  %saved = load i64, ptr @binding
  store i64 1019, ptr @binding
  %answer = call i64 @current()
  store i64 %saved, ptr @binding
  ret i64 %answer
}
define void @suppressed_operand() {
  %old = load i64, ptr @operands
  %next = add i64 %old, 1
  store i64 %next, ptr @operands
  ret void
}
define i32 @main() {
  store i64 19, ptr @binding
  call void @suppressed_operand()
  %captured = call i64 @shared(i64 11, i64 13, i64 7)
  %before = call i64 @current()
  %local = call i64 @dependency()
  %after = call i64 @current()
  %ops = load i64, ptr @operands
  %p = call i32 (ptr, ...) @printf(ptr @fmt, i64 %captured, i64 %before, i64 %local, i64 %after)
  %valid = icmp eq i64 %ops, 1
  %exit = select i1 %valid, i32 0, i32 2
  ret i32 %exit
}
'''
    for i in range(cold_count):
        code += f'''define i64 @cold_{i}(i64 %value) {{
  %result = add i64 %value, {i}
  ret i64 %result
}}
'''
    return code


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--repetitions', type=int, default=5)
    args = parser.parse_args()
    assert args.repetitions >= 1
    node = shutil.which('node')
    lli = ROOT / 'build/bin/lli'
    assert node and lli.is_file()
    matrix = [(3, 3, 0), (8, 4, 0), (32, 16, 0),
              (3, 3, 20000), (8, 4, 20000), (32, 16, 20000)]
    rows = []
    for repeat in range(args.repetitions):
        work = [(t, v, n, m) for t, v, n in matrix for m in ('interpret', 'prepared', 'tiered')]
        random.Random(repeat + 827).shuffle(work)
        for types, views, iterations, mode in work:
            command = [node, '--no-wasm-lazy-compilation', '--no-wasm-tier-up',
                       str(HERE / 'world_execution.mjs'), mode, str(types), str(views), str(iterations)]
            result = invoke(command)
            assert result['exit'] == 0 and not result['stderr'], result
            row = json.loads(result['stdout'])
            row.update(repeat=repeat, processSeconds=result['seconds'])
            rows.append(row)
    # Shared scaffolding and all observable results must agree; raw timings are
    # retained. No cross-runtime numerical performance extrapolation is valid.
    observables = ('artifactSha256', 'observations', 'externalResult', 'checksum',
                   'relocation', 'regions', 'trace', 'replacementTrace', 'events', 'checks')
    for types, views, iterations in matrix:
        group = [r for r in rows if (r['typeCount'], r['viewCount'], r['iterations']) == (types, views, iterations)]
        assert all(all(row[key] == group[0][key] for key in observables) for row in group)
    summaries = []
    for types, views, iterations in matrix:
        for mode in ('interpret', 'prepared', 'tiered'):
            group = [r for r in rows if (r['typeCount'], r['viewCount'], r['iterations'], r['mode']) == (types, views, iterations, mode)]
            summaries.append(dict(types=types, views=views, iterations=iterations, mode=mode,
                artifactBytes=group[0]['artifactBytes'], tupleProduct=group[0]['tupleProduct'],
                nativeKernels=group[0]['nativeKernels'], nativeModuleBytes=group[0]['nativeModuleBytes'],
                measurements={k: dict(min=min(r[k] for r in group), median=statistics.median(r[k] for r in group), max=max(r[k] for r in group))
                              for k in ('checkingMs', 'coldMs', 'executionMs', 'compileMs', 'processSeconds')}))
    native_rows = []
    with tempfile.TemporaryDirectory(prefix='cod-architecture-') as temp:
        temp = Path(temp)
        for cold in (10, 100, 1000):
            source = native_probe(cold)
            path = temp / f'world-{cold}.ll'
            path.write_text(source)
            for repeat in range(args.repetitions):
                order = [False, True] if repeat % 2 == 0 else [True, False]
                for whole in order:
                    command = [str(lli), '--jit-kind=orc-lazy', '--orc-lazy-debug=funcs-to-stdout']
                    if whole:
                        command.append('--per-module-lazy')
                    command.append(str(path))
                    result = invoke(command, temp)
                    # Dump names prove which definitions reach the native layer.
                    assert result['exit'] == 0 and '31 43 1043 43\n' in result['stdout'], result
                    materialized = sorted(set(re.findall(r'\bcold_\d+\b', result['stdout'])))
                    assert len(materialized) == (cold if whole else 0), result
                    native_rows.append(dict(coldDefinitions=cold, wholeModule=whole, repeat=repeat,
                        irBytes=len(source.encode()), irSha256=hashlib.sha256(source.encode()).hexdigest(),
                        materializedColdCount=len(materialized), **result))
    report = dict(schema=1, recordedUtc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        baselineCommit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
        platform=platform.platform(), sourceSha256={str(p.relative_to(ROOT)): digest(p) for p in (Path(__file__), HERE / 'world_execution.mjs')},
        tools={'node': {'path': node, 'sha256': digest(node)}, 'lli': {'path': str(lli.relative_to(ROOT)), 'sha256': digest(lli)}},
        repetitions=args.repetitions, rows=rows, summaries=summaries, llvmProbe=native_rows,
        boundaries=['Finite handwritten semantic model, not CoD parsing/inference or proof of general checking completeness.',
            'TLS, owners, complete copies and completion transitions share a JS scaffold. Native methods use real WebAssembly code and shared linear memory; native stack unwinding, shilos integration and multithreaded TLS are unverified.',
            'Remapping copies region bytes to simulate a changed mapping and changes one base record. It proves unchanged relative payload fields in this model, not zero physical remapping cost or production allocator compatibility.',
            'Prepared means all three method kernels compiled before model execution under explicit V8 flags; whole-program AOT coverage is not implemented.',
            'Tiered mode compiles shared method kernels after eight calls. It is not tracing, OSR, deoptimization or full CoD specialization. Eight is an experimental trigger only.',
            'Unknown input tuples use definitions already present in the finite artifact. Admission of new definitions and external graph integration remain unselected.',
            'Semantic dispatch uses distinct type IDs despite equal layout hashes. SHA-256/JSON/finite tree encoding is experimental, not a collision-proof semantic identity or ABI proposal.',
            'LLVM probe emits all IR in both modes. Native materialization counts say nothing about avoiding frontend expansion, semantic checking or final AOT image elimination.',
            'Process timing includes host startup; loop timing includes JS dispatch/owner checks and native boundary calls. Module bytes are WebAssembly bytes, not native code size. Host JS optimization and system load remain factors.'])
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(report, indent=2) + '\n')
    print(f'{len(rows)} model runs and {len(native_rows)} LLVM runs passed; report: {args.report}')


if __name__ == '__main__':
    main()
