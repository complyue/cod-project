#!/usr/bin/env python3
"""Identity guards retain selected-reference null facts through boolean storage."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;using "std:exceptions" NullPointerException;'
CALLS = 'val x<-7,y<-11;ptr absent:i64=null in here;choose(x,x);choose(x,y);choose(x,absent);choose(absent,x);choose(absent,absent);'
runner.CASES = {}


def program(body, prefix='', parameters='ptr p:i64 in O,ptr q:i64 in O', owners='owner O'):
    return IO+prefix+'proc choose('+parameters+')->void where {'+owners+'}{ '+body+' }'+CALLS


def guard(condition, target='p'):
    return 'if('+condition+'){behave @print('+target+');}else{behave @print(0);};'


def safe(name, body, output='7\n0\n0\n0\n0\n', **kwargs):
    runner.CASES[name] = (program(body, **kwargs), 0, output, '')


def reject(name, body, **kwargs):
    runner.CASES[name] = (program(body, **kwargs), 2, '', 'requires a covering catch')


for reverse in (False, True):
    identity = 'q is p' if reverse else 'p is q'
    different = 'q is not p' if reverse else 'p is not q'
    label = 'reverse_' if reverse else ''
    safe(label+'nonnull_operand', 'if(q is not null){'+guard(identity)+'}else{behave @print(0);};')
    safe(label+'null_operand', 'ptr empty:i64=null in O;'+guard(('empty is not p' if reverse else 'p is not empty')),
         '7\n7\n7\n0\n0\n')
    for first in (False, True):
        suffix = '_relation_first' if first else '_null_first'
        same = 'same&&p is not null' if first else 'p is not null&&same'
        unequal = 'different&&p is null' if first else 'p is null&&different'
        safe(label+'saved_equal'+suffix, 'val same<-'+identity+';'+guard(same, 'q'))
        safe(label+'saved_unequal'+suffix, 'val different<-'+different+';'+guard(unequal, 'q'), '0\n0\n0\n7\n0\n')
    safe(label+'null_complement', 'ptr empty:i64=null in O;val same<-'+('empty is p' if reverse else 'p is empty')+';'+guard('!same'),
         '7\n7\n7\n0\n0\n')

safe('nonnull_other_owner', 'val local<-19;ref known=local;'+guard('p is known'), '0\n0\n0\n0\n0\n')
safe('saved_copy', 'val first<-p is q;val same<-first;first<-false;'+guard('p is not null&&same', 'q'))
safe('saved_alias', 'val first<-p is q;ref same:const bool=first;'+guard('p is not null&&same', 'q'))
safe('saved_swap', 'val first<-p is q,same<-false;first<->same;'+guard('p is not null&&same', 'q'))
safe('false_branch', 'val same<-p is q;if(p is null){if(same){behave @print(0);}else{behave @print(q);};}else{behave @print(0);};',
     '0\n0\n0\n7\n0\n')
safe('negated_disjunction', 'val same<-p is q;'+guard('!(p is not null||same)', 'q'), '0\n0\n0\n7\n0\n')
safe('unknown_owners_equal', 'val same<-p is q;'+guard('p is not null&&same', 'q'),
     parameters='ptr p:i64 in P,ptr q:i64 in Q', owners='owner P,owner Q')
safe('saved_snapshot', 'ptr empty:i64=null in O;val different<-p is not empty;empty=q;'+guard('different'), '7\n7\n7\n0\n0\n')
safe('right_rebind_keeps_left_snapshot', 'ptr kept=q;if(kept is not null){'+guard('p is {q=null;kept}')+'}else{behave @print(0);};')
safe('rebound_known_nonnull_snapshot', 'if(q is not null){val same<-p is q;q=null;'+guard('same')+'}else{behave @print(0);};')
safe('left_rebind_keeps_right_target', 'ptr empty:i64=null in O;'+guard('{q=p;empty} is not q', 'q'), '7\n7\n7\n0\n0\n')
safe('payload_change', 'val same<-p is q;if(p is not null&&same){p<-23;behave @print(q);}else{behave @print(0);};', '23\n0\n0\n0\n0\n')
safe('loop_stable', 'val same<-p is q;for(i in 0u64..2u64){'+guard('p is not null&&same', 'q')+'};', '7\n7\n0\n0\n0\n0\n0\n0\n0\n0\n')
runner.CASES['cleanup_preserves'] = (IO+'proc choose(ref p:i64 in O,ref q:i64 in O)->void where {owner O}{val same<-p is q;{{defer{q<-q;};}};'+
    guard('p is not null&&same','q')+'}val x<-7,y<-11;choose(x,x);choose(x,y);', 0, '7\n0\n', '')
runner.CASES['nonnull_ref'] = (IO+'proc choose(ptr p:i64 in O,ref q:i64 in O)->void where {owner O}{'+guard('p is q')+'}'+
    'val x<-7,y<-11;ptr absent:i64=null in here;choose(x,x);choose(x,y);choose(absent,x);', 0, '7\n0\n0\n', '')

for name, mutation in {
    'rebind_right': 'q=null;',
    'swap_targets': 'p<=>q;q=null;',
    'aliased_rebind': 'var target->q;target=null;',
    'boolean_write': 'same<-true;',
    'boolean_alias_write': 'ref value=same;value<-true;',
    'cleanup_rebind': '{{defer{q=null;{}};}};',
    'cleanup_boolean_write': '{{defer{same<-true;{}};}};',
}.items():
    reject(name, 'val same<-p is q;'+mutation+guard('p is not null&&same', 'q'))

reject('rebind_left', 'val same<-p is q;p=null;'+guard('q is not null&&same'))
reject('right_operand_rebind', guard('p is {p=null;q;}&&q is not null'))
reject('unknown_owners_unequal', 'val different<-p is not q;'+guard('p is null&&different', 'q'),
       parameters='ptr p:i64 in P,ptr q:i64 in Q', owners='owner P,owner Q')
reject('null_in_other_owner', '{{ptr empty:i64=null in here;'+guard('p is not empty')+'}};')
reject('nonnull_disequality', 'if(q is not null){'+guard('p is not q')+'}else{};')
reject('unknown_nullness_equal', guard('p is q'))
reject('loop_rebound', 'val same<-p is q;for(i in 0u64..2u64){'+guard('p is not null&&same','q')+'q=null;};')
reject('loop_boolean_write', 'val same<-p is q;for(i in 0u64..2u64){'+guard('p is not null&&same','q')+'same<-true;};')

runner.CASES['uninitialized_payload'] = (IO+'val x<-make local i64;ptr p=x;if(p is x){behave @print(p);}else{};', 2, '', 'before definite initialization')
runner.CASES['readonly_payload'] = ('val x<-7;ptr p:const i64=x;if(p is x){p<-9;}else{};', 2, '', 'readonly destination')
runner.CASES['expired_owner'] = ('ptr p={{val x<-7;x}};p is null;', 2, '', 'owner has ended')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
