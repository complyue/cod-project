#!/usr/bin/env python3
"""Reference intrinsics share operand preparation without reading target payloads."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
OBS = IO + 'proc observe(x:i64)->void {behave @print(x);}'
EX = 'using "std:exceptions" MissingReturnError;func missing()->const i64 {}'
SELECT_AFTER_CLEANUP = (
    'func select(ref first:i64 in O,ref next:i64 in O)->ref i64 in O where {owner O}'
    '{defer{return next;};return first;}'
)
SNAPSHOT_TARGET = (
    'func snapshot(ptr original:i64 in O,ref next:i64 in O)->ptr i64 in O where {owner O}'
    '{defer{original=next;original<-4;};return original;}'
)

runner.CASES = {
    'deferred_uninitialized': ('val x<-make local i64;ptr p=x;defer (is)(p,null);', 0, '', ''),
    'deferred_readonly': ('val x<-make local i64;const ptr p:const i64=x;defer intrinsic (is)(p,null);', 0, '', ''),
    'deferred_null_literal': ('defer (is)(null in here,null);', 0, '', ''),
    'registration_order': (OBS + '{{defer observe(3);ptr p:i64=null in here;defer (is)({observe(1);p},null);observe(2);}};', 0, '1\n2\n3\n', ''),
    'late_rebinding': (IO+'proc run()->void{val target<-make local i64;ptr p:i64=null in here;'
        'defer{behave @print((p is null));};defer (is)(p,null);defer{p=target;};}run();',0,'false\n',''),
    'preparation_failure': (OBS + EX + '{{defer observe(3);ptr p:i64=null in here;'
        'try{defer (is)({observe(1);missing();p},null);}catch(e:const MissingReturnError){observe(2);};}};',
        0, '1\n2\n3\n', '[-Wmissing-return]'),
    'preparation_return': (OBS + 'proc f()->const i64 {ptr p:i64=null in here;defer observe(3);'
        'defer (is)({observe(1);return 7;p},null);observe(99);return 0;}observe(f());', 0, '1\n3\n7\n', ''),
    'scope_local': (OBS + '{{val x<-make local i64;defer (is)(x,null);defer observe(1);}};', 0, '1\n', ''),
    'expired_target': ('func leak(ref input:i64 in O)->ref i64 in O where {owner O}'
        '{ref local:i64=1;return local;}val x<-1;defer (is)(leak(x),null);',
        2,'','borrowed result does not retain its declared external owner'),
    'iteration_target': ('for(i in 0..2){defer (is)(i,null);};', 2, '', 'does not outlive its cleanup scope'),
    'iteration_cleanup': (OBS + 'for(i in 0..2){{defer (is)(i,null);defer observe(i);}};', 0, '0\n1\n', ''),
    'no_initialization_from_test': ('val x<-make local i64;{{defer (is)(x,null);}};x;', 2, '', 'before definite initialization'),
    'generic_nullable_borrow': (IO + 'func test(ptr p:T in O)->void where {T,owner O}{defer (is)(p,null);}'
        'val x<-make local i64,y<-make local f64;ptr p:i64=null in here;test(p);test(x);test(y);behave @print((p is null));',
        0, 'true\n', ''),
    'recursive_borrow': (IO + 'func test(ptr p:T in O,n:i64)->bool where {T,owner O}{defer (is)(p,null);'
        'return if(n==0){(p is null)}else{test(p,n-1)};}ptr p:i64=null in here;behave @print(test(p,40));',
        0, 'true\n', ''),
    'captured_callable_holder': (IO + 'func base()->i64{return 7;}func f=func[base]()->i64{return base();};'
        '{{ptr p=f;defer (is)(p,null);}};behave @print(f());', 0, '7\n', ''),
    'local_proc_test': (OBS + 'proc f()->void{ptr p:i64=null in here;defer (is)(p,null);defer observe(1);}f();', 0, '1\n', ''),
    'swap_saved_slots': (OBS + 'val x<-1,y<-2,z<-3;ptr p=x,q=y,r=z;'
        '{if(true){p}else{q}}<=>{observe(8);p=r;q};observe(p);observe(q);', 0, '8\n2\n3\n', ''),
    'swap_cleanup_before_invocation': (OBS+SELECT_AFTER_CLEANUP+'val x<-1,y<-2,z<-3;ptr p=x,q=y;'
        '{p=select(p,z);p}<=>{observe(8);q};observe(p);observe(q);',0,'8\n2\n3\n',''),
    'null_test_operand_cleanup': (IO+SNAPSHOT_TARGET+'ptr p:i64=null in here;val x<-1;'
        'behave @print((snapshot(p,x) is null));behave @print((p is null));behave @print(x);',
        0,'true\ntrue\n4\n',''),
    'fixed_reference_swap': ('ptr p=1;const ptr q=2;intrinsic (<=>)(p,q);', 2, '', 'cannot rebind a const reference binding'),
    'deferred_slot_swap': (OBS+'proc run()->void{ptr p=1,q=2;defer{observe(p);observe(q);};'
        'defer intrinsic (<=>)(p,q);}run();',0,'2\n1\n',''),
}
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {
    'generic_nullable_borrow': ['p'],
    'recursive_borrow': ['p'],
    'swap_saved_slots': ['x'],
    'swap_cleanup_before_invocation': ['x'],
}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
