#!/usr/bin/env python3
"""Prepared selections retain initialization from their actual capture frames."""
from itertools import product
from pathlib import Path

import foundation_tests as runner

IO = (
    'using symbol "std:io" @print;'
    'using "std:exceptions" NullPointerException;'
    'using "std:exceptions" OutOfMemoryError;'
)
SETUP = 'val a<-7u64,b<-8u64,c<-make local u64;ptr p=a,q=a;'
COPY = 'p=stage(p,q,c);q=a;'
READ = 'behave @print(p);'
MISSING_TARGET = "initialization of 'c'"

runner.CASES = {}


def prepared_result(name, preparation, extra_parameters=''):
    return (
        f'func {name}(ptr dst:T in O,ptr src:T in O,ptr spare:T in O{extra_parameters})->ptr T in O where {{T,owner O}}'
        '{val prepared<-false;'
        'defer{if(prepared){return dst;}else{};};'
        + preparation
        + 'src=spare;prepared<-true;return dst;}'
    )


STAGE = prepared_result('stage', 'defer (=)(dst,src);')
TWICE = prepared_result('stage_twice', 'defer (=)(dst,src);src=next;defer (=)(dst,src);', ',ptr next:T in O')
NESTED = (
    'func stage_nested(ptr dst:T in O,ptr src:T in O,ptr spare:T in O)->ptr T in O where {T,owner O}'
    '{defer{return stage(dst,src,spare);};return dst;}'
)
DECLARED_SLOT = prepared_result('stage_slot', 'ptr var slot->dst;defer (=)(slot,src);')
INTRINSIC = prepared_result('stage_intrinsic', 'defer intrinsic (=)(dst,src);')
RECOVER = prepared_result('stage_recover',
    'try{defer (=)(dst,src);}catch(e:const OutOfMemoryError){dst=src;{}};')
WEAK = (
    'proc stage_selected(ptr first:u64 in O,ptr second:u64 in O,ptr src:u64 in O,ptr spare:u64 in O,select_first:bool)'
    '->void throws NullPointerException where {owner O}'
    '{val prepared<-false;'
    'defer{if(prepared){behave @print(first);behave @print(second);}else{};};'
    'defer (=)({if(select_first){first}else{second}},src);src=spare;prepared<-true;return;}'
)


def case(name, body=READ + COPY, setup=SETUP, after='', definitions=STAGE, output='7\n' * 4, reject=False,
         diagnostic=MISSING_TARGET, action=None):
    if action is None:
        action = 'defer{for(i in 0u64..4u64){ ' + body + ' };};'
    source = (
        IO + definitions + 'proc run()->void throws NullPointerException{' + setup + action + after
        + 'return;}try{run();}catch(e:const NullPointerException){behave @print(99u64);};'
    )
    runner.CASES[name] = (source, 2, '', diagnostic) if reject else (source, 0, output, '')


case('captured_entry')
case('read_after_invocation', body=COPY + READ)
case('changed_capture_entry', after='q=c;', reject=True)
case('written_capture_entry', after='c<-9u64;q=c;', output='7\n9\n7\n7\n')
case('later_action_changes_capture', after='defer{q=c;};', reject=True)
case('later_action_writes_capture', after='defer{c<-9u64;q=c;};', output='7\n9\n7\n7\n')
case('late_binding_does_not_initialize', body=READ + COPY.replace('q=a;', 'q=b;'), after='q=c;', reject=True)

TWO_SNAPSHOTS = READ + 'p=stage_twice(p,q,c,b);q=a;'
case('two_prepared_snapshots', definitions=TWICE, body=TWO_SNAPSHOTS)
case('two_snapshots_bad_first', definitions=TWICE, body=TWO_SNAPSHOTS, after='q=c;', reject=True)

# A deferred action can invoke another callable that owns its registrations.
# A direct registration into a currently draining scope still fails.
case('nested_action', definitions=STAGE + NESTED, body=READ + 'p=stage_nested(p,q,c);q=a;')
case('nested_action_bad_entry', definitions=STAGE + NESTED, body=READ + 'p=stage_nested(p,q,c);q=a;',
     after='q=c;', reject=True)
NESTED_SCOPE = (
    '{{ptr local=a;defer{behave @print(local);};'
    'defer (=)(local,q);q=spare:void;}};q=a;'
)
NESTED_SETUP = 'ref a:u64=7u64;val b<-8u64,c<-make local u64;ptr p=a,q=a,spare=c;'
case('nested_queue', definitions='', setup=NESTED_SETUP, body=NESTED_SCOPE)
case('nested_queue_bad_entry', definitions='', setup=NESTED_SETUP, body=NESTED_SCOPE, after='q=c;', reject=True)
case('deep_action', definitions='', action='defer{defer{behave @print(p);};};', reject=True,
     diagnostic='currently draining scope')

