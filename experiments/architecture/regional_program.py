#!/usr/bin/env python3
"""Known regional program fixture and native borrow-boundary experiment.

Uses the existing mapper and compiler; defines no CoD storage/ABI/admission
protocol. Dynamic guards are experiment assertions, not replacements for CoD's
required static owner checks. Temporary files stay outside the main build tree.
"""
from copy import deepcopy
from pathlib import Path
import argparse
import datetime
import hashlib
import json
import os
import re
import subprocess
import tempfile
import time

import semantic_program as semantic

ROOT, HERE = semantic.ROOT, semantic.HERE
KINDS = {'i64': 0, 'bool': 1, 'view': 2}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def fixture():
    graph = semantic.fixture()
    for view, name in enumerate(('stock_base', 'stock_a', 'stock_b')):
        expression = graph['functions'][name]['body'][0]['value']
        expression['args'][0]['args'][1] = {'op': 'policy', 'view': view, 'args': []}
    return graph


def project(graph, divisors=(2, 3, 5)):
    """Type-check i64 policy reads via a checked scalar projection, not inference."""
    def visit(value):
        if isinstance(value, dict):
            if value.get('op') == 'policy':
                semantic.require(value.get('view') in (0, 1, 2) and value.get('args') == [], 'invalid policy read')
                return semantic.const(divisors[value['view']])
            return {key: visit(child) for key, child in value.items()}
        if isinstance(value, list): return [visit(child) for child in value]
        return value
    return visit(graph)


class Records:
    """Fixture-only flattening, with typed nodes and self-relative links in C++."""
    def __init__(self, graph, checked):
        self.graph = graph
        self.names = checked['reachable']
        self.methods = {name: index for index, name in enumerate(self.names)}
        self.nodes = [dict(op='nop', value=0, slot=0, type=0, a=0, b=0, c=0, next=0)]
        self.procedures = []

    def node(self, op, **fields):
        record = dict(op=op, value=0, slot=0, type=0, a=0, b=0, c=0, next=0)
        record.update(fields); self.nodes.append(record)
        return len(self.nodes) - 1

    def chain(self, ids):
        for left, right in zip(ids, ids[1:]): self.nodes[left]['next'] = right
        return ids[0] if ids else 0

    def expression(self, e):
        if 'const' in e: return self.node('constant', value=e['const'], type=KINDS[e['type']])
        if 'var' in e: return self.node('reg', slot=self.slots[e['var']], type=KINDS[self.types[e['var']]])
        op = e['op']; children = [self.expression(a) for a in e.get('args', [])]
        kind = 'view' if op == 'current_view' else 'bool' if op == 'eq' else 'i64'
        if op == 'select': kind = next(k for k, v in KINDS.items() if v == self.nodes[children[1]]['type'])
        return self.node(op, value=e.get('view', 0), type=KINDS[kind], **dict(zip(('a', 'b', 'c'), children)))

    def statement(self, s):
        op = s['op']; fields = {}
        if op in ('let', 'set', 'call', 'invoke', 'lease'): fields['slot'] = self.slots[s['name']]
        if op in ('let', 'set', 'return', 'throw', 'release'): fields['a'] = self.expression(s['value'])
        if op == 'if':
            fields.update(a=self.expression(s['condition']), b=self.block(s['yes']), c=self.block(s['no']))
        if op == 'event': fields.update(zip(('a', 'b', 'c'), (self.expression(s[k]) for k in ('tag', 'a', 'b'))))
        if op == 'install': fields['a'] = self.expression(s['view'])
        if op == 'defer': fields.update(a=self.block(s['body']), b=self.block(s['catch'] or []), c=self.block(s['finally_']), value=int(s['catch'] is not None))
        if op == 'call': fields.update(value=self.methods[s['target']], a=self.chain([self.expression(e) for e in s['args']]))
        if op == 'invoke':
            assert s['family'] == 'stock'
            fields.update(a=self.expression(s['view']), b=self.chain([self.expression(e) for e in s['args']]))
        return self.node({'if': 'branch', 'return': 'ret', 'throw': 'raise'}.get(op, op), **fields)

    def block(self, statements):
        return self.chain([self.statement(s) for s in statements])

    def emit(self):
        for name in self.names:
            f = self.graph['functions'][name]
            self.types = dict(f['params']); self.types['error'] = 'i64'
            for s in semantic.walk_statements(f['body']):
                if s['op'] in ('let', 'call', 'invoke', 'lease'): self.types[s['name']] = s.get('type', 'i64')
            self.slots = {name: i for i, name in enumerate(self.types)}
            assert len(self.slots) <= 32 and len(f['params']) <= 8
            body = self.block(f['body'])
            slots = [self.slots[p] for p in f['params']] + [0] * (8 - len(f['params']))
            self.procedures.append((body, len(f['params']), self.slots['error'], KINDS[f['result']], slots))
        lines = ['// Generated known fixture; no persistent format or ABI commitment.', f'static constexpr std::array<NodeSeed, {len(self.nodes)}> fixture_nodes{{{{']
        for n in self.nodes:
            lines.append('  {Op::' + n['op'] + ', ' + ', '.join(str(n[k]) for k in ('value', 'slot', 'type', 'a', 'b', 'c', 'next')) + '},')
        lines += ['}};', f'static constexpr std::array<ProcSeed, {len(self.procedures)}> fixture_procedures{{{{']
        for body, count, error, kind, slots in self.procedures:
            lines.append(f'  {{{body}, {count}, {error}, {kind}, {{{", ".join(map(str, slots))}}}}},')
        lines += ['}};', 'static constexpr std::array<std::uint32_t, 3> fixture_views{' + ', '.join(str(self.methods[v['families']['stock']]) for v in self.graph['views']) + '};', f'static constexpr std::uint32_t fixture_entry = {self.methods[self.graph["entry"]]};']
        return '\n'.join(lines) + '\n'


