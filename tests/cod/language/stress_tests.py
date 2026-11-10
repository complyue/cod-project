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
            (directory / f'pure-{i}.cod').write_text(body + 'export f;\n')
        entry = directory / 'pure-main.cod'
        entry.write_text('using symbol "std:io" @print;\nusing "pure-0.cod" f;\n'
                         'behave @print(fold (seed in 1u64 .. 129u64, total = 0u64) { total + f(seed) });\n')
        factor = pow(16807, depth, 2147483647)
        expected = sum(seed * factor % 2147483647 for seed in range(1, 129))
        results.append(run(executable, root, entry, f'{expected}\n', '512-module-pure-chain-loop-inputs'))
        # Reuse the same demanded chain to check that a leaf error identifies
        # both the failing definition and the entry call, without 512 notes.
        (directory / f'pure-{depth-1}.cod').write_text('function f(x::u64)::u64 { @missing }\nexport f;\n')
        command = [str(executable), '--check', str(entry)]
        diagnostic = subprocess.run(command, cwd=root, capture_output=True, text=True, timeout=120)
        diagnostic_passed = (diagnostic.returncode == 2 and diagnostic.stdout == ''
                             and 'a function cannot read symbolic effects' in diagnostic.stderr
                             and f'pure-{depth-1}.cod:1:' in diagnostic.stderr
                             and 'pure-main.cod:3:' in diagnostic.stderr
                             and diagnostic.stderr.count(': note: in call to function ') == 8
                             and f'{depth-8} intervening calls omitted' in diagnostic.stderr)
        results[-1]['leaf_error'] = {'command': command, 'exit_code': diagnostic.returncode,
                                     'stdout': diagnostic.stdout, 'stderr': diagnostic.stderr,
                                     'passed': diagnostic_passed}
        results[-1]['passed'] &= diagnostic_passed
        print(json.dumps({'case': 'deep-leaf-diagnostic', 'passed': diagnostic_passed}), flush=True)

        depth = 1000
        (directory / 'operation-vocabulary.cod').write_text('symbol @base :: i64;\nexport @base;\n')
        (directory / 'operation-leaf.cod').write_text(
            'symbol from "operation-vocabulary.cod" @base;\n'
            'unit Argument;\n'
            'procedure (+)(value::i64(Argument), delta::i64)::i64 { scalar(value) + delta + @base }\n'
            'export Argument;\nexport (+);\n')
        lines = ['using symbol "std:io" @print;',
                 'symbol from "operation-vocabulary.cod" @base;',
                 'using units "operation-leaf.cod" {Argument};', 'using "operation-leaf.cod" (+);',
                 'effect @base <- 5;']
        for i in range(depth):
            lines.append(f'symbol @p{i} :: proc(i64) -> i64;')
            body = 'Argument(x) + 0' if i == 0 else f'behave @p{i-1}(x + 1)'
            lines.append(f'procedure @p{i}(x::i64)::i64 {{ {body} }}')
        lines += ['effect @base <- 9;', f'behave @print(perform @p{depth-1}(0));',
                  f'behave @print(behave @p{depth-1}(0));', 'behave @print(@base);']
        entry = directory / 'procedure-main.cod'
        entry.write_text('\n'.join(lines) + '\n')
        results.append(run(executable, root, entry, f'{depth-1+5}\n{depth-1+9}\n9\n',
                           '1000-procedure-inline-chain-with-operation-leaf'))
        (directory / 'operation-leaf.cod').write_text(
            'symbol @missing :: i64;\n'
            'unit Argument;\n'
            'procedure (+)(value::i64(Argument), delta::i64)::i64 { @missing }\n'
            'export Argument;\nexport (+);\n')
        command = [str(executable), '--check', str(entry)]
        diagnostic = subprocess.run(command, cwd=root, capture_output=True, text=True, timeout=120)
        diagnostic_passed = (diagnostic.returncode == 2 and diagnostic.stdout == ''
                             and 'perform requires an unavailable definition-time capture' in diagnostic.stderr
                             and 'operation-leaf.cod:3:' in diagnostic.stderr
                             and 'procedure-main.cod:' in diagnostic.stderr
                             and diagnostic.stderr.count(': note: in call to procedure ') == 8
                             and f'{depth+1-8} intervening calls omitted' in diagnostic.stderr)
        results[-1]['leaf_error'] = {'command': command, 'exit_code': diagnostic.returncode,
                                     'stdout': diagnostic.stdout, 'stderr': diagnostic.stderr,
                                     'passed': diagnostic_passed}
        results[-1]['passed'] &= diagnostic_passed
        print(json.dumps({'case': 'deep-operation-binding-failure', 'passed': diagnostic_passed}), flush=True)

        layers = 10
        for level in range(layers, -1, -1):
            for side in ('a', 'b'):
                body = ''
                if level < layers:
                    body = (f'using "dag-{level+1}-a.cod" f as a;\n'
                            f'using "dag-{level+1}-b.cod" f as b;\n')
                body += 'function f(x::i64)::i64 { ' + ('a(x) + b(x)' if level < layers else 'x + 1') + ' }\n'
                (directory / f'dag-{level}-{side}.cod').write_text(body + 'export f;\n')
        entry = directory / 'dag-main.cod'
        entry.write_text('using symbol "std:io" @print;\nusing "dag-0-a.cod" f;\nbehave @print(f(2));\n')
        results.append(run(executable, root, entry, f'{3 * 2**layers}\n', 'layered-diamond-22-modules-2047-calls'))

        units = 32
        names = ', '.join(f'unit{i}' for i in range(units))
        definitions = [f'unit unit{i};' for i in range(units)]
        definitions += [f'conversion function step{i}(x::i64(unit{i}))::i64(unit{i+1}) {{ unit{i+1}(scalar(x) + 1) }}'
                        for i in range(units-1)]
        definitions += ['conversion function back(x::i64(unit16))::i64(unit15) { unit15(scalar(x) - 1) }']
        unit_exports = ''.join(f'export unit{i};\n' for i in range(units))
        conversion_exports = ''.join(f'export step{i};\n' for i in range(units-1)) + 'export back;\n'
        (directory / 'unit-base.cod').write_text('\n'.join(definitions) + '\n' + unit_exports + conversion_exports)
        previous = 'unit-base.cod'
        for i in range(16):
            current = f'unit-layer-{i}.cod'
            (directory / current).write_text(f'using units "{previous}" {{{names}}};\n'
                                             'function finish(x::i64(unit31*))::i64 { scalar(x) }\n'
                                             'export finish;\n' + unit_exports + conversion_exports)
            previous = current
        entry = directory / 'unit-main.cod'
        entry.write_text('using symbol "std:io" @print;\n'
                         f'using units "{previous}" {{{names}}};\n'
                         f'using "{previous}" finish;\nbehave @print(finish(unit0(1)));\n')
        results.append(run(executable, root, entry, '32\n', '16-unit-reexports-31-conversion-steps-with-cycle'))

        (directory / 'regional-types.cod').write_text(
            'data Node { value::f32; next::ref<Node>; }\n'
            'function create(region arena, value::f32)::ref<Node> { alloc arena Node(value, null) }\n'
            'export Node;\nexport create;\n')
        (directory / 'regional-interface.cod').write_text('type Link;\nexport Link;\n')
        depth = 96
        for i in range(depth):
            parent = f'regional-interface-{i+1}.cod' if i + 1 < depth else 'regional-interface.cod'
            (directory / f'regional-interface-{i}.cod').write_text(
                f'using "{parent}" Link as Parent;\ntype Link <: Parent;\nexport Link;\n')
        (directory / 'regional-adapter.cod').write_text(
            'export import "regional-types.cod" Node;\nusing "regional-interface-0.cod" Link;\ndata Node <: Link;\n')
        (directory / 'regional-rank.cod').write_text(
            'using "regional-interface.cod" Link;\nfunction score(node::Link)::i64 { -1 }\nexport score;\n')
        for name, value in [('first', 2), ('second', 3)]:
            (directory / f'regional-rank-{name}.cod').write_text(
                'using "regional-adapter.cod" Node;\n'
                'import "regional-rank.cod" score as weight;\n'
                f'function weight(node::Node)::i64 {{ {value} }}\n'
                'import "regional-0.cod" shared_measure as chain;\n'
                'function measure(node::Node)::i64 { chain(node) }\nexport weight;\nexport measure;\n')
        (directory / 'regional-vocabulary.cod').write_text('using "regional-types.cod" Node;\nsymbol @root :: ref<Node>;\nexport @root;\n')
        (directory / 'regional-body.cod').write_text(
            'using "regional-types.cod" Node;\n'
            'symbol from "regional-vocabulary.cod" @root as @input;\n'
            'symbol @walk :: proc() -> ref<Node>;\nfunction find(node::Node)::ref<Node> { node.next }\n'
            'procedure @walk()::ref<Node> { find(deref(@input)) }\nexport @walk;\n')
        for i in range(depth):
            body = ('using "regional-types.cod" Node;\nusing "regional-interface.cod" Link as Root;\n'
                    f'using "regional-interface-{i}.cod" Link;\n')
            body += ('symbol from "regional-vocabulary.cod" @root;\n'
                     f'symbol @node{i} :: ref<Node>;\neffect @node{i} = @root;\nsymbol @walk :: proc() -> ref<Node>;\n')
            if i + 1 < depth:
                body += f'using "regional-{i+1}.cod" find as next;\nusing "regional-{i+1}.cod" retain as retain_next;\n'
                body += f'using "regional-{i+1}.cod" measure as measure_next;\n'
                body += f'using "regional-{i+1}.cod" borrow as borrow_next;\n'
            body += 'using "regional-rank.cod" score as weigh;\n'
            if i + 1 < depth:
                body += f'import "regional-{i+1}.cod" shared_measure as shared_next;\n'
            body += 'function shared_measure(node::Link)::i64 { '
            body += ('shared_next(node) + ' if i + 1 < depth else '') + 'weigh(node) }\n'
            body += 'function borrow(region arena)-> region { ' + ('borrow_next(arena)' if i + 1 < depth else 'arena') + ' }\n'
            body += 'function retain(node::Link)::Root { ' + ('retain_next(node)' if i + 1 < depth else 'node') + ' }\n'
            body += f'function rank(node::Link)::i64 {{ {i} }}\nfunction rank(node::Root)::i64 {{ -1 }}\n'
            body += 'function measure(node::Link)::i64 { rank(node)' + (' + measure_next(node)' if i + 1 < depth else '') + ' }\n'
            body += 'function find(node::Node)::ref<Node> { '
            body += ('next(deref(node.next))' if i + 1 < depth else 'node.next') + ' }\n'
            source = f'regional-{i+1}.cod' if i + 1 < depth else 'regional-body.cod'
            role = f'node{i+1}' if i + 1 < depth else 'input'
            body += f'mixin "{source}" @walk as @walk with {{ @node{i} as @{role}; }};\n'
            body += 'export find;\nexport retain;\nexport measure;\nexport borrow;\nexport shared_measure;\nexport @walk;\n'
            body += f'export @node{i};\n'
            (directory / f'regional-{i}.cod').write_text(body)
        entry = directory / 'regional-main.cod'
        entry.write_text('using symbol "std:io" @print;\nusing symbol "std:io" @assert;\n'
                         'using "regional-adapter.cod" Node;\nusing "regional-types.cod" create;\n'
                         'symbol from "regional-vocabulary.cod" @root as @head;\n'
                         'region symbol @arena;\nsymbol @tail :: ref<Node>;\nsymbol @alternate :: ref<Node>;\n'
                         'effect @arena <- region(16384u64);\n'
                         'effect @tail = alloc @arena Node(11.0f32, null);\n'
                         f'effect @head = fold (i in 0u32 .. {depth}u32, node = @tail) {{\n'
                         '  alloc @arena Node(f32(i), node)\n};\n'
                         'using "regional-0.cod" find;\nusing "regional-0.cod" retain;\nusing symbol "regional-0.cod" @walk;\n'
                         'behave @print(retain(deref(@head)).value);\n'
                         'using "regional-0.cod" measure;\nbehave @print(measure(deref(@head)));\n'
                         'symbol from "regional-0.cod" @node0 as @selected;\n'
                         'behave @print(deref(find(deref(@head))).value);\n'
                         'setfield(@tail, value, 29.0f32);\n'
                         'behave @print(deref(find(deref(@head))).value);\n'
                         'using "regional-0.cod" borrow;\n'
                         '{\nlet original = @arena;\nlet old_used = used_bytes(original);\n'
                         'effect @arena <- region(4096u64);\n'
                         'let borrowed = borrow(@arena);\nlet initial = used_bytes(borrowed);\n'
                         'effect @alternate = create(borrowed, 47.0f32);\n'
                         'behave @assert(used_bytes(borrowed) > initial);\n'
                         'behave @assert(used_bytes(original) == old_used);\n'
                         'setfield(@alternate, next, @alternate);\n'
                         'effect @selected = @alternate;\n'
                         'behave @print(deref(perform @walk()).value);\n'
                         'behave @print(deref(behave @walk()).value);\n};\n'
                         'using "regional-rank-first.cod" measure as first;\n'
                         'using "regional-rank-second.cod" measure as second;\n'
                         'using "regional-0.cod" shared_measure;\n'
                         'behave @print(first(deref(@head)));\n'
                         'behave @print(second(deref(@head)));\n'
                         'behave @print(shared_measure(deref(@head)));\n')
        results.append(run(executable, root, entry, f'95\n{sum(range(depth))}\n11\n29\n29\n47\n{2*depth}\n{3*depth}\n{-depth}\n',
                           '96-level-regional-borrow-mixin-and-nominal-dispatch-chain'))
        # A same-alias union supplies both providers to an imported helper.
        # Separately used helpers retain their own implementation contexts.
        entry.write_text(entry.read_text() +
                         'using "regional-rank-first.cod" weight;\n'
                         'using "regional-rank-second.cod" weight;\n'
                         'import "regional-0.cod" shared_measure as combined;\n'
                         'combined(deref(@head));\n')
        command = [str(executable), '--check', str(entry)]
        diagnostic = subprocess.run(command, cwd=root, capture_output=True, text=True, timeout=120)
        diagnostic_passed = (diagnostic.returncode == 2 and diagnostic.stdout == ''
                             and "ambiguous method 'weigh'" in diagnostic.stderr
                             and f'regional-{depth-1}.cod:' in diagnostic.stderr
                             and 'regional-main.cod:' in diagnostic.stderr
                             and diagnostic.stderr.count(': note: in call to function ') == 8
                             and f'{depth-8} intervening calls omitted' in diagnostic.stderr)
        results[-1]['implementation_conflict'] = {'command': command, 'exit_code': diagnostic.returncode,
                                                 'stdout': diagnostic.stdout, 'stderr': diagnostic.stderr,
                                                 'passed': diagnostic_passed}
        results[-1]['passed'] &= diagnostic_passed
        print(json.dumps({'case': 'deep-import-implementation-conflict', 'passed': diagnostic_passed}), flush=True)

    if report := os.environ.get('COD_LANGUAGE_STRESS_REPORT'):
        sources = [root / 'cod/lang_syntax.hh', root / 'cod/lang_frontend.hh', root / 'cod/lang_llvm.hh',
                   root / 'cod/lang_native_runtime.hh', root / 'include/shilos/region.hh', Path(__file__).resolve()]
        Path(report).write_text(json.dumps({
            'schema_version': 2, 'recorded_utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
            'platform': platform.platform(), 'results': results,
            'executable': {'path': str(executable.relative_to(root)),
                           'sha256': hashlib.sha256(executable.read_bytes()).hexdigest()},
            'notes': ['Each demanded program is checked/emitted once, then linked with its emitted native services.',
                      'Executable size includes debug metadata from the C++ system services; .text is recorded separately.',
                      'The pure chain uses loop-varying inputs; other fixed-input cases permit constant folding. Sizes are not worst-case bounds.',
                      'The ordinary language corpus separately exercises the --run driver path.'],
            'source_sha256': {str(path.relative_to(root)): hashlib.sha256(path.read_bytes()).hexdigest()
                              for path in sources}}, indent=2) + '\n')
    return 0 if all(r['passed'] for r in results) else 1


if __name__ == '__main__':
    raise SystemExit(main())
