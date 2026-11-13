#!/usr/bin/env python3
"""Experimental typed control graph, explicit reference frames and LLVM lowering.

No CoD syntax, production IR/ABI, general inference, shilos allocation or external
input integration is defined here. Business control and deferred actions are data.
"""
from copy import deepcopy
from dataclasses import dataclass, field
from pathlib import Path
import argparse
import datetime
import hashlib
import json
import re
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent


def const(value, kind='i64'):
    return {'const': value, 'type': kind}


def var(name):
    return {'var': name}


def op(name, *args):
    return {'op': name, 'args': list(args)}


def stmt(operation, **values):
    return {'op': operation, **values}


def ret(value):
    return stmt('return', value=value)


def branch(condition, yes, no=None):
    return stmt('if', condition=condition, yes=yes, no=no or [])


def event(tag, a=None, b=None):
    return stmt('event', tag=const(tag), a=a or const(0), b=b or const(0))


def fixture():
    n, source, mode = var('n'), var('source'), var('mode')
    functions = {}
    for name, divisor, origin in [('stock_base', 2, 'entry'), ('stock_a', 3, 'dependency-a'), ('stock_b', 5, 'dependency-b')]:
        functions[name] = dict(origin=origin, params={'n': 'i64'}, result='bool',
                               body=[ret(op('eq', op('mod', n, const(divisor)), const(0)))])
    functions['dependency_helper'] = dict(origin='dependency-a', params={'n': 'i64', 'source': 'i64'}, result='bool', body=[
        stmt('let', name='local_view', type='view', value=op('select',
             op('eq', op('mod', op('add', n, source), const(2)), const(0)), const(1, 'view'), const(2, 'view'))),
        stmt('install', view=var('local_view')),
        event(20, n, var('local_view')),
        stmt('invoke', name='available', type='bool', family='stock', view=op('current_view'), args=[n]),
        ret(var('available')),
    ])
    # mode0 propagates; mode1 catches normally; mode2 explicitly returns;
    # mode3 replaces completion from finally. These are fixture inputs.
    functions['checkout'] = dict(origin='entry', params={'n': 'i64', 'source': 'i64', 'fail_at': 'i64', 'cleanup_at': 'i64', 'mode': 'i64'}, result='i64', body=[
        branch(op('eq', n, const(0)), [ret(const(0))]),
        stmt('lease', name='lease'),
        stmt('defer', name='release', body=[], catch=None, finally_=[
            event(50, n, var('lease')), stmt('release', value=var('lease'))]),
        stmt('defer', name='recover', body=[], catch=[
            event(40, n, var('error')),
            branch(op('eq', mode, const(0)), [stmt('throw', value=var('error'))]),
            branch(op('eq', mode, const(2)), [ret(const(777))]),
        ], finally_=[
            event(41, n, mode),
            branch(op('eq', mode, const(3)), [ret(const(888))]),
        ]),
        stmt('defer', name='cleanup_failure', body=[
            branch(op('eq', n, var('cleanup_at')), [stmt('throw', value=const(901))]),
        ], catch=None, finally_=[event(30, n)]),
        stmt('let', name='captured_view', type='view', value=op('current_view')),
        stmt('let', name='local_view', type='view', value=op('select',
            op('eq', op('mod', op('add', n, source), const(3)), const(0)), const(0, 'view'),
            op('select', op('eq', op('mod', op('add', n, source), const(3)), const(1)), const(1, 'view'), const(2, 'view')))),
        stmt('install', view=var('local_view')),
        event(10, n, var('local_view')),
        branch(op('eq', n, var('fail_at')), [stmt('throw', value=const(902))]),
        stmt('invoke', name='captured', type='bool', family='stock', view=var('captured_view'), args=[n]),
        stmt('call', name='available', type='bool', target='dependency_helper', args=[n, source]),
        stmt('invoke', name='restored', type='bool', family='stock', view=op('current_view'), args=[n]),
        event(21, var('captured'), var('available')),
        event(22, var('restored'), op('current_view')),
        stmt('let', name='score', type='i64', value=const(1)),
        branch(var('available'), [stmt('set', name='score', value=n)]),
        branch(var('captured'), [stmt('set', name='score', value=op('add', var('score'), const(2)))]),
        stmt('call', name='rest', type='i64', target='checkout', args=[op('sub', n, const(1)), source, var('fail_at'), var('cleanup_at'), mode]),
        ret(op('add', var('score'), var('rest'))),
    ])
    return dict(schema=1, entry='checkout', initial_view=0,
                views=[{'id': 0, 'origin': 'entry', 'families': {'stock': 'stock_base'}},
                       {'id': 1, 'origin': 'dependency-a', 'families': {'stock': 'stock_a'}},
                       {'id': 2, 'origin': 'dependency-b', 'families': {'stock': 'stock_b'}}],
                families={'stock': {'params': ['i64'], 'result': 'bool'}}, functions=functions)