class Lowering(semantic.Lowering):
    def __init__(self, graph, checked, boundary):
        super().__init__(graph, checked); self.boundary = boundary
        self.destinations = None

    def checked_path(self, valid, error):
        passed, failed = self.unique('boundary_ok'), self.unique('boundary_fail')
        self.emit(f'br i1 {valid}, label %{passed}, label %{failed}')
        self.label(failed); self.abrupt('throw', str(error), *self.destinations)
        self.label(passed)

    def expression(self, e):
        if e.get('op') != 'policy': return super().expression(e)
        view = e['view']
        if self.boundary == 'address':
            pointer = self.reg(f'call ptr @exp_policy_address(i64 {view})')
            self.checked_path(self.reg(f'icmp ne ptr {pointer}, null'), 905)
            return self.reg(f'load i64, ptr {pointer}')
        slot = self.reg('alloca i64')
        status = self.reg(f'call i64 @exp_policy_value(i64 {view}, ptr {slot})')
        self.checked_path(self.reg(f'icmp eq i64 {status}, 0'), 905)
        return self.reg(f'load i64, ptr {slot}')

    def block(self, statements, return_to, throw_to):
        previous = self.destinations; self.destinations = (return_to, throw_to)
        try:
            for s in statements:
                if s['op'] == 'lease':
                    token = self.reg('call i64 @exp_acquire()')
                    self.checked_path(self.reg(f'icmp ne i64 {token}, 0'), 904)
                    self.store(token, 'local_' + s['name']); self.store('1', 'live_' + s['name'])
                    count = self.reg('load i64, ptr @acquired'); count = self.reg(f'add i64 {count}, 1')
                    self.emit(f'store i64 {count}, ptr @acquired')
                else:
                    if s['op'] == 'release':
                        status = self.reg(f'call i64 @exp_release(i64 {self.expression(s["value"])})')
                        valid = self.reg(f'icmp eq i64 {status}, 0')
                        passed, failed = self.unique('released_owner'), self.unique('invalid_owner')
                        self.emit(f'br i1 {valid}, label %{passed}, label %{failed}')
                        self.label(failed); self.emit('call void @exit(i32 97)'); self.emit('unreachable'); self.label(passed)
                    if super().block([s], return_to, throw_to): return True
            return False
        finally:
            self.destinations = previous

    def emit_module(self):
        self.lines = ['%Completion = type { i1, i64 }', '@binding = thread_local global i64 0', '@acquired = global i64 0', '@released = global i64 0',
                      'declare void @exit(i32) noreturn', 'declare void @event(i64, i64, i64) nounwind',
                      'declare i64 @exp_acquire() nounwind', 'declare i64 @exp_release(i64) nounwind',
                      'declare ptr @exp_policy_address(i64) nounwind', 'declare i64 @exp_policy_value(i64, ptr) nounwind']
        for name in self.checked['reachable']: self.function(name)
        self.lines.append('define void @exp_native(ptr %inputs, i64 %initial, ptr %out) {'); self.label('entry')
        self.emit('store i64 %initial, ptr @binding'); self.emit('store i64 0, ptr @acquired'); self.emit('store i64 0, ptr @released')
        args = []
        for i in range(5):
            ptr = self.reg(f'getelementptr i64, ptr %inputs, i64 {i}'); args.append(self.reg(f'load i64, ptr {ptr}'))
        result = self.reg('call %Completion @' + self.graph['entry'] + '(' + ', '.join('i64 ' + a for a in args) + ')')
        thrown = self.reg(f'extractvalue %Completion {result}, 0')
        values = [self.reg(f'zext i1 {thrown} to i64'), self.reg(f'extractvalue %Completion {result}, 1')]
        values += [self.reg(f'load i64, ptr @{name}') for name in ('binding', 'acquired', 'released')]
        for i, value in enumerate(values):
            ptr = self.reg(f'getelementptr i64, ptr %out, i64 {i}'); self.emit(f'store i64 {value}, ptr {ptr}')
        self.emit('ret void'); self.lines.append('}')
        return ('\n'.join(self.lines) + '\n').replace('@event(', '@exp_event(')


