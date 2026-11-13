#!/usr/bin/env python3
"""Measure LLVM materialization independently of frontend lowering.

Uses the existing build/bin/lli in a temporary directory. Does not configure,
install, clean or modify the build tree. Run only when experiments are authorized.
"""
from pathlib import Path
import argparse
import datetime
import hashlib
import json
import platform
import re
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
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--repetitions', type=int, default=5)
    args = parser.parse_args()
    assert args.repetitions >= 1
    lli = ROOT / 'build/bin/lli'
    assert lli.is_file()
    rows = []
    with tempfile.TemporaryDirectory(prefix='cod-materialization-') as directory:
        work = Path(directory)
        for cold in (10, 100, 1000):
            source = native_probe(cold)
            path = work / f'world-{cold}.ll'
            path.write_text(source)
            for repeat in range(args.repetitions):
                order = [False, True] if repeat % 2 == 0 else [True, False]
                for whole in order:
                    command = [str(lli), '--jit-kind=orc-lazy', '--orc-lazy-debug=funcs-to-stdout']
                    if whole:
                        command.append('--per-module-lazy')
                    command.append(str(path))
                    result = invoke(command, work)
                    assert result['exit'] == 0 and '31 43 1043 43\n' in result['stdout'], result
                    materialized = sorted(set(re.findall(r'\bcold_\d+\b', result['stdout'])))
                    assert len(materialized) == (cold if whole else 0), result
                    rows.append(dict(coldDefinitions=cold, wholeModule=whole, repeat=repeat,
                        irBytes=len(source.encode()), irSha256=hashlib.sha256(source.encode()).hexdigest(),
                        materializedColdCount=len(materialized), **result))
    report = dict(schema=1, experiment='LLVM function-level versus whole-module materialization',
        recordedUtc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        baselineCommit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
        platform=platform.platform(), sourceSha256={str(Path(__file__).relative_to(ROOT)): digest(__file__)},
        tools={'lli': {'path': str(lli.relative_to(ROOT)), 'sha256': digest(lli)}},
        repetitions=args.repetitions, llvmProbe=rows,
        boundaries=['Both modes receive the same complete generated LLVM IR; this does not avoid frontend expansion or prove general semantic checking.',
                    'Materialization counts establish emission granularity, not final AOT image elimination or a production code-lifetime protocol.',
                    'TLS and suppressed-operand checks cover this scalar fixture only; complete captures, ownership and exception paths require separate evidence.',
                    'Process durations include startup, code generation and trace output; they are not steady-state execution throughput.'])
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(report, indent=2) + '\n')
    print(f'{len(rows)} LLVM materialization runs passed; report: {args.report}')


if __name__ == '__main__':
    main()
