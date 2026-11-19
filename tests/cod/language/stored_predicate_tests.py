#!/usr/bin/env python3
"""Stored null-test values retain evidence until their target or test changes."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;using "std:exceptions" NullPointerException;'
WRITE = 'func write(ref target:bool in O,value:bool)->void where {owner O}{target<-value;}'
READ = 'func read(ref target:const bool in O)->void where {owner O}{target;}'
SAVED = 'val saved<-p is not null;'
GUARD = 'if(saved){behave @print(p);}else{behave @print(0);};'
runner.CASES = {}


def program(body, prefix=''):
    return IO+prefix+'proc choose(ptr p:i64 in O,ptr q:i64 in O)->void where {owner O}{'+body+'}'+\
        'val x<-7,y<-11;ptr absent:i64=null in here;choose(x,y);choose(absent,y);choose(x,absent);choose(absent,absent);'


def safe(name, body, output='7\n0\n7\n0\n', prefix=''):
    runner.CASES[name] = (program(body, prefix), 0, output, '')


def rejected(name, body, prefix=''):
    runner.CASES[name] = (program(body, prefix), 2, '', 'requires a covering catch')


safe('value', SAVED+GUARD)
safe('const_value', 'const '+SAVED+GUARD)
safe('copied_value', 'val original<-p is not null;val saved<-original;original<-true;'+GUARD)
safe('readonly_alias', 'val original<-p is not null;ref saved:const bool=original;'+GUARD)
safe('writable_alias', 'val original<-p is not null;ref saved=original;'+GUARD)
safe('binding_alias', 'val original<-p is not null;var saved->original;'+GUARD)
safe('initialized_local', 'val saved<-make local bool{p is not null};'+GUARD)
safe('late_initialization', 'val saved<-make local bool;saved<-p is not null;'+GUARD)
safe('write_new_test', 'val saved<-true;saved<-p is not null;'+GUARD)
safe('assignment_value', 'val saved<-false;if(saved<-p is not null){behave @print(p);}else{behave @print(0);};')
safe('rebound_view', 'val saved<-false,other<-p is not null;ptr selected=saved;if(selected=other){behave @print(p);}else{behave @print(0);};')
safe('negation', 'val saved<-p is null;if(!saved){behave @print(p);}else{behave @print(0);};')
safe('conjunction', 'val first<-p is not null,second<-q is not null,saved<-first&&second;'+GUARD, '7\n0\n0\n0\n')
safe('block_copy', 'val original<-p is not null;val saved<-{{return original;}};'+GUARD)
safe('same_value_alternatives', 'val first<-p is not null,second<-!(p is null);ref saved=if(q is null){first}else{second};'+GUARD)
safe('same_branch_writes', 'val saved<-false;if(q is null){saved<-p is not null;}else{saved<-!(p is null);};'+GUARD)
safe('unrelated_branch', SAVED+'if(q is null){}else{};'+GUARD)
safe('many_unrelated_branches', SAVED+'if(q is null){}else{};'*64+GUARD)
safe('payload_write', SAVED+'if(saved){p<-13;behave @print(p);}else{behave @print(0);};', '13\n0\n13\n0\n')
safe('readonly_call', SAVED+'read(saved);'+GUARD, prefix=READ)
safe('readonly_callable', 'func observer=read;'+SAVED+'observer(saved);'+GUARD, prefix=READ)
safe('unrelated_call', SAVED+'val other<-false;write(other,true);'+GUARD, prefix=WRITE)
safe('swap_values', 'val saved<-false,other<-p is not null;saved<->other;'+GUARD)
safe('swap_self', SAVED+'saved<->saved;'+GUARD)
safe('swap_same_target', SAVED+'ptr alias=saved;alias<->saved;'+GUARD)
safe('finally_preserves', SAVED+'try{}finally{q=null;{}};'+GUARD)
safe('finally_writes_proof', 'val saved<-false;try{}finally{saved<-p is not null;{}};'+GUARD)
safe('cleanup_preserves', SAVED+'{{defer{q=null;{}};}};'+GUARD)
safe('prepared_read_preserves', SAVED+'{{defer read(saved);}};'+GUARD, prefix=READ)
safe('cleanup_snapshot', 'ref original:bool=p is not null;val saved<-{{defer{original<-true;};return original;}};'
     'behave @print(original);'+GUARD, 'true\n7\ntrue\n0\ntrue\n7\ntrue\n0\n')
safe('finally_snapshot', 'val original<-p is not null;val saved<-try{original}finally{original<-true;{}};'+GUARD)
safe('caught_alternatives', 'val saved<-false;try{q;saved<-p is not null;}catch(e:const NullPointerException){saved<-!(p is null);};'+GUARD)

for name, mutation in {
    'direct_write': 'saved<-true;',
    'aliased_write': 'ref alias=saved;alias<-true;',
    'binding_alias_write': 'var alias->saved;alias<-true;',
    'selected_write': 'val other<-false;{if(q is null){saved}else{other}}<-true;',
    'branch_write': 'if(q is null){saved<-true;{}}else{};',
    'writable_call': 'write(saved,true);',
    'writable_callable': 'func writer=write;writer(saved,true);',
    'swap_unknown': 'val other<-true;saved<->other;',
    'finally_write': 'try{}finally{saved<-true;{}};',
    'catch_write': 'try{q;{}}catch(e:const NullPointerException){saved<-true;{}};',
    'nested_cleanup': '{{defer{defer{saved<-true;{}};};}};',
    'pointer_rebind': 'p=null;',
    'pointer_alias_rebind': 'var alias->p;alias=null;',
    'pointer_swap': 'p<=>q;',
}.items():
    rejected(name, SAVED+mutation+GUARD, prefix=WRITE)

BORROWED_SAVED = SAVED+'ptr destination=saved;'
rejected('cleanup_write', BORROWED_SAVED+'{{defer{destination<-true;};}};'+GUARD)
rejected('prepared_write', BORROWED_SAVED+'{{defer (<-)(destination,true);}};'+GUARD)
rejected('prepared_call', BORROWED_SAVED+'{{defer write(destination,true);}};'+GUARD, prefix=WRITE)
rejected('prepared_swap', BORROWED_SAVED+'ref other:bool=true;{{defer (<->)(destination,other);}};'+GUARD)
rejected('pointer_cleanup_rebind', SAVED+'defer{'+GUARD+'};defer{p=null;};')

rejected('readonly_alias_mutated', 'val original<-p is not null;ref saved:const bool=original;write(original,true);'+GUARD, prefix=WRITE)
rejected('unknown_target_alternative', SAVED+'val other<-true;ref selected=if(q is null){saved}else{other};'+GUARD.replace('if(saved)', 'if(selected)'))
rejected('snapshot_after_mutation', SAVED+'saved<-true;val copy<-saved;'+GUARD.replace('if(saved)', 'if(copy)'))
rejected('deferred_reads_invocation_state', SAVED+'defer{'+GUARD+'};saved<-true;')
rejected('deferred_handler_write', SAVED+'defer{'+GUARD+'};'
         'defer catch(e:const MissingReturnError){saved<-true;};defer{missing();};',
         prefix='using "std:exceptions" MissingReturnError;func missing()->i64{}')
rejected('prepared_slot_payload_write', SAVED+'val other<-false;ptr a=other,b=other;defer{'+GUARD+'};defer (<=>)(a,b);a=saved;',
         prefix='func (<=>)(ref left:bool in O,ref right:bool in O)->void where {owner O}{left<-true;}')

safe('loop_unchanged', SAVED+'for(i in 0u64..2u64){'+GUARD+'};', '7\n7\n0\n0\n7\n7\n0\n0\n')
safe('loop_restored_test', 'val saved<-true;for(i in 0u64..2u64){saved<-p is not null;'+GUARD+'saved<-true;};', '7\n7\n0\n0\n7\n7\n0\n0\n')
safe('loop_readonly_call', SAVED+'for(i in 0u64..2u64){read(saved);'+GUARD+'};', '7\n7\n0\n0\n7\n7\n0\n0\n', prefix=READ)
safe('loop_saved_copy', 'val original<-p is not null;val saved<-original;for(i in 0u64..2u64){original<-true;'+GUARD+'};', '7\n7\n0\n0\n7\n7\n0\n0\n')
safe('loop_single_iteration', SAVED+'for(i in 0u64..1u64){'+GUARD+'saved<-true;};')
safe('loop_empty', SAVED+'for(i in 0u64..0u64){saved<-true;};'+GUARD)
safe('loop_unrelated_slot_call', SAVED+'val first<-false,second<-false;ptr a=first,b=second;for(i in 0u64..2u64){ {{defer (<=>)(a,b);}};'+GUARD+'};',
     '7\n7\n0\n0\n7\n7\n0\n0\n', prefix='func (<=>)(ref left:bool in O,ref right:bool in O)->void where {owner O}{left<-true;}')
safe('loop_readonly_slot_call', SAVED+'ptr a=saved,b=saved;for(i in 0u64..2u64){ {{defer (<=>)(a,b);}};'+GUARD+'};',
     '7\n7\n0\n0\n7\n7\n0\n0\n', prefix='func (<=>)(ref left:const bool in O,ref right:const bool in O)->void where {owner O}{left;right;}')
safe('loop_intrinsic_reference_swap', SAVED+'ptr a=saved,b=saved;for(i in 0u64..2u64){ {{defer intrinsic (<=>)(a,b);}};'+GUARD+'};',
     '7\n7\n0\n0\n7\n7\n0\n0\n')
safe('loop_recursive_call_preserves_payload', SAVED+'ptr selected=saved;for(i in 0u64..2u64){ {{inspect(selected,3u64);}};'+GUARD+'};',
     '7\n7\n0\n0\n7\n7\n0\n0\n', prefix='func inspect(ref target:bool in O,n:u64)->void where {owner O}'
     '{if(n>0u64){inspect(target,n-1u64);}else{target;{}};}')
rejected('loop_prepared_write_changes_payload', SAVED+'ptr selected=saved;val incoming<-saved;'
         'for(i in 0u64..2u64){ {{defer (<-)(selected,incoming);}};'+GUARD+'incoming<-true;};')
rejected('loop_rebound_slot_call', SAVED+'val first<-false,second<-false;ptr a=first,b=second;for(i in 0u64..3u64){'+GUARD+'{{defer (<=>)(a,b);}};a=saved;};',
         prefix='func (<=>)(ref left:bool in O,ref right:bool in O)->void where {owner O}{left<-true;}')

for name, mutation in {
    'write': 'saved<-true;',
    'equal_first_write': 'saved<-incoming;incoming<-true;',
    'rebound_write': 'selected<-true;selected=saved;',
    'rebound_call': 'write(selected,true);selected=saved;',
    'call': 'write(saved,true);',
    'finally': 'try{}finally{saved<-true;{}};',
    'pointer_rebind': 'p=null;',
}.items():
    rejected('loop_'+name, SAVED+'val incoming<-p is not null,other<-false;ptr selected=other;for(i in 0u64..3u64){'+GUARD+mutation+'};', prefix=WRITE)

rejected('loop_cleanup', BORROWED_SAVED+
         'for(i in 0u64..3u64){'+GUARD+'{{defer{destination<-true;};}};};')
rejected('loop_prepared_call', BORROWED_SAVED+
         'for(i in 0u64..3u64){'+GUARD+'{{defer write(destination,true);}};};', prefix=WRITE)

for name, write in (('direct', 'other<-true;'), ('call', 'write(other,true);'), ('cleanup', '{{defer{other<-true;{}};}};')):
    runner.CASES['borrowed_alias_'+name] = (IO+WRITE+
        'proc run(ptr p:i64 in P,ref saved:bool in O,ref other:bool in Q)->void where {owner O,owner P,owner Q}{'+
        'saved<-p is not null;'+write+GUARD+'}val b<-false;ptr p:i64=null in here;run(p,b,b);', 2, '', 'requires a covering catch')

runner.CASES['uninitialized_target'] = (IO+'val x<-make local i64;ptr p=x;'+SAVED+GUARD, 2, '', 'before definite initialization')
runner.CASES['readonly_target'] = (IO+'val x<-7;ptr p:const i64=x;'+SAVED+'if(saved){p<-9;}else{};', 2, '', 'readonly destination')
runner.CASES['expired_target'] = (IO+'func keep(ref input:i64 in O)->ref i64 in O where {owner O}'
    '{ref local:i64=7;return local;}val x<-7;ptr p=keep(x);'+SAVED+GUARD,
    2, '', 'borrowed result does not retain its declared external owner')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__)]


from language_test_support import expect_source_rejections
expect_source_rejections(runner, (
    'nested_cleanup',
), 'currently draining scope')

if __name__ == '__main__':
    raise SystemExit(runner.main())
