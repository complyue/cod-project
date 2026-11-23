#!/usr/bin/env python3
"""Caller refs remain initialized and non-null at every intermediate write."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
BASE = 'val x<-1,y<-7;ref p=x,q=y;'
SHOW = 'behave @print(p);behave @print(q);'
DIAGNOSTIC = 'caller ref preservation'


def method(body):
    return 'func (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void where {owner O}{' + body + '{}}'


def history_result(history):
    slots = ['x', 'y']
    valid = True
    for action in history:
        if action == 'swap':
            slots.reverse()
        else:
            target, source = action
            slots[target] = None if source is None else slots[source]
        valid = valid and all(target is not None for target in slots)
    values = {'x': '1', 'y': '7'}
    return valid, ''.join(values[target] + '\n' for target in slots) if valid else ''


def history_source(history):
    result = ''
    for action in history:
        if action == 'swap':
            result += 'intrinsic (<=>)(a,b);'
        else:
            target, source = action
            result += ('a', 'b')[target] + '=' + ('null' if source is None else ('a', 'b')[source]) + ';'
    return result


HISTORIES = {
    'unchanged': [],
    'swap': ['swap'],
    'first_from_second': [(0, 1)],
    'second_from_first': [(1, 0)],
    'two_swaps': ['swap', 'swap'],
    'first_restored': [(0, None), (0, 1)],
    'second_restored': [(1, None), (1, 0)],
    'both_absent': [(0, None), (1, None), 'swap'],
    'swap_around_absence': ['swap', (0, None), 'swap', (1, 0)],
    'copy_before_absence': [(0, 1), 'swap', (1, None), (1, 0)],
}

runner.CASES = {}
for name, history in HISTORIES.items():
    valid, output = history_result(history)
    body = history_source(history)
    for context in ('direct', 'prepared', 'finally', 'deferred_body', 'nested'):
        invocation = '{{defer (<=>)(p,q);}};' if context == 'prepared' else 'p<=>q;'
        implementation = 'try{}finally{' + body + '{}};' if context == 'finally' else body
        if context == 'deferred_body':
            implementation = 'defer{' + body + '};'
        source = IO + method(implementation) + BASE + invocation + SHOW
        if context == 'nested':
            source = {
                'inner.cod': method(body) + 'export (<=>);',
                'main.cod': IO + 'using "./inner.cod" (<=>) as inner;' + method('inner(a,b);') + BASE + invocation + SHOW,
            }
        runner.CASES[context + '_' + name] = (source, 0 if valid else 2, output, '' if valid else DIAGNOSTIC)

GENERIC = 'func (<=>)(ptr a:T in O,ptr b:T in O)->void where {T,owner O}{intrinsic (<=>)(a,b);}'
RECURSIVE = ('func (<=>)(ptr a:u64 in O,ptr b:u64 in O)->void throws NullPointerException where {owner O}{'
             'if(a>0u64){a<-a-1u64;intrinsic (<=>)(a,b);a<=>b;}else{};}')
COUNTERS = 'val x<-40u64,y<-40u64;ref p=x,q=y;'
runner.CASES.update({
    'self_slot': (IO + method('intrinsic (<=>)(a,b);') + BASE + 'p<=>p;' + SHOW, 0, '1\n7\n', ''),
    'selected_ref_slots': (IO + method('intrinsic (<=>)(a,b);') + BASE +
        'ref var chosen->if(false){p}else{q};chosen<=>p;' + SHOW, 0, '7\n1\n', ''),
    'generic_scalar_refs': (IO + GENERIC + 'val x<-false,y<-true;ref p=x,q=y;p<=>q;' + SHOW, 0, 'true\nfalse\n', ''),
    'complete_callable_refs': (IO + GENERIC +
        'func one()->u64{return 1u64;}func seven()->u64{return 7u64;}'
        'func read(ref p:func()->u64 in O,ref q:func()->u64 in O)->u64 where {owner O}{p<=>q;return p();}'
        'func first=one;func second=seven;behave @print(read(first,second));behave @print(first());behave @print(second());',
        0, '7\n1\n7\n', ''),
    'recursive_slots': (IO + NPE + RECURSIVE + COUNTERS +
        'try{p<=>q;}catch(e:const NullPointerException){behave @print(999u64);};'
        'behave @print(x);behave @print(y);' + SHOW, 0, '0\n0\n0\n0\n', ''),
    'recursive_declared_failure_not_waived': (NPE + RECURSIVE + COUNTERS + 'p<=>q;', 2, '', 'requires a covering catch or throws'),
    'nullable_source_unproved': (method('intrinsic (<=>)(a,b);') +
        'val x<-1,y<-7;ref p=x;ptr q=y;p<=>q;', 2, '', DIAGNOSTIC),
    'uninitialized_source_unproved': (method('intrinsic (<=>)(a,b);') +
        'val x<-1,y<-make local i64;ref p=x;ptr q=y;p<=>q;', 2, '', DIAGNOSTIC),
    'fixed_slot': (method('') + 'val x<-1,y<-7;const ref p=x;ref q=y;p<=>q;', 2, '', 'const reference binding'),
    'readonly_target_permission': (method('a=b;').replace('ptr a:i64', 'ptr a:const i64').replace('ptr b:i64', 'ptr b:const i64') +
        'val x<-1,y<-7;ref p=x;ref q:const i64=y;ref var ro:const->p;ref var other:const->q;ro<=>other;',
        2, '', 'permission preservation'),
    'different_owners': (method('intrinsic (<=>)(a,b);') + 'val x<-1;ref p=x;{{val y<-7;ref q=y;p<=>q;}};',
        2, '', 'owner'),
    'cleanup_after_failure': (IO + 'using "std:exceptions" MissingReturnError;func missing()->i64{}' +
        method('defer{intrinsic (<=>)(a,b);};missing();') + BASE +
        'try{p<=>q;}catch(e:const MissingReturnError){};' + SHOW, 0, '7\n1\n', '[-Wmissing-return]'),
})
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
