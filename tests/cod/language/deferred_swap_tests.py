#!/usr/bin/env python3
"""Prepared bits swaps retain addresses, read promises and cleanup lifetimes."""
from pathlib import Path
import foundation_tests as runner
from language_test_support import observe_after_cleanup

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
ERROR = 'using "std:exceptions" MissingReturnError;func missing()->const i64{}'
EXCEPTIONS = ERROR + NPE + '''using "std:exceptions" Exception;
func error()->const MissingReturnError{try{missing():void;}catch(e:const MissingReturnError){return e;};}
func absent()->const NullPointerException{ptr p:i64=null in here;try{p;{}}catch(e:const NullPointerException){return e;};}
'''
WARN = '[-Wmissing-return]'
METHOD = 'proc (<->)(x:bool in O,y:bool in P)->void where {owner O,owner P}{intrinsic (<->)(x,y);behave @print(9);return;}'
OUT = lambda value: 'behave @print(' + value + ');'
TARGETS = 'ref x:i64=1,y:i64=2;'
SLOTS = 'val x<-1,y<-2,z<-3;ptr p=x,q=y;'
PAIR = OUT('x') + OUT('y')
TRIPLE = PAIR + OUT('z')
PREPARE_TARGET = (
    'func prepare(ref selected:i64 in O,ref marker:i64 in O)->ref i64 in O where {owner O}'
    '{defer (<-)(marker,3);return selected;}'
)
BORROWED_RESULT_OWNER = 'borrowed result does not retain its declared external owner'


def swap_program(setup, work, observation=PAIR, definitions=''):
    return IO + definitions + 'proc run()->void{' + setup + observe_after_cleanup(work, observation) + 'return;}run();'