case('finally', action='defer{}finally{for(i in 0u64..4u64){' + READ + COPY + '};};')
WEAK_SETUP = SETUP.replace('q=a', 'q=b') + 'ptr other=a;'
WEAK_BODY = 'stage_selected(p,other,q,c,i==0u64);q=b;'
case('weak_destination', setup=WEAK_SETUP, definitions=WEAK, body=WEAK_BODY, output='8\n7\n' + '7\n8\n' * 3)
case('weak_bad_entry', setup=WEAK_SETUP, definitions=WEAK, body=WEAK_BODY, after='q=c;', reject=True)
case('alias_destination', definitions=DECLARED_SLOT, body=READ + COPY.replace('stage(', 'stage_slot('))
case('intrinsic', definitions=INTRINSIC, body=READ + COPY.replace('stage(', 'stage_intrinsic('))
RECOVERED_COPY = COPY.replace('stage(', 'stage_recover(')
case('recovered_registration', definitions=RECOVER, body=READ + RECOVERED_COPY)
case('recovered_registration_bad_entry', definitions=RECOVER, body=READ + RECOVERED_COPY, after='q=c;', reject=True)
case('unrelated_uninitialized_cell', body=READ + COPY + 'behave @print(c);', reject=True)
case('null_snapshot', after='q=null;', output='7\n99\n')
case('call_after_invocation', definitions=STAGE + 'proc observe(x:u64)->void{behave @print(x);return;}',
     body=COPY + 'observe(p);')

CALL_BODY = (
    'ptr output=source,current=source;'
    'for(i in 0u64..3u64){output=stage(output,current,spare);current=source;behave @print(output);};'
)
CALL_HEAD = 'proc invoke(ptr source:u64 in O,ptr spare:u64 in O)->void throws NullPointerException where {owner O}'
INVOKE = STAGE + CALL_HEAD + '{' + CALL_BODY + 'return;}'
GENERIC = INVOKE.replace('source:u64', 'source:T').replace('spare:u64', 'spare:T').replace('where {owner O}', 'where {T,owner O}')
RECURSIVE = (
    STAGE + CALL_HEAD.replace('spare:u64 in O)', 'spare:u64 in O,n:u64)')
    + '{if(n==0u64){' + CALL_BODY + '}else{invoke(source,spare,n-1u64);};return;}'
)
DISPATCH = (
    'proc dispatch(f:F,ptr source:u64 in O,ptr spare:u64 in O)->void throws NullPointerException where {F,owner O}'
    '{f(source,spare);return;}'
)
case('call_entry', definitions=INVOKE, action='invoke(a,c);', output='7\n' * 3)
case('call_bad_entry', definitions=INVOKE, action='invoke(c,a);', reject=True)
case('call_generic', definitions=GENERIC, action='invoke(a,c);', output='7\n' * 3)
case('call_generic_bad', definitions=GENERIC, action='invoke(c,a);', reject=True)
case('call_recursive', definitions=RECURSIVE, action='invoke(a,c,3u64);', output='7\n' * 3)
case('call_recursive_bad', definitions=RECURSIVE, action='invoke(c,a,3u64);', reject=True)
case('call_callable', definitions=INVOKE, setup=SETUP + 'proc callable=invoke;', action='callable(a,c);', output='7\n' * 3)
case('call_callable_bad', definitions=INVOKE, setup=SETUP + 'proc callable=invoke;', action='callable(c,a);', reject=True)
case('call_prepared', definitions=INVOKE, action='defer invoke(a,c);', output='7\n' * 3)
case('call_saved_before_rebind', definitions=INVOKE, action='defer invoke(q,c);', after='q=c;', output='7\n' * 3)
case('call_saved_uninitialized', definitions=INVOKE, setup=SETUP.replace('q=a', 'q=c'),
     action='defer invoke(q,c);', after='q=a;', reject=True)
case('call_later_payload_write', definitions=INVOKE, setup=SETUP.replace('q=a', 'q=c'),
     action='defer invoke(q,c);', after='c<-9u64;q=a;', output='9\n' * 3)
case('call_transitive', definitions=INVOKE + DISPATCH, setup=SETUP + 'proc callable=invoke;',
     action='dispatch(callable,a,c);', output='7\n' * 3)
case('call_transitive_bad', definitions=INVOKE + DISPATCH, setup=SETUP + 'proc callable=invoke;',
     action='dispatch(callable,c,a);', reject=True)
case('call_missing_restore', definitions=INVOKE.replace('current=source;behave', 'current=spare;behave'),
     action='invoke(a,c);', reject=True)
case('call_deferred_frame', definitions=INVOKE, action='defer{invoke(q,c);};', output='7\n' * 3)
case('call_late_block_rebind', definitions=INVOKE, action='defer{invoke(q,c);};', after='q=c;', reject=True)

# Interpret the saved reference and the reverse registration queue directly.
# Repeated source nodes represent separate dynamic captures.
for number, (initial, next_target, b_initialized, last) in enumerate(product(('a', 'b'), ('a', 'b'), (False, True), ('a', 'c'))):
    cells = {'a': 7, 'b': 8 if b_initialized else None, 'c': None}
    source = initial
    captures = []
    for _ in range(3):
        captures.append(source)
        source = next_target
    values = [cells[target] for target in reversed(captures)]
    setup = SETUP.replace('b<-8u64', 'b<-' + ('8u64' if b_initialized else 'make local u64')).replace('q=a', 'q=' + initial)
    action = (
        'val prepared<-false;for(i in 0u64..3u64){'
        'defer{if(prepared){' + READ + '}else{};};defer (=)(p,q);q=' + next_target + ';};'
        'q=' + last + ';prepared<-true;'
    )
    case('model_' + str(number), setup=setup, definitions='', action=action, reject=None in values,
         diagnostic="initialization of 'b'", output=''.join(str(value) + '\n' for value in values))

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
