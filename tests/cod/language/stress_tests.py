#!/usr/bin/env python3
"""Measure native CoD composition on generated dependency/call graphs."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import tempfile
import time


def run(executable, root, entry, expected, label):
    generated, bitcode, runtime = [entry.with_suffix(suffix) for suffix in ('.ll', '.bc', '.cc')]
    program = entry.with_suffix('.native')
    toolchain = executable.parent.parent
    commands = [
        ('check', [executable, '--check', entry, '--emit-llvm', generated,
                   '--emit-bc', bitcode, '--emit-runtime', runtime]),
        ('compile_link', [toolchain / 'bin/clang++', '-std=c++20', '-O2', '-g',
                          '-stdlib=libc++', '-fuse-ld=lld', '-I' + str(root / 'include'),
                          bitcode, runtime, '-o', program, '-L' + str(toolchain / 'lib'),
                          '-Wl,-rpath,' + str(toolchain / 'lib')]),
        ('run', [program]),
        ('size', [toolchain / 'bin/llvm-size', '--format=sysv', '--radix=10', program]),
    ]
    phases = {}
    for phase, command in commands:
        command = [str(arg) for arg in command]
        started = time.monotonic()
        process = subprocess.run(command, cwd=root, capture_output=True, text=True, timeout=120)
        phases[phase] = {'command': command, 'seconds': time.monotonic() - started,
                         'exit_code': process.returncode, 'stdout': process.stdout, 'stderr': process.stderr}
        if process.returncode or process.stderr:
            break
    sections = {}
    for line in phases.get('size', {}).get('stdout', '').splitlines():
        fields = line.split()
        if len(fields) == 3 and fields[1].isdigit():
            sections[fields[0]] = int(fields[1])
    text_bytes = sections.get('.text', sections.get('__text'))
    result = {'case': label, 'backend': 'llvm', 'phases': phases, 'expected_stdout': expected,
              'llvm_ir_bytes': generated.stat().st_size if generated.exists() else None,
              'bitcode_bytes': bitcode.stat().st_size if bitcode.exists() else None,
              'native_executable_bytes': program.stat().st_size if program.exists() else None,
              'native_text_bytes': text_bytes}
    result['passed'] = (len(phases) == len(commands) and text_bytes is not None
                        and all(p['exit_code'] == 0 and p['stderr'] == '' for p in phases.values())
                        and phases['check']['stdout'] == '' and phases['compile_link']['stdout'] == ''
                        and phases['run']['stdout'] == expected)
    print(json.dumps(result), flush=True)
    return result


def main():
    root = Path(__file__).resolve().parents[3]
    executable = root / os.environ.get('COD_TEST_TOOLCHAIN', 'build') / 'bin/cod'
    results = []
    with tempfile.TemporaryDirectory(prefix='cod-composition-stress-') as scratch:
        directory = Path(scratch)
        depth = 512
        for i in range(depth):
            body = f'using "pure-{i+1}.cod" f as next;\n' if i + 1 < depth else ''
            step = '(x * 16807u64) % 2147483647u64'
            body += 'function f(x::u64)::u64 { ' + (f'next({step})' if i + 1 < depth else step) + ' }\n'
            (directory / f'pure-{i}.cod').write_text(body)
        entry = directory / 'pure-main.cod'
        entry.write_text('using symbol "std:io" @print;\nusing "pure-0.cod" f;\n'
                         'behave @print(fold (seed in 1u64 .. 129u64, total = 0u64) { total + f(seed) });\n')
        factor = pow(16807, depth, 2147483647)
        expected = sum(seed * factor % 2147483647 for seed in range(1, 129))
        results.append(run(executable, root, entry, f'{expected}\n', '512-module-pure-chain-loop-inputs'))

        depth = 1000
        lines = ['using symbol "std:io" @print;', 'symbol @base;', 'effect @base = 5;']
        for i in range(depth):
            lines.append(f'symbol @p{i};')
            body = '@base + x' if i == 0 else f'behave @p{i-1}(x + 1)'
            lines.append(f'procedure @p{i}(x::i64)::i64 {{ {body} }}')
        lines += ['effect @base = 9;', f'behave @print(perform @p{depth-1}(0));',
                  f'behave @print(behave @p{depth-1}(0));', 'behave @print(@base);']
        entry = directory / 'procedure-main.cod'
        entry.write_text('\n'.join(lines) + '\n')
        results.append(run(executable, root, entry, f'{depth-1+5}\n{depth-1+9}\n9\n', '1000-procedure-inline-chain'))

        layers = 10
        for level in range(layers, -1, -1):
            for side in ('a', 'b'):
                body = ''
                if level < layers:
                    body = (f'using "dag-{level+1}-a.cod" f as a;\n'
                            f'using "dag-{level+1}-b.cod" f as b;\n')
                body += 'function f(x::i64)::i64 { ' + ('a(x) + b(x)' if level < layers else 'x + 1') + ' }\n'
                (directory / f'dag-{level}-{side}.cod').write_text(body)
        entry = directory / 'dag-main.cod'
        entry.write_text('using symbol "std:io" @print;\nusing "dag-0-a.cod" f;\nbehave @print(f(2));\n')
        results.append(run(executable, root, entry, f'{3 * 2**layers}\n', 'layered-diamond-22-modules-2047-calls'))

        units = 32
        names = ', '.join(f'unit{i}' for i in range(units))
        definitions = [f'unit unit{i}::i64;' for i in range(units)]
        definitions += [f'conversion function step{i}(x (unit{i}))::unit{i+1} {{ unit{i+1}(scalar(x) + 1) }}'
                        for i in range(units-1)]
        definitions += ['conversion function back(x (unit16))::unit15 { unit15(scalar(x) - 1) }']
        (directory / 'unit-base.cod').write_text('\n'.join(definitions) + '\n')
        previous = 'unit-base.cod'
        for i in range(16):
            current = f'unit-layer-{i}.cod'
            (directory / current).write_text(f'using units "{previous}" {{{names}}};\n'
                                             'function finish(x (unit31*))::i64 { scalar(x) }\n')
            previous = current
        entry = directory / 'unit-main.cod'
        entry.write_text('using symbol "std:io" @print;\n'
                         f'using units "{previous}" {{{names}}};\n'
                         f'using "{previous}" finish;\nbehave @print(finish(unit0(1)));\n')
        results.append(run(executable, root, entry, '32\n', '16-unit-reexports-31-conversion-steps-with-cycle'))

        (directory / 'regional-types.cod').write_text(
            'regional type Node { value::f32; next::ref<Node>; }\n')
        depth = 96
        for i in range(depth):
            body = 'using "regional-types.cod" Node;\n'
            if i + 1 < depth:
                body += f'using "regional-{i+1}.cod" find as next;\n'
            body += 'function find(node::Node)::ref<Node> { '
            body += ('next(deref(node.next))' if i + 1 < depth else 'node.next') + ' }\n'
            (directory / f'regional-{i}.cod').write_text(body)
        entry = directory / 'regional-main.cod'
        entry.write_text('using symbol "std:io" @print;\nusing "regional-types.cod" Node;\n'
                         'using "regional-0.cod" find;\nsymbol @arena;\nsymbol @head;\nsymbol @tail;\n'
                         'effect @arena = region(16384u64);\n'
                         'effect @tail = alloc @arena Node(11.0f32, null);\n'
                         f'effect @head = fold (i in 0u32 .. {depth}u32, node = @tail) {{\n'
                         '  alloc @arena Node(f32(i), node)\n};\n'
                         'behave @print(deref(find(deref(@head))).value);\n'
                         'setfield(@tail, value, 29.0f32);\n'
                         'behave @print(deref(find(deref(@head))).value);\n')
        results.append(run(executable, root, entry, '11\n29\n', '96-module-regional-snapshot-chain'))
    if report := os.environ.get('COD_LANGUAGE_STRESS_REPORT'):
        sources = [root / 'cod/lang_syntax.hh', root / 'cod/lang_frontend.hh', root / 'cod/lang_llvm.hh',
                   root / 'cod/lang_native_runtime.hh', root / 'include/shilos/region.hh', Path(__file__).resolve()]
        Path(report).write_text(json.dumps({
            'schema_version': 2, 'recorded_utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
            'platform': platform.platform(), 'results': results,
            'notes': ['Each demanded program is checked/emitted once, then linked with its emitted native services.',
                      'Executable size includes debug metadata from the C++ system services; .text is recorded separately.',
                      'The pure chain uses loop-varying inputs; other fixed-input cases permit constant folding. Sizes are not worst-case bounds.',
                      'The ordinary language corpus separately exercises the --run driver path.'],
            'source_sha256': {str(path.relative_to(root)): hashlib.sha256(path.read_bytes()).hexdigest()
                              for path in sources}}, indent=2) + '\n')
    return 0 if all(r['passed'] for r in results) else 1


if __name__ == '__main__':
    raise SystemExit(main())