def scenarios():
    rows = []
    def add(name, n=8, source=1, body=-1, cleanup=-1, mode=0, initial=0, acquire=-1, alias=0, expired=0, divisor=2):
        rows.append(dict(name=name, arguments=[n, source, body, cleanup, mode, initial, acquire, alias, expired, divisor]))
    for depth in (4, 20, 200, 1000):
        for source in (0, 1, 2): add(f'depth-{depth}-source-{source}', n=depth, source=source)
    for mode in range(4):
        add(f'body-mode-{mode}', body=4, mode=mode)
        add(f'cleanup-mode-{mode}', cleanup=4, mode=mode)
    add('body-and-cleanup', body=4, cleanup=4, mode=2)
    for initial in (1, 2): add(f'copied-selected-view-{initial}', initial=initial)
    add('policy-payload-7', divisor=7)
    add('two-views-one-owner', divisor=7, alias=1)
    for attempt in (1, 3):
        for mode in (0, 1, 2, 3): add(f'acquire-{attempt}-mode-{mode}', acquire=attempt, mode=mode)
    for mode in range(4): add(f'expired-owner-mode-{mode}', expired=1, mode=mode)
    add('expired-aliased-owner', initial=1, expired=1, alias=1, mode=2)
    return rows


def oracle(graph, arguments):
    n, source, body, cleanup, mode, initial, acquire, alias, expired, divisor = arguments
    g = project(graph, (divisor, divisor if alias else 3, 5)); g['initial_view'] = initial
    # Independent logical fault projection. The executable injects real region
    # construction failure and checks a real borrowed owner's lifetime instead.
    if 1 <= acquire <= n:
        statements = g['functions']['checkout']['body']
        statements.insert(1, semantic.branch(semantic.op('eq', semantic.var('n'), semantic.const(n - acquire + 1)), [semantic.stmt('throw', value=semantic.const(904))]))
    if expired:
        for name in ('stock_base', 'stock_a') if alias else ('stock_base',):
            g['functions'][name]['body'] = [semantic.stmt('throw', value=semantic.const(905))]
    semantic.check(g)
    machine = semantic.Reference(g); result = machine.run([n, source, body, cleanup, mode])
    return [int(result.kind == 'throw'), result.value, machine.view, machine.acquired, machine.released], machine.events


