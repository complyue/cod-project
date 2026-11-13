#!/usr/bin/env python3
"""Measure a managed regional adapter using the existing JDK and LLVM build.

HotSpot foreign-memory integration is not Truffle integration or an admission
protocol. Compilation uses temporary directories and does not install tools.
"""
from pathlib import Path
import argparse
import datetime
import hashlib
import json
import os
import shutil
import statistics
import subprocess
import tempfile
import time

import regional_program as regional

ROOT, HERE = regional.ROOT, regional.HERE


def digest(data):
    return hashlib.sha256(data).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=HERE / 'managed-runtime-results.json')
    parser.add_argument('--repetitions', type=int, default=3)
    parser.add_argument('--smoke', action='store_true')
    args = parser.parse_args()
    assert args.repetitions > 0
    graph = regional.fixture(); checked = regional.semantic.check(regional.project(graph))
    header = regional.Records(graph, checked).emit()
    ir = regional.Lowering(graph, checked, 'address').emit_module()
    cases = regional.scenarios()
    if args.smoke: cases = [c for c in cases if c['name'] in ('depth-20-source-1', 'expired-owner-mode-2', 'acquire-3-mode-2', 'two-views-one-owner')]
    case_text = []; expected = {}
    for case in cases:
        result, events = regional.oracle(graph, case['arguments'])
        case_text.append(' '.join(map(str, ['CASE', case['name'], *case['arguments'], *result, len(events)])))
        case_text += [' '.join(map(str, e)) for e in events]
        expected[case['name']] = dict(**case, result=result, events=len(events), trace_sha256=digest(json.dumps(events, separators=(',', ':')).encode()))
    case_bytes = ('\n'.join(case_text) + '\n').encode()
    sources = ['managed_runtime.py', 'ManagedRegionalProgram.java', 'managed_adapter.cc', 'regional_fixture.hh', 'regional_program.py', 'semantic_program.py']
    report = dict(schema=1, status='incomplete', experiment='managed control over actual regional records and borrowed native policies', smoke=args.smoke,
                  recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  base_commit=regional.execute(['git', 'rev-parse', 'HEAD'], cwd=ROOT).stdout.strip(),
                  sources={str((HERE / name).relative_to(ROOT)): digest((HERE / name).read_bytes()) for name in sources},
                  tools={}, generated_header_sha256=digest(header.encode()), generated_llvm_sha256=digest(ir.encode()), cases_sha256=digest(case_bytes),
                  repetitions=args.repetitions, checked=checked, cases=expected, rows=[], warmup=dict(rounds=10, modes=3, sample='depth-20-source-1', validated=True),
                  limitations=[
                      'Uses installed OpenJDK HotSpot with preview FFM; does not implement or measure Truffle, GraalVM, RPython or OMR.',
                      'Known fixture, local offsets and explicit signatures only; no production ABI, static owner proof, input admission or world evolution.',
                      'Program data remains in native read-only regions; managed execution frames/tasks are temporary Java heap objects, not migrated or persisted regional state.',
                      'Direct and accessor interpreters share control logic; native LLVM is a separate lowering; every full trace is compared with an independent logical reference.',
                      'Java scopes coordinate owner release on one thread; this does not establish concurrent borrow invalidation or unloading protocols.',
                      'Timed execution excludes setup, mapping, expected-trace comparison and native trace transfer. Managed tracing allocations are timed; native tracing uses reserved storage.',
                      'Thread-allocated bytes measure Java heap allocation only and exclude native regions; region-entry/free counts are tracked separately.',
                      'Native mode is one coarse foreign call; direct/accessor modes interpret records in Java. Results cannot be extrapolated to optimizing-framework throughput.',
                      'Only the earlier regional harness is ASAN validated; the installed JVM is not an ASAN build.'])
    for relative in ('include/shilos/region.hh', 'include/shilos/vector.hh', 'include/shilos/dbmr.hh'):
        report['sources'][relative] = digest((ROOT / relative).read_bytes())
    compiler = ROOT / 'build/bin/clang++'; java = Path(shutil.which('java')).resolve(); javac = Path(shutil.which('javac')).resolve()
    for name, path, flag in [('clang++', compiler, '--version'), ('java', java, '-version'), ('javac', javac, '-version')]:
        version = regional.execute([path, flag]); report['tools'][name] = dict(path=str(path), sha256=digest(path.read_bytes()), version=version.stdout + version.stderr)
    with tempfile.TemporaryDirectory(prefix='cod-managed-regional-') as directory:
        work = Path(directory); (work / 'regional_fixture.inc').write_text(header); (work / 'program.ll').write_text(ir); (work / 'cases.txt').write_bytes(case_bytes)
        flags = ['-std=c++20', '-O1', '-g', '-UNDEBUG', '-fPIC']
        library = work / 'libregional_fixture.so'
        command = [compiler, *flags, '-shared', '-Wl,-Bsymbolic-functions', '-I' + str(ROOT / 'include'), '-I' + str(work), HERE / 'managed_adapter.cc', work / 'program.ll', '-o', library]
        started = time.perf_counter(); result = regional.execute(command)
        report['native_build'] = dict(flags=flags + ['-shared', '-Wl,-Bsymbolic-functions'], seconds=time.perf_counter() - started, library_bytes=library.stat().st_size, stderr=result.stderr)
        started = time.perf_counter(); result = regional.execute([javac, '--enable-preview', '--release', '21', '-d', work, HERE / 'ManagedRegionalProgram.java'])
        report['java_build'] = dict(seconds=time.perf_counter() - started, stderr=result.stderr, class_bytes=sum(p.stat().st_size for p in work.glob('*.class')))
        command = [java, '--enable-preview', '--enable-native-access=ALL-UNNAMED', '-ea', '-cp', work, 'ManagedRegionalProgram', library, work / 'program.dbmr', work / 'cases.txt', args.repetitions]
        started = time.perf_counter(); result = regional.execute(command, timeout=180)
        report['process_seconds'] = time.perf_counter() - started; report['stderr'] = result.stderr
        report['raw_output'] = result.stdout
        for line in result.stdout.splitlines():
            words = line.split()
            if words[0] == 'ROW':
                name, mode = words[1:3]
                values = dict(zip(('repetition', 'mapping', 'execution_nanoseconds', 'java_thread_allocated_bytes', 'execution_foreign_calls', 'field_reads', 'steps', 'peak_frames', 'policy_reads', 'events', 'region_entries', 'region_frees'), map(int, words[3:])))
                assert values['events'] == expected[name]['events'] and values['region_entries'] == values['region_frees']
                report['rows'].append(dict(name=name, mode=mode, **values))
            elif words[0] == 'GRAPH': report['graph'] = dict(zip(('occupied_bytes', 'nodes', 'procedures'), map(int, words[1:])))
        assert len(report['rows']) == len(cases) * 3 * args.repetitions
        assert 'PASS managed regional control' in result.stdout
        assert report['graph'] == {'occupied_bytes': 16056, 'nodes': 168, 'procedures': 5}
        report['comparisons'] = len(report['rows'])
        report['guards'] = ['different native program addresses', 'identical immutable program bytes', 'readonly Java write rejection', 'out-of-bounds Java read rejection', 'cached borrow rejected after owner scope closes', 'native owner released by Java scope close', 'closed program view rejection', 'native root entry/free equality']
        report['summary'] = {}
        for case in cases:
            modes = {}
            for mode in ('direct', 'accessor', 'native'):
                rows = [r for r in report['rows'] if r['name'] == case['name'] and r['mode'] == mode]
                modes[mode] = {key: statistics.median(r[key] for r in rows) for key in ('execution_nanoseconds', 'java_thread_allocated_bytes', 'execution_foreign_calls', 'field_reads', 'peak_frames')}
            assert modes['direct']['field_reads'] == modes['accessor']['field_reads']
            assert modes['accessor']['execution_foreign_calls'] == modes['direct']['execution_foreign_calls'] + modes['direct']['field_reads'] + next(r['policy_reads'] for r in report['rows'] if r['name'] == case['name'] and r['mode'] == 'direct')
            report['summary'][case['name']] = modes
    report['status'] = 'passed'; args.output.write_text(json.dumps(report, indent=2) + '\n')
    print(f'PASS {report["comparisons"]} managed/native full-trace comparisons plus30 warmups; report: {args.output}')
    for name in ('depth-20-source-1', 'depth-1000-source-1'):
        if name in report['summary']: print(name, json.dumps(report['summary'][name]))


if __name__ == '__main__':
    main()