class InvalidGraph(ValueError):
    pass


def require(ok, message):
    if not ok:
        raise InvalidGraph(message)


def walk_statements(body):
    for s in body:
        yield s
        for key in ('yes', 'no', 'body', 'catch', 'finally_'):
            if isinstance(s.get(key), list):
                yield from walk_statements(s[key])


def check(graph):
    """Check this small explicit-signature graph once per definition, not call."""
    views = {v['id']: v for v in graph['views']}
    require(len(views) == len(graph['views']), 'duplicate interpretation identity')
    require(graph['initial_view'] in views, 'missing initial interpretation')
    for view in views.values():
        for family, name in view['families'].items():
            require(family in graph['families'] and name in graph['functions'], 'unknown family method')
            contract, method = graph['families'][family], graph['functions'][name]
            require(list(method['params'].values()) == contract['params'] and method['result'] == contract['result'], 'family signature mismatch')
    reachable, checking = set(), [graph['entry']]
    nodes = 0
    def expression(e, env):
        if 'const' in e:
            kind = e['type']; require(kind in ('i64', 'bool', 'view'), 'unknown value type')
            require(isinstance(e['const'], int) and -(1 << 63) <= e['const'] < (1 << 63), 'invalid integer payload')
            require(kind != 'bool' or e['const'] in (0, 1), 'invalid bool')
            require(kind != 'view' or e['const'] in views, 'missing selected interpretation')
            return kind
        if 'var' in e:
            require(e['var'] in env, 'uninitialized or unknown register: ' + e['var'])
            return env[e['var']]
        operation, args = e['op'], e.get('args', [])
        kinds = [expression(a, env) for a in args]
        if operation == 'current_view':
            require(not args, 'current_view arity'); return 'view'
        if operation in ('add', 'sub', 'mod'):
            require(kinds == ['i64', 'i64'], 'arithmetic type mismatch'); return 'i64'
        if operation == 'eq':
            require(len(kinds) == 2 and kinds[0] == kinds[1], 'comparison type mismatch'); return 'bool'
        if operation == 'select':
            require(len(kinds) == 3 and kinds[0] == 'bool' and kinds[1] == kinds[2], 'selection type mismatch'); return kinds[1]
        raise InvalidGraph('unknown expression operation: ' + operation)
    def block(body, env, result, cleanup=False):
        nonlocal nodes
        for s in body:
            nodes += 1; operation = s['op']
            if operation == 'if':
                require(expression(s['condition'], env) == 'bool', 'branch requires bool')
                block(s['yes'], dict(env), result, cleanup); block(s['no'], dict(env), result, cleanup)
            elif operation in ('let', 'set'):
                kind = expression(s['value'], env)
                if operation == 'let':
                    require(s['name'] not in env and kind == s['type'], 'register declaration mismatch'); env[s['name']] = kind
                else:
                    require(env.get(s['name']) == kind, 'assignment type mismatch')
            elif operation == 'return':
                require(expression(s['value'], env) == result, 'result type mismatch')
            elif operation in ('throw', 'release'):
                require(expression(s['value'], env) == 'i64', operation + ' type mismatch')
            elif operation == 'event':
                for k in ('tag', 'a', 'b'): expression(s[k], env)
            elif operation == 'lease':
                require(s['name'] not in env, 'duplicate lease'); env[s['name']] = 'i64'
            elif operation == 'install':
                require(expression(s['view'], env) == 'view', 'install requires selected view')
            elif operation in ('call', 'invoke'):
                if operation == 'call':
                    require(s['target'] in graph['functions'], 'unknown procedure')
                    target = graph['functions'][s['target']]; params = list(target['params'].values()); checking.append(s['target'])
                else:
                    require(expression(s['view'], env) == 'view', 'invoke requires selected view')
                    require(s['family'] in graph['families'], 'unknown family')
                    target = graph['families'][s['family']]; params = target['params']
                    for view in views.values():
                        require(s['family'] in view['families'], 'uncovered family interpretation')
                        checking.append(view['families'][s['family']])
                require([expression(e, env) for e in s['args']] == params and s['type'] == target['result'], 'call signature mismatch')
                require(s['name'] not in env, 'duplicate result register'); env[s['name']] = s['type']
            elif operation == 'defer':
                require(not cleanup, 'nested registration outside this experiment')
                block(s['body'], dict(env), result, True)
                if s['catch'] is not None: block(s['catch'], {**env, 'error': 'i64'}, result, True)
                block(s['finally_'], dict(env), result, True)
            else:
                raise InvalidGraph('unknown statement: ' + operation)
    while checking:
        name = checking.pop()
        if name in reachable: continue
        reachable.add(name); f = graph['functions'][name]
        block(f['body'], dict(f['params']), f['result'])
    return {'checked_definitions': len(reachable), 'checked_statements': nodes, 'reachable': sorted(reachable)}