runner.CASES = {
 'saved_addresses_late_payloads': (swap_program(SLOTS, 'defer (<->)(p,q);p=z;x<-4;', PAIR + OUT('p')),
     0, '2\n4\n3\n', ''),
 'infix_call': (IO + TARGETS + '{{defer x<->y;}};' + PAIR, 0, '2\n1\n', ''),
 'explicit_builtin_bypasses_method': (IO + METHOD + 'ref x:bool=false,y:bool=true;{{defer intrinsic (<->)(x,y);}};' + OUT('x'),
     0, 'true\n', ''),
 'evaluate_once_left_to_right': (IO + TARGETS +
     '{{defer (<->)({' + OUT('3') + 'x},{' + OUT('4') + 'y});' + OUT('5') + '}};' + PAIR,
     0, '3\n4\n5\n2\n1\n', ''),
 'later_operand_rebinds_source': (swap_program(SLOTS, 'defer (<->)(p,{p=z;q});', PAIR + OUT('p')),
     0, '2\n1\n3\n', ''),
 'later_operand_initializes_first': (swap_program('val x<-make local i64,y<-2;', 'defer (<->)(x,{x<-3;y});'),
     0, '2\n3\n', ''),
 'operand_cleanup_retains_address': (swap_program(SLOTS + 'val marker<-0;',
     'defer (<->)(prepare(p,marker),{p=z;if(marker==3){q}else{p}});',
     PAIR + OUT('p'), PREPARE_TARGET), 0, '2\n1\n3\n', ''),
 'source_rebound_to_null_after_registration': (swap_program('val x<-1,y<-2;ptr p=x;',
     'defer (<->)(p,y);p=null;', PAIR + OUT('(p is null)')), 0, '2\n1\ntrue\n', ''),
 'self_exchange': (swap_program('val x<-1;ptr p=x,q=x;', 'defer (<->)(p,q);x<-4;', OUT('x')),
     0, '4\n', ''),
 'overlapping_lifo': (swap_program('val x<-1,y<-2,z<-3;', 'defer (<->)(x,y);defer (<->)(y,z);', TRIPLE),
     0, '3\n1\n2\n', ''),
 'selected_alias_retains_its_target': (IO + 'proc run(flag:bool)->void{' + SLOTS +
     'ptr var selected->if(flag){p}else{q};'
     + observe_after_cleanup('defer (<->)(selected,z);selected=z;', TRIPLE) + 'return;}run(true);run(false);',
     0, '3\n2\n1\n1\n3\n2\n', ''),
 'different_target_owners': (IO + 'ref x:i64=1;{{val y<-2;defer{' + OUT('y') + '};defer (<->)(x,y);}};' + OUT('x'),
     0, '1\n2\n', ''),
 'selected_targets_across_owners': (IO + 'proc run(flag:bool)->void{ref x:i64=1;'
     '{{val y<-2,z<-3;defer{' + OUT('y') + OUT('z') + '};'
     'if(flag){defer (<->)(x,z);}else{defer (<->)(y,z);};defer{z<-4;};}};' + OUT('x') + 'return;}run(true);run(false);',
     0, '2\n1\n4\n4\n2\n1\n', ''),
 'fixed_bindings_writable_payloads': (IO+'const ptr p=1,q=2;{{defer (<->)(p,q);}};behave @print(p);behave @print(q);',0,'2\n1\n',''),
 'borrowed_targets_outlive_call': (IO+'func swap(ref x:T in O,ref y:T in P)->void where {T,owner O,owner P}{defer (<->)(x,y);}val x<-1,y<-2;swap(x,y);behave @print(x);behave @print(y);',0,'2\n1\n',''),
 'borrowed_read_promises_propagate': (IO+NPE+'proc swap(ptr x:i64 in O,ptr y:i64 in P)->void throws NullPointerException where {owner O,owner P}{defer (<->)(x,y);return;}val x<-1,y<-2;try{swap(x,y);}catch(e:const NullPointerException){};behave @print(x);behave @print(y);',0,'2\n1\n',''),
 'nested_registration': (swap_program(TARGETS, 'defer{ {{defer (<->)(x,y);}};};'),
     0, '2\n1\n', ''),
 'return_snapshot_preserved': (IO+'proc run()->const i64{val x<-1,y<-2;defer{behave @print(x);};defer (<->)(x,y);return x;}behave @print(run());',0,'2\n1\n',''),
 'borrowed_return_observes_swapped_payload': (IO+'func run(ref x:i64 in O,ref y:i64 in O)->ref i64 in O where {owner O}{defer (<->)(x,y);return x;}val x<-1,y<-2;behave @print(run(x,y));behave @print(y);',0,'2\n1\n',''),
 'pending_exception_and_remaining_cleanup': (IO + ERROR + 'proc run()->void{val x<-1,y<-2;'
     'defer{' + OUT('y') + '};defer catch(e:const MissingReturnError){' + OUT('x') + 'return;};'
     'defer (<->)(x,y);missing();return;}run();', 0, '2\n1\n', WARN),
 'loop_accumulates_distinct_addresses': (swap_program('val x<-1,y<-2,z<-3;ptr p=x;',
     'for(i in 0..3){defer (<->)(p,z);p=y;};', TRIPLE), 0, '3\n2\n1\n', ''),
 'per_iteration_scope': (IO + TARGETS + 'for(i in 0..3){ {{defer (<->)(x,y);}};};' + PAIR,
     0, '2\n1\n', ''),
 'generic_recursive_mixed_dispatch': (IO+METHOD+'proc run(ref x:T in O,ref y:T in P,n:i64)->void where {T,owner O,owner P}{if(n>0){defer (<->)(x,y);run(x,y,n-1);}else{};return;}val x<-1,y<-2,a<-false,b<-true;run(x,y,3);run(a,b,2);behave @print(x);behave @print(a);',0,'9\n9\n2\nfalse\n',''),
 'zero_loop_registers_nothing': (IO+'val x<-1,y<-2;for(i in 0..0){defer (<->)(x,y);};behave @print(x);',0,'1\n',''),
 'exception_categories_not_exchanged_at_registration': (swap_program('val a:Exception<-error(),b:Exception<-absent();',
     'defer (<->)(a,b);try{throw a;}catch(e:const Exception){' + OUT('1') + '};',
     OUT('a.category') + OUT('b.category'), EXCEPTIONS), 0, '1\n4\n5\n', WARN),
 'exception_causes_read_at_invocation': (swap_program('val a<-error(),b<-error();',
     'defer (<->)(a,b);b<-try{throw b from b;}catch(e:const MissingReturnError){e};',
     OUT('a.cause_count') + OUT('b.cause_count'), EXCEPTIONS), 0, '1\n0\n', WARN),
 'first_null_checked_after_both_operands': (IO+NPE+'val x<-1;ptr p:i64=null in here;try{defer (<->)(p,{behave @print(2);x});}catch(e:const NullPointerException){behave @print(3);};behave @print(x);',0,'2\n3\n1\n',''),
 'second_null_preserves_first': (IO+NPE+'val x<-1;ptr p:i64=null in here;try{defer (<->)(x,p);}catch(e:const NullPointerException){behave @print(2);};behave @print(x);',0,'2\n1\n',''),
 'later_operand_failure_registers_nothing': (IO+ERROR+'val x<-1,y<-2;try{defer (<->)(x,{x<-3;missing();y});}catch(e:const MissingReturnError){};behave @print(x);behave @print(y);',0,'3\n2\n',WARN),
 'operand_return_registers_nothing': (IO+'proc run()->void{val x<-1,y<-2;defer{behave @print(x);};defer (<->)({return;x},{behave @print(9);y});}run();',0,'1\n',''),
 'uninitialized_at_registration': ('val x<-make local i64,y<-2;defer (<->)(x,y);x<-3;',2,'','before definite initialization'),
 'borrowed_uninitialized_at_registration': (NPE+'proc swap(ptr x:i64 in O,ptr y:i64 in P)->void throws NullPointerException where {owner O,owner P}{defer (<->)(x,y);return;}val x<-make local i64,y<-2;try{swap(x,y);}catch(e:const NullPointerException){};',2,'','cannot prove initialization'),
 'expired_target': ('val y<-1;proc leak(ref target:i64 in O)->ref i64 in O where {owner O}'
     '{ref local:i64=2;return local;}defer (<->)(leak(y),y);', 2, '', BORROWED_RESULT_OWNER),
 'expired_selected_alternative': ('val y<-1;proc leak(ref target:i64 in O,flag:bool)->ref i64 in O where {owner O}'
     '{ref local:i64=2;return if(flag){local}else{target};}defer (<->)(leak(y,true),y);',
     2, '', BORROWED_RESULT_OWNER),
 'readonly_target': ('const val x<-1;val y<-2;defer (<->)(x,y);',2,'','readonly destination'),
 'no_destination_widening': ('val x<-1u8,y<-2u64;defer (<->)(x,y);',2,'','exact same type'),
 'fresh_value_is_not_a_destination': ('val x<-1;defer (<->)(x,2);',2,'','swap requires two storage destinations'),
 'no_callable_builtin': ('func one()->const i64{return 1;}func a=one;func b=one;defer intrinsic (<->)(a,b);',2,'','builtin bits swap requires two bits storage'),
 'null_check_requires_covering_handler': ('val x<-1;ptr p:i64=null in here;defer (<->)(x,p);',2,'','requires a covering catch'),
 'post_cleanup_checked_category_remains_possible': (EXCEPTIONS +
     'proc run()->void{val a:Exception<-error(),b:Exception<-absent();defer{throw a;};defer (<->)(a,b);}run();',
     2, '', 'coverage within the owning pass'),
}
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {
    'explicit_builtin_bypasses_method': ['x', 'y'],
    'borrowed_targets_outlive_call': ['x', 'y'],
    'borrowed_read_promises_propagate': ['x', 'y'],
    'borrowed_return_observes_swapped_payload': ['x', 'y'],
    'generic_recursive_mixed_dispatch': ['x', 'y', 'x', 'y'],
}
runner.EXTRA_SOURCES = [Path(__file__), Path(__file__).with_name('language_test_support.py')]

if __name__ == '__main__':
    raise SystemExit(runner.main())