def execute(command, **kwargs):
    result = subprocess.run(list(map(str, command)), text=True, capture_output=True, **kwargs)
    if result.returncode:
        raise RuntimeError(f'command failed ({result.returncode}): {command}\n{result.stdout[-6000:]}\n{result.stderr[-14000:]}')
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=HERE / 'regional-program-results.json')
    parser.add_argument('--smoke', action='store_true', help='one regular case per boundary; no ASAN; not full evidence')
    args = parser.parse_args()
    graph = fixture(); checked = semantic.check(project(graph)); records = Records(graph, checked); header = records.emit()
    sources = ['experiments/architecture/regional_program.py', 'experiments/architecture/regional_program.cc', 'experiments/architecture/semantic_program.py', 'include/shilos/region.hh', 'include/shilos/vector.hh', 'include/shilos/dbmr.hh']
    compiler = ROOT / 'build/bin/clang++'
    report = dict(schema=1, experiment='actual regional graph/state and native borrowed-policy boundary', status='incomplete',
                  recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  base_commit=execute(['git', 'rev-parse', 'HEAD'], cwd=ROOT).stdout.strip(), smoke=args.smoke,
                  sources={name: digest((ROOT / name).read_bytes()) for name in sources},
                  compiler=dict(path=str(compiler), sha256=digest(compiler.read_bytes()), version=execute([compiler, '--version']).stdout),
                  checked=checked, nodes=len(records.nodes), procedures=len(records.procedures), generated_header_sha256=digest(header.encode()),
                  boundaries={}, rows=[], limitations=[
                      'Known fixture mappings only; no external admission, persistent format or world evolution protocol.',
                      'Finite explicit signatures, scalar policy reads and simple copied image header; not complete CoD inference or capture ABI.',
                      'Runtime expiry guards are assertions for this experiment; required static owner/access checks remain a separate obligation.',
                      'State region is monotonic and includes finished frames/tasks; no frame-reuse or reclaim design is selected.',
                      'Native completion uses integer status and explicit cleanup; a throwing C++ root is caught inside the native callback, not across generated LLVM frames.',
                      'Value and address callbacks run on one thread with no concurrent owner destruction; no concurrent borrow protocol is selected.',
                      'No runtime-performance or framework-engineering ranking follows from process duration or this fixed fixture.'])
    rows = scenarios() if not args.smoke else scenarios()[:1]
    with tempfile.TemporaryDirectory(prefix='cod-regional-program-') as directory:
        work = Path(directory); (work / 'regional_fixture.inc').write_text(header)
        for boundary in ('value', 'address'):
            ir = Lowering(graph, checked, boundary).emit_module(); path = work / (boundary + '.ll'); path.write_text(ir)
            report['boundaries'][boundary] = dict(llvm_bytes=len(ir.encode()), sha256=digest(ir.encode()), definitions=len(re.findall(r'^define ', ir, re.M)))
            configurations = [('regular', rows)]
            if not args.smoke:
                selected = [r for r in rows if r['name'] in ('depth-1000-source-1', 'body-mode-2', 'cleanup-mode-1', 'two-views-one-owner', 'copied-selected-view-2', 'acquire-1-mode-0', 'acquire-3-mode-2', 'expired-owner-mode-0', 'expired-owner-mode-2', 'expired-aliased-owner')]
                configurations.append(('asan', selected))
            for configuration, cases in configurations:
                target = work / (boundary + '-' + configuration); obj = work / (boundary + '-' + configuration + '.o')
                flags = ['-O1', '-g']
                source = path
                if configuration == 'asan':
                    source = work / (boundary + '-asan.ll')
                    source.write_text(re.sub(r'^(define [^\n]+) \{$', r'\1 sanitize_address {', ir, flags=re.M))
                    flags += ['-fsanitize=address', '-fno-omit-frame-pointer']
                execute([compiler, *flags, '-c', source, '-o', obj])
                if configuration == 'asan':
                    nm = execute([ROOT / 'build/bin/llvm-nm', obj]).stdout
                    symbols = sorted(set(re.findall(r'__asan_report_(?:load|store)\w+', nm)))
                    assert any(s.startswith('__asan_report_load') for s in symbols)
                    assert any(s.startswith('__asan_report_store') for s in symbols)
                    report['boundaries'][boundary]['asan_llvm_load_store_instrumented'] = True
                    report['boundaries'][boundary]['asan_llvm_memory_report_symbols'] = symbols
                command = [compiler, '-std=c++20', *flags, '-UNDEBUG', '-I' + str(ROOT / 'include'), '-I' + str(work), HERE / 'regional_program.cc', obj, '-o', target]
                started = time.perf_counter(); execute(command); compile_seconds = time.perf_counter() - started
                report['boundaries'][boundary][configuration] = dict(executable_bytes=target.stat().st_size, harness_compile_link_seconds=compile_seconds, flags=flags)
                for index, case in enumerate(cases):
                    expected, events = oracle(graph, case['arguments'])
                    env = dict(os.environ, ASAN_OPTIONS='detect_leaks=1:halt_on_error=1')
                    started = time.perf_counter()
                    result = execute([target, work / f'fixture-{boundary}-{configuration}-{index}.dbmr', *case['arguments']], env=env)
                    seconds = time.perf_counter() - started
                    lines = result.stdout.splitlines()
                    actual_events = [list(map(int, line.split()[1:])) for line in lines if line.startswith('E ')]
                    actual = [list(map(int, line.split()[1:])) for line in lines if line.startswith('RESULT ')]
                    metrics = [list(map(int, line.split()[1:])) for line in lines if line.startswith('METRICS ')]
                    assert actual == [expected], (boundary, configuration, case, actual, expected)
                    assert actual_events == events, (boundary, configuration, case['name'], 'trace differs from independent projection')
                    assert len(metrics) == 1
                    m = dict(zip(('program_bytes', 'state_bytes', 'region_entries', 'region_frees', 'peak_live_regions', 'events', 'trace_fingerprint', 'policy_reads'), metrics[0]))
                    assert m['region_entries'] == m['region_frees'] and m['events'] == len(events)
                    expected_entries = 5 + 3 * expected[3] + 3 * int(1 <= case['arguments'][6] <= case['arguments'][0])
                    assert m['region_entries'] == expected_entries, (case, m, expected_entries)
                    report['rows'].append(dict(boundary=boundary, configuration=configuration, **case, result=expected, metrics=m, trace_sha256=digest(json.dumps(events, separators=(',', ':')).encode()), process_seconds=seconds, stderr=result.stderr))
                print(f'PASS {boundary}/{configuration}: {len(cases)} regional/remapped/native/oracle traces', flush=True)
    if not args.smoke:
        observations = {r['name']: r['result'] for r in report['rows'] if r['boundary'] == 'address' and r['configuration'] == 'regular'}
        assert observations['policy-payload-7'] != observations['two-views-one-owner'], 'owner alias control must affect business result'
        assert observations['copied-selected-view-1'] != observations['copied-selected-view-2'], 'copied selection control must affect business result'
    report['status'] = 'passed'; report['comparisons'] = len(report['rows'])
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    print(f'PASS: {len(report["rows"])} cases; each compares two regional mappings, native execution, and independent logical fault projection. Report: {args.output}')


if __name__ == '__main__':
    main()