def integer(value):
    return (value + (1 << 63)) % (1 << 64) - (1 << 63)


@dataclass
class Completion:
    kind: str = 'normal'
    value: int = 0


@dataclass
class Frame:
    name: str
    values: dict
    saved_view: int
    tasks: list
    receiver: str | None = None
    pending: Completion = field(default_factory=Completion)
    saved_return: int | None = None
    defers: list = field(default_factory=list)
    cleanup: bool = False
    leases: set = field(default_factory=set)
    action: dict | None = None
    stage: str = ''


class Reference:
    def __init__(self, graph):
        self.graph = graph; self.view = graph['initial_view']; self.frames = []
        self.events = []; self.acquired = 0; self.released = 0; self.live = set()
        self.steps = 0; self.max_frames = 0

    def expression(self, e, frame):
        if 'const' in e: return e['const']
        if 'var' in e: return frame.values[e['var']]
        name = e['op']; args = [self.expression(a, frame) for a in e.get('args', [])]
        if name == 'current_view': return self.view
        if name == 'add': return integer(args[0] + args[1])
        if name == 'sub': return integer(args[0] - args[1])
        if name == 'mod':
            quotient = abs(args[0]) // abs(args[1])
            if (args[0] < 0) != (args[1] < 0): quotient = -quotient
            return args[0] - quotient * args[1]
        if name == 'eq': return int(args[0] == args[1])
        if name == 'select': return args[1] if args[0] else args[2]
        raise AssertionError(name)

    def push(self, name, values, receiver=None):
        f = self.graph['functions'][name]
        self.frames.append(Frame(name, dict(zip(f['params'], values)), self.view, list(reversed(f['body'])), receiver))
        self.max_frames = max(self.max_frames, len(self.frames))

    def start_stage(self, frame, stage):
        frame.stage = stage
        frame.tasks = list(reversed(frame.action['finally_' if stage == 'finally' else stage] or []))
        if stage == 'catch': frame.values['error'] = frame.pending.value

    def next_action(self, frame):
        frame.cleanup = True
        if not frame.defers:
            frame.action = None; frame.tasks = []; frame.stage = 'done'; return
        frame.action = frame.defers.pop()
        if frame.pending.kind == 'throw':
            self.start_stage(frame, 'catch' if frame.action['catch'] is not None else 'finally')
        else:
            self.start_stage(frame, 'body')

    def abrupt(self, frame, kind, value):
        frame.pending = Completion(kind, value)
        if kind == 'return': frame.saved_return = value
        frame.tasks = []
        if not frame.cleanup: self.next_action(frame)
        elif frame.stage == 'body' and kind == 'throw' and frame.action['catch'] is not None: self.start_stage(frame, 'catch')
        elif frame.stage != 'finally': self.start_stage(frame, 'finally')
        else: self.next_action(frame)

    def run(self, args):
        self.push(self.graph['entry'], args)
        while self.frames:
            frame = self.frames[-1]
            if not frame.tasks:
                if not frame.cleanup: self.next_action(frame); continue
                if frame.stage == 'body': self.start_stage(frame, 'finally'); continue
                if frame.stage == 'catch':
                    frame.pending = Completion('return', frame.saved_return) if frame.saved_return is not None else Completion()
                    self.start_stage(frame, 'finally'); continue
                if frame.stage == 'finally': self.next_action(frame); continue
                assert frame.stage == 'done'
                result = frame.pending if frame.pending.kind != 'normal' else Completion('throw', 903)
                assert not frame.leases
                self.view = frame.saved_view; self.frames.pop()
                if not self.frames:
                    assert not self.live
                    return result
                parent = self.frames[-1]
                if result.kind == 'throw': self.abrupt(parent, 'throw', result.value)
                else: parent.values[frame.receiver] = result.value
                continue
            self.steps += 1; s = frame.tasks.pop(); operation = s['op']
            evaluate = lambda e: self.expression(e, frame)
            if operation == 'if':
                frame.tasks.extend(reversed(s['yes'] if evaluate(s['condition']) else s['no']))
            elif operation in ('let', 'set'): frame.values[s['name']] = evaluate(s['value'])
            elif operation in ('return', 'throw'): self.abrupt(frame, operation, evaluate(s['value']))
            elif operation == 'defer': frame.defers.append(s)
            elif operation == 'event': self.events.append([evaluate(s[k]) for k in ('tag', 'a', 'b')])
            elif operation == 'install': self.view = evaluate(s['view'])
            elif operation == 'lease':
                self.acquired += 1; self.live.add(self.acquired); frame.leases.add(self.acquired); frame.values[s['name']] = self.acquired
            elif operation == 'release':
                token = evaluate(s['value']); assert token in self.live and token in frame.leases
                self.live.remove(token); frame.leases.remove(token); self.released += 1
            elif operation in ('call', 'invoke'):
                target = s.get('target')
                if operation == 'invoke':
                    view = next(v for v in self.graph['views'] if v['id'] == evaluate(s['view']))
                    target = view['families'][s['family']]
                self.push(target, [evaluate(e) for e in s['args']], s['name'])
            else: raise AssertionError(operation)
        raise AssertionError('no result')


class Lowering:
    """One native function per reachable definition, with explicit completion.

    The aggregate and frame slots are experimental carriers, not a selected ABI.
    No runtime context is a key for generating another definition.
    """
    def __init__(self, graph, checked):
        self.graph = graph; self.checked = checked; self.lines = []; self.serial = 0

    def unique(self, prefix='v'):
        self.serial += 1; return f'{prefix}{self.serial}'

    def emit(self, line): self.lines.append('  ' + line)
    def label(self, name): self.lines.append(name + ':')
    def reg(self, operation):
        name = '%' + self.unique(); self.emit(name + ' = ' + operation); return name
    def store(self, value, slot): self.emit(f'store i64 {value}, ptr %{slot}')
    def load(self, slot): return self.reg(f'load i64, ptr %{slot}')

    def expression(self, e):
        if 'const' in e: return str(e['const'])
        if 'var' in e: return self.load('local_' + e['var'])
        operation = e['op']; args = [self.expression(a) for a in e.get('args', [])]
        if operation == 'current_view': return self.reg('load i64, ptr @binding')
        if operation in ('add', 'sub', 'mod'):
            instruction = {'add': 'add', 'sub': 'sub', 'mod': 'srem'}[operation]
            return self.reg(f'{instruction} i64 {args[0]}, {args[1]}')
        if operation == 'eq':
            value = self.reg(f'icmp eq i64 {args[0]}, {args[1]}'); return self.reg(f'zext i1 {value} to i64')
        if operation == 'select':
            condition = self.reg(f'icmp ne i64 {args[0]}, 0')
            return self.reg(f'select i1 {condition}, i64 {args[1]}, i64 {args[2]}')
        raise AssertionError(operation)

    def abrupt(self, kind, value, return_to, throw_to):
        self.store('1' if kind == 'throw' else '0', 'thrown'); self.store(value, 'pending')
        if kind == 'return': self.store('1', 'has_return'); self.store(value, 'saved_return')
        self.emit('br label %' + (throw_to if kind == 'throw' else return_to))

    def block(self, statements, return_to, throw_to):
        for s in statements:
            operation = s['op']
            if operation == 'if':
                condition = self.reg(f"icmp ne i64 {self.expression(s['condition'])}, 0")
                yes, no, done = self.unique('yes'), self.unique('no'), self.unique('join')
                self.emit(f'br i1 {condition}, label %{yes}, label %{no}')
                self.label(yes); yt = self.block(s['yes'], return_to, throw_to)
                if not yt: self.emit(f'br label %{done}')
                self.label(no); nt = self.block(s['no'], return_to, throw_to)
                if not nt: self.emit(f'br label %{done}')
                if yt and nt: return True
                self.label(done)
            elif operation in ('let', 'set'): self.store(self.expression(s['value']), 'local_' + s['name'])
            elif operation in ('return', 'throw'):
                self.abrupt(operation, self.expression(s['value']), return_to, throw_to); return True
            elif operation == 'event':
                args = ', '.join('i64 ' + self.expression(s[k]) for k in ('tag', 'a', 'b'))
                self.emit(f'call void @event({args})')
            elif operation == 'install': self.emit(f"store i64 {self.expression(s['view'])}, ptr @binding")
            elif operation == 'defer': self.store('1', 'registered_' + s['name'])
            elif operation == 'lease':
                old = self.reg('load i64, ptr @acquired'); token = self.reg(f'add i64 {old}, 1')
                self.emit(f'store i64 {token}, ptr @acquired'); self.store(token, 'local_' + s['name']); self.store('1', 'live_' + s['name'])
            elif operation == 'release':
                token = self.expression(s['value']); valid = 'false'
                for name in self.lease_names:
                    active = self.load('live_' + name)
                    equal = self.reg(f'icmp eq i64 {token}, {self.load("local_" + name)}')
                    live = self.reg(f'icmp ne i64 {active}, 0')
                    matches = self.reg(f'and i1 {equal}, {live}')
                    valid = self.reg(f'or i1 {valid}, {matches}')
                    self.store(self.reg(f'select i1 {matches}, i64 0, i64 {active}'), 'live_' + name)
                passed, failed = self.unique('release_ok'), self.unique('release_bad')
                self.emit(f'br i1 {valid}, label %{passed}, label %{failed}')
                self.label(failed); self.emit('call void @exit(i32 97)'); self.emit('unreachable')
                self.label(passed)
                old = self.reg('load i64, ptr @released'); new = self.reg(f'add i64 {old}, 1')
                self.emit(f'store i64 {new}, ptr @released')
            elif operation in ('call', 'invoke'):
                args = ', '.join('i64 ' + self.expression(e) for e in s['args'])
                if operation == 'call': target = '@' + s['target']
                else:
                    view = self.expression(s['view'])
                    # The checked fixture has exhaustive selected interpretations.
                    # A production unknown-view failure policy is not selected.
                    target = '@' + self.graph['views'][-1]['families'][s['family']]
                    for candidate in reversed(self.graph['views'][:-1]):
                        condition = self.reg(f"icmp eq i64 {view}, {candidate['id']}")
                        target = self.reg(f"select i1 {condition}, ptr @{candidate['families'][s['family']]}, ptr {target}")
                result = self.reg(f'call %Completion {target}({args})')
                thrown = self.reg(f'extractvalue %Completion {result}, 0'); value = self.reg(f'extractvalue %Completion {result}, 1')
                failed, passed = self.unique('failed'), self.unique('passed')
                self.emit(f'br i1 {thrown}, label %{failed}, label %{passed}')
                self.label(failed); self.abrupt('throw', value, return_to, throw_to)
                self.label(passed); self.store(value, 'local_' + s['name'])
            else: raise AssertionError(operation)
        return False

    def function(self, name):
        f = self.graph['functions'][name]
        slots = set(f['params']) | {'error'}
        defers = []; self.lease_names = []
        for s in walk_statements(f['body']):
            if s['op'] in ('let', 'call', 'invoke', 'lease'): slots.add(s['name'])
            if s['op'] == 'defer': defers.append(s)
            if s['op'] == 'lease': self.lease_names.append(s['name'])
        self.lines.append('define %Completion @' + name + '(' + ', '.join('i64 %arg_' + n for n in f['params']) + ') {')
        self.label('entry')
        for slot in ['thrown', 'pending', 'has_return', 'saved_return', 'saved_view'] + ['local_' + n for n in sorted(slots)] + ['registered_' + d['name'] for d in defers] + ['live_' + n for n in self.lease_names]:
            self.emit(f'%{slot} = alloca i64'); self.store('0', slot)
        self.store(self.reg('load i64, ptr @binding'), 'saved_view')
        for parameter in f['params']: self.store('%arg_' + parameter, 'local_' + parameter)
        cleanup = 'cleanup_' + defers[-1]['name'] if defers else 'exit'
        if not self.block(f['body'], cleanup, cleanup): self.emit(f'br label %{cleanup}')
        reversed_defers = list(reversed(defers))
        for index, action in enumerate(reversed_defers):
            base = 'cleanup_' + action['name']; body, catch, final = base + '_body', base + '_catch', base + '_finally'
            next_action = 'cleanup_' + reversed_defers[index + 1]['name'] if index + 1 < len(defers) else 'exit'
            self.label(base)
            active = self.reg(f"icmp ne i64 {self.load('registered_' + action['name'])}, 0")
            dispatch = base + '_dispatch'; self.emit(f'br i1 {active}, label %{dispatch}, label %{next_action}')
            self.label(dispatch); thrown = self.reg(f"icmp ne i64 {self.load('thrown')}, 0")
            self.emit(f'br i1 {thrown}, label %{catch if action["catch"] is not None else final}, label %{body}')
            self.label(body)
            if not self.block(action['body'], final, catch if action['catch'] is not None else final): self.emit(f'br label %{final}')
            if action['catch'] is not None:
                self.label(catch); self.store(self.load('pending'), 'local_error')
                if not self.block(action['catch'], final, final):
                    self.store('0', 'thrown'); self.store(self.load('saved_return'), 'pending'); self.emit(f'br label %{final}')
            self.label(final)
            if not self.block(action['finally_'], next_action, next_action): self.emit(f'br label %{next_action}')
        self.label('exit')
        for name in self.lease_names:
            live = self.reg(f'icmp ne i64 {self.load("live_" + name)}, 0')
            passed, failed = self.unique('released'), self.unique('leaked')
            self.emit(f'br i1 {live}, label %{failed}, label %{passed}')
            self.label(failed); self.emit('call void @exit(i32 97)'); self.emit('unreachable'); self.label(passed)
        self.emit(f"store i64 {self.load('saved_view')}, ptr @binding")
        thrown = self.reg(f"icmp ne i64 {self.load('thrown')}, 0"); returned = self.reg(f"icmp ne i64 {self.load('has_return')}, 0")
        no_return = self.reg(f'xor i1 {returned}, true'); error = self.reg(f'or i1 {thrown}, {no_return}')
        missing = self.reg(f'select i1 {returned}, i64 {self.load("pending")}, i64 903')
        value = self.reg(f'select i1 {thrown}, i64 {self.load("pending")}, i64 {missing}')
        aggregate = self.reg(f'insertvalue %Completion poison, i1 {error}, 0')
        aggregate = self.reg(f'insertvalue %Completion {aggregate}, i64 {value}, 1')
        self.emit(f'ret %Completion {aggregate}'); self.lines.append('}')

    def emit_module(self):
        self.lines = ['%Completion = type { i1, i64 }', '@binding = thread_local global i64 0', '@acquired = global i64 0', '@released = global i64 0',
                      '@event_format = private constant [15 x i8] c"E %ld %ld %ld\\0A\\00"',
                      '@result_format = private constant [28 x i8] c"RESULT %ld %ld %ld %ld %ld\\0A\\00"',
                      'declare i32 @printf(ptr, ...)', 'declare void @exit(i32) noreturn', 'declare i64 @strtoll(ptr, ptr, i32)',
                      'define void @event(i64 %tag, i64 %a, i64 %b) {',
                      '  %p = call i32 (ptr, ...) @printf(ptr @event_format, i64 %tag, i64 %a, i64 %b)', '  ret void', '}']
        for name in self.checked['reachable']: self.function(name)
        self.lines.append('define i32 @main(i32 %argc, ptr %argv) {'); self.label('entry')
        valid = self.reg('icmp eq i32 %argc, 6'); self.emit(f'br i1 {valid}, label %run, label %bad')
        self.label('bad'); self.emit('ret i32 2'); self.label('run')
        args = []
        for i in range(1, 6):
            slot = self.reg(f'getelementptr ptr, ptr %argv, i64 {i}'); text = self.reg(f'load ptr, ptr {slot}')
            args.append(self.reg(f'call i64 @strtoll(ptr {text}, ptr null, i32 10)'))
        self.emit(f'store i64 {self.graph["initial_view"]}, ptr @binding')
        result = self.reg('call %Completion @' + self.graph['entry'] + '(' + ', '.join('i64 ' + a for a in args) + ')')
        thrown = self.reg(f'extractvalue %Completion {result}, 0'); status = self.reg(f'zext i1 {thrown} to i64'); value = self.reg(f'extractvalue %Completion {result}, 1')
        binding = self.reg('load i64, ptr @binding'); acquired = self.reg('load i64, ptr @acquired'); released = self.reg('load i64, ptr @released')
        self.emit(f'call i32 (ptr, ...) @printf(ptr @result_format, i64 {status}, i64 {value}, i64 {binding}, i64 {acquired}, i64 {released})')
        self.emit('ret i32 0'); self.lines.append('}')
        return '\n'.join(self.lines) + '\n'


def digest(data):
    return hashlib.sha256(data if isinstance(data, bytes) else data.encode()).hexdigest()


def business_score(n, depth, source):
    captured_view = 0 if n == depth else (n + 1 + source) % 3
    dependency_divisor = 3 if (n + source) % 2 == 0 else 5
    return (n if n % dependency_divisor == 0 else 1) + (2 if n % [2, 3, 5][captured_view] == 0 else 0)


def expected_outcome(name, inputs):
    depth, source, fail_at, cleanup_at, mode = inputs
    if mode == 3: return [0, 888]
    if mode == 0 and fail_at > 0: return [1, 902]
    if mode == 0 and cleanup_at > 0: return [1, 901]
    if mode == 1 and fail_at > 0: return [1, 903]
    boundary = max(fail_at, cleanup_at) if mode == 2 else 0
    return [0, (777 if boundary > 0 else 0) + sum(business_score(n, depth, source) for n in range(max(1, boundary + 1), depth + 1))]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--repetitions', type=int, default=3)
    args = parser.parse_args()
    graph = fixture(); serialized = json.dumps(graph, indent=2) + '\n'
    start = time.monotonic(); checked = check(graph); check_seconds = time.monotonic() - start
    start = time.monotonic(); llvm = Lowering(graph, checked).emit_module(); lowering_seconds = time.monotonic() - start
    controls = {}
    bad = deepcopy(graph); bad['views'] = bad['views'][:1]
    try: check(bad)
    except InvalidGraph as e: controls['entry_only'] = str(e)
    else: raise AssertionError('entry-only artifact accepted')
    bad = deepcopy(graph); bad['functions']['stock_base']['body'] = [ret(const(1))]
    try: check(bad)
    except InvalidGraph as e: controls['wrong_result'] = str(e)
    else: raise AssertionError('wrong method result accepted')
    bad = deepcopy(graph); next(s for s in bad['functions']['dependency_helper']['body'] if s['op'] == 'invoke')['family'] = 'missing'
    try: check(bad)
    except InvalidGraph as e: controls['missing_family'] = str(e)
    else: raise AssertionError('unknown family accepted')
    flattened = deepcopy(graph)
    for view in flattened['views']: view['families']['stock'] = 'stock_base'
    check(flattened)  # Signature compatibility alone does not prove equivalent interpretation.
    correct, wrong = Reference(graph), Reference(flattened)
    correct_result, wrong_result = correct.run([20, 1, -1, -1, 0]), wrong.run([20, 1, -1, -1, 0])
    assert correct.events != wrong.events and correct_result != wrong_result
    controls['flattened_interpretations'] = dict(type_checks=True, expected_result=correct_result.value, incorrect_result=wrong_result.value, trace_equal=False)
    scenarios = [('depth', [depth, source, -1, -1, 0]) for depth in (4, 20, 200, 1000) for source in (0, 1, 2)]
    scenarios += [(f'body_failure_mode_{mode}', [8, 1, 2, -1, mode]) for mode in range(4)]
    scenarios += [(f'cleanup_failure_mode_{mode}', [8, 1, -1, 2, mode]) for mode in range(4)]
    scenarios += [('both_failures', [8, 2, 3, 5, 2]), ('missing_return', [4, 0, 4, -1, 1])]
    rows = []
    lli = ROOT / 'build/bin/lli'
    with tempfile.TemporaryDirectory(prefix='cod-semantic-program-') as temp:
        path = Path(temp) / 'program.ll'; path.write_text(llvm)
        # An ill-behaved graph with a type-correct wrong lease token must
        # fail in each engine. Exit97 is a prototype assertion, not CoD policy.
        invalid = deepcopy(graph)
        release = next(s for s in walk_statements(invalid['functions']['checkout']['body']) if s['op'] == 'release')
        release['value'] = const(99999)
        invalid_checked = check(invalid)
        try: Reference(invalid).run([4, 0, -1, -1, 0])
        except AssertionError: pass
        else: raise AssertionError('wrong lease token accepted by reference')
        invalid_path = Path(temp) / 'invalid-lease.ll'
        invalid_path.write_text(Lowering(invalid, invalid_checked).emit_module())
        rejected = subprocess.run([str(lli), '--jit-kind=orc', str(invalid_path), '4', '0', '-1', '-1', '0'], cwd=temp, capture_output=True, text=True, timeout=60)
        assert rejected.returncode == 97 and not rejected.stderr, rejected
        controls['wrong_lease_token'] = dict(type_checks=True, reference_rejected=True, native_exit=rejected.returncode)
        for repeat in range(args.repetitions):
            for name, inputs in scenarios:
                reference = Reference(graph); start = time.monotonic(); outcome = reference.run(inputs); reference_seconds = time.monotonic() - start
                start = time.monotonic(); command = [str(lli), '--jit-kind=orc', str(path), *map(str, inputs)]
                native = subprocess.run(command, capture_output=True, text=True, timeout=60); native_seconds = time.monotonic() - start
                assert native.returncode == 0 and not native.stderr, native.stderr
                events = []; result = None
                for line in native.stdout.splitlines():
                    tag, *values = line.split()
                    if tag == 'E': events.append(list(map(int, values)))
                    elif tag == 'RESULT': result = list(map(int, values))
                    else: raise AssertionError(line)
                assert events == reference.events, (name, inputs, 'event mismatch')
                expected = [int(outcome.kind == 'throw'), outcome.value, graph['initial_view'], reference.acquired, reference.released]
                assert result == expected, (name, inputs, result, expected)
                assert result[:2] == expected_outcome(name, inputs), (name, inputs, result)
                assert [event[2] for event in events if event[0] == 50] == list(range(reference.acquired, 0, -1))
                assert reference.acquired == reference.released and not reference.live
                rows.append(dict(name=name, inputs=inputs, repeat=repeat, result=result,
                    trace_count=len(events), trace_sha256=digest(json.dumps(events)), trace_prefix=events[:8], trace_suffix=events[-8:],
                    reference_steps=reference.steps, max_reference_frames=reference.max_frames,
                    reference_seconds=reference_seconds, native_process_seconds=native_seconds,
                    native_stdout_bytes=len(native.stdout.encode()), command=command))
        assert path.read_text() == llvm
    assert json.dumps(graph, indent=2) + '\n' == serialized
    report = dict(schema=1, recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        baseline_commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
        source_sha256=digest(Path(__file__).read_bytes()), compiler='experimental graph lowering',
        lli_sha256=digest(lli.read_bytes()), artifact_sha256=digest(serialized), artifact_bytes=len(serialized.encode()),
        checking=checked, checking_seconds=check_seconds, lowering_seconds=lowering_seconds,
        llvm_sha256=digest(llvm), llvm_bytes=len(llvm.encode()), native_definitions=len(re.findall(r'^define ', llvm, re.M)),
        repetitions=args.repetitions, controls=controls, cases=rows, graph_unchanged=True, llvm_unchanged=True,
        limitations=['Handwritten explicit-signature finite graph; no CoD parser or general type/owner inference.',
            'Logical lease obligations and exact token/order traces are tested; no shilos allocation, actual region payload or lifetime proof is implemented here.',
            'Pending return/throw/missing-return and cleanup are explicit in both engines. Exception payloads are integer codes and catches match all; no production exception ABI or native C++ unwinding is selected.',
            'Selected callable context is represented by a view ID; complete captured environments and region remapping are separate boundary-model evidence.',
            'All selected definitions are lowered once before native execution. This proves finite shared lowering for the fixture, not general AOT coverage or lazy semantic lowering.',
            'Native timing includes lli startup, compilation and full trace output; reference timing includes explicit frame interpretation and trace collection. These are not throughput comparisons.',
            'No persistent continuation, world-evolution or external-definition admission protocol is implemented.'])
    args.report.parent.mkdir(parents=True, exist_ok=True); args.report.write_text(json.dumps(report, indent=2) + '\n')
    args.report.with_suffix('.graph.json').write_text(serialized); args.report.with_suffix('.ll').write_text(llvm)
    print(f'{len(rows)} whole-program trace comparisons passed; {checked}; {len(llvm.encode())} LLVM bytes')


if __name__ == '__main__': main()
